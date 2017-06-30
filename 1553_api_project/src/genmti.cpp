/**
 * \file genmti.cpp
 * 1553Bʾ��ģ�飺ʵ��Adapter����DDC��MTI�ӿ����1553B���߼��.
 *
 *    \date 2012-9-4
 *  \author anhu.xie
 */
//#define _CRT_SECURE_NO_WARNINGS

#include "zynq_1553b_api.h"
#include <iostream>
#include <sstream>
//#include "platform/timeutil.h"
#include "propcfg.h"
#include "net2host.h"
//#include "../../../FrameWork//include/Utility.h"
#ifdef IPCORE
#include "IPCORE.h"
#else
#include "stdemace.h"
#endif
#include "genmti.h"


inline void throw_ace_error(const char *msg, S16BIT err) {
	std::stringstream err_str;
	err_str << msg;
	throw std::runtime_error(err_str.str());
}

ssize_t ParseData(MSGSTRUCT msg, Agent::DevDataHeader *head, char *buffer, size_t length, bool big_endian)
{
	size_t ll = sizeof msg + sizeof(U16BIT);
	if ( length < ll )
		return -static_cast<int>(ll);

	//��������Դ��Ŀ�ĵĵ�ַ���ӵ�ַ��wTR1 == 1��ʾ����Դ��
	// ��ַ
	U16BIT wRT = 0;
	// �շ�״̬1
	U16BIT wTR1 = 0;
	// �շ�״̬2
	U16BIT wTR2 = 0;
	// �ӵ�ַ
	U16BIT wSA = 0;
	// �����ֻ�ʸ����
	U16BIT wWC = 0;
	aceCmdWordParse(msg.wCmdWrd1, &wRT, &wTR1, &wSA, &wWC);

	//��������ͷ��Ϣ
//	head->status = GIST::PACK_STATUS_OK; 
	/* zhanghao close.
	switch (msg.wType) {
	case ACE_MSG_MODENODATA:
		sprintf(head->src_address, "%s", "BC");
		sprintf(head->src_subaddress, "%s", "MODE_NODATA");
		sprintf(head->dest_address, "%d", wRT);
		sprintf(head->dest_subaddress, "%d", wWC);
		break;
	case ACE_MSG_MODEDATARX:
		sprintf(head->src_address, "%s", "BC");
		sprintf(head->src_subaddress, "%s", "MODE_DATARX");
		sprintf(head->dest_address, "%d", wRT);
		sprintf(head->dest_subaddress, "%d", wWC);
		break;
	case ACE_MSG_MODEDATATX:
		sprintf(head->dest_address, "%s", "BC");
		sprintf(head->dest_subaddress, "%s", "MODE_DATATX");
		sprintf(head->src_address, "%d", wRT);
		sprintf(head->src_subaddress, "%d", wWC);
		break;
	case ACE_MSG_BCTORT:
		sprintf(head->src_address, "%s", "BC");
		sprintf(head->dest_address, "%d", wRT);
		sprintf(head->dest_subaddress, "%d", wSA);
		break;
	case ACE_MSG_RTTOBC:
		sprintf(head->dest_address, "%s", "BC");
		sprintf(head->src_address, "%d", wRT);
		sprintf(head->src_subaddress, "%d", wSA);
		break;
	case ACE_MSG_BRDCST:
		sprintf(head->dest_address, "%d", wRT);
		sprintf(head->dest_subaddress, "%d", wSA);
		sprintf(head->src_address, "%s", "BC");
		sprintf(head->src_subaddress, "%s", "BROADDCAST");
	default:
		if ( wTR1 ) {
			sprintf(head->src_address, "%d", wRT);
			sprintf(head->src_subaddress, "%d", wSA);
		} else {
			sprintf(head->dest_address, "%d", wRT);
			sprintf(head->dest_subaddress, "%d", wSA);
		}
		if ( msg.wCmdWrd2Flg ) {
			aceCmdWordParse(msg.wCmdWrd2, &wRT, &wTR2, &wSA, &wWC);
			if (wTR2 == 1) {
				sprintf(head->src_address, "%d", wRT);
				sprintf(head->src_subaddress, "%d", wSA);
			} else {
				sprintf(head->dest_address, "%d", wRT);
				sprintf(head->dest_subaddress, "%d", wSA);
			}
		}
		else {
			if (wTR1 == 1) {
				sprintf(head->dest_address, "%s", "BC");
				sprintf(head->dest_subaddress, "%d", wSA);
			} else {
				sprintf(head->src_address, "%s", "BC");
				sprintf(head->src_subaddress, "%d", wSA);
			}
		}
		break;
	}
	*/

	
	U16BIT flag = msg.wWordCount;

	head->extra_size = sizeof(MSGSTRUCT) - sizeof(msg.aDataWrds ); 
	//����1553B��Ϣ
	//����Ϣ��д�봫��������
	size_t isize = (char*)msg.aDataWrds - (char*)&msg;
	memcpy(buffer, &msg, isize);
	memcpy(buffer + isize, &(msg.wBCCtrlWrd), head->extra_size - isize);
	memcpy(buffer + head->extra_size, &flag, sizeof(U16BIT));
	char *p = buffer + head->extra_size + sizeof(U16BIT);
	memcpy(p, msg.aDataWrds, sizeof msg.aDataWrds);
	short* pshort = reinterpret_cast<short*>(p); 
	if ( big_endian ){
		for(int i=0; i<32; i++){
			//zhanghao close.  Br_Util::swap_bytes(*(pshort+i));
		}
	}
	return ll;
}


/**
 * ��ȡһ��������MTI��Ϣ.
 * ���ص���Ϣ����ԭMT�豸����Ϣ������ԭ���Կؼ��鿴��
 * ����������Ϊһ��MSGSTRUCT�ṹ�����ݣ����һ�������ֶΡ����岼�����£�
 * ������MSGSTRUCT�ṹ����������(aDataWrds)Ų����󣬲���������֮ǰ������һ�������ֶΣ���ʾ��Ч���ݵĳ��ȡ�
 * ���У����ӵĳ����ֶκ�������������Ϊ��ʽ���ݷ��أ�֮ǰ�Ĳ���(MSGSTRUCT�г�����֮�����Ϣ)��Ϊ˽������(extra_size)���ء�
 * ���ԣ��������ݵĳ��ȹ̶�Ϊsizeof(MSGSTRUCT) + sizeof(U16BIT)��extra_size�̶�Ϊsizeof(MSGSTRUCT) - sizeof(MSGSTRUCT::aDataWrds)��
 * \param head ����·�ɵ���Ϣ��
 * \param buffer ��ȡ�����ݴ���ڴ˴�
 * \param buf_len ���õ����ݻ������Ĵ�С
 * \return ��ȡ�����ݰ����ȡ�
 * - MT������Ϣ�ĳ��ȣ�
 * - ��ʱû������ʱ������0��
 * - ���û�����̫Сʱ������MT������Ϣ���ȵ��෴����
 */
ssize_t GIST_MT2::read(/* DataTime *time,  zhanghao close. */ Agent::DevDataHeader *head, char *buffer, size_t length) {
	MSGSTRUCT msg;
	if ( length < sizeof msg + sizeof(U16BIT) )
		return -static_cast<ssize_t>(sizeof msg + sizeof(U16BIT));
	if ( host_buffer ) {
		U32BIT dwStkLost = 0;
		U32BIT dwHBufLost = 0;
		U32BIT dwMsgCount = 0;
		S16BIT nResult = aceMTGetHBufMsgDecoded(devNum, &msg, &dwMsgCount, &dwStkLost,& dwHBufLost, ACE_MT_MSGLOC_NEXT_PURGE);
		if ( nResult ) {
			throw_ace_error("aceMTGetHBufMsgDecoded Failed!", nResult);
		}
		if ( !dwMsgCount )
			return 0;
		if ( (dwStkLost || dwHBufLost) && (m_StkLost != dwStkLost || m_HBufLost != dwHBufLost) ) {
			m_StkLost = dwStkLost;
			m_HBufLost = dwHBufLost;
			printf("mt message lost: stack %d, host %d\r\n", dwStkLost, dwHBufLost);
		}
	}
	else {
		S16BIT nResult = aceMTGetStkMsgDecoded(devNum, &msg, ACE_MT_MSGLOC_NEXT_PURGE, ACE_MT_STKLOC_ACTIVE);	
		if (nResult < 0) {
			throw_ace_error("aceMTGetStkMsgDecoded Failed!", nResult);
		}
		if (nResult == 0)
			return 0;
	}
	//time->time_code = msg.wTimeTag;  zhanghao close.
	return ParseData(msg, head, buffer,length,big_endian);
}

U16BIT GIST_MT2::source_init(SyncEvent *sync) 
{
	m_StkLost = 0;
	m_HBufLost = 0;
	/* Initialize Device */
#ifdef IPCORE
	S16BIT nResult = aceInitialize(devNum, ACE_ACCESS_CARD, ACE_MODE_MT, 0, 0, 0, sync);
#else
	S16BIT nResult = aceInitialize(devNum, ACE_ACCESS_CARD, ACE_MODE_MT, 0, 0, 0);	
#endif
	if ( nResult )
	{
		printf("aceInitialize Failed!\n");
		return RET_1553B_ERR;
	}
	if ( host_buffer )
	{
		MTINFO sMTInfo;
		aceMTGetInfo(devNum, &sMTInfo);
		aceMTInstallHBuf(devNum, ((sMTInfo.wCmdStkSize / 4) * 40) * 80);
	}
	else 
	{
		/* set MT configure */
		nResult = aceMTConfigure(devNum, ACE_MT_SINGLESTK, ACE_MT_CMDSTK_4K, ACE_MT_DATASTK_32K, 0);
		if ( nResult ) 
		{
			printf("aceMTConfigure Failed!\n");
			return RET_1553B_ERR;
		}
	}

	return RET_1553B_OK;
}

U16BIT GIST_MT2::start()
{
	  /* start RT/MT Operations */
	  S16BIT nResult  = aceMTStart(devNum);
	  if (nResult )
	  {
		  printf("aceMTStart Failed!\n");
		  return RET_1553B_ERR;
	  }
	  std::cout << "1553B MT started!" << std::endl;
	  return RET_1553B_OK;
}

U16BIT GIST_MT2::stop()
{
	/* Stop MTi */
	S16BIT wResult=aceMTStop(devNum);
	if (wResult != ACE_ERR_SUCCESS) 
	{
		 printf("aceMTIStop Failed!\n");
		 return RET_1553B_ERR;
	}
	/* free the card */
	wResult = aceFree(devNum);
	if (wResult != ACE_ERR_SUCCESS)
	{
		 printf("aceFree Failed!\n");
		 return RET_1553B_ERR;
	}

	if(NULL != fp)
	{
		 fclose(fp);
	}
	std::cout << "1553B MT stop!" << std::endl;
	return RET_1553B_OK;
}

U16BIT GIST_MT2:: msg_save (const char *save_path)
{
	  if(NULL == fp)
	  {
			fp = fopen(save_path, "w+");
			if(NULL == fp)
			{
              printf("fopen %s failed\n",save_path);
				return RET_1553B_ERR;
			}
	  }
	  mt_set_file(fp);

	 return RET_1553B_OK;
}
