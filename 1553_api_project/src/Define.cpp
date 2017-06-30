#include "Define.h"
#include "utility.h"
#include "common_qiu.h"

bool RT_MSG::operator == (const RT_MSG & other)
{
	if (other.subAddr != this->subAddr)
		return false;
	if (other.readFlag != this->readFlag)
		return false;

	if (other.modeCode != this->modeCode)
		return false;
	return true;
}


#define MAKE_I8(a, b) ((int64_t)(a)) | (((int64_t)(b)) << 32)
// #define LO_I8(i8)           long((__int64)(i8) & 0xffffffff)
// #define HI_I8(i8)           long(((__int64)(i8) >> 32) & 0xffffffff)

#ifndef WIN32
#define MAKELONG(a, b)      ((LONG)(((unsigned short)(((ULONG)(a)) & 0xffff)) | ((ULONG)((unsigned short)(((ULONG)(b)) & 0xffff))) << 16))
#define MAKEWPARAM(l, h)      ((ULONG)(ULONG)MAKELONG(l, h))
#endif


// �õ��������µ���Ϣ����
// RT6-SA15-13 -> BC
string RT_MSG::GetDesLine()
{
	char buff[100];
	memset(buff,0,100);
	// �����֣��ӵ�ַ����д��ǣ��ּ���/��ʽ��
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif
	sprintf(buff,"������: �ӵ�ַ%d-��дλ%d-�ֳ�/��ʽ��%d",subAddr,readFlag,modeCode);

#ifdef WIN32
#pragma warning( pop )
#endif
	return buff;
}


// ���رȽ������������map��keyֵ
bool RT_MSG::operator < (const RT_MSG & other) const
{
	ULONG ul1 = MAKEWPARAM(subAddr,readFlag);

	int64_t i8_t;

	// ���Ƚ��ֳ�����ʽ��ʱ�ȽϷ�ʽ��
	if (0 == subAddr || 31 == subAddr)
		i8_t = MAKE_I8(ul1,(LONG)modeCode);
	else
		i8_t = MAKE_I8(ul1,(LONG)modeCode);

	ULONG ul_o = MAKEWPARAM(other.subAddr,other.readFlag);
	int64_t i8_t_o ;
	if (0 ==other.subAddr || 31 == other.subAddr)
		i8_t_o = MAKE_I8(ul_o,(LONG)other.modeCode);
	else
		i8_t_o = MAKE_I8(ul_o,(LONG)other.modeCode);


	return i8_t < i8_t_o;
}



RT_MSG::RT_MSG(const S16BIT & subAddr_p,const S16BIT & readF,const S16BIT & msg_modecode)
{
	subAddr = subAddr_p;
	readFlag = readF;

	//  0��31�Ƿ�ʽ����
	if (subAddr_p ==0 || subAddr_p == 31)
	{
		modeCode = msg_modecode;
	}
	else
	{
		modeCode = msg_modecode;
	}
}

RT_MSG::RT_MSG(const MSGSTRUCT & msg,const S16BIT & subAddr_p,const S16BIT & readF,const S16BIT & msg_modecode)
{
	subAddr = subAddr_p;
	readFlag = readF;

	//  0��31�Ƿ�ʽ����
	if (0 == subAddr_p || 31 == subAddr_p)
	{
		modeCode = msg_modecode;
	}
	else
	{
		modeCode = msg.wWordCount;
	}
}

// MSG5-SA0-MODE1
string RT_MSG::getLine() const
{
	char buff[100];
	memset(buff,0,100);
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif
	sprintf(buff,"readFlag%d_sa%d_mode%d",(int)readFlag,(int)subAddr,(int)modeCode);
#ifdef WIN32
#pragma warning( pop )
#endif
	return string(buff,strlen(buff));
}

RT_MSG::RT_MSG(const string & strLine)
{
	if (strLine.length() <5)
	{
		throw std::logic_error("��������");
	}

	int pos =0;
	int iFind = strLine.find('_');
	if (iFind <= 0)
	{
		throw std::logic_error("����");
	}

	string str = strLine.substr(0,iFind); 
	readFlag = (S16BIT)atol_my(str.c_str()); //��д���
	pos =iFind+1;
	iFind = strLine.find('_',pos);
	if (iFind <= 0)
	{
		throw std::logic_error("����");
	}

	str = strLine.substr(pos,iFind-pos);
	subAddr = (S16BIT)atol_my(str.c_str());//�ӵ�ַ

	if ((int)strLine.length() <=  iFind)
	{
		throw std::logic_error("����");
	}
	str = strLine.substr(iFind+1,strLine.length() - iFind+1);

	readFlag = (readFlag==0 ? 0 : 1);

	//  0��31�Ƿ�ʽ����
	modeCode = atol_my(str.c_str());//��ʽ��
}

// ��ͬ����true
bool RT_MSG::CompareMsgAndCopy(const S16BIT & subAddr_p,const S16BIT & msg_type,const S16BIT & msg_modecode)
{
	if (msg_type != readFlag)
	{
		return false;
	}	
	if(subAddr != subAddr_p)
	{
		return false;
	}

	//  0��31�Ƿ�ʽ����
	if (0 == subAddr_p || 31 == subAddr_p) 
	{
		if ( modeCode !=msg_modecode )
		{
			return false;
		}
	}
	return true;
}


// ���ַ�����ʼ��
bool  SimlBag::CheckError(string & errorInfo,int bagLen)
{
	if (bagLen < 32)
	{
		errorInfo = "С�����Ȳ���С��32";
		return false;
	}
	return true;
}

// ���ַ�����ʼ��
bool  SimlBag::SplitBag(string str)
{
	m_subLen.clear();

	// ��ʼ
	S32BIT numLen = 0;
	S16BIT numLenCount = 0;
	bool bSucess; 

	vector<string> vecSim ;
	SubData(str,vecSim,'_');

	for (size_type iSim =0;iSim<vecSim.size();iSim++)
	{
		string strNode = vecSim.at(iSim);
		int num = atol_my(strNode.c_str());
		if (0 == iSim)
		{
			// С���ĳ��ȡ�
			if (num <= 0 ) 
			{
				printf("\t������С������С��0:%d\n",num);
				bSucess = false;
				break; // ��Чֵ
			}
			if (num <= 32 ) 
			{
				printf("\t������С������С��32:%d\n",num);
				bSucess = false;
				break; // ��Чֵ
			}
			bSucess = true;
			numLen = num;
			SetLen(num);
		}
		else
		{
			// С���Ľ�ȡ����
			if (numLenCount > numLen)
			{
				bSucess = false;
				break;
			}
			else
			{
				numLenCount += num;
				SetSubLen(num);
			}
		}
	}
	return bSucess;
}


// ���ַ�������
ThreeResult::ThreeResult(const string & str)
{
	memset((void*)dataArr,0,sizeof(U16BIT)*32);
	if (str.length() <= 0)
	{
		return;
	}

	vector<string> vecStr;
	SubData(str,vecStr,':');
	int iIndex = 0;
	for (vector<string>::iterator itVec=vecStr.begin(); itVec != vecStr.end() && iIndex<32 ;itVec++)
	{
		dataArr[iIndex]= atol_my_Hex(itVec->c_str());
		iIndex++;
	}

	return ;
}

/**
 * �����ַ���
 * 
 */
string ThreeResult::getLineStr()
{
	string strLine;
	strLine.clear();
	for (int i=0;i<32;i++)
	{
		if (strLine.length() >0)
		{
			strLine += ":";
		}

		char buff[100];
		memset(buff,0,100);
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif
		sprintf(buff,"0x%x",dataArr[i]);
#ifdef WIN32
#pragma warning( pop )
#endif
		strLine += buff;
	}
	return strLine;
}
