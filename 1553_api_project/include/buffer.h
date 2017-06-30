/**
 * \file buffer.h
 * ��̬����������.
 * ������뷨Դ��0062��Ŀ
 *
 *    \date 2009-8-23
 *  \author anhu.xie
 */

#ifndef net_BUFFER_H_
#define net_BUFFER_H_

#include <queue>
#include "alloc.h"
#include "thread.h"

/**
 * ����ӿ�ʹ�����ݰ��Ļ����������������д����ʹ�õ����ݽṹ.
 * ��Ҫ�����Ǹ������Ƿ�����ڴ�ռ���Զ��ͷţ�
 * �ڴ������(���俽��)�����������ڣ������������������������ڵ����ݣ�
 * �ڶ����������ڽ���ʱ�����Զ�ɾ����������ڴ�ռ䡣
 */
class AutoBuffer {
	class DataRef;
	friend class DataRef;
	DataRef *rep; // �����Ļ�����(�������)
public:
	/// �������
	AutoBuffer(const AutoBuffer&);
	AutoBuffer& operator=(const AutoBuffer&);
	virtual ~AutoBuffer();
	/// Ĭ�Ϲ��캯��
	AutoBuffer();
	/// ���仺����
	AutoBuffer(IAllocator &allocator, size_t mem_len);
	// ȡ�û�������ַ
	// ���ʹ��ѭ���������������ͣ���������ַ��ʹ����û�����塣���Բ����ػ�������ַ��������RetrieveData()��StoreData()����ȡ���ݡ�
// 	char *GetBuffer() const;
	/// ��ȡ����
	void RetrieveData(void *buffer, size_t offset, size_t count) const;
	/// �������
	void StoreData(size_t offset, const void *data, size_t count);
	/// ����������
	size_t GetLength() const;
};

/**
 * ʵ�ʱ������ݵĽṹ.
 * �ؼ������ڴ����ʹ�û����ع����ڴ�����⡣
 */
class AutoBuffer::DataRef : MXLock {
	friend class AutoBuffer;
	IAllocator &allocator; // �洢������
	char *data_ptr; // ����Ļ�����ָ��
	size_t size; // ����Ļ���������
	long ref_count; // �������ü���

	DataRef(IAllocator &m, size_t len);
	// ��������
	size_t reference();
	// �������ã�����Ѿ�û�������ˣ��ͷ�����ڴ�
	size_t de_reference();
};

/**
 * ʹ��ѭ������Ĵ洢��
 */
class CircularStore : public CircularAllocator {
	typedef CircularAllocator Parent;
	SyncEvent not_full;
	MXLock alloc_lock;
	MXLock free_lock;
public:
	CircularStore(size_t buflen) : CircularAllocator(buflen) {}
	bool wait_space(unsigned long time_out) {
		return not_full.wait(time_out);
	}
	virtual char *allocate(size_t len) {
		AutoLock al(alloc_lock);
		char *p = Parent::allocate(len);
		return p;
	}
	virtual void deallocate(char *ptr, size_t len) {
		AutoLock al(free_lock);
		Parent::deallocate(ptr, len);
		not_full.signal();
	}
};

/**
 * Agent�ڲ�ʹ�õ���Ϣ����
 */
template <class M>
class MsgQue : public MXLock {
	std::deque<M> que;
	CondEvent ready;
public:
	MsgQue() : ready(false) {}
	/// �ж϶����Ƿ��
	bool empty() const { return que.empty(); }
	/// �������
	void clear() { que.clear(); }
	/// �ȴ�����������
	bool wait(unsigned int u_t_o = 1000000) {
		AutoLock al(*this);
		bool r = que.size() > 0 || ready.wait(this, u_t_o);
		return r;
	}
	/// ���ݷ������
	void push(const M &m) {
		AutoLock al(*this);
		que.push_back(m);
		ready.signal();
	}
	/// �Ӷ�����ȡ������
	M pop() {
		AutoLock al(*this);
		M r = que.front();
		que.pop_front();
		return r;
	}
};

// DataRefʵ��

// ���캯��
inline AutoBuffer::DataRef::DataRef(IAllocator &m, size_t len) : allocator(m), data_ptr(m.allocate(len)), size(len), ref_count(0) {}
// ��������
inline size_t AutoBuffer::DataRef::reference() {
	AutoLock al(*this);
	return ref_count++;
}
// �������ã�����Ѿ�û�������ˣ��ͷ�����ڴ�
inline size_t AutoBuffer::DataRef::de_reference() {
	AutoLock al(*this);
	if ( --ref_count == 0 ) {
		// !!!����ʹ��delete this����Ϊ���ǻ�Ҫ����MXLock�Ӷ���
		allocator.deallocate(data_ptr, size);
		data_ptr = NULL;
		size = 0;
	}
	return ref_count;
}


// AutoBufferʵ��

inline AutoBuffer::AutoBuffer() : rep(NULL) {
}

inline AutoBuffer::AutoBuffer(IAllocator &allocator, size_t len) : rep(len > 0 ? new DataRef(allocator, len) : NULL) {
	if ( rep ) {
		if ( rep->data_ptr )
			rep->reference();
		else {
			delete rep;
			rep = NULL;
		}
	}
}

inline AutoBuffer::AutoBuffer(const AutoBuffer &src) : rep(src.rep) {
	if ( rep )
		rep->reference();
}

inline AutoBuffer &AutoBuffer::operator=(const AutoBuffer &src) {
	if ( rep )
		if ( rep->de_reference() == 0 ) {
			delete rep;
		}
	rep = src.rep;
	if ( rep )
		rep->reference();
	return *this;
}

inline AutoBuffer::~AutoBuffer() {
	if ( rep ) {
		if ( rep->de_reference() == 0 ) {
			delete rep;
		}
		rep = 0;
	}
}

// ��������ַ
//inline char *AutoBuffer::GetBuffer() const {
//	return rep ? rep->data_ptr : NULL;
//}

// ��ȡ����
inline void AutoBuffer::RetrieveData(void *buffer, size_t offset, size_t count) const {
	if ( rep )
		rep->allocator.retrieve(rep->data_ptr + offset, buffer, count);
	else
		throw std::runtime_error("no data to retrieve");
}
// �������
inline void AutoBuffer::StoreData(size_t offset, const void *data, size_t count) {
	if ( rep )
		rep->allocator.store(rep->data_ptr + offset, data, count);
	else
		throw std::runtime_error("no memory allocated");
}
// ����������
inline size_t AutoBuffer::GetLength() const {
	return rep ? rep->size : 0;
}

#endif /* net_BUFFER_H_ */
