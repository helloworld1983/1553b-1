#ifndef __COMMUNICATION_H_
#define __COMMUNICATION_H_

/*
 *  Comm.h
 *
 * ������Ŀ
 * ͨ�Žӿ����
 *
 *  Created by anhu.xie on 2009-01-24.
 * Copyright (C) 2004 - 2011 �������ƻ���ռ似�����޹�˾ ��������Ȩ��
 *
 */
#include <vector>
#include <set>
#include <utility>
#include "thread.h"
//#include "pub/datagram.h"
//#include "platform/types.h"
//#include "gist/header.h"
// ����������
#ifdef WIN32

#ifdef _COMM_IMPL_
#define _COMM_API_ __declspec(dllexport)
#else
#define _COMM_API_ __declspec(dllimport)
#endif

#include <WinSock2.h>
#undef errno
#define errno WSAGetLastError()

typedef int socklen_t;
#if _MSC_VER >= 1700
#undef EWOULDBLOCK
#undef ECONNREFUSED
#undef ENETUNREACH
#undef EHOSTUNREACH
#undef ETIMEDOUT
#endif
#undef EINTR
#undef ECONNABORTED
#define SHUT_RDWR SD_BOTH
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ECONNREFUSED WSAECONNREFUSED
#define ENETUNREACH WSAENETUNREACH
#define	EHOSTUNREACH WSAEHOSTUNREACH
#define ETIMEDOUT WSAETIMEDOUT
#define EINTR WSAEINTR
#define ECONNABORTED WSAECONNABORTED

#else /* _MSC_VER */

#include <sys/socket.h>
#include <arpa/inet.h>
#define _COMM_API_
typedef int SOCKET;
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
inline void Sleep(unsigned int ms) { usleep(ms * 1000); }
inline int closesocket(SOCKET s) { return ::close(s); }

#endif /* _MSC_VER */

// �ڲ�ʵ���ã��������ݻ�������
class data_rep;
// ͨ��(�ŵ�)�����ڲ�ʵ�ֵ����ݽṹ��
class comm_impl;

enum NET_MESSAGE {
	ON_CONNECT = 0x1, // ������
	ON_CLOSE = 0x2 ,   // �Ͽ�
};

/**
 * ����ӿ�ʹ�����ݰ��Ļ����������������д����ʹ�õ����ݽṹ��
 * ��Ҫ�����Ǹ������Ƿ�����ڴ�ռ���Զ��ͷţ�
 * �ڴ������(���俽��)�����������ڣ������������������������ڵ����ݣ�
 * �ڶ����������ڽ���ʱ�����Զ�ɾ����������ڴ�ռ䡣
 */
class _COMM_API_ BrBufferMan {
	friend class comm_impl;
	data_rep *rep; /// �����Ļ�����(�������)
public:
	// �������
	BrBufferMan(const BrBufferMan&);
	BrBufferMan& operator=(const BrBufferMan&);
	~BrBufferMan();
	
	explicit BrBufferMan(size_t mem_len);/// ���仺����
	
	char *get_buffer() const; /// ȡ�û�������ַ
	
	size_t get_length() const;/// ����������
};

/*
 * ʵ��ʹ�õ����ݰ��ṹ��
 *
 * ע�������õ��İ�ͷ����(HEADER_TYPE)��һ��ʹ���߶�������ͣ�������GenericHeader�����࣡
 */
template<class HEADER_TYPE>
class BrPackage : public BrBufferMan {
	HEADER_TYPE *header; ///��ͷ
	char *data;          /// ��������
public:
	/** ���Ѷ���������͵İ�ͷ���������ݹ������ݰ������ڴ������е����ݡ�
	 Ĭ������£����ݻᱻ�������·�����ڴ�����
	 �����û������İ�ͷ��������û�б�Ҫ������һ��ģ������ڲ�ͬ�ĵط���
	 ����Է��㴦����ΰ�ͷ�������
	 �˶�����Լ��ͷ��Լ�������ڴ档
	 */
	BrPackage(HEADER_TYPE *hdr, char*dat, bool copy = true) : BrBufferMan(copy ? hdr->pack_len : 0) {
		char *buffer = copy ? get_buffer() : 0;
		if ( buffer ) {
			header = (HEADER_TYPE*)buffer;
			data = buffer + sizeof(HEADER_TYPE);
			memcpy(header, hdr, sizeof(HEADER_TYPE));
			memcpy(data, dat, hdr->pack_len - sizeof(HEADER_TYPE));
		}
		else {
			header = hdr;
			data = dat;
		}
	}

	/** ����������ȡ���ݰ�ʱʹ�á���ʱ������ڴ沢�������յ��ͷš�
	 ���ڻ�ȡ�����϶�ȡ�����ݡ�*/
	BrPackage(BrBufferMan databuf = BrBufferMan(0)) : BrBufferMan(databuf) {
		char *buffer = get_buffer();
		if ( buffer ) { // ��ʱ���Ƿ��ؿ����ݰ���
			header = (HEADER_TYPE*)buffer;
			data = buffer + sizeof(HEADER_TYPE);
		}
		else {
			header = 0;
			data = 0;
		}
	}

	
	// ��ȡ��ͷ�����������ݰ����ȵ���Ϣ����������õ�
	HEADER_TYPE *get_header() const { return header; }
	// ��ȡ���ݱ������ݵĳ��ȿ��Ը��ݰ�ͷ�������
	char *get_data() const { return data; }
	/// ����������
	size_t get_length() const {
		size_t buf_len = BrBufferMan::get_length();
		if ( buf_len || !header )
			return buf_len;
		return header->pack_len;
	}
};

// TCP/IP�����ַ
class _COMM_API_ BrAddress {
	sockaddr_in inet_a;
public:
	BrAddress(unsigned short port, const char *ip = 0); /// ����Listen��˵��IP���Ǳ���ģ�
	BrAddress(sockaddr_in a) : inet_a(a) {}
	sockaddr *operator &() const { return (sockaddr*)&inet_a; }
	sockaddr_in addr() const { return inet_a; }  /// ����SOCKET�����ַ
	size_t name_len() const { return sizeof inet_a; } ///����
};

// ͳ����Ϣ
struct BrIOStat;
// ͨ�Ź��̿��ܷ������쳣
struct  BrSockError : public os_error{ // �ײ�winsock����
	BrSockError(int err) : os_error(err) {}
};
// ͨ�Ų�(winsock֮��)����
struct  BrCommError : BrSockError {
	BrCommError(int e) : BrSockError(e) {
		switch ( reason() )
		{
		case CE_INVALID_ARG:
			msg = "�������󡣱��磬ָ����Channel�������Ƿ��ص�ֵ��";
			break;
		case CE_INVALID_CHN:
			msg = "�ŵ��Ѿ��رգ����߲������Ƿ��ص�ֵ";
		case CE_INCOMP_SYNC_FLAG:
			msg = "ͬ���벻һ�£�һ����ͨ�š��Ѿ�ʹ�ù�һ������ǰָ��ͬ���벻ͬ��ͬ����";
			break;
		case CE_ASYNC_ERROR:
			msg = "��̨��д��������";
			break;
		case CE_INVALID_STAT:
			msg = "����״̬���󣬱�����δ��ʼ��(connect/listen/accept)�Ϳ�ʼ��д����";
			break;
		case CE_CONNECT_CLOSE:
			msg = "�����ѹر�";
		default:
			break;
		}
	}
	enum ErrorCode { // ��SockError��һ���Ĵ������
		CE_INVALID_ARG = 0xE0000000, // �������󡣱��磬ָ����Channel�������Ƿ��ص�ֵ��
		CE_INVALID_CHN, // �ŵ��Ѿ��رգ����߲������Ƿ��ص�ֵ
		CE_INCOMP_SYNC_FLAG, // ͬ���벻һ�£�һ����ͨ�š��Ѿ�ʹ�ù�һ������ǰָ��ͬ���벻ͬ��ͬ����
		CE_CONNECT_CLOSE,	// �����Ѿ��ر�
		CE_ASYNC_ERROR, // ��̨��д��������
		CE_INVALID_STAT // ����״̬���󣬱�����δ��ʼ��(connect/listen/accept)�Ϳ�ʼ��д����
	};
};
/**
 *��ͨ�š�
 * ͨ�ŵĻ�����λ�ǡ��������������ֽ���
 */
class _COMM_API_ BrComm {
protected:
	comm_impl *comm_desc; // ʹ��ָ�������Ϊ�˴ӽӿ�������comm_impl�Ķ��壡

	BrComm(size_t gen_buf_size, size_t chn_buf_size, comm_impl * = 0); // Ҳ��������ʹ�õġ�ͨ��ʵ�֡�����в�ͬ
protected:
	void OnConnection(const BrAddress &peer) {} // �������ʱ�Ļص�!
public:
	typedef SOCKET Channel; // �ŵ���ʶ
	typedef void (* BrNetCallback)(NET_MESSAGE message,Channel ch,void * pobj);
	bool RegisterCallback(BrNetCallback pfun);
	enum {
		ANY_CHANNEL = INVALID_SOCKET,
		ALL_CHANNEL = -10,
	};
	~BrComm();
	/**
	* �������ݣ������������첽�ģ����û��ָ���ŵ��Ͳ�֪�������������﷢��
	* @param header Ҫ���͵����ݰ��İ�ͷ������ͬ����־�Ͱ�����Ϣ
	* @param data Ҫ�������ݰ�������ָ��
	* @param ch ѡ�����ŵ��������Ҫ�����ݰ�����һ�����е��ŵ����ͣ�ʹ��ANY_CHANNEL
	* @return ��ǰ���Ͷ��г���
	* @exception SockError �ײ�(winsock)ͨ���쳣
	*/
	template<class HT> size_t write(HT *header, char*data, Channel channel = ANY_CHANNEL)
	{
		return write(reinterpret_cast<char*>(header), sizeof(HT), data, header->pack_len - sizeof (HT), channel);
	}
	/**
	 * ��������
	 * �ر�Լ����ÿ����ͨ�š�������ж�����ŵ����Ļ�����ͨ���ܽ��յ����а���ͬ���ֶε�ֵ������ͬ�ģ�
	 * ����һ����ͨ�š��Ϻ����ġ���������ʹ�õ�ͬ���ֶα����ǰ��ʹ�õ���ͬ��
	 * @param head Ҫ���͵����ݰ��İ�ͷ�ṹ(�����ͷͬ���ֶε�ֵ)
	 * �����ݰ�����ʵ�������������һ�ְ����ǹ̶��ģ���������ڲ�û�а��������ֶΣ�����һ�֣������ڰ����˳����ֶΡ�
	 * һ����ͨ�š�ֻ��ͬʱ������һ����������Ҷ��ڰ����ǹ̶������������ȡ���������ݰ���������ͬһ��������
	 * ע�⣺����������������ص����ݰ����ǰ��������ֶεģ�
	 * ���Readʱ����head.pack_len������һ���������ֵ����ͨ�š����̾���Ϊ��Ҫ��һ���̶����ȵ����ݰ���
	 * ��ʱ�����head.pack_len��Ϊԭʼ����������ͬ���ֶ�(�����������ֶ�)��
	 * @param ch ѡ�����ŵ�����������������������ĸ��ŵ���ʹ��Comm::ANY_CHANNEL�Ա����κ�һ���������ݵ��ŵ����ݰ�������
	 * @return ���յ����������ݰ������Pacakge::GetLength() == 0����˵���Է��ѹرշ���ͨ����(���ŵ�)�����������ݰ���
	 * @exception SockError �ײ�(winsock)ͨ���쳣
	 */
	template<class HT> BrPackage<HT> read(const HT &head, Channel ch) {
		BrBufferMan packdata = read(&head, ch, true);
		BrPackage<HT> pack(packdata);
		return pack;
	}
	/**
	 * �����ݰ�������һ����ʽ
	 * ������������һ���ŵ��������ݣ�����ϣ��֪���յ������ݾ����������ĸ��ŵ�ʱ��ʹ���������
	 * @param head Ҫ���͵����ݰ��İ�ͷ�ṹ(�����ͷͬ���ֶε�ֵ)
	 * ���ڶ�ȡ��������˵������ǰһ��Read��
	 * @return ���յ����������ݰ����Լ��յ����ݵ��ŵ���
	 * ���Pacakge::GetLength() == 0����˵������Ŀǰ���ӵ�����ͨ���ķ��Ͷ˶��ѱ��Է��رգ������������ݰ���
	 * @exception SockError �ײ�(winsock)ͨ���쳣
	 */
	template<class HT> std::pair<BrPackage<HT>,Channel> read(const HT &head) 
	{
		BrComm::Channel chn = BrComm::ANY_CHANNEL;
		BrBufferMan packdata = read(&head, chn, true);
		BrPackage<HT> pack(packdata);
		return std::make_pair(pack,chn);
	}

	/**
	* ��������(������)
	* ��ȡ�Ѿ��յ������ݰ��������ǰû���ֳɵ����ݰ��ڣ��򷵻ؿ�
	* @param head Ҫ���͵����ݰ��İ�ͷ�ṹ(�����ͷͬ���ֶε�ֵ)����ǰ��Read()��˵��
	* @param ch ѡ�����ŵ�����������������������ĸ��ŵ���ʹ��Comm::ANY_CHANNEL�Ա����κ�һ���������ݵ��ŵ����ݰ�������
	* @return ���յ����������ݰ������Pacakge::GetLength() == 0����Ŀǰû�п��õ����ݰ�
	* @exception BrSockError �ײ�(winsock)ͨ���쳣
	* @exception BrCommError ���������Ϊ���������Ѿ��ر����׳�CE_CONNECT_CLOSE�쳣
	*/
	template<class HT> BrPackage<HT> noblock_read(const HT &head, Channel ch = BrComm::ANY_CHANNEL) 
	{
		BrBufferMan packdata = read(&head,ch,false);
		BrPackage<HT> pack(packdata);
		return pack;
	}
	/**
	 * ������ȡ
	 * һ�ζ�ȡ������ݰ���ֱ�����ն�����ȡ�գ����߻������Ѿ��Ų��¸�������ݰ���
	 * ������ȡ���ȴ���������ջ�����û�����ݻ��������ء�
	 * ������ȡֻ�ܶ�ȡ����һ��ͨ�������ݰ���
	 * @param buffer ������ݰ��Ļ�����
	 * @param buf_len �������ĳ���
	 * @param head Ҫ���͵����ݰ��İ�ͷ�ṹ(�����ͷͬ���ֶε�ֵ)����ǰ��Read()��˵��
	 * @param ch ѡ�����ŵ�����������������������ĸ��ŵ���ʹ��Comm::ANY_CHANNEL�Ա����κ�һ���������ݵ��ŵ����ݰ�������
	 * @param wait_data Ŀǰ��û�����ݵĻ����Ƿ�Ҫ�ȴ����ݵĵ���
	 * @return ���յ����������ݰ����飬�յ������ʾû�����ݻ��߻������ĳ��Ȳ������һ�����ݰ�
	 * @exception SockError �ײ�(winsock)ͨ���쳣
	 */
	template<class HT>
	std::vector<BrPackage<HT> > batch_read(char *buffer, size_t buf_len, const HT &head, Channel ch, bool wait_data = true) {
		size_t nread = read(&head, ch, buffer, buf_len, wait_data);
		std::vector<BrPackage<HT> > v;
		while ( nread > 0 ) {
			HT *hd = reinterpret_cast<HT*>(buffer);
			v.push_back(BrPackage<HT>(hd, buffer + sizeof(HT), false));
			nread -= hd->pack_len;
			buffer += hd->pack_len;
		}
		return v;
	}
	/*
	 * ��ȡIO������ͳ����Ϣ

	 * @param input_stat ��š����ա���Ϣ�ı���
	 * @param output_stat ��š����͡���Ϣ�ı���
	 */
	void get_iostat(BrIOStat &input_stat, BrIOStat &output_stat);
	/**
	 * ��ȡ�첽�����Ĵ�����Ϣ
	 * @param chn Ҫ��ѯ������Ϣ��ͨ�������ΪANY_CHANNEL�����ص�һ���д�����û�з��ع������ͨ����Ϣ��
	 * @return Winsock�����룬�Լ����������ͨ����û�д���ʱ�����뷵��0��
	 */
	std::pair<int, Channel> fetch_sock_error(Channel chn = ANY_CHANNEL);
	/**
	 * ��ȡ�Զ˵�ַ
	 * @param chn ѡ��ͨ��
	 * @return �Զ˵�ַ������Ҳ���������ͨ�������ص�ַANY��
	 */
	BrAddress get_peer(Channel chn);

	/**
	 * ��ȡ��ǰ�ͻ��˵ĵ�ַ���˿�
	 * @param local 
	 */
	BrAddress get_local();

	/**
	 * �ر�ͨ�ţ����ӵ�����δ������İ����ر�ʹ����Դ
	 */
	void close();
	/**
	 * ��ն�д���С������е����ݱ�����
	 */
	void clear();
private:
	/**
	 * �������ݰ�
	 * �������ȷ��Ͱ�ͷ��Ȼ������ŷ������ݡ�
	 * @param packet Ҫ���͵����ݰ�������
	 * @param channel ���룺Ҫ�������ݵ��ŵ��������ʵ�ʷ������ݵ��ŵ�(����ANY_CHANNELʱ)
	 * @return ��ǰ���Ͷ��г���
	 */
	size_t write(const char *head, size_t head_len, const char *data, size_t data_len, Channel channel);
	/**
	 * �������ݰ�
	 * Ҫ������ݸ�ʽ�����ǰ�ͷ��Ȼ������������ݡ���ͷ������GenericHeader��ʽ��ͷ�����ֶηֱ��Ǳ�־�����ݰ����ȡ�
	 * �ر�Լ����ÿ����ͨ�š�������ж�����ŵ����Ļ�����ͨ���ܽ��յ����а���ͬ���ֶε�ֵ������ͬ�ģ�
	 * @param head Ҫ�������ݰ���ͨ�ð�ͷ��Ϣ��Ҫʹ�õ����е�sync_flag�����ݵĳ����ɶ�ȡ���ݵ�pack_lenȷ��
	 * @param channel ���룺Ҫ�������ݵ��ŵ��������ʵ�ʽ������ݵ��ŵ�(����ANY_CHANNELʱ)
	 * @param sync	���û�������Ƿ�ȴ���true���ȴ���false�����ȴ�ֱ�ӷ���
	 * @return �յ������ݣ��ڴ��ڶ�ȡ�����з��䣬�ڷ��ض���(�������п���)�������ڽ�����ʱ����Զ��ͷ��ڴ�
	 */
	BrBufferMan read(const BrGenericHeader *head, Channel &channel, bool sync);

	/**
	 * Ϊ��������ȡ�����ṩ����һ��ʵ��
	 * �˷����ᾡ����ȡ�������ݰ���ֱ�����������ܴ�Ÿ�������ݰ������߽��ջ����Ѿ�û�����ݰ��˲ŷ��ء�
	 * @param head ͬ��һ������
	 * @param channel ͬ��
	 * @param buffer ���ڴ�ŷ��ص����ݰ��Ļ�����
	 * @param size ����������
	 * @return ���ص����ݰ����ܳ��ȡ����ݰ���һ������һ���ķ�ʽ��ţ�ÿ�����ݰ��Ŀ�ͷ����һ��GenericHeader��
	 * ���а����˰��ĳ��ȡ��ӻ�������ʼ�����ܳ��Ⱥ͸������ȿ��԰�ȫ�������������
	 */
	size_t read(const BrGenericHeader *head, Channel &channel, char *buffer, size_t size, bool sync);
};



// �Է�������ɫ�����"ͨ��"
class _COMM_API_ BrServerComm : public BrComm {
public:
	/**
	 * @param gen_buf_size	����δָ��ͨ���Ĵ������ݵķ��ͻ��峤��
	 * @param chn_buf_size	ÿ��ͨ���Ļ��峤�ȣ����ջ���ͷ��ͻ��嶼��������ȣ�
	 */
	BrServerComm(size_t gen_buf_size = 1024*1024*8, size_t chn_buf_size = 1024*1024*2);
	/**
	 * �������˵�һ����Ҫ���ܾ��Ǽ����ͽ�������
	 * ����
	 * @param sap ������ַ(������ʵ㣬������ip+port��Ҳ����ֻ��port����ʱ�������б���ip��ַ)
	 * @exception SockError Winsock����CommErrorͨ���߼�����
	 */
	void listen(const BrAddress &sap);

	// �Ƿ���Ҫ��ʽ��accept����
	/**
	 * ���տͻ��˷��������
	 * @return �ѽ������ӵ��ŵ���ʶ���������ANY_CHANNEL����˵�����Ӳ��ɹ�������ϵͳ������������ӡ�
	 * @exception SockError Winsock����CommErrorͨ���߼�����
	 */
	BrComm::Channel accept();
	/**
	 * �ر�ָ�����ŵ�
	 */
	void close(BrComm::Channel s);
	/**
	* �ر�ȫ���ŵ�
	*/
	void close() { return BrComm::close(); }

	/**
	 * �������л���ŵ�
	 */
	std::set< BrComm::Channel > enum_channels();
};
// �Կͻ��˽�ɫ�����"ͨ��"
class _COMM_API_ BrClientComm : public BrComm {
public:
	/**
	 * @param gen_buf_size	����δָ��ͨ���Ĵ������ݵķ��ͻ��峤��
	 * @param chn_buf_size	ÿ��ͨ���Ļ��峤�ȣ����ջ���ͷ��ͻ��嶼��������ȣ�
	 */
	BrClientComm(size_t gen_buf_size = 1024*1024*8, size_t chn_buf_size = 1024*1024*2);
	/**
	 * �ͻ���������������
	 * @param sap ͨ�ŶԷ��ĵ�ַ(������ʵ㣬Ӧ����ip+port)
	 * @return �ѽ������ӵ��ŵ���ʶ
	 */
	BrComm::Channel connect(const BrAddress &sap);
	/**
	 * ��������ǰ����ǿ�ư󶨱��ض˿ڼ���ַ
	 * @param local ���ض˿�/��ַ
	 */
	void bind(const BrAddress &local);
	
};

/**
 * ��������鲥������
 */
class BrDGPackSender : public BrDataGramSender
{
public:
	BrDGPackSender(unsigned short port, const char *multicast_ip, const char *if_ip):
		BrDataGramSender(port, multicast_ip, if_ip)
	{

	}
	template<class HT> void send( HT *header, const char *data)
	{
		char *buf = new char[header->pack_len];
		memmove(buf, header, sizeof(HT));
		memmove(buf + sizeof(HT), data, header->pack_len - sizeof(HT));
		size_t sended_all = 0;
		do
		{
			int sended = BrDataGramSender::send(buf, header->pack_len);
			if (sended < 0)
			{
				delete []buf;
				
				throw BrSockError(BrDataGramSender::GetClearError());
			}
			sended_all += sended;
		} while( sended_all < header->pack_len );
		delete []buf;
	}
};
// ͳ����Ϣ
struct BrIOStat {
	int64_t total_packets; // �շ������ݰ���
	int64_t total_bytes; // socket������ɵ��ֽ���
	int64_t bytes_in_packet; // �շ��İ����ֽ�����
	int64_t bytes_in_queue; // ��Ȼ�ڶ����е��ֽ���
	int64_t bytes_drop; // �ӵ����ֽ��������ղ�ͬ�������ͳ����ᵼ�¶������ݣ�
};

#endif // __COMMUNICATION_H_
