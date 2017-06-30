#pragma once
#include "Define.h"


// �ӵ�ַ��ϢԴ��ȡ���㷨�ṹ
typedef struct _three_struct 
{
	RT_MSG msg;

	_three_Type eType;     // ��ȡ��������
	_three_Type_or orStu;  // ��λ��
	_three_Type_Array retArry; // �ȶ�
	U16BIT num;			// һ����ֵ
	U16BIT lastData[3]; //������ε�����
	int iIndex; // ��ǰ������	
	_three_struct(){
		eType = _three_error;
		iIndex = 0;
		memset((void *)lastData,0,sizeof(U16BIT)*3);
	};
} _three_struct;

/// ��ȡ������Դ��
class ThreeCatch
{
public:
	ThreeCatch(void);
	~ThreeCatch(void);

	// ����ֵ
	void SetDefVal(U16BIT * pData,S16BIT dataLen);

	// ������ȡ��������Դ
	bool SetThreeSource(U16BIT * pData,S16BIT dataLen);

	// ȡ�ñȽϺ������
	S16BIT CopyData(U16BIT pData[]);

	// ��Ϣ����Դ����ȡ���㷨
	_three_struct m_threeStu;

private:

	// ��ȡ���Ļ���ֵ,��ʱ�ж����ȡ���������������
	U16BIT m_threeBuff[32];

	// ��ȡ���л���ĳ���
	int m_BuffLen;

	// ��ȡ���㷨
	U16BIT ThreeGetTwo(U16BIT one,U16BIT one2,U16BIT one3);

	// �ж��Ƿ�a!=b!=c
	bool IsNoData(U16BIT a,U16BIT b,U16BIT c);

	// ��ȡ���Ľ����������ȶԵõ����
	bool GetArrayValue(U16BIT par,U16BIT threeBuff[],int len,const map<U16BIT,ThreeResult> & mapVal);

	// ��ȡ���Ľ����������������õ�һ�����
	// ��ȡ���������������32�����е�����һλ
	// threeBuff: 32���ֵĻ���
	// len: ���泤��
	// mapVal: �Ƚ�ֵ��λ�õ�map
	// result_or: ��ȡ���õ��Ľ��,map��keyֵ
	void SetBitValue(const map<U16BIT,int> & mapVal,U16BIT result_or);


	// ��32������ĳһλ��1
	void func_bit(long l_bit,bool val);
};
