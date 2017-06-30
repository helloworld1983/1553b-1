/**
 * \file propcfg.h
 * property��ʽ�������ļ�����.
 * ��VxWorksû��xml������������Ҫʹ��property��ʽ(��KEY=VALUE��ʽ)�������ļ���
 * ����Ϊ���ָ�ʽ�����ý����ṩͳһ�ķ��������㲢�򻯸�Adapter�Ŀ�����
 *
 *  \date 2011-03-09
 *  \author anhu.xie
 */
#ifndef PROPERTY_CONFIG_H_
#define PROPERTY_CONFIG_H_

#include "common_qiu.h"
/*
 * ��ȡ�������ֵ.
 * �������ַ����ж�ȡָ�������������ֵ������������ַ�����δ�ҵ�ָ�������������ֵ���䡣
 * �����ַ���(�����ļ�)�ĸ�ʽ�������£�
 *   -# ÿ��������������������(key)��������ֵ(value)��������ɣ�����Ϊ
 * key = value
 * �����ĸ�ʽ�����У��Ⱥ�(=)ǰ������пհף���Щ�հױ����ԣ�
 *   -# ������֮�䣬�ɷֺ�(;)������(\n)���س�(\r)���ŷָ������key��value���ֶ����ܳ�����Щ������ţ�
 *   -# �����������ݱ���Ϊ��key����value��һ���֣������հ׷��š� 
 * \param cfg �������������ַ���
 * \param key Ҫ��ȡ����������
 * \param return_value �����ҵ����������ֵ������������ַ�����δ�ҵ�ָ�������������ı�˲�����ֵ
 * \tparam T ������ֵ������.
 * ������ȡ���̱���Ƴ�C++��ģ�庯�������ڶ���ֵ�������Ƚ���ת��������ʵ�á�
 */
template <typename T> void get_config_value(const std::string &cfg, const char *key, T &return_value);

//============����Ϊʵ�ֲ��֣�ֻ��Ϊ��C++����ʹ�÷����������������ǲ��ǽӿڵ�һ����======================

/// std::string�����������ȡ��ʵ�֣�Ҳ����������ʵ�ֵĻ�����
template <> inline void get_config_value(const std::string &cfg, const char *key, std::string &return_value) {
	std::string::size_type start_pos = cfg.find(key);
	if ( start_pos != std::string::npos ) {
		start_pos += strlen(key);
		// skip leading white spaces
		while ( start_pos != std::string::npos && isspace(cfg[start_pos]) )
			++start_pos;
		if ( start_pos != std::string::npos && cfg[start_pos] == '=' ) {
			++start_pos;
			// skip trailing white spaces
			while ( start_pos != std::string::npos && isspace(cfg[start_pos]) )
				++start_pos;
			std::string::size_type end_pos = cfg.find_first_of(";\n\r", start_pos);
			if ( end_pos != std::string::npos ) {
				while ( isspace(cfg[--end_pos]) )
					;
			}
			return_value = cfg.substr(start_pos, end_pos != std::string::npos ? end_pos - start_pos + 1 : end_pos);
		}
	}
}

/// �����������������ȡ��ʵ�֡�
template <> inline void get_config_value(const std::string &cfg, const char *key, double &return_value) {
	std::string val;
	get_config_value(cfg, key, val);
	if ( !val.empty() )
		return_value = atof(val.c_str());
}

/// �������������ȡ��ʵ�֡�
template <> inline void get_config_value(const std::string &cfg, const char *key, bool &return_value) {
	std::string val;
	get_config_value(cfg, key, val);
	if ( !val.empty() )
		return_value = val == "YES" || val == "yes" || val == "Yes" || val == "TRUE" || val == "true" || val == "True" || val == "1";
}

/// �����������������ȡ��ʵ�֡�
template <typename T> inline void get_config_value(const std::string &cfg, const char *key, T &return_value) {
	std::string val;
	get_config_value(cfg, key, val);
	if ( !val.empty() )
		return_value = atoi(val.c_str());
}

#endif /* PROPERTY_CONFIG_H_ */
