#pragma once

#include "zynq_1553b_api.h"
#include "RT_DataSource.h"

//#include "../../../../../include/gist/icd/icdman.h"
//#include "../../../../../include/gist/icd/BrBasicParser.h"

class RT_ICD
{
public:
	RT_ICD(void);
	~RT_ICD(void);
};

// ʵ�ʵõ�ʱ��ļ�������
class timeInter_obj:public virtual time_Inter
{
public:

	timeInter_obj();
	virtual ~timeInter_obj();
public:
	// ʱ����ת��,����,��time���뵽pData��
	//virtual int ConvertTime_Code(char * pData,int len,const GIST::BrTime & time);  zhanghao close.

	// ʱ����ת��,����,��pData�н�����time
	//virtual	int ConvertTime_Encode(const char * pData,int len,GIST::BrTime & time);  zhanghao close.

private:
	//BrBasicParser* parser;

	//Block* block;

	// ICDMan����
	//const ICD::ICDMan * m_IcdMan;

	// �ӿ�ָ��
	//const Interface* m_interface;  

	U16BIT m_timeData[32];

public:
	// ����ICDMan��ϵͳ����
	//void SetIcdManAndServ(const ICD::ICDMan * icd,Agent::ISysServ * sysServ,const Interface * pInterface);

	// ���ýӿ�ָ��
	//void SetTimeCode(_TimeCode & m_TimeCode);
};

