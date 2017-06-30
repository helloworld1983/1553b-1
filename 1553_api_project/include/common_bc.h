/**
 * \file common_bc.h
 * BC������ͷ�ļ�.
 *
 *    \date 2012-9-4
 *  \author xiaoqing.lu
 */
#ifndef _COMMON_BC_H_
#define _COMMON_BC_H_

#include "thread.h"
#include "common_qiu.h"
#include "stdio.h"
#include "stdlib.h"
//#include "gist/agent.h"
//#include "gist/gist.h"
//#include "gist/factory.h"
#include "propcfg.h"
#ifdef WIN32_DDC
#include "include/stdemace.h"
#elif defined(IPCORE)
#include "IPCORE.h"
#else
//#include "include_vx/stdemace.h"
#endif
#include <time.h>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <memory>
//#include "gist/icd/icdman.h"
//#include "gist/icd/BrBasicParser.h"
#include "comm_bc.h"
#include "types.h"
#include "zynq_1553b_api.h"
#include "msgop.h"
//using namespace ICD;
using namespace Agent;
using namespace std;

typedef struct HeadInfo 
{
	size_t     len;
	enum_operate_type  type;
}stHeadInfo;

typedef unsigned char byte;
///�豸��Ϣ
typedef struct _DEVICE_INFO 
{
	string hostAddr;
	int cardNum; ///< �߼��豸��
	unsigned long interval; ///< ʱ��������λ��us
	std::string address; ///< ��ַ
	std::string subAddress[32]; ///< �ӵ�ַ
}Device_info;

///��Ϣ
class BC_Msg : public BC_Msg_info
{
public:
	BC_Msg();
	~BC_Msg();
	void Create( S16BIT numCard);///< ����һ����Ϣ
	void Modify( S16BIT numCard);///< �޸�һ����Ϣ
	short GPF;					///< ����ͬ����Ϣ�Ƿ���	
	unsigned long long m_irqCount;///< ��ǰ��Ϣ�����ж��еĴ���
	U16BIT m_missFreq;			///< ©����ϢƵ�ʣ�ÿmissFreq����©��һ����0:ȫ��;1:ͣ��
	//BrBasicParser* m_parser; //zhanghao close.
	//Block* m_block; //zhanghao close.

	//void InitTimecode(const ICDMan* icdman, const Interface* ref_if);  //zhanghao close.
	//void SetTimecode(Agent::ISysServ *svr);   //zhanghao close.

	void loadSimFile();		///< ��������ļ�
	void SetBCData(S16BIT numCard, Agent::ISysServ *svr); ///< ��������
	void SetFileData();		///< ���ô��ļ�����������
	void SetCRC();			///< ����У���
	char* m_buff;			///< ��������ļ�����
	U32BIT m_buffSize;	///< m_buff�ĳ���
	U64BIT m_packCount;		///< ������Ĵ���
	char* m_pbuff; 			///< �����ȡ�ļ�����ĵ�ǰλ��
	char* m_CRCBuff; 		///< ��βУ��ʱ���������
	short m_CRCBuffSize;	///< m_CRCBuff�ĳ���
	BOOL m_isBrokenMode; 	///< �Ƿ��ڹ���ģʽ

	U16BIT nowOpt;			///< ��ǰ��ͨ��������
private:
	U16BIT GetOpt();		///< ȡ����Ϣ��ͨ��������ѡ��
};

///����
class BC_Ops
{
private:
	static S16BIT s_op_id;
public:
	static S16BIT GetAOpID() { return s_op_id ++; }///< ��ȡһ���µĲ���ID
	void AddOp( S16BIT devNum, S16BIT codeType, S16BIT msgid = 0 );///< ��Ӳ���
	void AddMsg( S16BIT devNum, BC_Msg &msg, S16BIT condition = ACE_CNDTST_ALWAYS );///< �����Ϣ
	void AddDelay( S16BIT devNum, U16BIT interval, S16BIT condition = ACE_CNDTST_ALWAYS );///< �����ʱ
	S16BIT* GetArrayAddr();///< ��ȡ�����б�Ĳ���ID�б�
	U16BIT GetSize() { return (U16BIT)m_lstOps.size(); } ///< ��ȡ�����б�ĳ���
	BC_Ops():m_pOps( NULL ) {}
	~BC_Ops(){
		if( m_pOps != NULL )
			delete [] m_pOps;
	}
	static void init(){///< ��ʼ������ID
		s_op_id = 400;
	}
private:
	std::vector<S16BIT> m_lstOps;///< ����ID�б�
	S16BIT* m_pOps;///< ����ID�ڴ�ָ��
};

///�ж���Ϣ
class FollowInfo{
public:
	FollowInfo(short pfollowMsgID, unsigned short pfollowValue, unsigned short pfollowMask, 
			   short pfollowDataindex, BOOL pisCheckSrvbit)
		:followMsgID(pfollowMsgID), 
		followValue(pfollowValue), 
		followMask(pfollowMask), 
		followDataindex(pfollowDataindex),
		isCheckSrvbit(pisCheckSrvbit){}
	///����==������
	bool operator==( const FollowInfo f){
		if (followMsgID == f.followMsgID &&
			followValue == f.followValue &&
			followMask == f.followMask &&
			followDataindex == f.followDataindex &&
			isCheckSrvbit == f.isCheckSrvbit){
			return true;
		} 
		else{
			return false;
		}
	}
private:
	short followMsgID;			///< �����ĸ���Ϣ�ж��Ƿ�Ҫ��
	unsigned short followValue;	///< �ж�ֵ
	unsigned short followMask;	///< �ж�MASK
	short followDataindex;		///< �жϸ���Ϣ�ĵڼ���������
	BOOL isCheckSrvbit;			///< �Ƿ��ж���Ϣ�ķ�������λ
};

/// ����write���������ݣ��Ա���receiver�߳��д�������������ݽṹҪ�����̰߳�ȫ��
class BrDataBuffer
{
	map<U16BIT, queue< pair<stHeadInfo, const char *> > > map_buf; ///< ����dataheader������
	map<U16BIT, queue< pair<stHeadInfo, const char *> > >::iterator it;
	MXLock m_lock;///< ��֤����Ķ�дʱ�̰߳�ȫ��
public:
	BrDataBuffer(){}
	~BrDataBuffer()
	{
		map_buf.clear();
	}
	
	void push(U16BIT index, const stHeadInfo &h , const char* d)
	{
		if (d)
		{
		       size_t len = h.len;
		       char *data = new char[len];
			memset((void *)data, 0, sizeof(char) * len);
			memcpy(data, (void *)d, len);
			if(map_buf.find(index) == map_buf.end())
			{
				queue< pair<stHeadInfo , const char *> > my_que;
				map_buf.insert(std::make_pair(index, my_que));
			}
		    AutoLock l(m_lock);	
		    map_buf.find(index)->second.push(std::make_pair(h, data));		
		}
	}

	   /// ��ö��׵� dataheader
	stHeadInfo *get_front_header(U16BIT index)
	{
	       it = map_buf.find(index);
		if(it != map_buf.end())
		{
		    AutoLock l(m_lock);
		    stHeadInfo *h = (it->second.size() > 0) ? &(it->second.front().first) : NULL;
		    return h;
		}
		else
		{
		   return NULL;
		}
	}
	   
	/// ��ö��׵�����
	const char * get_front_data(U16BIT index)
	{
		it = map_buf.find(index);
		if(it != map_buf.end())
		{
		   AutoLock l(m_lock);
		   const char *d = (it->second.size() > 0) ? (it->second.front().second) : NULL;
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
	
	void clear()
	{
		map_buf.clear();
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

///BC������
class BrAda1553B_BC_COMM: public Agent::IAdapter
{
public:
	vector<BC_Msg> m_vec_msg; ///< ��Ϣ�б�
	set<S16BIT> m_set_followMsgID; ///< ��Щ��Ϣ���жϱ����Ϣ�Ƿ��͵�����
	multimap<S16BIT, S16BIT> m_multimap_followMsg; ///< <������ϢID�����ж���Ϣindex>
	int m_minorFrameCount; ///< С֡����
	map<S16BIT, S16BIT> m_mapID; ///< <��ϢID�� ��ϢIndex>
	set< pair<S16BIT, S16BIT> > m_set_msgPlace; ///< <С֡ID����ϢID>
	map< S16BIT, vector<S16BIT> > m_map_msgPlace; ///< <С֡ID��vec<��ϢID>>
	set< S16BIT > m_set_msgID_inFrame; ///<����С֡�е���Ϣ>
	BOOL m_isBrokenMode; ///< �Ƿ��ڹ���ģʽ
	string m_fileName;   ///< �����ļ���

	set<S16BIT> m_setSaveFlag; ///< <��ϢID>
	MXLock my_Lock;

public:
	static std::map<S16BIT, BrAda1553B_BC_COMM*> s_adapt; ///< ͨ���ţ���Ӧ��ʵ��ָ��
	BrAda1553B_BC_COMM();
	virtual ~BrAda1553B_BC_COMM()
	{
		s_adapt.erase( device_info_bc.cardNum );
		work_stat = WS_EXIT;
	};

	//virtual long capability() const {return DC_WRITE | DC_IFID_REQ; }; zhanghao close.
	//virtual void set_interface(const Interface *if_obj);  zhanghao close.
	virtual U16BIT set_config(const std::string &cfg);
	virtual void set_address(const string &host_addr, const std::string &if_addr);
	virtual U16BIT source_init(); 
	virtual U16BIT start();
	virtual U16BIT stop();
	virtual U16BIT write(ctl_data_wrd_info *datahead);
	virtual void ioctl(U16BIT msg_id, msg_is_save flag, const char *save_path);
	//virtual ssize_t read( Agent::DevDataHeader *head, char *buffer,size_t);  zhanghao close.
	//virtual long version() { return DIV_ORIGIN; };   zhanghao close.

	static void _DECL MyISR( S16BIT DevNum, U32BIT dwIrqStatus );//�жϴ�����

private:
	volatile enum {
		WS_INIT, WS_RUN, WS_STOP, WS_ERROR, WS_EXIT
	} work_stat;


	void MakeMinorOps( S16BIT devNum, BC_Ops* ops );///< ����С֡��Ĳ���
	void MakeMajorOps( S16BIT devNum, BC_Ops& ops );///< ������֡��Ĳ���

public:
	static const int MAJOR_FRAME_ID;///< ��֡ID
	unsigned int MINOR_FRAME_TIME;	///< С֡��ʱ
	unsigned int MAJOR_FRAME_TIME;	///< ��֡��ʱ
	FILE  *fp;
private:
	BrDataBuffer m_write_data_buf;	///< ���ڻ���write���������ݣ��Ա���receive�߳��д���
	BrDataBuffer m_read_data_buf;	///< ���ڻ��潫Ҫ��read���ߵ�����
	//const Interface* m_interface;  	///<�ӿ�ָ��  zhanghao close.

public:
	Agent::ISysServ * m_sys_service;///< ����ϵͳ�������
	Device_info device_info_bc;		///< �豸��Ϣ
	unsigned long long m_itr_count;	///< �������жϸ���
	short m_GPF;					///< ����GPF�ã�������Ϣ��˵ȡֵΪ��2��3��4��5��6
	short m_itrMsgCount;			///< BC�����жϵĸ���
	map<short,short> m_mapGPF; 		///< <GPF, messId>ʹ�ø�GPF�����һ����Ϣ������ͬ���ж���Ϣ��GPF����
	map<string,string> m_item_value;   ///< ���ñ�������ݵı���
	map<int,vector<string> >data_value;  ///< DATA[32]���ݵı���
	map<int,vector<string> >member_value;  ///<����bc_msg������Ա��ֵ
	void ProcessVector(void);		///< �������������Ϣ���ݷ��͵���Ϣ
	void RecvAsyncMsg(short msgID);
	void deal_data(U16BIT messID);				///< ����write����������
	bool  readConfFile(string fileName); 	///<���������ļ�
	void CombSetConfig(const string & cfg);  ///<������Ϻ����������ļ�
	void parseConfFile(void);		///< ���������ļ�
	static void print_err_str(S16BIT result);		///< ����errorno���������Ϣ
	void Sub_Data(string str,vector<string> & vecStr,char cc);
	void CombBcMsgData(void);
	void ParserData(string &str);
	void GetBcMsgData(int i, char  *data);
	bool get_item(const string &item, string &value);
};

#endif /*_ADA1553_BC_H_*/
