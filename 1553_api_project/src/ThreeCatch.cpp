#include "ThreeCatch.h"
#include "common_qiu.h"
#include <bitset>
using namespace std;

ThreeCatch::ThreeCatch(void)
{
	m_BuffLen = 32;
	memset((void*)m_threeBuff,0,32 * sizeof(U16BIT));
}

ThreeCatch::~ThreeCatch(void)
{

}


S16BIT ThreeCatch::CopyData(U16BIT pData[])
{
	memcpy((void *)pData,(void *)m_threeBuff,sizeof(U16BIT)*m_BuffLen);
	return m_BuffLen;
}

// ����ֵ
void ThreeCatch::SetDefVal(U16BIT * pData,S16BIT dataLen)
{
	memcpy((void *)m_threeBuff,(void *)pData,sizeof(U16BIT) * dataLen);
}

/*
��A��B��Cʱ���������ݹ������£� 
������ȡ������ָ��ֳ�������Ϣ�涨�ֳ������ݲ��ַ���Ĭ��ֵ���������������
������ȡ����λ�޸ķ���ֵ������Ĭ��ֵ�������ϴν�����ȷ��ȡ��ָ��ʱ���صĽ����
������ȡ������ƥ�����飬�ֳ�������Ϣ�涨�ֳ�������ƥ����������ΪĬ��ֵ���������������
*/
// ������ȡ��������Դ
bool ThreeCatch::SetThreeSource(U16BIT * pData,S16BIT dataLen)
{
	if (dataLen >32 )
	{
		return false;
	}
	switch (m_threeStu.eType)
	{
	case _three_error:
		return false;
	case _three_INS:
		// һ����Ϣ���ж��ָ��
		{
			if (m_threeStu.num*3 >32)
			{
				return false;
			}
			U16BIT a,b,c,tNum = m_threeStu.num;
			for (int i=0;i<tNum;i++)
			{
				a = *(pData +i);
				b = *(pData + i + tNum);
				c = *(pData + i + tNum*2);
				m_threeBuff[i] = ThreeGetTwo(a,b,c);
			}
		
			m_BuffLen = tNum;
		}
		break;
	case _three_or:
		{
			// ��λ������,��ȡ���Ľ�������õ�ֵ�Ƚ�,�õ�λ��,�����λ��������
			U16BIT a = (*pData);
			U16BIT b = *(pData +1);
			U16BIT c = *(pData +2);

			if(!IsNoData(a,b,c))//a!=b!=c
			{
				U16BIT ret = ThreeGetTwo(a,b,c);
				
				SetBitValue(m_threeStu.orStu.mapVal,ret);
			}
			m_BuffLen = 32;
		}
		break;
	case _three_Array:
		{
			// ��ȡ����������
			m_threeStu.lastData[0] = pData[0];
			m_threeStu.lastData[1] = pData[1];
			m_threeStu.lastData[2] = pData[2];
			if(IsNoData(pData[0],pData[1],pData[2]))//a!=b!=c
			{
				return false;
			}	
			
			U16BIT ret = ThreeGetTwo(m_threeStu.lastData[0],m_threeStu.lastData[1],m_threeStu.lastData[2]);

			// �Ƚϵõ���һ��ֵ
			if (m_threeStu.retArry.che)
			{
				// �Ƚϵõ���һ��ֵ
				return GetArrayValue(ret ,m_threeBuff,m_BuffLen,m_threeStu.retArry.listVal);
			}
			else
			{
				m_threeBuff[0]= ret;
			}

			m_BuffLen = 32;
		}
		break;
	}
	return true;
}

// ��ȡ���Ľ����������ȶԵõ�����
// û�бȶ�ֵ������false
bool ThreeCatch::GetArrayValue(U16BIT par,U16BIT threeBuff[],int len,const map<U16BIT,ThreeResult> & mapVal)
{
	map<U16BIT,ThreeResult>::const_iterator itList = mapVal.find(par);
	if(itList != mapVal.end())
	{
		memcpy((void*)threeBuff,(void *)(itList->second.dataArr),sizeof(U16BIT)*32);
		return true;
	}
	else
	{
		return false;
	}
}

// ��ȡ���Ľ����������������õ�һ�����
void ThreeCatch::SetBitValue(const map<U16BIT,int> & mapVal,U16BIT resultT)
{
	map<U16BIT,int>::const_iterator itVal = mapVal.find(resultT);

	if( itVal != mapVal.end())
	{
		// ������õ����
		int iVal = itVal->second;
		// ����32����������һλ
		func_bit(iVal,1);
	}
}

void ThreeCatch::func_bit(long l_bit,bool val)
{
	if (l_bit <= 0)
	{
		return ;
	}
	if (l_bit > 512)
	{
		return;
	}

	long idx = (l_bit + 15) / 16 - 1;
	long pos = (l_bit + 15) % 16;
	bitset<16> b16(m_threeBuff[idx]);
	b16.set(pos,val);

	ULONG new_val = b16.to_ulong();
	m_threeBuff[idx] = (WORD)new_val;
}

// �ж��Ƿ�a!=b!=c
bool ThreeCatch::IsNoData(U16BIT a,U16BIT b,U16BIT c)
{
	if (a != b && a != c && b != c)
	{
		return true;
	}
	return false;
}

// ��ȡ���㷨
U16BIT ThreeCatch::ThreeGetTwo(U16BIT a,U16BIT b,U16BIT c)
{
	if (a == b && a == c && b == c)
	{
		return a;
	}
	else
	{
		if (a == b && b != c)
		{
			return a;
		}
		if (b == c && a != c)
		{
			return b;
		}
		if (a == c && b != c)
		{
			return a;
		}

		return 0;
	}
}




