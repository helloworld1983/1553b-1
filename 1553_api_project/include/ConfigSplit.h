#pragma once
#include "RT_DataSource.h"
#include "RT_StateWord.h"
#include "utility.h"


///
//  �����ļ�����
//
class SplitBase
{
public:
	SplitBase(void);
	~SplitBase(void);

	void clear()
	{	
	
	};
public:

	// ���õ��ӵ�ַ
	void Split_SubAddrInfo(__in string strSubAddr,__in string ConfigFileStr,__out map<S16BIT,_addrData> & m_SubAddr);

	void GetSubAddrCyle(__in string str,__out map<S16BIT,_CycleIndex> & m_SubAddrCyle);

	//
	void GetChgMsgData(__in string str,__out map<RT_MSG, vector<S16BIT> > & m_chgMsg_subAddr);

	void GetChgMsgState(__in string str,__out set<RT_MSG> & m_chgMsg_subState);

	void GetResourceCopy(__in string str,__out map<RT_MSG,vector<S16BIT> > & m_ResourceCopy_subAddr);

private:

	// ���ļ�������Ϣ
	void GetResourceFile(__in string str,__out map<S16BIT,_fileConfig> & ResourceFile_sub);

	// ������С����Ϣ
	bool SplitSimlBag(string str,SimlBag & fileBag);

public:

};

// RT�����ļ������ͱ�����ַ���
class RTconfigSeq
{
public:
	RTconfigSeq();

public:

	// ͨ���ļ�����װ�������ַ�����Ϣ
	// fileName: �ļ�·�����Ƶ�ȫ��
	// �����Ƿ�����ɹ�
	bool LoadFile(const string & fileName);

	// ���������ַ�������
	// cfg: �����ַ���
	// �����Ƿ�����ɹ�
	bool LoadConfigStr(const std::string & cfg);

	// �������еĸ�������
	void clear();


// ����
public:
	// �õ������е�RT��ַ
	// iAddress: ���ص�RT��ַ
	bool GetRTAddress(S16BIT & iAddress);

	// ���õ��ӵ�ַ����Դ������Ϣ
	// subAddrInfo: �������ӵ�ַ����Դ��Ϣ
	// forceFlush: �ӵ�ַǿ��ˢ�µ���Ϣ
	void GetSubAddrInfo(__out map<S16BIT,_addrData> & subAddrInfo,__out set<S16BIT> & forceFlush);

	// �ӵ�ַ����Ҫ����ˢ�µ���Ϣ
	void GetSubAddrCyle(__out map<S16BIT,_CycleIndex> & m_SubAddrCyle);

	// �õ�״̬�ӵ�����
	// ���ظ�ֵ˵��û������
	long getCyleState();

	// �õ���Ϣ�仯���ӵ�ַ
	void GetChgMsgData(__out map<RT_MSG, vector<S16BIT> > & m_chgMsg_subAddr);

	// �õ�״̬����Ϣ�仯���ӵ�ַ
	void GetChgMsgState(__out set<RT_MSG> & m_chgMsg_subState);

	// �õ���Ϣ��������Դ
	void GetResourceCopy(__out map<RT_MSG,vector<S16BIT> > & m_ResourceCopy_subAddr);

	// �õ�״̬��
	void GetStateWord(__out RT_StateWord & stateWord);

private:

	// �õ�ʱ������Ϣ
	void GetTimeCode(string strTimeCode,__out map<S16BIT,_TimeCode> & m_TimeCode);

	// �õ����õ���ȡ��
	void GetThreeChg(string strThreeChg,__out map<S16BIT,_three_struct> & mapThreeChg);

	// ȡ����ȡ�������У����ĸ���Ϣ�õ�һ�����ݵ�����
	bool GetThreeChg_end(string str_end,map<U16BIT,ThreeResult> & mapVal);

	// ȡ����ȡ�������У���Ҫ�����������
	bool GetThreeChg_or(string str_or,map<U16BIT,int> & mapVal);

// ƴ��
public:

	// ���:
	// Address=9;
	bool GetRTAddress(S16BIT iAddress,string & outStr);

	// ���:
	// string & Sub_Address: Sub_Address = {sa1:[num10]:[che1]:[cheIndex11],SA2,SA3,SA4,SA5};
	// string & Resource_file:  Resource_file = {SA1:fileName:def0000:sim67_30_30_7,SA2:fileName};
	// string & ForceFlush:  ForceFlush={sa1,sa2 };
	// vector<string> & subAddrDefval:  Ĭ��ֵ�������ַ���
	void GetSubAddrInfo(__in const map<S16BIT,_addrData> & m_SubAddr,__in const set<S16BIT> & m_ForceFlush,string & Sub_Address,string & Resource_file,string & ForceFlush,vector<string> & subAddrDefval);

	// ���
	// CHG_cyle_data = {sa1_ms2000_num1, SA1_ms200_num2}
	void GetSubAddrCyle(__in const map<S16BIT,_CycleIndex> & m_SubAddrCyle,string & CHG_cyle_data);

	// ���
	// CHG_cyle_state = ms2000;
	void getCyleState(long cyle,string & CHG_cyle_state);

	// CHG_msg_data = {SA2_MSG0_MODE0:[SA1],sa0_msg3-mode0:[sa4]};
	void GetChgMsgData(__in const map<RT_MSG, vector<S16BIT> > & m_chgMsg_subAddr,string & CHG_msg_data);

	// CHG_msg_state = {SA7-MSG3-MODE16,SA7-MSG4-MODE10};
	void GetChgMsgState(__in const set<RT_MSG> & m_chgMsg_subState,string & CHG_msg_state);

	// Resource_copy = {SA0-MSG5-MODE1:[SA1,SA4],SA0-MSG6-MODE16:[SA2,SA3]}; 
	void GetResourceCopy(__in const map<RT_MSG,vector<S16BIT> > & m_ResourceCopy_subAddr,string & Resource_copy);

	// ServiceRequest = 0; 	//��������λ,��ʼֵ
	// SubSystem_Flag = 0; 	//��ϵͳ����λ,��ʼֵ
	// Busy = 0; 		//æλ,��ʼֵ
	// Terminal_Flag = 0; //�ն�����λ,��ʼֵ
	void GetStateWord(__in RT_StateWord & stateWord,string & ServiceRequest,string & SubSystem_Flag,string & Busy,string & Terminal_Flag);

private:
	// SA1:fileName:def0000:sim67_30_30_7
	string GetConfigStr_f(const _fileConfig & config,S16BIT subAddr);
private:

	// �õ��ӵ�ַ��Ĭ��ֵ
	void SetSubAddrDefValue(_addrData & addrData,const string & strValue);


	// ������Ϣ
	void GetThreeChg_end_s(S16BIT subAddr,string & str_orStr,string & str_config_orName,map<U16BIT,ThreeResult> & mapVal);
	
	// ������Ϣ
	void GetThreeChg_or_s(S16BIT subAddr,string & str_orStr,string & str_config_orName,map<U16BIT,int> & mapVal);

private:
	
	// ��������
	SplitBase config_split;

	// �����ַ���
	string m_StrConfig; 

	RT::BrConfig m_config;
private:

	// �ӵ�ַ
	string strSubAddressArr;

	// ״̬����
	string strCycle ;

	// �仯����
	string chgCyle_dArr ;

	// ��Ϣ�仯
	string chgMsg_dArr;
	string chgMsg_sArr;

	//�ļ�����Դ
	string Resource_file;
	string Resource_copy;

	// ʱ����
	string strTimeCode;
	
	// ״̬��
	string strTemp_sr;
	string strTemp_sf;
	string strTemp_b;
	string strTemp_tf;

	// ��Ҫȡ�ߺ�ˢ�µ�
	string strForceFlush;

	// ����
public:

	// ����
	//void SaveConfig(const string & fileFullName,const map<S16BIT,_SubAddrConfig> & config);
};

