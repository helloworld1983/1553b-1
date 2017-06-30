#pragma once

#include <time.h>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <queue>
#include <set>
using namespace std;

// 
// �����ļ���
// �ṩ�����ļ��Ķ�ȡ�������
class SimFile
{
public:
	SimFile();
	~SimFile();

private:
	int fileOffset; 

	// �ļ����泤��
	int m_fileLen;

	// �ļ�����
	char * m_pFileBuff;

	//�����ļ�
	FILE * m_pFile;

	void clear();

public:

	// �ļ��Ƿ�򿪳ɹ�
	bool IsOpen(){
		return m_pFileBuff!= NULL?true:false;
	};
	// ���ļ�
	// �����Ƿ�򿪳ɹ�
	bool open(const string & fileNameStr);

	// ���ļ�����
	// pBuff: Ŀ�ĵ�ַ
	// readLen: ����ĳ���
	int fread_buff(char * pBuff,int readLen);

	// �������¶�λ
	void fseek_buff(int offset);

	// �����ļ�����,������ļ�һ�ζ����ڴ�
	void LoadFileData();
};




