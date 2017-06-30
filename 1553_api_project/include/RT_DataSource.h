#pragma once
#include <time.h>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <queue>
#include <set>
using namespace std;
#include "Define.h"
#include "SA_SimFile.h"
#include "ThreeCatch.h"
#include "common_qiu.h"
//#include "../../../../../include/gist/gist.h"
//#include "../../../../../include/gist/agent.h"

//using namespace ICD;
class DataSource;

// �����ļ�����Դ�Ľṹ
typedef struct _fileConfig
{
	//string m_strFileName;
	//bool bIsBagBag; // �Ƿ���С��	
	//SimlBag m_bagBag;// С����
	string fileName; // �ļ�����
	U16BIT defWord;  // ����32��ʱ�Ĳ�ֵ
	bool bAffect;	 // С���Ƿ���Ч
	SimlBag bagBag;	 // С��
	_fileConfig()
	{
		bAffect = false;
	}
}_fileConfig;


// ʱ����Ľӿ�
class time_Inter
{
public:
	time_Inter() {

	};
	virtual ~time_Inter(){

	};

public:
	/*  zhanghao close.
	// ʱ����ת��,����,��time���뵽pData��
	// ���ر����ĳ���
	virtual int ConvertTime_Code(char * pData,int len,const GIST::BrTime & time){
		return true;
	};

	// ʱ����ת��,����,��pData�н�����time
	// ���ؽ�����ĳ���
	virtual int ConvertTime_Encode(const char * pData,int len,GIST::BrTime & time){
		return true;
	};
	*/
};

// �ӵ�ַ���������ݽṹ              
typedef struct _addrData 
{
	U16BIT dataBuff[32 +1];   //32���ֵĻ���
	DataSource * pDataSource; // ����Դ��Ϣ

	S16BIT blockID_TX; 
	S16BIT blockID_RX; 
	bool flushFlag ; // ˢ�±��

	_addrData()
	{
		flushFlag = false;
		blockID_RX = 0;
		blockID_TX =0;
		memset((void *)dataBuff,0,sizeof(S16BIT) * 33);
	}
} _addrData;

class DataSource;

// CRCУ��
S16BIT CheckoutCRC(U16BIT * pData,S16BIT len);

// �ۼӺ�У��
S16BIT CheckoutADD(U16BIT * pDAta,S16BIT len);

// У��ʹ�����
typedef S16BIT (* FunCheckout)(U16BIT * pData,S16BIT len);

// У�麯����
const FunCheckout g_FunArr[3] = {NULL,CheckoutCRC,CheckoutADD};

// 
// �ӵ�ַ������Ϣ�ṹ
class SA_Config
{
public:
	SA_Config(){
		saAddr = 0;
		memset((void*)defDataArry,0,sizeof(U16BIT)*32);
		timeCode.codeType = TIME_CODE_error;
		theckOut.type = _ERROR;
		threeObj.eType = _three_error;
		wordLen = 32;
		bNoCgh = false;
		m_bForceFlush = false;
		IsBigWord = false;
	};

	~SA_Config(){
	};

public:
	S16BIT saAddr; // �ӵ�ַ

	// ʱ����
	_TimeCode timeCode;

	// ��ȡ������
	_three_struct  threeObj;

	// У���
	_CheckOutStu theckOut;

	// �Ƿ���
	bool IsBigWord;
public:
	// �ӵ�ַ���ݳ���
	S16BIT wordLen; 

	// �ļ�����Դ
	_fileConfig fileCfg;

	// Ĭ��ֵ�Ļ���
	U16BIT defDataArry[32];   //32���ֵĻ���

	// ��Ϣ����
	// set<RT_MSG> m_msgCopy; 

	// ���ݱ仯������
public:

	// ��Ϣ�ı�
	// set<RT_MSG> m_msgChg;

	// ���ڱ仯
	_CycleIndex m_CyleInfo;

	// �̶�ֵ
	bool bNoCgh;

	// ȡ�ߺ����
	bool m_bForceFlush;
};

// ״̬�ֵ�����
class StateWord_Config
{
public:
	StateWord_Config(){
		bConfigFlag =false;
	};
	~StateWord_Config();

	bool bConfigFlag;
public:
	int ServiceRequest;// ��������λ
	int SystemFlag;   // ��ϵͳ����λ
	int TerimterFlag; // �ն˱�ʶλ
	int BusyFlal ;// æλ

public:
	// ��������Ϊ���ڱ仯
	_CycleIndex m_CyleInfo;

	// ��������λ��Ϣ����
	set<RT_MSG> m_msgChg;
};


// ʸ���ֵ�����
class VectorWord_Config
{
public:
	VectorWord_Config();
	~VectorWord_Config();
};

class WriteDataBuffer
{
	  map<U16BIT, queue< pair<size_t, U16BIT *> > > map_buf;
	  map<U16BIT, queue< pair<size_t, U16BIT *> > >::iterator it;
	  //U16BIT data[32+1]; // //32���ֵĻ���
	  MXLock m_lock;///< ��֤����Ķ�дʱ�̰߳�ȫ��
public:
	WriteDataBuffer(){}
	~WriteDataBuffer()
	{
		map_buf.clear();
	}
	void push(U16BIT index, size_t len, const char* d)
	{
		if (d)
		{
		       U16BIT *data = new U16BIT[32];
			size_t  data_size_1553 = len;
			bool is_odd_size = len % 2 > 0;
			if (is_odd_size)
			{
			    data_size_1553 = len + 1;
			}
			char *data1553 = const_cast<char*>(d);
			if(is_odd_size)
			{
                           data1553 = new char[data_size_1553];
				memset(data1553, 0, data_size_1553);
				memcpy(data1553, d, len);
			}
			memset((void *)data, 0, sizeof(data));
			memcpy(data, (unsigned char *)data1553, data_size_1553);
			if (is_odd_size)
			{
			    delete[] data1553;
			}
			if(map_buf.find(index) == map_buf.end())
			{
				queue< pair<size_t , U16BIT *> > my_que;
				map_buf.insert(std::make_pair(index, my_que));
			}
		    AutoLock l(m_lock);	
		    map_buf.find(index)->second.push(std::make_pair(data_size_1553, data));		
		}
	}

	 /// ��ö��׵� dataheader
	size_t *get_front_header(U16BIT index)
	{
	       it = map_buf.find(index);
		if(it != map_buf.end())
		{
		    AutoLock l(m_lock);
		    size_t *msg_len = (it->second.size() > 0) ? &(it->second.front().first) : NULL;
		    return msg_len;
		}
		else
		{
		   return NULL;
		}
	}
	   
	/// ��ö��׵�����
	U16BIT * get_front_data(U16BIT index)
	{
		it = map_buf.find(index);
		if(it != map_buf.end())
		{
		   AutoLock l(m_lock);
		   U16BIT *d = (it->second.size() > 0) ? (it->second.front().second) : NULL;
		   return d;
		}
		else
		{
		   return NULL;
		}
	}
	void pop(U16BIT index)
	{
		it = map_buf.find(index);
		if(it != map_buf.end())
		{
		   AutoLock l(m_lock);
		   delete []it->second.front().second;
		   it->second.pop();
		}
	}
	void clear(U16BIT index)
	{
		it = map_buf.find(index);
		if(it != map_buf.end())
		{
		   AutoLock l(m_lock);
		   while (it->second.size() > 0)
		   {
		          delete [] (it->second.front().second);
			   it->second.pop();
		   }
		}
	}
	size_t size(U16BIT index)
	{
		size_t s = 0;
		it = map_buf.find(index);
		if(it != map_buf.end())
		{
		       AutoLock l(m_lock);
			s = it->second.size();
		}
		return s;
	}

	bool empty(U16BIT index)
	{
            it = map_buf.find(index);
	     if(it != map_buf.end())
	     {
	 	   AutoLock l(m_lock);
	 	  return it->second.empty();
	     }
	}
};


// 
// �ӵ�ַ������Դ,�����ṩ�ӵ�ַ����������
// 1.�ļ�����Դ
// 2.��Ϣ��������Դ
// 3.ָ������Դ
class DataSource
{
public:
	DataSource();

	DataSource(S16BIT addr,S16BIT subAddr,S16BIT cardNum);

	~DataSource();

	// ��ʼ��
	void init();

	// ����
	void clear(); 

private:

	// ������Ϣ����Դ
	void CopyData_msg(U16BIT * pData,U16BIT copyLen);

	// ����ָ������Դ
	void CopyData_ins(U16BIT * pData,U16BIT copyLen);

	// �����ļ�����Դ
	void CopyData_file(U16BIT * pData,U16BIT copyLen);

public:
	//���ļ�
	void OpenSimFile(S16BIT addr,S16BIT subAddr,S16BIT cardNum);

	// ����ָ�����ȵķ�������
	void CopyData(U16BIT * pData,U16BIT copyLen);
	
       void CopyData(S16BIT subAddr, U16BIT * pData,U16BIT copyLen);
	// ��������Դ
	void SetSource(const MSGSTRUCT & msg);

	// ��������Դ
	void SetSource(const char * pData,int len);

	// ��������Դ
	void SetSource(S16BIT subAddr, const char * pData,int len);

	// ����ʱ������Դ
	void SetTimeSource();

	// �����Ϣ�������,ָ������Դ���
	void EnSimSource();
	//�ж�д���buff�Ƿ�Ϊ��
       bool WriteBuffIsEmpty(S16BIT subAddr);
	// ����ʱ��Уʱ
	//void ShipTimeCheck(char * pData,int dataLen);

	// ������ȡ��������Դ
	void SetThreeSource(U16BIT * pData,S16BIT dataLen);
private:
	// ����ʱ����,���ﲻ��Դ���32���ֵ������Ĭ���ڻ������������
	void GetTimeData(U16BIT timeData[]);

	// ����У��ֵ
	void GetCheckValue(U16BIT paramData[],S16BIT len);

	// �õ�GIST����ʱ
	void GetTimeData_GIST(U16BIT timeData[]);

public:
	// ����У��
	// cheType : У������� 
	// cheIndex : У��ֵ��λ��
	void SetCheckout(S16BIT cheType,S16BIT cheIndex);

	// �������õ��ļ�����Դ
	void SetFileResource(const _fileConfig & fileStr);

	// �õ����õļ�����Ϣ
	void GetCheckout(__out S16BIT & cheType,__out S16BIT & cheIndex);

	// �õ����õ��ļ�Դ��Ϣ
	void GetFileResource(__out _fileConfig & RecourceFile);

	// ��������Դ��ʱ����
	void SetTimeCode(_TimeCode & timeC);

	// �õ�ʱ����
	void GetTimeCode(_TimeCode & timeC);

	// ��С�˵ߵ�
	void BigWord(U16BIT * pData,U16BIT wordLen);

	// ��С�˱�Ǳ���
	bool m_bBigWord;

	// ���ô�С������
	void SetBigWord(bool bBigWord);

	// ������ȡ���ṹ
	void SetThreeStu(_three_struct & threeS);

	// �õ���ȡ���ṹ
	void GetThreeStu(_three_struct & threeS);

	// �õ������ֳ���
	S16BIT GetSubLen();

	// ���������ֳ���
	void SetSubLen(S16BIT saDataLen);

	// ʱ����
	_TimeCode m_TimeCode;

	// �����Ƿ�������ģʽ,���߽�������ģʽ
	// ����ģʽ,�������ö���������,��copyData�����������κζ���
	void SetWorkMode(CHAR val);

public:

	// ���ð忨�ż�����ʱ��
	void SetDevNum(S16BIT devNum);
	// ���ð忨�ż�����ʱ��
	//void SetStartTime();
	void SetTimeObj(time_Inter * timeInter){
		m_TimeInter = timeInter;
	};

private:

	// ������
	int m_iCount;

	// ʱ���ʽ�����ļ�������
	time_Inter * m_TimeInter;

	// ��ȡ������Դ
	ThreeCatch  m_ThreeObj;

	S16BIT m_devNum; // �豸���,�Ӱ忨��ȡʱ��Ҫ��
	//GIST::BrTime m_startTime; // �Ӱ忨��ȡ�Ŀ�ʼʵ��ʱ��  //zhanghao close.
	
	// ����GIST����ʱ
	//void TestStart_GIST();

public:

	// У���
	_CheckOutStu m_Checkout;

	// С����Ĭ��ֵ
	S16BIT m_defWord;

	// С����
	SimlBag m_bagBag;

	// �ļ�����
	string m_strFileName;

	// ��С��
	bool m_bSmilBag;

	// �ļ�Ĭ��ֵ
	bool m_bDefFile; 

	// �ӵ�ַ���ݳ���
	S16BIT m_saDataLen; 

private:

	// С�����ñ��
	bool m_bBagStart;

	// �Ƿ����ģʽ
	bool m_bErrorMode;

	// �ӱ���С�����������
	void CopyDataByBag(U16BIT * pData,S16BIT len_Sub,S16BIT offset);

private:
	// �����ļ���
	SimFile * m_pFileObj;

	// С��������
	U16BIT m_BagData[MAX_LEN_COPYMSG];

	// ��������
	S16BIT m_CopyResource_msg[32];

	// ��������
	S16BIT m_CopyResource_ins[MAX_LEN_COPYMSG];

	// ��Ϣ����
	bool m_IsCopy_msg;
	_CopyMsg_type  m_CopMsgType;
	S16BIT m_msgLen;

	// ָ������Դ
	bool m_IsCopy_ins;
	bool m_IsCopy_ins_more ; // ָ���Ƿ����32���֣�����Ҫ��η���
	S16BIT m_insLen;
	S16BIT m_offset;	// ָ������Դ����Դ��ƫ��

	WriteDataBuffer my_write_data_buf;	///< ���ڻ���write����������
	size_t r_offset;
public:
	// Ĭ��ֵ�Ļ���
	U16BIT m_defDataArry[32];   //32���ֵĻ���
};



