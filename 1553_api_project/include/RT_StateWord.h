#pragma once
#include "Define.h"

// RT��״̬�ֹ���
class RT_StateWord
{
public:
	RT_StateWord(void);
	~RT_StateWord(void);

public:

	// ��������λ
	int ServiceRequest;

	// ��ϵͳ����λ
	int SubSystem_Flag ;

	// æλ
	int Busy_flag;

	// �ն�����λ
	int Terminal_Flag;
private:

	// RT�ĵ�ַ
	S16BIT  m_RTAddr;

public:

	// ����RT��ַ
	int SetRtAddr(S16BIT rtAddr);

	// �õ�״̬��
	int GetStaus(S16BIT & wStatus);

	// ���÷�������λ
	void ResetSerRequest();
};
