#ifndef UTILITY_H
#define UTILITY_H
//#include <strings.h>
#include "common_qiu.h"
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <set>
using std::auto_ptr;
using std::string;
using std::map;
using std::vector;
using std::set;

typedef std::string::size_type size_type;

#include "types.h"

#ifdef __vxworks
// forward reference
#include <sys/times.h>
int gettimeofday(struct timeval *tp, void *tzp);
int settimeofday(const struct timeval *tp, void *tzp);
#endif /* __vxworks */

#ifndef  WIN32
#define __out
#define __in
#endif

#ifdef WIN32
#include <Windows.h>
#endif

// ��ʱ��ʱ������
typedef struct _time_rt
{
#ifdef WIN32
	SYSTEMTIME time_s;
#endif

#ifdef __vxworks
	struct timeval time_s;
#endif

//zhanghao add.
#ifdef __linux__
    struct timeval time_s;
#endif
}_time_rt;

// �õ�ʱ��
void GetTime_rt(__out _time_rt & rtTime);

// �Ƚ�����ʱ�䣬�õ�����
// paramTime ԭʱ��
// newTime �µ�ʱ��
int CompareTime_rt(__in const _time_rt & paramTime,__in const _time_rt & newTime);

// �ַ���ת��,���ַ�������ĸ���������תΪlong���ء�
int atol_my(const char * _dst);
unsigned atol_my_Hex(const char * _dst);

// ��Сдת��
void _strupr_s_My(char * pData,int Len);

// ����:2,3,4,5
void SubData(string str,vector<string> & vecStr,char cc);

namespace RT
{
	// ���ڽ���ICD��ÿ���ӿڵ�������Ϣ
	class BrConfig
	{
	private:
		// �������ƺ�����ֵ
		map<string, string> m_item_value;
	public:
		BrConfig();

		BrConfig(const string &cfg);

		void SetConfig(const string & cfg);

		void clear()
		{
			m_item_value.clear();
		}
	public:

		bool get_item(const string &item, long &value)
		{
			// ��Сдת��
			int len = item.length() +1;
			auto_ptr<char> pChar((char *)malloc(len));
			memset(pChar.get(),0,len);
			memcpy(pChar.get(),item.c_str(),len);
			_strupr_s_My((char *)pChar.get(),len);

			map<string, string>::iterator it = m_item_value.find(pChar.get());
			if (it != m_item_value.end())
			{
				value = atoi(it->second.c_str());
				return true;
			}
			return  false;
		}
		bool get_item(const string &item, unsigned long &value)
		{
			// ��Сдת��
			int len = item.length() +1;
			auto_ptr<char> pChar((char *)malloc(len));
			memset(pChar.get(),0,len);
			memcpy(pChar.get(),item.c_str(),len);
			_strupr_s_My((char *)pChar.get(),len);

			map<string, string>::iterator it = m_item_value.find(pChar.get());
			if (it != m_item_value.end())
			{
				value = static_cast<unsigned long> (atoi(it->second.c_str()));
				return true;
			}
			return  false;
		}
		bool get_item(const string &item, string &value)
		{
			// ��Сдת��
			int len = item.length() +1;
			auto_ptr<char> pChar((char *)malloc(len));
			memset(pChar.get(),0,len);
			memcpy(pChar.get(),item.c_str(),len);
			_strupr_s_My((char *)pChar.get(),len);

			map<string, string>::iterator it = m_item_value.find(pChar.get());
			if (it != m_item_value.end())
			{
				value = it->second;
				return true;
			}
			return  false;
		}
		bool get_item(const string &item, double &value)
		{
			// ��Сдת��
			int len = item.length() +1;
			auto_ptr<char> pChar((char *)malloc(len));
			memset(pChar.get(),0,len);
			memcpy(pChar.get(),item.c_str(),len);
			_strupr_s_My((char *)pChar.get(),len);

			map<string, string>::iterator it = m_item_value.find(pChar.get());
			if (it != m_item_value.end())
			{
				value = atof(it->second.c_str());
				return true;
			}
			return  false;
		}
		bool get_item(const string &item, bool &value)
		{
			// ��Сдת��
			int len = item.length() +1;
			auto_ptr<char> pChar((char *)malloc(len));
			memset(pChar.get(),0,len);
			memcpy(pChar.get(),item.c_str(),len);
			_strupr_s_My((char *)pChar.get(),len);

			map<string, string>::iterator it = m_item_value.find(pChar.get());
			if (it != m_item_value.end())
			{
				if (strcmp(it->second.c_str(), "true") == 0 || atoi(it->second.c_str()) >= 1)  //luxq
					value = true;
				else if (strcmp(it->second.c_str(), "false") == 0 || atoi(it->second.c_str()) == 0)
					value = false;
				else
					return false;
				return true;
			}
			return  false;
		}
	};

	// �˺�����Ҫ���ڴ���1553B�����ݡ������þ��ǽ��ƶ������е����ݰ��ֽ��еߵ����������ĳ��Ȳ���2�����������򲻴���β��������
	inline void swap_by_word(char *buf, size_t size)
	{
		if (!buf || size <= 0) return;
		size_t swap_size = size /2 * 2;
		for (size_t i = 0 ; i < swap_size - 1 ; i+=2)
		{
			*(buf + i) ^= *(buf + i + 1);
			*(buf + i + 1) ^= *(buf + i);
			*(buf + i) ^= *(buf + i + 1);
		}
	}

}


#endif
