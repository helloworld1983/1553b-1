/**
 * \file genmti.h
 * 1553Bʾ��ģ�飺ʵ��Adapter����DDC��MTI�ӿ����1553B���߼��.
 *
 *    \date 2012-9-4
 *  \author anhu.xie
 */
//#define _CRT_SECURE_NO_WARNINGS

#pragma once
//#include "gist/agent.h"
//#include "gist/factory.h"
#include <map>
using namespace std; //zhanghao adds.

/**
 * 1553B���߼��.
 * ʹ��DDC MTI�ӿڻ�ȡ1553B������Ϣ������ԭMT���ݵĸ�ʽ���ء�
 * 
 * ProgIDΪAdapter.1553B.Generic.MTI��
 * ֧�ֵ����ò�����
 *  - BIG_ENDIAN �����ͣ�����ʱ���������Դ�˸�ʽ���أ�
 *  - DATA_CHANNEL ����������������MTI�ӿڶ�ȡ��Ϣ����ͨ��ID������Ĭ��ֵΪ0��
 */
class GIST_MT2 : public Agent::IAdapter_V2 {
	//static GIST::DynaClass<GIST_MT2, Agent::IDevBase> s_class_reg;  zhanghao close.
	static const size_t MT_TIMER_BITS = 16;
protected:
	S16BIT devNum; ///< MT�豸��
	bool big_endian; ///< ������Ϣ�����ֵĴ�С�˸�ʽ��
	bool host_buffer; ///< �Ƿ�ʹ��HostBuffer�ɼ�����
private:
	U32BIT m_StkLost; // ��Ϣջ�϶�ʧ����Ϣ��
	U32BIT m_HBufLost; // host buffer��ʧ����Ϣ��
public:
	GIST_MT2() :
		devNum(0), big_endian(false), host_buffer(false) {
	}
	~GIST_MT2() {
	}
public:
	/*
	virtual long capability() const {
#ifdef IPCORE  //IPCORE�汾����Ҫ��һ��syncEvent������
		return DC_READ | DC_DATA_TIME | DC_MONITOR | DC_TIMING;
#else
		return DC_READ | DC_DATA_TIME | DC_MONITOR;
#endif
	}
	*/
	virtual void get_time_resolution(long &resolution, int64_t &overflow) {
		resolution = 64;
		overflow = 1ll << MT_TIMER_BITS;
	}
	virtual void set_config(const std::string &cfg) {
		get_config_value(cfg, "BIG_ENDIAN", big_endian);
		get_config_value(cfg, "HOST_BUFFER", host_buffer);
	}
	virtual void set_address(const std::string &host_addr, const std::string &if_addr) {
		devNum = atoi(if_addr.c_str());
	}
	virtual std::multimap<std::string, std::string> get_bus_addr() {
		return std::multimap<std::string, std::string>();
	}
	virtual unsigned long get_timer_interval() {
#ifdef IPCORE
		return 5000;
#else
		return 0;
#endif
	}
	virtual U16BIT source_init(SyncEvent *sync);
	virtual U16BIT start();
	virtual U16BIT stop();
	virtual U16BIT msg_save (const char *save_path);
	virtual ssize_t read(/* DataTime *time, zhanghao close.*/ Agent::DevDataHeader *head, char *buffer, size_t buf_len);
	/* virtual GIST::BR_OPERATION_R write(const Agent::MsgDataHeader *head, const char *data, size_t len) {
		return GIST::R_FAILURE;
	} zhanghao close.*/
	FILE *fp; //�����ļ����ļ�������
};