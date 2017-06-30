/**
 * \file alloc.h
 * ��̬����������.
 * ��ԭbuffer.h�з������
 * ��������һ�ֻ��ƣ�����ʹ�ù̶���(ѭ��)����������Ҫ���ǲ�ͣ�ķ�����ͷ��ڴ档
 *
 *    \date 2009-12-30
 *  \author anhu.xie
 */

#ifndef net_ALLOCATOR_H_
#define net_ALLOCATOR_H_
#include <map>

/**
 * ������������/�洢�������ĳ���ӿ�.
 * �������������շ��ġ�
 * �������շ������У��Ի������������������ģ�
 * ���ȷ���洢(allocate)��Ȼ��������Ŀռ���洢����(store)���������ݻᱻʹ����ȡ��(retrieve)����󣬻������ᱻ�ͷ�(deallocate)��
 * ���ԣ����Ƕ��������½ӿڣ����Ҽ���������ûᰴ������˳��������Ȼ����ȡʱ������ÿ��ֻ��ȡһ�������ݣ�Ҫ��β�����ɴ�ȡ��
 * ���⣬Ҳ���ܴ洢�����ݻᱻ���˷���ʹ�á����ǣ���Ҫ��֤���ǣ�allocate������store֮ǰ������ֻ��store���֮�󣬲���retrieve��
 * ��deallocate֮�����ܽ����κβ�����
 */
class IAllocator {
public:
	/**
	 * ���仺����
	 * \param len Ҫ��Ļ���������
	 * \return ��õĻ������ĵ�ַ��NULL��ʾ���ܷ���ָ����С�Ļ�����
	 */
	virtual char *allocate(size_t len) = 0;
	/**
	 * �ͷŻ�����
	 * \param ptr Ҫ�ͷŵĻ���������������ǰallocate()���ص�ֵ������������Ԥ��
	 * \param len �������̶ȣ�Ҳ�����ǵ���allocate()ʱ�ĳ���
	 */
	virtual void deallocate(char *ptr, size_t len) = 0;
	/**
	 * �ӻ�������ȡ����
	 * \param position �����ڻ��������е�(��ʼ)λ��
	 * \param data ������ݵ�ָ��
	 * \param count Ҫ��ȡ�����ݵ���Ŀ���ֽ���
	 */
	virtual void retrieve(const char *position, void *data, size_t count) = 0;
	/**
	 * �򻺳����д������
	 * \param position �����ڻ��������е�(��ʼ)λ��
	 * \param data Ҫ��ŵ����ݵ�ָ��
	 * \param count Ҫ���ݵĳ��ȣ��ֽ���
	 */
	virtual void store(char *position, const void *data, size_t count) = 0;
	/// �����������ͷ���Դ
	virtual ~IAllocator() {
	}
};

/**
 * ��̬������ͷſռ�Ļ�����������
 */
class DynaAllocator : public IAllocator {
	virtual char *allocate(size_t len) {
		return new char[len];
	}
	virtual void deallocate(char *ptr, size_t len) {
		delete []ptr;
	}
	virtual void retrieve(const char *position, void *data, size_t count) {
		memcpy(data, position, count);
	}
	virtual void store(char *position, const void *data, size_t count) {
		memcpy(position, data, count);
	}
};

/**
 * ѭ��������
 */
class CircularAllocator : public IAllocator {
private:
	char* m_Buffer;
	size_t m_BufSize;
	size_t m_AllocPos;
	size_t m_FreePos;
	std::map<char *, size_t> m_PendFree;
public:
	CircularAllocator(size_t sz) : m_Buffer(NULL), m_BufSize(sz), m_AllocPos(0), m_FreePos(0) {}
	/// �����������ͷ���Դ
	~CircularAllocator() {
		Destroy();
	}
	/**
	 * �������û�����.
	 * �ջ������ѷ���Ļ���������Ҫ�Ļ�������������չ�������ռ䵽ָ����С
	 * \param new_size �µĻ�������С��0��ʾ��С���䣬ֻ�����ѷ���Ŀռ䡣
	 */
	void Reset(size_t new_size = 0) {
		if ( new_size && new_size != m_BufSize ) {
			Destroy();
			m_Buffer = new char[new_size];
			m_BufSize = new_size;
		}
		else {
			m_AllocPos = 0;
			m_FreePos = 0;
		}
	}
	/// ����
	void Destroy() {
		delete []m_Buffer;
		m_Buffer = NULL;
		m_AllocPos = 0;
		m_FreePos = 0;
	}
	/// �������û���������
	size_t UsedCount() const {
		int count = m_AllocPos - m_FreePos;
		if ( count < 0 )
			count += m_BufSize;
		return count;
	}

public: // IAllocator�ӿ�
	virtual char *allocate(size_t len) {
		if ( m_Buffer == NULL && m_BufSize > 0 ) {
			m_Buffer = new char[m_BufSize]; // �ӳٷ���ռ�
		}
		if ( len >= m_BufSize )
			throw std::bad_alloc();
		if ( m_BufSize - UsedCount() <= len ) // ���ÿռ䡣ע��һ����<=����Ϊm_WritePos==m_ReadPos��ʾ�գ���������������
			return NULL;
		size_t curPos = m_AllocPos;
		size_t nextPos = m_AllocPos + len;
		if ( nextPos >= m_BufSize )
			nextPos -= m_BufSize;
		m_AllocPos = nextPos;
		return m_Buffer + curPos;
	}
	virtual void deallocate(char *ptr, size_t len) {
		if ( len >= m_BufSize )
			throw std::bad_alloc();
		if ( UsedCount() < len || ptr < m_Buffer || ptr >= m_Buffer + m_BufSize ) // ������ȫ������Ŀռ�󣬻��߲������Ƿ����
			throw std::bad_alloc();
		if ( ptr != m_Buffer + m_FreePos ) { // ���ǵ�һ�������
			if ( m_PendFree.empty() )
				m_PendFree[ptr] = len;
			else { // �Ƿ������ǰ��Ĵ��ͷſ�ϲ�
				std::map<char *, size_t>::iterator next = m_PendFree.begin();
				while ( next != m_PendFree.end() && ptr > next->first ) { // ��һ�������Ǵ��
					++next;
				}
				// next == m_PendFree.end() || ptr <= next->first
				char *merged_next = NULL;
				if ( next != m_PendFree.end() ) {// ptr <= next->first!
					if ( ptr + len == next->first ) {
						merged_next = next->first;
						len += next->second;
					}
				}
				if ( next != m_PendFree.begin() && (--next)->first + next->second == ptr ) // �ϲ�ǰ���Ǹ���
					next->second += len;
				else
					m_PendFree[ptr] = len; // ���룡
				if ( merged_next )
					m_PendFree.erase(merged_next);
			}
			return;
		}
		if ( !m_PendFree.empty() ) { // �ϲ����ͷŵ����ڿ�
			char *next_block = ptr + len;
			if ( next_block >= m_Buffer + m_BufSize )
				next_block -= m_BufSize;
			std::map<char *, size_t>::iterator next = m_PendFree.find(next_block);
			while ( next != m_PendFree.end() ) {
				len += next->second;
				m_PendFree.erase(next);
				next_block = ptr + len;
				if ( next_block >= m_Buffer + m_BufSize )
					next_block -= m_BufSize;
				next = m_PendFree.find(next_block);
			}
		}
		size_t dwEndPos = (m_FreePos + len);
		if ( dwEndPos >= m_BufSize ) // wrap
			dwEndPos -= m_BufSize;
		m_FreePos = dwEndPos;
	}
	virtual void retrieve(const char *position, void *data, size_t count) {
		if ( m_FreePos == 0 && m_AllocPos == 0 )
			return; // no data(after reset)!
		if ( position > m_Buffer + m_BufSize )
			position -= m_BufSize; // ѭ������
		if ( position < m_Buffer || position > m_Buffer + m_BufSize )
			throw std::bad_alloc(); // �������ǵĻ�������Χ��
		size_t nowrap = m_Buffer + m_BufSize - position; // ��������β���ĳ���
		if ( nowrap >= count ) // no wrap
			memcpy(data, position, count);
		else { // wrapped!
			memcpy(data, position, nowrap);
			memcpy(reinterpret_cast<char*>(data) + nowrap, m_Buffer, count - nowrap);
		}
	}
	virtual void store(char *position, const void *data, size_t count) {
		if ( m_FreePos == 0 && m_AllocPos == 0 )
			return; // must re-allocate after reset!
		if ( position > m_Buffer + m_BufSize )
			position -= m_BufSize; // ѭ�������ˣ�
		if ( position < m_Buffer || position > m_Buffer + m_BufSize )
			throw std::bad_alloc(); // �������ǵĻ�������Χ��
		size_t nowrap = m_Buffer + m_BufSize - position;
		if ( nowrap >= count ) // no wrap
			memcpy(position, data, count);
		else { // wrapped!
			memcpy(position, data, nowrap);
			memcpy(m_Buffer, reinterpret_cast<const char*>(data) + nowrap, count - nowrap);
		}
	}
};


#endif /* net_ALLOCATOR_H_ */
