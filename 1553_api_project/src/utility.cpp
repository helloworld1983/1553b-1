  
#include <sys/time.h>
#include "utility.h"



// �õ�ʱ��
void GetTime_rt(__out _time_rt & rtTime)
{
#ifdef WIN32
	GetLocalTime(&(rtTime.time_s));
#endif

#ifdef __vxworks
	gettimeofday(&(rtTime.time_s),NULL);
#endif

//zhanghao adds.
#ifdef __linux__
	gettimeofday(&(rtTime.time_s),NULL);
#endif
}

/**
 * ʱ��ת��.
 * ΢��FILETIMEʱ���UNIX timevalʱ��֮���ת��
 */
class TimeValue : public timeval {
	static const int64_t TM_MS_OFFSET = 11644473600ll; //< ΢��FILETIMEʱ���time_tʱ�䡰�롱֮��Ĳ�ֵ
	static const long TM_MS_USEC_UNIT = 10; //< ��΢�롱��΢��ʱ���еĵ�λ��
	static const long TM_MS_SEC_UNIT = 10000000; //< ���롱��΢��ʱ���еĵ�λ��
public:
	/// 1���롱��΢��ʱ���б�ʾʱ����ֵ
	static int64_t OneSecond() {
		return TM_MS_SEC_UNIT;
	}
public:
	/// ��timeval����
	TimeValue(const timeval &src) : timeval(src) {}
	/// ��΢��ʱ��ֵ����
	TimeValue(int64_t ms_file_time) {
		tv_sec = long(ms_file_time / TM_MS_SEC_UNIT - TM_MS_OFFSET);
		tv_usec = (ms_file_time % TM_MS_SEC_UNIT) / TM_MS_USEC_UNIT;
	}
	/// ת��Ϊ΢��ʱ���ʽ������ֵ
	operator int64_t() {
		return (tv_sec + TM_MS_OFFSET) * TM_MS_SEC_UNIT + tv_usec * TM_MS_USEC_UNIT;
	}
};

#ifdef __vxworks
inline int usleep(unsigned long usec) {
	struct timespec nano;
	nano.tv_sec = usec / 1000000;
	nano.tv_nsec = (usec % 1000000) * 1000;
	return nanosleep(&nano, NULL);
}

inline int gettimeofday(struct timeval *tp, void *tzp) {
	if ( tp == NULL ) {
		errno = EINVAL;
		return -1;
	}
	struct timespec ts;
	int r = clock_gettime(CLOCK_REALTIME, &ts);
	if ( r != -1 ) {
		tp->tv_sec = ts.tv_sec;
		tp->tv_usec = ts.tv_nsec / 1000;
	}
	return r;
}

inline int settimeofday(const struct timeval *tp, void *tzp) {
	if ( tp == NULL ) {
		errno = EINVAL;
		return -1;
	}
	struct timespec ts;
	ts.tv_sec = tp->tv_sec;
	ts.tv_nsec = tp->tv_usec * 1000;
	return clock_settime(CLOCK_REALTIME, &ts);
}
#endif /* __vxworks */

// �Ƚ�����ʱ�䣬�õ�����
// paramTime ԭʱ��
// newTime �µ�ʱ��
int CompareTime_rt(__in const _time_rt & paramTime,__in const _time_rt & newTime)
{
#ifdef WIN32
	int nDay = (newTime.time_s.wMonth - paramTime.time_s.wMonth) * 30 + newTime.time_s.wDay - paramTime.time_s.wDay;
	int nHour = nDay * 24 + newTime.time_s.wHour - paramTime.time_s.wHour;
	int nMinu = nHour * 60 + newTime.time_s.wMinute - paramTime.time_s.wMinute;
	int nSecond = nMinu * 60 + newTime.time_s.wSecond - paramTime.time_s.wSecond;
	int nSec = nSecond * 1000 + newTime.time_s.wMilliseconds - paramTime.time_s.wMilliseconds;	

	return nSec;
#endif

#ifdef __vxworks
	int nSec = 0;
	struct timeval now;
	gettimeofday(&now, NULL);
	nSec = now.tv_usec/1000; //�õ�����ֵ
	return nSec;
#endif

//zhanghao adds.
#ifdef __linux__
	long nSec = 0;
	nSec =  (newTime.time_s.tv_sec - paramTime.time_s.tv_sec)*1000
            + (newTime.time_s.tv_usec - paramTime.time_s.tv_usec)/1000; //�õ�����ֵ
	return nSec;
#endif
}

void _strupr_s_My(char * pData,int Len)
{
	int i =0 ;
	while(i<Len)
	{
		char cTemp = *(pData+i);
		if ('a' <= cTemp && cTemp <= 'z')
		{
			*(pData+i) = (cTemp-32);
		}

		i++;
	}
}

// �ַ���ת��,���ַ�������ĸ���������תΪlong���ء�
int atol_my(const char * _dst)
{
	if (NULL == _dst)
	{
		return 0;
	}

	int offset =0 ;

	while ( *(_dst + offset) != 0)
	{
		if (*(_dst+offset) >= '0' && *(_dst+offset) <= '9')
		{
			const char * _dst_num = _dst+offset;
			int num = (int)atol(_dst_num);
			return num;
		}
		offset++;
	}
	return 0;
}

// �ַ���ת��,���ַ�������ĸ���������תΪlong���ء�
unsigned atol_my_Hex(const char * _dst)
{
	if (NULL == _dst)
	{
		return 0;
	}

	int offset =0 ;

	if (1 == strcmp(_dst,"0x"))
	{
		offset = 2;
	}
	while ( *(_dst + offset) != 0)
	{
		char val = *(_dst+offset);
		if (  (val>= 'a' && val<= 'f') || (val >= 'A' && val <='F') || (val>= '0' && val <= '9') )
		{
			const char * _dst_num = _dst+offset;
			char * end_char;
			unsigned long num = strtoul(_dst_num,&end_char,16);
			return num;
		}
		offset++;
	}
	return 0;
}

// ����:1,2,3,4
void SubData(string str,vector<string> & vecStr,char cc)
{
	int pos = 0;
	int iFind = str.find(cc);
	while(iFind > 0)
	{
		string sub = str.substr(pos,iFind - pos);

		vecStr.push_back(sub);

		pos = iFind + 1;

		iFind = str.find(cc,pos);
	}

	if ((int)str.length() > pos )
	{
		string sub = str.substr(pos,str.length() - pos);

		vecStr.push_back(sub);
	}
}

namespace RT{ 

	BrConfig::BrConfig()
	{

	}

	void BrConfig::SetConfig(const string & cfg)
	{
		size_type pos1 = std::string::npos;
		size_type pos2 = std::string::npos;
		std::string cfg_key, cfg_value;

		string cfg_copy = cfg;

		// ȥ���ո�ͻ��� 
		while ((pos1 = cfg_copy.find(' ')) != std::string::npos || (pos1
			= cfg_copy.find('\n')) != std::string::npos || (pos1
			= cfg_copy.find('\r')) != std::string::npos || ( pos1 = cfg_copy.find('\t') ) != std::string::npos ) 
		{
			cfg_copy.erase(pos1, 1);
		}

		size_type start_pos = 0;
		while ((
			(pos1 = cfg_copy.find('=', start_pos)) != std::string::npos
			) && (
			(pos2= cfg_copy.find(';', pos1)) != std::string::npos
			)) 
		{
			// ���Ⱥ�ǰ����ַ�������cfg_key���Ⱥźͷֺ�֮����ַ�������cfg_value
			string strKey = cfg_copy.substr(start_pos, pos1-start_pos);
			string strValue =cfg_copy.substr(pos1+1, pos2-pos1-1); 

			// ��Сдת��
			int len = strKey.length() +1;
			auto_ptr<char> pChar((char *)malloc(len));
			memset(pChar.get(),0,len);
			memcpy(pChar.get(),strKey.c_str(),len);
			_strupr_s_My((char *)pChar.get(),len);

			strKey = string((char *)pChar.get());
			m_item_value[strKey] = strValue;

			start_pos = pos2+1;
		}
	}

	BrConfig::BrConfig(const string &cfg) 
	{
		SetConfig(cfg);
	}
}



