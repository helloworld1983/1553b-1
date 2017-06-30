#pragma once

#include "thread.h"
//#include "../../../../../include/gist/agent.h"
//#include "../../../../../include/gist/gist.h"
//#include "../../../../../include/gist/factory.h"


#include <time.h>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <queue>
#include <set>

using namespace std;

#include "RT_DataSource.h"
#include "ConfigSplit.h"
#include "RT_StateWord.h"

#ifndef WIN32
#include "time.h"
#endif

#ifndef IPCORE
#ifdef __vxworks
extern "C" void aceEnumCards();
extern "C" int ddccm(int initFamily, unsigned char bShowTable);
static class DDCDriverLoader {
public:
	DDCDriverLoader() {
		//		aceEnumCards();
		ddccm(1, 1);
	}
} load_ddc_driver;
#endif
#endif

/// ����ʸ���ֵĵ�ַ33
const S16BIT  VECTOR_ADDR  = 33;

/// 1.Ӧ�Ը����յ�����Ϣ������������ͨ��RT���ܴﵽ�������ַ�ʽ��
/// �ṩ���������ӿ�

/// ���յ���Ϣ��ĺ���
typedef void (* FUN_MSGRECV)(S16BIT wSa,S16BIT len,U16BIT data[]);

/// �޸��ӵ�ַ������
typedef void (* FUN_SUBADDRDATA)(S16BIT wSa,S16BIT len,U16BIT data[]);

/// �����ṩ�Ľӿ�
typedef struct _IntfaceFun 
{
	FUN_MSGRECV fun_msg;
	FUN_SUBADDRDATA  funData;
}_IntfaceFun;

class BrAda1553B_RT ;


/**
* �̳�ʵ��,Adapter�ӿڡ�
* 
* ʵ��1553BЭ���£����ͨ��RT�ĸ�������
*/
class BrAda1553B_RT: public Agent::IAdapter_V2,public Thread
{
public:

	/// �����ӵ�ַ����Դ�����Դ���32����
	void SetSubAddrData(S16BIT subAddr,int len,U16BIT data[]);

	/// ���յ����ݴ���
	virtual void onMsgRecv(S16BIT wSa,S16BIT len,U16BIT data[]){
		return;
	};

public:

	BrAda1553B_RT(void);

	~BrAda1553B_RT(void);

public:
	///��ַ
	S16BIT iAddress ;

	///�忨��
	S16BIT cardNum;

      ///�Ƿ񱣴���Ϣ�ı��
	S16BIT IsSaveFlag;

	FILE  *fp;

	/// �����ļ����������Ϣ

public:

	/// �ӵ�ַ����map��Ϊ�˷������,��Ӧ����������
	map<S16BIT,_addrData> m_SubAddr; 

	/// ÿ���ӵ�ַ������
	map<S16BIT,_CycleIndex>  m_SubAddrCyle; 

	/// ��Ҫ�����ݱ����ߺ�ˢ�µ��ӵ�ַ��
	set<S16BIT> m_forceSubAddr;

	/// ��������λ���ڱ仯
	_CycleIndex m_chgCyle_s;

	/// �ӵ�ַ��Ӧ�ı仯��Ϣ
	map<RT_MSG, vector<S16BIT> > m_chgMsg_subAddr;	

	/// �յ�ָ����Ϣ����������λ��1
	set<RT_MSG> m_chgMsg_subState;

	/// ������ʱ������ӵ�ַ,Уʱ���ϱ�ʱ�䶼������
	set<S16BIT> m_mapTimeCode;	

	/// ��������ȡ������Ϣ����Դ
	map<RT_MSG,vector<S16BIT> > m_mapThreeMsg;

	/// ���ݿ�����Ӧ���ӵ�ַ
	map<RT_MSG,vector<S16BIT> > m_ResourceCopy_subAddr; 

	///״̬�ֱ�ʶλ
	RT_StateWord  m_StateWord;

	///  ·�ɴ���ĺ���
private:

	/* zhanghao close.
	/// ����·��a
	GIST::BR_OPERATION_R parse_rout_a(const char *data, size_t len );

	/// ����·��a,�µ�·��a����·�ɵ�Դ��ַ��Դ�ӵ�ַ��Ŀ�ĵ�ַ��Ŀ���ӵ�ַ������ָ��RT��ַ�ӵ�ַ
	GIST::BR_OPERATION_R parse_rout_a(const char *data, size_t len ,S16BIT subAddr);

	/// ����·��b
	GIST::BR_OPERATION_R parse_rout_b(const char *data, size_t len );

	/// ����·��c
	GIST::BR_OPERATION_R parse_rout_c(const char *data, size_t len );

	/// ����·�ɴ���
	GIST::BR_OPERATION_R parse_rout_error(const string & src_subAddr);
	*/
private:

	/// ����ʱ��
	_time_rt m_startTime;

	//BrTimeTriple  m_time_br;  zhanghao close.

	/// ������
	int m_TimeCount;

	/// ����ͬ���ź�
	map<int,SyncEvent *> event_map;

	/// ��ʱ��ͬ������
	SyncEvent *m_tim_mutex;

	/// ����1553B�忨ʱ����
	MXLock m_Lock_card;

	///����
	void clear();

	/// ������Ϣ���ı��ӵ�������
	int MsgMake_chgMsgAddr(vector<S16BIT> vecSubAddr);

	/// ������Ϣ�ı��������λ
	int MsgMake_chgMsgSq(const RT_MSG & vecRtMsg, S16BIT subAddr);

	/// ������Ϣ����������Դ
	int MsgMake_chgMsgCopy(RT_MSG & rt,const MSGSTRUCT & msg,const vector<S16BIT> & vecSubAddr);

	/// ��������ȡ��
	/// vector�е������ӵ�ַ����Ҫ������ȡ����Դ
	int MsgMake_chgMsgThree(const vector<S16BIT> & vecSubAddr,int wWC,U16BIT * pData);

	/// ����Ϣ������
	size_t ReadMsg();

	/// �������ݿ�
	/// �����Ƿ�������
	/// ����ˢ�»�ʹ������
	bool UpdataBlkData(S16BIT subAddr,int updataLen);

	/// ǿ��ˢ��
	void UpdataBlkData_force(S16BIT subAddr,int updataLen);

	/// ����״̬��
	void WriteStatus();

	/// ������Ϣ��������
	int MakeMsg_Parse(MSGSTRUCT * pMsg,U16BIT & wCmdWrd);

	/// ����������ɵ�����
	void SizeCyle_Parse();

public:
	/// �жϴ������
	static std::map<S16BIT, BrAda1553B_RT*> s_adapt;

	/// �жϴ�����
	static void _DECL MyISR( S16BIT DevNum, U32BIT dwIrqStatus );

	/// �������Ϣ�ĸ���
	unsigned long long m_msg_count; 

private:

	/// ���ý�������
	RTconfigSeq  m_configSeq ; 

	bool m_bConfigError;

	/// ICDMan����
	//const ICD::ICDMan * m_IcdMan;  zhanghao close.

	/// �ӿ�ָ��
	//const Interface* m_interface;  zhanghao close.

	SyncEvent m_sync; ///< �������߳���ֹͣ��֪ͨ���߳�

	bool m_Is_write_data;
public:
	/// ����ĺ�����adapter�ı�׼�ӿ�
	//virtual long version() { return DIV_V2; }

	//virtual long capability() const ;

	virtual void set_config(const std::string &cfg);

	virtual void set_address(const string &host_addr, const std::string &if_addr);
	virtual U16BIT source_init(SyncEvent *tim_mutex);
	virtual U16BIT  start();
	virtual U16BIT stop();

	//virtual ssize_t read(DataTime *time, Agent::DevDataHeader *head, char *buffer,size_t);  zhanghao close.

	//virtual std::multimap<std::string, std::string> get_bus_addr();

	//virtual void write_data(const char *data, size_t len, const std::pair<std::string, std::string> *channel);
	virtual int write(U16BIT subaddr, const char *data, size_t len);

	virtual void do_job();

	virtual unsigned long get_timer_interval();

	//virtual void set_interface(const Interface *if_obj); zhanghao close.

       virtual void  ioctl(U16BIT subaddr, msg_is_save flag, const char *save_path);
        
	/*void start() {
		Thread::start("1553B_commu_RT");
	}*/
};
