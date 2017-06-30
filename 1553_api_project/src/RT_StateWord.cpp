#include "RT_StateWord.h"

RT_StateWord::RT_StateWord(void)
{
	ServiceRequest = 0;
	SubSystem_Flag = 0;
	Busy_flag = 0;
	Terminal_Flag = 0;
	m_RTAddr = 0;
}

RT_StateWord::~RT_StateWord(void)
{

}


// ����RT��ַ
int RT_StateWord::SetRtAddr(S16BIT rtAddr)
{
	m_RTAddr = rtAddr;
	return 1;
}

// ���÷�������λ
void RT_StateWord::ResetSerRequest()
{
	if (0 == ServiceRequest )
	{
		ServiceRequest = 1;
	}
	else
	{
		ServiceRequest = 0;
	}

	return ;
}

// �õ�״̬��
int RT_StateWord::GetStaus(S16BIT & wStatus)
{
	wStatus = 0;
	
	// ǰ��λ��RT��ַ
	wStatus = m_RTAddr;

	// д�����λ
	if (1 == ServiceRequest)
	{
		wStatus = ACE_RT_STSBIT_SREQ;
		ServiceRequest = 0;
	}
	if (1 == SubSystem_Flag)
	{
		wStatus = wStatus | ACE_RT_STSBIT_SSFLAG;
		SubSystem_Flag = 0;
	}
	if (1 == Busy_flag)
	{
		wStatus = wStatus | ACE_RT_STSBIT_BUSY;
		Busy_flag = 0;
	}
	if (1 == Terminal_Flag)
	{
		wStatus = wStatus | ACE_RT_STSBIT_RTFLAG;
		Terminal_Flag = 0;
	}

	return 1;
}

