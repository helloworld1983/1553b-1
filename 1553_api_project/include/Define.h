#pragma once

#include "zynq_1553b_api.h"

#ifdef WIN32_DDC
#include "../../1553B_BC/Include/stdemace.h"
#pragma warning( disable : 4819 )
#elif defined(IPCORE)
//#include "../../../../FrameWork/vxWorks/1553B_IPCore_driver/IPCORE.h"
#include "IPCORE.h"
#else
//#include "../../1553B_BC/Include_vx/stdemace.h"
#endif
#include "types.h"


#ifndef  WIN32
#define __out
#define __in
#endif
#include <string>
#include <vector>
#include <map>
#include <list>
using namespace std;

// ������Ϣ����󻺴�
const int MAX_LEN_COPYMSG  = 10 * 1024;

// �ӵ�ַ�������ڱ仯�ṹ                                                                     
typedef struct _CycleIndex
{
	long cycle;
	int cyleIndex;
	S16BIT updataLen; //�����µĳ���
	_CycleIndex()
	{
		cyleIndex = 0;
		cycle=0;
		updataLen=0;
	}
}_CycleIndex;

// ʱ��������
typedef enum enum_TimeCode{
	TIME_CODE_error= 0,
	TIME_CODE_send = 1,
	TIME_CODE_check = 2,
	TIME_CODE_And = 3 ,  // �����ַ�
}enum_TimeCode;

// ʱ����ṹ
typedef struct _TimeCode
{
	enum_TimeCode  codeType; // ʱ��������

	string rout_addr;		 // ·��Դ��ַ
	string rout_subAddr;     // ·��Դ�ӵ�ַ
	string rout_des_addr;	 // ·��Ŀ�ĵ�ַ
	string rout_des_subAddr; // ·��Ŀ���ӵ�ַ

	_TimeCode()
	{
		codeType = TIME_CODE_error;
	}
}_TimeCode;

// У�鷽ʽ
typedef enum {
	_ERROR = 0,
	CRC =1,	 // CRCУ��
	ADD = 2, // �ۼӺ�
} enum_Check;		// У�鷽ʽ

// У��ṹ
typedef struct _CheckOutStu
{
	enum_Check type;		// У��ķ�ʽ
	S16BIT wordIndex;   // У��͵�λ��,�ڰ��е���λ��
}_CheckOutStu;

// ��Ϣ������������ʽ
typedef enum _CopyMsg_type
{
	_copyMsg_normal ,  // ��ͨ��Ϣ����
	_copyMsg_three,	   // ����������ȡ�������
	_copyMsg_More      // ����������
}_CopyMsg_type;

// ��Ϣ����Դ�е���ȡ���㷨

// ��ȡ���ȶԽ��
// �����32����
class ThreeResult
{
public:
	ThreeResult(){
		memset((void*)dataArr,0,sizeof(U16BIT)*32);
	};
	// ���ַ�������
	ThreeResult(const string & str);

	~ThreeResult(){
	};
	ThreeResult(const ThreeResult & other){
		memcpy((void *)dataArr,(void*)other.dataArr,sizeof(U16BIT)*32);
	};
	ThreeResult & operator = (const ThreeResult & other){
		memcpy((void *)dataArr,(void*)other.dataArr,sizeof(U16BIT)*32);
		return (*this);
	};

	// �����ַ���
	string getLineStr();

public:
	U16BIT dataArr[32];
};
// ��ȡ��������
typedef enum _three_Type
{
	_three_error = 0,
	_three_or = 1,
	_three_INS = 2,
	_three_Array = 3,
}_three_Type;

// ��λ��
typedef struct _three_Type_or 
{
	U16BIT def;
	map<U16BIT,int> mapVal;
}_three_Type_or ;

// �ȶ�
typedef struct _three_Type_Array 
{
	bool che;
	map<U16BIT,ThreeResult> listVal; // һ��ֵ��Ӧ��һ��ֵ
}_three_Type_Array;


/**********************************************************************
�ļ�����Դ�е�С��

һ������Դ�ļ�������Ҫ�ֳ�����С����ÿ��ȡһ��С�����ͣ����ĳ��ȿ��ܴ���32

С������Ҫָ��ÿ�η��͵ĳ��ȡ�
***********************************************************************/
class SimlBag 
{
public:
	SimlBag(){
		m_index = 0;
		m_SubLen_Count = 0;
		m_offset = 0;
		m_bEnd =false;
	}
private:
	int m_len_2; //С���ĳ���
	vector<S16BIT> m_subLen; // ��ֺ�ÿ�η��͵ĳ���
	size_t m_index; // ��¼����
	// ��ȡ�ߵĳ���
	int m_SubLen_Count;
	// ����Ƿ����
	bool m_bEnd;
	// ƫ��
	int m_offset;
public:
	S32BIT GetLen() const {
		return m_len_2;
	}
	void SetLen(S32BIT len)	{
		m_len_2 = len;
	}

	// ����һ���ӳ���
	void SetSubLen(S16BIT len) {
		m_subLen.push_back(len);
	}

	// ����
	void clear(){
		m_subLen.clear();
	}

	// ���ַ�����ʼ��
	bool  SplitBag(string str);

	// ���һ�������Ƿ�Ϸ�
	static bool CheckError(string & errorInfo,int bagLen);

	// �õ�һ���ӳ��� 
	S16BIT GetSubLen(){
		if(m_index >= m_subLen.size())
		{
			m_index = 0;
			m_SubLen_Count = 0;
			m_offset =0;
			m_bEnd =false;
		}

		S16BIT subLen =m_subLen.at(m_index);
		m_index ++;
		//m_SubLen_Count += subLen;
		m_offset = m_SubLen_Count-subLen;
		m_bEnd =  (m_index >= m_subLen.size() )?true:false;

		return subLen;
	}

	bool IsEnd() const{
		return m_bEnd;
	}

	// �õ�ƫ��
	S16BIT GetOffsetLen() const{
		return m_offset;
	}
public:
	void GetSublen(vector<S16BIT> & vecSublen) const{
		vecSublen = m_subLen;
	}
	void SetSubLen(const vector<S16BIT> & subLenVec){
		m_subLen = subLenVec;
	}
};


// 
// ����һ���յ�����Ϣ�Ľṹ,�������Ķ���
// ������Ϊmap��keyֵ��������ҡ�                                                                      
class RT_MSG
{
public:
	RT_MSG(){	
		addr = 0;
		subAddr =0;
		readFlag =0;
		modeCode =1;
		return ;
	};
	~RT_MSG()	{	
		return ;
	};
	RT_MSG(const RT_MSG & other){
		addr = other.addr;
		subAddr = other.subAddr;
		readFlag = other.readFlag;
		modeCode = other.modeCode;
	};
	RT_MSG operator =(const RT_MSG & other){
		addr = other.addr;
		subAddr = other.subAddr;
		readFlag = other.readFlag;
		modeCode = other.modeCode;
		return *this;
	};
public:
	RT_MSG(const MSGSTRUCT & msg,const S16BIT & subAddr_p,const S16BIT & readFlag,const S16BIT & msg_modecode);

	// ����: MSG5-SA0-MODE1	// ����: �ӵ�ַ_����_��ʽ��
	RT_MSG(const string & strLine);

	// ����
	// subAddr:
	// msg_type:
	// msg_modecode:
	RT_MSG(const S16BIT & subAddr,const S16BIT & readFlag,const S16BIT & msg_modecode);

	// ����: MSG5-SA0-MODE1	// ����: �ӵ�ַ_����_��ʽ��
	string getLine() const;

	// �õ��������µ���Ϣ����
	// RT6-SA15-13 -> BC
	string GetDesLine();

	//�Ƚ�
	bool CompareMsgAndCopy(const S16BIT & subAddr_p,const S16BIT & msg_type,const S16BIT & msg_modecode);

	// �Ƚϲ�����������
	bool operator == (const RT_MSG & other);

	bool operator < (const RT_MSG & other) const;
public:
	S16BIT addr; // ��ַ��0��31�ǹ㲥
	// �ӵ�ַ
	S16BIT subAddr;
	// ��дλ
	S16BIT readFlag; // 0Ϊ����1Ϊд
	// �����ֳ���/��ʽ������
	S16BIT modeCode;
};
