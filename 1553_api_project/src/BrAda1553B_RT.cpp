#include "zynq_1553b_api.h"
#include "BrAda1553B_RT.h"
#include "utility.h"
#include "ConfigSplit.h"
#include "RT_ICD.h"
#include "stdio.h"
#include "stdlib.h"
#include "actel1553io.h"
#include "head.h"  //zhanghao add.
#include <time.h> 
#include "callback.h"
#ifdef IPCORE
//#include "platform\timeutil.h"
#endif
#ifdef __vxworks
	#include <unistd.h>
#endif

// ����
///* Bcst   Mode  C2/Data  Tx/~Rx	*/
//#define ACE_MSG_BCTORT				0 	/*   0		0		0		0	*/
//#define ACE_MSG_RTTOBC				1 	/*   0		0		0		1	*/
//#define ACE_MSG_RTTORT				2 	/*   0		0		1		0	*/
//#define ACE_MSG_MODENODATA			5 	/*   0		1		0		1	*/
//#define ACE_MSG_MODEDATARX			6 	/*   0		1		1		0	*/
//#define ACE_MSG_MODEDATATX			7 	/*   0		1		1		1	*/
//#define ACE_MSG_BRDCST				8 	/*   1		0		0		0   */
//#define ACE_MSG_BRDCSTRTTORT		    10 	/*   1		0		1		0	*/
//#define ACE_MSG_BRDCSTMODENODATA	    13 	/*   1		1		0		1   */
//#define ACE_MSG_BRDCSTMODEDATA		14	/*   1		1		1		0   */
//#define ACE_MSG_INVALID				15	/*	 1		1		1		1   */

// ��̬��������
std::map<S16BIT, BrAda1553B_RT*> BrAda1553B_RT::s_adapt;
//ȫ�ֱ���
extern bool RT_Adapter_changeEndian;

//zhanghao add.
extern map<S16BIT, CDeviceInfo*> map_devInfo;

BrAda1553B_RT::BrAda1553B_RT(void)
{
	m_TimeCount = 0;
	iAddress = -1;
	IsSaveFlag = 0;
	fp = NULL;
	RT_Adapter_changeEndian = false;
}

BrAda1553B_RT::~BrAda1553B_RT(void)
{
	// ������
	Thread::stop();

	clear();
}
void BrAda1553B_RT::set_address(const string &host_addr, const std::string &if_addr)
{
	// �Թ� // address = host_addr;
	cardNum = atoi(if_addr.c_str());

	// ����ͬ���¼�
	SyncEvent* event = new SyncEvent();
	event_map.insert(std::make_pair( cardNum, event));

	BrAda1553B_RT::s_adapt[cardNum] = this;
	return ;
}

/*
long BrAda1553B_RT::capability() const
{
	return DC_READ | DC_WRITE | DC_TIMING | DC_IFID_REQ ; 
}
*/

// �ο���GIST2��Ŀ����ָ��.docx��
unsigned long BrAda1553B_RT::get_timer_interval()
{
	int val = 1 * 1000; // 1����
	return val;
}

U16BIT BrAda1553B_RT::source_init(SyncEvent *tim_mutex)
{
		
	if (m_bConfigError)
	{
         printf("bc config parse error\n");
		  return RET_1553B_ERR;
	}

	S16BIT DevNum =  cardNum;
	S16BIT nResult = 0x0000;
	m_tim_mutex = tim_mutex;

	/* Initialize Device */
	nResult = aceInitialize(DevNum, ACE_ACCESS_CARD, ACE_MODE_RTMT, 0, 0, 0);
	if (nResult) {

		printf("[commu_RT::Start][ERROR] rt%d aceInitialize failed.cardNum %d.\n",iAddress,cardNum);
       return RET_1553B_ERR;
	}

	SyncEvent* event = new SyncEvent();
	event_map.insert(std::make_pair(DevNum, event));

	/* Set RT Address */
	aceRTSetAddress(DevNum, iAddress);
	 
	U16BIT buff[32];
	memset((void *)buff,0,sizeof(U16BIT) * 32);

	// ����ÿһ���ӵ�ַ������һ��dataBlock,����֮����,dataBlock��Ϊ1~sub_addr_count
	map<S16BIT,_addrData>::iterator it =  m_SubAddr.begin();
	int i=0;
	while(it !=  m_SubAddr.end())
	{
		// ȡ���ӵ�ַ
		S16BIT wRTSubAddress =(int)it->first;

		/* Create data block for RT use */
		aceRTDataBlkCreate(DevNum, it->second.blockID_TX, ACE_RT_DBLK_DOUBLE,NULL,0); //(U16BIT *) it->second.dataArry_TX, 32);
		aceRTDataBlkCreate(DevNum,it->second.blockID_RX, ACE_RT_DBLK_DOUBLE,NULL,0); //(U16BIT *)it->second.dataArry_RX, 32);

		aceRTDataBlkWrite(DevNum,it->second.blockID_TX,buff,32 ,0);
		aceRTDataBlkWrite(DevNum,it->second.blockID_RX,buff,32 ,0);

		/* Map data block to given sub-address */
		aceRTDataBlkMapToSA(DevNum, it->second.blockID_TX, wRTSubAddress, ACE_RT_MSGTYPE_TX, ACE_RT_DBLK_EOM_IRQ,TRUE);
		aceRTDataBlkMapToSA(DevNum, it->second.blockID_RX, wRTSubAddress, ACE_RT_MSGTYPE_RX,ACE_RT_DBLK_EOM_IRQ,TRUE); //ACE_RT_DBLK_CIRC_IRQ

		it++;
		i++;
	}

	// �򿪷����ļ�	,�ж����õ��ļ��Ƿ��ܹ���
	it =  m_SubAddr.begin();
	while(it !=  m_SubAddr.end())
	{
		S16BIT subAddr = it->first;
		m_SubAddr[subAddr].pDataSource->OpenSimFile(iAddress,subAddr,cardNum);

		// �忨��
		m_SubAddr[subAddr].pDataSource->SetDevNum(cardNum);
		it++;
	}

	// ʹ��ICD����
	it =  m_SubAddr.begin();
	while(it !=  m_SubAddr.end())
	{
		S16BIT subAddr = it->first;

		timeInter_obj * inter_obj = new timeInter_obj();
		_TimeCode timeCode;
		m_SubAddr[subAddr].pDataSource->GetTimeCode(timeCode);
		//m_SubAddr[subAddr].pDataSource->Set_Serv(m_SysServ);
		it++;
	}

	// д��Ĭ��ֵ
	{
		map<S16BIT,_addrData>::iterator dataIt = m_SubAddr.begin();
		while (dataIt != m_SubAddr.end())
		{
			dataIt->second.pDataSource->CopyData(dataIt->second.dataBuff,dataIt->second.pDataSource->GetSubLen());
			if (33 == dataIt->first|| 0 == dataIt->first|| 31 == dataIt->first)
			{			
				aceRTModeCodeWriteData(cardNum,ACE_RT_MCDATA_TX_TRNS_VECTOR,dataIt->second.dataBuff[0]);
			}
			else
			{
				aceRTDataBlkWrite(cardNum,dataIt->second.blockID_TX, (U16BIT*)dataIt->second.dataBuff,32,0);
			}
			dataIt ++;
		}
	}
	//д״̬��
	WriteStatus();
	
    return RET_1553B_OK;
}

void BrAda1553B_RT::clear()
{
	map<S16BIT,_addrData>::const_iterator it = m_SubAddr.begin();
	while (it != m_SubAddr.end())
	{
		if (it->second.pDataSource != NULL)
		{
			delete it->second.pDataSource;
		}
		it++;
	}
	iAddress = -1;

	m_SubAddr.clear();
	m_SubAddrCyle.clear();

	m_chgMsg_subAddr.clear();
	m_chgMsg_subState.clear();

	m_ResourceCopy_subAddr.clear();
	m_forceSubAddr.clear();

	m_mapThreeMsg.clear();
}

U16BIT BrAda1553B_RT::start()
{
     S16BIT DevNum =  cardNum;
	  S16BIT nResult = 0x0000;

	 /**ACE_IMR1_RT_MODE_CODE (Bit 1)  		  Enable RT Mode Code Events**/
	 /**ACE_IMR1_RT_SUBADDR_EOM (Bit 4)		  Enable RT Subaddress Access Events**/
	/**ACE_IMR1_RT_CIRCBUF_ROVER (Bit 5)		Indicates an RT Circular Buffer has rolled over**/
	/**ACE_IMR1_TT_ROVER (Bit 6)		        Indicates the Hardware Timetag has rolled over**/
	/**ACE_IMR1_BCRT_CMDSTK_ROVER (Bit 12)	Indicates the RT Command Stack has reached rolled over**/
	/**ACE_IMR2_RT_CIRC_50P_ROVER		        Indicates an RT Circular Buffer reached the 50% mark**/
	/**ACE_IMR2_RT_CSTK_50P_ROVER		        Indicates the RT Command Stack reached the 50% mark**/
	/**ACE_IMR2_RT_ILL_CMD		              Indicates the RT received an illegal command**/	
	 nResult = aceSetIrqConditions(DevNum, TRUE, ACE_IMR1_EOM, &BrAda1553B_RT::MyISR);
	 if (nResult)
	{
        printf("BrAda1553B_RT::start  aceSetIrqConditions error\n");
		 return RET_1553B_ERR;
	}
    /* start RT/MT Operations */
	nResult = aceRTMTStart(DevNum);
	if(nResult != 0)
	{
       printf("BrAda1553B_RT::start  aceRTMTStart error\n"); 
		return RET_1553B_ERR;
	}
	
	GetTime_rt(m_startTime);

	//BrAda1553B_RT::work_stat = BrAda1553B_RT::WS_RUN;	
	//start();
	printf("RT%d started sucess!\r\n",iAddress);
	return RET_1553B_OK;
}

U16BIT BrAda1553B_RT::stop()
{
	S16BIT DevNum =  cardNum;
	S16BIT nResult = 0x0000;

	Thread::exit();
	/* Stop RT/MT Operations */
	nResult = aceRTMTStop(DevNum);
   if(nResult != 0)
	{
       printf("BrAda1553B_RT::start  aceRTMTStart error\n"); 
	}
	/* Free all resources */
	nResult += aceFree(DevNum);
	if (nResult) 
	{
		printf("[commu_RT stop] rt%d aceFree fail!\n",iAddress);
	}

	Thread::exit();

	printf("RT%d stoped sucess!\r\n",iAddress);
	clear();

   if(NULL != fp)
   	{
        fclose(fp);
	}
	return nResult;
}


void BrAda1553B_RT::ioctl(U16BIT subaddr, msg_is_save flag, const char *save_path)
{
		map<S16BIT,_addrData>::iterator itSub = m_SubAddr.find(subaddr);
		if (itSub != m_SubAddr.end())
		{
            if(flag == ENUM_MSG_SAVE)
            {
                   IsSaveFlag |= 1 << subaddr;
					  if(NULL == fp)
					  {
							 fp = fopen(save_path, "w+");
					  }
			  }
			  else
			  {
                 IsSaveFlag &= ~(1 << subaddr); 
			  }

			  if(IsSaveFlag == 0)
			  {
					if(fp)
					{
						 fclose(fp);
						 fp = NULL;
					}
			  }
		}
		return;
}

/* zhanghao close.*/
/*ssize_t BrAda1553B_RT::read(DataTime *time,  Agent::DevDataHeader *head, char *buffer,size_t length)
{
	return 0;
}*/

int BrAda1553B_RT::write(U16BIT subaddr, const char *data, size_t len )
{
       U16BIT ret = 0;
			 
		m_Lock_card.lock();

	   //�����ݿ������ӵ�ַ�Ļ��壬��writeBlkData
		map<S16BIT,_addrData>::iterator itSub = m_SubAddr.find(subaddr);
		if (itSub != m_SubAddr.end())
		{
			size_t dui_len = 0;
			size_t packlen = 64;
			size_t size = 0;
			const char * p = data;
		   while( dui_len < len )
			{
				 size = ((len - dui_len) > packlen ) ? packlen : (len - dui_len);
				 (itSub->second).pDataSource->SetSource(subaddr, (p + dui_len), size);
				 dui_len += size;
			}

			m_Is_write_data = true;
			m_forceSubAddr.insert(subaddr);// ˢ��
			ret = RET_1553B_OK;
		}
		else
		{
			printf("\t [commu_RT error] rt%d rout_a, config no the subAddr! \n",iAddress);
			m_Is_write_data = false;
			ret = RET_1553B_ERR;
		}

        m_Lock_card.unlock();        		
      	 return ret;
}

#if 0
GIST::BR_OPERATION_R BrAda1553B_RT::write(const  Agent::MsgDataHeader *head, const char *data, size_t len )
{
	// д������������ָ��
	if (head == NULL)
	{
		return GIST::R_FAILURE; 
	}
	string strFlag = head->src_address;

#define  WORD_SIZE  sizeof(S16BIT)

	m_Lock_card.lock();

	GIST::BR_OPERATION_R ret = GIST::R_SUCCESS;
	try
	{
		bool bFind = false;

		string src_addr = head->src_address;
		string src_subAddr = head->src_subaddress;
		string des_addr = head->dest_address;
		string des_subAddr = head->dest_subaddress;
#ifdef _DEBUG
		printf("src_addr:%s,src_subAddr:%s,des_addr:%s,des_subAddr:%s \n",src_addr.c_str(),src_subAddr.c_str(),des_addr.c_str(),des_subAddr.c_str()); 
#endif

		if ("b" == src_addr && des_addr.length()==0)
		{
			bFind = true;
			ret = parse_rout_b(data,len);
		}
		else if (strFlag == "c" && des_addr.length()==0) // ·��c,������
		{
			bFind = true;
			ret = parse_rout_c(data,len);
		}
		else if (src_addr.length() >0 || des_addr.length() >0)  // ·��a,������
		{	
			bFind = true;
			// �õ�������Ϣ
			string strBus ; //������Ϣ
			vector<string> vec;
			SubData(src_addr,vec,':');
			if (vec.size() >= 2)
			{
				strBus = vec.at(0);
				src_addr = vec.at(1);
			}

			S16BIT iAddr = (S16BIT)atol_my(src_addr.c_str());
			S16BIT iSubAddr= (S16BIT)atol_my(src_subAddr.c_str());
			S16BIT addr = (S16BIT)atol_my(des_addr.c_str());
			S16BIT subAddr = (S16BIT)atol_my(des_subAddr.c_str());

#ifdef _DEBUG
			printf("[commu_RT]:%d,%d,%d,%d\n",iAddr,iSubAddr,addr,subAddr); 
#endif

			// ȡ���Ƚ� 
			if (iAddr == iAddress)
			{
				parse_rout_a(data,len,iSubAddr);
			}
			else if (addr == iAddress)
			{
				parse_rout_a(data,len,subAddr);
			}
		}
		if ("a" == src_addr)
		{
			bFind = true;
			ret = parse_rout_a(data,len);
		}

		if ("error" == src_addr)
		{
			// ����·��ģʽ
			bFind = true;
			parse_rout_error(src_subAddr);
		}

		if(!bFind)
		{
			printf("[commu_RT] rt%d rout_a fail. \n",iAddress);
			ret = GIST::R_FAILURE;
		}
	}
	catch (...)
	{
		printf("[commu_RT] rt%d write error. \n",iAddress);
	}	

	m_Lock_card.unlock();

	return ret;
}


/* zhanghao close.*/
// ����·��a,�µ�·��a����·�ɵ�Դ��ַ��Դ�ӵ�ַ��Ŀ�ĵ�ַ��Ŀ���ӵ�ַ������ָ��RT��ַ�ӵ�ַ
GIST::BR_OPERATION_R BrAda1553B_RT::parse_rout_a(const char *data, size_t len ,S16BIT subAddr)
{
	GIST::BR_OPERATION_R ret = GIST::R_SUCCESS;
	S16BIT wordLen = len/sizeof(S16BIT);

	//�����ݿ������ӵ�ַ�Ļ��壬��writeBlkData
	map<S16BIT,_addrData>::iterator itSub = m_SubAddr.find(subAddr);
	if (itSub != m_SubAddr.end())
	{
		int byleLen = wordLen * sizeof(S16BIT);
		(itSub->second).pDataSource->SetSource(data,byleLen);

		UpdataBlkData_force(subAddr,itSub->second.pDataSource->GetSubLen());

		if (wordLen > 32)
		{
			m_forceSubAddr.insert(subAddr); //����32�����ڶ�ȡ��ˢ��
		}

		ret= GIST::R_SUCCESS;
		return ret;
	}
	else
	{
		printf("\t [commu_RT error] rt%d rout_a, config no the subAddr! \n",iAddress);
		ret = GIST::R_FAILURE;
		return ret;
	}
}


GIST::BR_OPERATION_R BrAda1553B_RT::parse_rout_a(const char *data, size_t len )
{	
	if (len < WORD_SIZE * 2)
	{
		printf("\t[commu_RT error]: rt %d routing error ;a \n",iAddress);
		return GIST::R_FAILURE;
	}		

	S16BIT subAddr=0,wordLen=0;
	memcpy((char*)&subAddr,(char*)data,2);
	memcpy((char*)&wordLen,(char*)(data + WORD_SIZE),2);


	if (wordLen <= 0)
	{
		// ȡ�����ݿ���
		map<S16BIT,_addrData>::iterator itSub = m_SubAddr.find(subAddr);
		if (itSub != m_SubAddr.end())
		{
			itSub->second.pDataSource->EnSimSource();
			UpdataBlkData_force(subAddr,itSub->second.pDataSource->GetSubLen());

			return GIST::R_SUCCESS;
		}
		else
		{
			printf("\t [commu_RT error]: rt %d routing a ,config no the subAddr! \n",iAddress);

			return GIST::R_FAILURE;
		}
	}
	else if (wordLen * sizeof(S16BIT) > (len - WORD_SIZE *2))
	{
		printf("\t[commu_RT error]: rt %d  routing a error,block error  \n",iAddress);
		return GIST::R_FAILURE;
	}
	else 
	{
		const char * pData = data + WORD_SIZE *2;
		//�����ݿ������ӵ�ַ�Ļ��壬��writeBlkData
		map<S16BIT,_addrData>::iterator itSub = m_SubAddr.find(subAddr);
		if (itSub != m_SubAddr.end())
		{
			int byleLen = wordLen * sizeof(S16BIT);
			(itSub->second).pDataSource->SetSource(pData,byleLen);

			UpdataBlkData_force(subAddr,itSub->second.pDataSource->GetSubLen());

			if (wordLen > 32)
			{
				m_forceSubAddr.insert(subAddr); //����32�����ڶ�ȡ��ˢ��
			}

			return GIST::R_SUCCESS;
		}
		else
		{
			printf("\t[commu_RT error]: rt %d  routing a error,config no the subAddr! \n",iAddress);
			return GIST::R_FAILURE;
		}
	}
}
GIST::BR_OPERATION_R BrAda1553B_RT::parse_rout_b(const char *data, size_t len )
{
	GIST::BR_OPERATION_R ret = GIST::R_SUCCESS;
	if (len != WORD_SIZE * 3)
	{
		printf("\t[commu_RT error]: rt%d routing b error,block error \n",iAddress);
		ret = GIST::R_FAILURE;
	}
	else
	{
		S16BIT busy_1=0,subSystemFlag=0,terminal=0;
		memcpy((char*)&busy_1,(char*)data,2);
		memcpy((char*)&subSystemFlag,(char*)(data + WORD_SIZE),2);
		memcpy((char*)&terminal,(char*)(data + WORD_SIZE*2),2);

		m_StateWord.Busy_flag = (long) busy_1;
		m_StateWord.SubSystem_Flag = (long)subSystemFlag;
		m_StateWord.Terminal_Flag = (long) terminal;

		WriteStatus();
	}
	return ret;
}
GIST::BR_OPERATION_R BrAda1553B_RT::parse_rout_c(const char *data, size_t len )
{
	GIST::BR_OPERATION_R ret = GIST::R_SUCCESS;
	if (len != WORD_SIZE *2)
	{
		printf("\t[commu_RT error]: rt%d routing c error,block error  \n",iAddress);
		ret = GIST::R_FAILURE;
	}
	else
	{
		S16BIT serverReq=0,cyleChg=0;
		memcpy((char*)&serverReq,(char*)data,2);
		memcpy((char*)&cyleChg,(char*)(data + WORD_SIZE),2);

		if (cyleChg > 0)
		{
			m_chgCyle_s.cycle = cyleChg;
		}
		if(1==serverReq || 0== serverReq)
		{
			m_StateWord.ServiceRequest = serverReq;
		}
		WriteStatus();
	}
	return ret;
}

// ����·�ɴ���
GIST::BR_OPERATION_R BrAda1553B_RT::parse_rout_error(const string & src_subAddr)
{
	GIST::BR_OPERATION_R ret = GIST::R_SUCCESS;

	map<S16BIT,_addrData>::iterator it = m_SubAddr.begin();
	while (it != m_SubAddr.end())
	{
		// ���ù���ģʽ
		if (src_subAddr == "1")
		{
			it->second.pDataSource->SetWorkMode(true);
		}
		else if(src_subAddr == "0")
		{
			it->second.pDataSource->SetWorkMode(false);
		}
		else{
			// δ֪·�� 
			ret = GIST::R_FAILURE;
		}

		it++;
	}
	return ret;
}


std::multimap<std::string, std::string> BrAda1553B_RT::get_bus_addr()
{
	std::multimap<std::string ,std::string> ss;

	return ss;
}

void BrAda1553B_RT::write_data(const char *data, size_t len, const std::pair<std::string, std::string> *channel)
{
	return ;
}
#endif

void _DECL BrAda1553B_RT::MyISR( S16BIT DevNum, U32BIT dwIrqStatus )
{
	try
	{
		BrAda1553B_RT* pThis = s_adapt[DevNum];
		// ����Ϣ��������Ϣ��ɵ��¡�1553B������д���ɰ忨�Զ���ɵģ����ǿ���ͨ���ж϶̴���
		if(pThis == NULL)
		{
                   printf("pThis is NULL\n");
		     return;
		}
	       pThis->ReadMsg();
	}
	catch(...)
	{
		printf("[commu_RT error]::MyISR. \n");
	}
}

void BrAda1553B_RT::do_job()
{
	m_tim_mutex->wait(1000);
	try
	{
		// 1.�������ڣ�����������ɵ�����
		SizeCyle_Parse();
		//printf("In do_job\n");
	}
	catch(...)
	{
		printf("[commu_RT error]::do_job rt%d. \n",iAddress);
	}
}

/* zhanghao close.
void BrAda1553B_RT::set_interface(const Interface *if_obj)
{
	m_interface = if_obj;
}
*/

void BrAda1553B_RT::SizeCyle_Parse()
{
	// ����ͳ�ƣ�����ͳ������
	if (m_TimeCount >= 0x1FFFFFFE)
	{
		m_TimeCount = 0;
	}
	m_TimeCount++;

	_time_rt nowTime;
	GetTime_rt(nowTime);

	long nSec =0;

     /*yinhonggen close*/
     #if 0
	if(m_TimeCount%2000 ==  0)
		printf("m_TimeCount = %d,   nSec = %d(ms),  now.tv_sec = %d, now.tv_usec = %d \n",
		    m_TimeCount, nSec, nowTime.time_s.tv_sec, nowTime.time_s.tv_usec);
      #endif
	  
	long DelTimes = nSec; // = m_TimeCount * TM_MS_USEC_UNIT;// �ܺ���

	// �ӵ�ַ�仯������,�����ӵ�ַ���ݻ���
	map<S16BIT,_CycleIndex>::iterator CyleIt =  m_SubAddrCyle.begin();
	while (CyleIt !=  m_SubAddrCyle.end())
	{
		S16BIT subAddr = CyleIt->first;
		long cycle = CyleIt->second.cycle;
		if (0 == cycle)
		{
			cycle = 1000;
		}

		int iCount =DelTimes/cycle;
		
		if(iCount > CyleIt->second.cyleIndex || (iCount == 1 && (DelTimes%cycle == 0)))  //�ж��Ƿ�����,���ﲻ����ģ����
		{
			CyleIt->second.cyleIndex = iCount;          // �������ں�,���������
			m_Is_write_data = true;
			UpdataBlkData_force(subAddr,CyleIt->second.updataLen);

			if (CyleIt->second.cyleIndex > 0x7FFFFFFF)
			{
				CyleIt->second.cyleIndex = 1;
			}
		}

		CyleIt++;
	}

	// ��������λ,�Ƚ�
	if (0 != m_chgCyle_s.cycle)
	{
		int countIndex =DelTimes/ m_chgCyle_s.cycle;
//		if(countIndex > m_chgCyle_s.cyleIndex) //if (CountTimeDel % m_chgCyle_s.cycle == 0)
		if (m_TimeCount % m_chgCyle_s.cycle == 0)
		{
			// �ﵽ����,��������λ��
			m_StateWord.ResetSerRequest();
			m_chgCyle_s.cyleIndex = countIndex;

			//д״̬��
			WriteStatus();
		}
	}
}

// ����״̬��
void BrAda1553B_RT::WriteStatus()
{
	// æλ Busy
	if (1 == m_StateWord.Busy_flag){
		aceRTStatusBitsSet(cardNum,ACE_RT_STSBIT_BUSY);
	}
	else{
		aceRTStatusBitsClear(cardNum,ACE_RT_STSBIT_BUSY);	
	}

	// ��������λ Service Request
	if (1 == m_StateWord.ServiceRequest){
		aceRTStatusBitsSet(cardNum,ACE_RT_STSBIT_SREQ);
	}
	else{
		aceRTStatusBitsClear(cardNum,ACE_RT_STSBIT_SREQ);	
	}

	// ��ϵͳ����λ Subsystem Flag
	if (1 == m_StateWord.SubSystem_Flag){
		aceRTStatusBitsSet(cardNum,ACE_RT_STSBIT_SSFLAG);
	}
	else{
		aceRTStatusBitsClear(cardNum,ACE_RT_STSBIT_SSFLAG); 
	}

	// �ն�����λ RT Flag
	if (1 == m_StateWord.Terminal_Flag){
		aceRTStatusBitsSet(cardNum,ACE_RT_STSBIT_RTFLAG);
	}
	else{
		aceRTStatusBitsClear(cardNum,ACE_RT_STSBIT_RTFLAG); 
	}
}

bool BrAda1553B_RT::UpdataBlkData(S16BIT subAddr,int updataLen)
{
	// ��������Դ
	map<S16BIT,_addrData>::iterator dataIt = m_SubAddr.find(subAddr);
	if (dataIt != m_SubAddr.end())
	{
		// �Ƿ�ˢ��
		if (!dataIt->second.flushFlag) 
		{
			return false;
		}
		memset(dataIt->second.dataBuff,0,32 );
		dataIt->second.pDataSource->CopyData(dataIt->second.dataBuff,updataLen);

		// дһ��ʸ����
		if (33 == subAddr|| 0 == subAddr|| 31 == subAddr)
		{			
			aceRTModeCodeWriteData(cardNum,ACE_RT_MCDATA_TX_TRNS_VECTOR,dataIt->second.dataBuff[0]);
		}
		else
		{
			// ����updataLen�ĵ�λ����
			S16BIT nResult = aceRTDataBlkWrite(cardNum,dataIt->second.blockID_TX, (U16BIT*)dataIt->second.dataBuff,32,0);
			if (nResult != ACE_ERR_SUCCESS)
			{
				printf("\t[commu_RT] error: aceRTDataBlkWrite Error: %d \n",(int)nResult);
			}
			dataIt->second.flushFlag =false;
		}		

		return true;
	}
	else
	{
		printf("\t [commu_RT error]: %d no the subAddr : %d ! \n",iAddress,subAddr);
		return false;
	}

}

// д�ӵ�ַ����
void BrAda1553B_RT::SetSubAddrData(S16BIT subAddr,int len,U16BIT data[])
{	
	// ��������Դ
	map<S16BIT,_addrData>::iterator dataIt = m_SubAddr.find(subAddr);
	if (dataIt != m_SubAddr.end())
	{
#ifdef _DEBUG
		printf("\t [commu_RT ]: UpdataBlkData_force subAddr %d! \n",subAddr);
#endif
		S16BIT nResult = aceRTDataBlkWrite(cardNum,dataIt->second.blockID_TX, data,len,0);
		if (nResult != ACE_ERR_SUCCESS)
		{
			printf("\t[commu_RT] error: aceRTDataBlkWrite Error: %d \n",(int)nResult);
		}
	}
	else
	{
		printf("\t [commu_RT error]: rt%d no the subAddr%d! \n",iAddress,subAddr);
	}
}


void BrAda1553B_RT::UpdataBlkData_force(S16BIT subAddr,int updataLen)
{	
	// ��������Դ
	map<S16BIT,_addrData>::iterator dataIt = m_SubAddr.find(subAddr);
	if (dataIt != m_SubAddr.end())
	{
		memset(dataIt->second.dataBuff,0,64 );
		if(m_Is_write_data)
		{
            dataIt->second.pDataSource->CopyData(subAddr, dataIt->second.dataBuff,updataLen);
		}
		else
		{
            dataIt->second.pDataSource->CopyData(dataIt->second.dataBuff,updataLen);
		}
		
		// дһ��ʸ����
		if (33 == subAddr|| 0 == subAddr|| 31 == subAddr)
		{			
			//aceRTModeCodeWriteData(cardNum,ACE_RT_MCDATA_TX_TRNS_VECTOR,dataIt->second.dataBuff[0]);
			//printf("modecode:0x%x   datebuf:0x%x\n",updataLen,dataIt->second.dataBuff[0]);
			aceRTModeCodeWriteData(cardNum,updataLen,dataIt->second.dataBuff[0]);	
		}
		else
		{
			S16BIT nResult = aceRTDataBlkWrite(cardNum,dataIt->second.blockID_TX, (U16BIT*)dataIt->second.dataBuff,32,0);
			if (nResult != ACE_ERR_SUCCESS)
			{
				printf("\t[commu_RT] error: aceRTDataBlkWrite Error: %d \n",(int)nResult);
			}
		}

		if(m_Is_write_data &&  dataIt->second.pDataSource->WriteBuffIsEmpty(subAddr))
		{
             set<S16BIT>::iterator it_force = m_forceSubAddr.find(subAddr);
		      if (it_force != m_forceSubAddr.end())
		      {
                  m_forceSubAddr.erase(subAddr);
				}

				m_Is_write_data = false;
		}
	}
}

size_t BrAda1553B_RT::ReadMsg()
{
	S16BIT DevNum =  cardNum;
	S16BIT nResult = 0x0000;

	MSGSTRUCT sMsg;

	//while (1) //��ջ�е���Ϣȫ������
	//{
		/* Check for RT messages */
		nResult = aceRTGetStkMsgDecoded(DevNum,&sMsg,ACE_RT_MSGLOC_LATEST_PURGE);

		// ��������
		if (nResult <= 0)
		{
			return 1;
			//break;
		}
		/* Msg found */
		if(nResult==1)
		{

#ifdef _DEBUG
		//	string str = aceGetMsgTypeString(sMsg.wType);
		//	printf("rt %d : %s .\n",iAddress,str.c_str());
#endif

			// �������������Ϣ, ��һ��������
			if (sMsg.wCmdWrd1Flg)
			{
				MakeMsg_Parse(&sMsg,sMsg.wCmdWrd1);
			}
			if(sMsg.wCmdWrd2Flg)
			{
				MakeMsg_Parse(&sMsg,sMsg.wCmdWrd2);
			}
		}
//	}

	return 1;
}

#if 0
void save_the_message_log(MSGSTRUCT * pMsg, U16BIT wCmdWrd)
{
      // ��ַ
	U16BIT wRT = 0;
	// �շ�״̬1
	U16BIT wTR1 = 0;
	// �ӵ�ַ
	U16BIT subAddr = 0;
	// �����ֻ�ʸ����
	U16BIT wWC = 0; 
	int i = 0, cnt = 0;
	 char ch;
    char tmp[50];
    char buf[1024];
    time_t timep;
	aceCmdWordParse(wCmdWrd, &wRT, &wTR1, &subAddr, &wWC);
	if(wTR1 == 0)
	{
        ch = 'R';
	}
	else if(wTR1 == 1)
	{
		 ch = 'T';
	}

   time (&timep);
   memset(buf,  0,   sizeof(buf)); 
   memset(tmp, 0 , sizeof(tmp));

   sprintf(tmp, "%s",asctime(gmtime(&timep)));
   tmp[(strlen(tmp) -1)] = '\0';
   sprintf(buf, "------------------%s------------------\n",tmp);
   sprintf((buf + strlen(buf)), "CMD:0x%x                      %02d-%c-%02d-%02d\n",wCmdWrd, wRT, ch, subAddr, wWC);

   if(wWC == 0)
   {
 		wWC = 32;
   }

   if(subAddr != 0 && subAddr != 32)
   {
   		sprintf((buf + strlen(buf)), "%s\n","Data:");	
   		for(i = 0; i < wWC; i++)
   		{
  			sprintf((buf + strlen(buf)), "%04x    ",pMsg->aDataWrds[i]);
  			cnt ++;
  			if(!(cnt % 8))
  			{
              	sprintf((buf + strlen(buf)), "\n"); 
  			}
   		}
  }
  sprintf((buf + strlen(buf)), "\n\n"); 
  fwrite(buf, 1, sizeof(buf), fp);    	   		
}
#endif

int BrAda1553B_RT::MakeMsg_Parse(MSGSTRUCT * pMsg,U16BIT & wCmdWrd)
{
	// ��ַ
	U16BIT wRT = 0;
	// �շ�״̬1
	U16BIT wTR1 = 0;
	// �ӵ�ַ
	U16BIT subAddr = 0;
	// �����ֻ�ʸ����
	U16BIT wWC = 0;

	aceCmdWordParse(wCmdWrd, &wRT, &wTR1, &subAddr, &wWC);
    //printf("wCmdWrd:0x%x wRT:0x%x wTR1:0x%x  subAddr:0x%x  wWC:0x%x\n",wCmdWrd, wRT, wTR1, subAddr, wWC);
	// ���Ǳ�RT��ַ�Ĳ�����,�Ҳ��ǹ㲥��Ϣ
	if (wRT !=  iAddress && wRT != 0 && wRT != 31 && wRT != 0)
	{
		return 0;
	}
	// ���ӵ�ַ������Ϣ
	if( 1 == wTR1 )
	{
		// ���ӵ�ַ���ݿ���ˢ��
		map<S16BIT,_addrData>::iterator it_subAddr = m_SubAddr.find(subAddr);
		if (it_subAddr != m_SubAddr.end())
		{
			it_subAddr->second.flushFlag = true; 
		}

		// ����32���ֵ�ָ��������ˢ��
		set<S16BIT>::iterator it_force = m_forceSubAddr.find(subAddr);
		if (it_force != m_forceSubAddr.end() && it_subAddr != m_SubAddr.end())
		{				
			if (wWC == 0) // ��ʵ��32
			{
				UpdataBlkData_force(subAddr,32);				
			}
			else
			{
			    
				UpdataBlkData_force(subAddr,wWC);				
			}
		}

	}

	if (wWC == 0)
	{
		wWC = 32;
	}
	RT_MSG rt_msg(subAddr,wTR1,wWC);
	//printf("out rt_msg\n");
	// ��������λ��Ϣ
	set<RT_MSG>::const_iterator it_sq = m_chgMsg_subState.find(rt_msg);
	if (it_sq != m_chgMsg_subState.end())
	{
		MsgMake_chgMsgSq(rt_msg,subAddr);
	}
#if 0
	// ���ݿ�����Ϣ
	map<RT_MSG,vector<S16BIT> >::const_iterator it_copy = m_ResourceCopy_subAddr.find(rt_msg);
	if (it_copy != m_ResourceCopy_subAddr.end())
	{
		MsgMake_chgMsgCopy(rt_msg,(*pMsg),it_copy->second);
	}

	// ʱ���뱾��Уʱ
	set<S16BIT>::iterator itTime = m_mapTimeCode.find(subAddr);
	if (itTime != m_mapTimeCode.end() && 0 == wTR1) // Уʱ���ǽ��ܵ�������
	{
		map<S16BIT,_addrData>::iterator it_subAddr = m_SubAddr.find(subAddr);

		int dataLen = (int)wWC;
		char * pData = (char *)pMsg->aDataWrds;
		//zhanghao close. it_subAddr->second.pDataSource->ShipTimeCheck(pData,dataLen*2);  //Уʱ
	}

	// ��ȡ������Դ
	map<RT_MSG,vector<S16BIT> >::iterator itThree= m_mapThreeMsg.find(rt_msg);
	if (itThree != m_mapThreeMsg.end())
	{
		MsgMake_chgMsgThree(itThree->second,wWC,pMsg->aDataWrds);
	}

	// ���ݱ仯Ӧ����������

	// ���ݱ仯��Ϣ
	map<RT_MSG,vector<S16BIT> >::const_iterator it_subAddr = m_chgMsg_subAddr.find(rt_msg);
	if (it_subAddr != m_chgMsg_subAddr.end())
	{
		MsgMake_chgMsgAddr(it_subAddr->second);
	}


#ifdef _DEBUG
	// ��ӡ״̬��
	if (pMsg->wStsWrd1Flg)
	{
		printf("\t[commu_RT]: wStsWrd1 : %d \n",(int)pMsg->wStsWrd1);
	}
	if (pMsg->wStsWrd2Flg)
	{
		printf("\t[commu_RT]: wStsWrd2 : %d \n",(int)pMsg->wStsWrd2);
	}
#endif
#endif

    if(NULL != g_pfucCallback)
    {
         stmsg_struct CallMsg;
         int i =0;
		  U16BIT iCount = pMsg->wWordCount;
			
         CallMsg.cmdwrd = wCmdWrd;
		  CallMsg.datalen  = wWC;
		  CallMsg.iaddrs    = wRT;
		  CallMsg.subaddr  = subAddr;
		  CallMsg.trflag      = wTR1;
	     CallMsg.stswrd  = 0;
		  
		  if((subAddr == 31) || (subAddr == 0))
		  {
				iCount = 0;
				memset(CallMsg.datawrd, 0, sizeof(CallMsg.datawrd));
		  }
		  else
		  {
			   CallMsg.datalen  = iCount; 
		  }
		  
		  for(i = 0; i< iCount; i++)
		  {
              CallMsg.datawrd[i] = pMsg->aDataWrds[i];
		  }
			
         g_pfucCallback(&CallMsg);
	 }
		
	return 0;
}


int BrAda1553B_RT::MsgMake_chgMsgAddr(vector<S16BIT> vecSubAddr)
{
	// ���ݱ仯��Ϣ
	for (int i=0;i< (int)vecSubAddr.size();i++)
	{
		S16BIT subAddr = vecSubAddr.at(i);

#ifdef _DEBUG
		printf("\t[commu_RT info] chgMsgAddr_subAddr:rt%d,%d\n",iAddress,(int)subAddr); 
#endif
		// �ӵ�ַ�����ֳ���
		map<S16BIT,_addrData>::iterator SubIt = m_SubAddr.find(subAddr);
		if (SubIt !=  m_SubAddr.end())
		{
			UpdataBlkData_force(subAddr,SubIt->second.pDataSource->GetSubLen());
		}
	}
	return 1;
}

int BrAda1553B_RT::MsgMake_chgMsgSq(const RT_MSG & RtMsg, S16BIT subAddr)
{
	//��������λ��Ϣ
#ifdef  _DEBUG
	printf("\t[commu_RT]: chgMsgSseviceRequest_subAddr:%d\n",(int)subAddr);
#endif

	// ����״̬��
	m_StateWord.ServiceRequest = 1;
	WriteStatus();

	return 1;
}

//  ��������ȡ��
// vector�е������ӵ�ַ����Ҫ������ȡ����Դ
int BrAda1553B_RT::MsgMake_chgMsgThree(const vector<S16BIT> & vecSubAddr,int wWC,U16BIT * pData)
{
	for (size_t i=0;i< vecSubAddr.size();i++)
	{
		S16BIT subAddr = vecSubAddr.at(i);
		map<S16BIT,_addrData>::iterator it_subAddr = m_SubAddr.find(subAddr);
		S16BIT dataLen = (S16BIT)wWC;

		it_subAddr->second.pDataSource->SetThreeSource(pData,dataLen); //������ȡ����Դ

		UpdataBlkData_force(subAddr,32);
	}
	return 0;
}

int BrAda1553B_RT::MsgMake_chgMsgCopy(RT_MSG & rt,const MSGSTRUCT & msg,const vector<S16BIT> & vecSubAddr)
{
	// ���ݿ�����Ϣ, ȡ�������ж�Ӧ���ӵ�ַ
	for (size_t i=0; i<vecSubAddr.size();i++)
	{
		S16BIT subAddr = vecSubAddr.at(i);
#ifdef  _DEBUG
		printf("\t[commu_RT info] chgMsgCopy_subAddr  source:sa%d,dst:sa%d \n",(int)rt.subAddr,(int)subAddr);
#endif

		map<S16BIT,_addrData>::const_iterator it_data = m_SubAddr.find(subAddr);
		if (it_data != m_SubAddr.end())
		{
			it_data->second.pDataSource->SetSource(msg); 

			// ����Ҫ������ָ���ĵ�ַ,��Ϣ��������Դʱ,����Ϣ�������ֳ���ˢ�¡�
			UpdataBlkData_force(subAddr,32);
		}
	}
	return 1;
}


void BrAda1553B_RT::set_config(const std::string &cfg)
{
	try
	{
		m_msg_count =0;
		string cfg_n;
		if (cfg.length() != 0)
		{
			cfg_n = cfg;
		}
		else
		{
			printf("\t[commu_RT error]: set_config error !\n");
			return ;
		}

		m_configSeq.clear();

		RT::BrConfig config(cfg_n);

		string strRTAddress; // RT�ĵ�ַ
		string strFileFullPath; // �����ļ�����
		string strBigWord; // ʹ�õĴ�С��

		config.get_item("Address",strRTAddress);
		config.get_item("ConfigFilePath",strFileFullPath);	
		config.get_item("ChangeEndian",RT_Adapter_changeEndian);

		if (strFileFullPath.length() <= 0)
		{
			printf("\t[commu_RT error]: config RtConfigFile not find! \n");
			throw std::logic_error("config error!");
		}

		if(!m_configSeq.LoadFile(strFileFullPath))
		{
			printf("\t[commu_RT error]: load ConfigFile error! \n");
				throw std::logic_error("config error!");			
		}

		if (strRTAddress.length() <= 0)
		{
			// �����е�RT��ַ
			m_configSeq.GetRTAddress(iAddress);
		}

		if (iAddress <= 0 && strRTAddress.length() <= 0 )
		{
			printf("\t[commu_RT error]: config RT_Address not find! \n");
			throw std::logic_error("config error!");
		}
		if (strRTAddress.length() > 0)
		{
			iAddress =(S16BIT)atol_my(strRTAddress.c_str());	
		}

		// ���õ��ӵ�ַ
		m_configSeq.GetSubAddrInfo(m_SubAddr,m_forceSubAddr);

		m_configSeq.GetSubAddrCyle(m_SubAddrCyle);

		m_configSeq.GetChgMsgData(m_chgMsg_subAddr);

		m_configSeq.GetResourceCopy(m_ResourceCopy_subAddr);

		// �仯��������
		m_chgCyle_s.cycle = m_configSeq.getCyleState();

		m_configSeq.GetChgMsgState(m_chgMsg_subState);

		m_configSeq.GetStateWord(m_StateWord);


		// ȡ��ʱ�������Ϣ
		m_mapTimeCode.clear();
		{
			map<S16BIT,_addrData>::const_iterator itTime = m_SubAddr.begin();
			while(itTime != m_SubAddr.end())
			{
				_TimeCode timeCo;
				itTime->second.pDataSource->GetTimeCode(timeCo);
				if (TIME_CODE_error != timeCo.codeType)
				{
					m_mapTimeCode.insert(itTime->first);
				}
				itTime ++;
			}
		}

		// ȡ����ȡ������Ϣ
		{
			map<S16BIT,_addrData>::const_iterator itThree = m_SubAddr.begin();
			while(itThree != m_SubAddr.end())
			{
				_three_struct stu ;
				itThree->second.pDataSource->GetThreeStu(stu);

				if (_three_error != stu.eType)
				{
					S16BIT sa = itThree->first;
					// һ����Ϣ���ܶ�Ӧ����ӵ�ַ
					map<RT_MSG,vector<S16BIT> >::iterator itFind = m_mapThreeMsg.find(stu.msg);
					if (itFind == m_mapThreeMsg.end())
					{
						vector<S16BIT> vec;
						vec.push_back(sa);
						m_mapThreeMsg[stu.msg] = vec;
					}
					else
					{
						itFind->second.push_back(sa);
					}
				}
				itThree ++;
			}
		}

        	// ״̬����RT��ַ
        	m_StateWord.SetRtAddr(iAddress);

        	// ����
        	m_bConfigError = false;
	}
	catch(...)
	{
		printf("\t[commu_RT::ERROR] adapter.1553B.commu_RT: set_config error !\n");
		m_bConfigError = true;
	}
	return ;
}
