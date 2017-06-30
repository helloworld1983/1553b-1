/**
 * \file thread.h
 * ����ͬ������.
 *
 * �ṩWindows/VxWorks/Linux/UNIX(posix)ȫ���ݵ��߳�/��������ࡢ
 * �߳�ͬ��/��������ĸ��ֻ��ƺ�һЩ���ߣ���������������������ͬ���¼���
 * �Լ��˳��������Զ��ͷŵ�����
 *
 *    \date 2009-8-22
 *  \author anhu.xie
 */

#ifndef __THREAD_SYNC_UTIL_H_
#define __THREAD_SYNC_UTIL_H_

#if defined(WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#define  _CRT_SECURE_NO_WARNINGS
#endif

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <errno.h>
#include <stdio.h>

// ����������
#ifdef __vxworks
#include <sysLib.h>
#include <taskLib.h>
#include <semLib.h>
#define OSAPI
typedef int thread_return_t;
typedef int thread_id_t;
const int DEFAULT_THREAD_PRIORITY = 100;
int usleep(unsigned long usec);
#elif defined(WIN32)
#include <Windows.h>
#define OSAPI WINAPI
const int DEFAULT_THREAD_PRIORITY = THREAD_PRIORITY_NORMAL;
typedef DWORD thread_return_t;
typedef HANDLE thread_id_t;
inline void usleep(unsigned long usec) {
	return Sleep((usec + 999) / 1000);
}
#else
#include <pthread.h>
#include <string.h>
#define OSAPI
const int DEFAULT_THREAD_PRIORITY = 100;
typedef void *thread_return_t;
typedef pthread_t thread_id_t;
#endif

/**
 * ����ϵͳ�쳣�İ�װ
 */
class os_error : public std::exception {
	int error_code;
	/*
	 * ��Ϊwhat()Ҫ�󷵻�ָ�룬��Ҫһ�����ָ����ָ���ݵĶ��󣡶����ܷ���һ��ָ����ʱ/�Զ������ָ�롣
	 */
protected:
	std::string msg;
public:
	/**
	 * ���캯�����ò���ϵͳ�Ĵ���������ʼ����
	 * \param eno ����ϵͳ������
	 */
	os_error(int eno) : error_code(eno) {
		std::stringstream ss;
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif
		ss << "system error: \"" << strerror(error_code) << "\"(" << error_code << ").";
#ifdef _MSC_VER
#pragma warning(pop)
#endif
		msg = ss.str();
	}
	/// ��������
	virtual ~os_error() throw () {
	}
	/// �쳣�Ĵ�����Ϣ����ʵ��
	virtual const char* what() const throw () {
		return msg.c_str();
	}
// 	/// �쳣�Ĵ�����Ϣ����ʵ��
// 	virtual const char* what() const {
// 		return msg.c_str();
// 	}
	/// ���ز���ϵͳ������ķ���
	virtual int reason() const {
		return error_code;
	}
};

/**
 * ���ó�Ա�������̺߳���.
 * ��CreateThreadWithMember()�õ���
 * \see CreateThreadWithMember
 * \tparam T ������
 * \tparam proc T��ĳ�Ա����������T�������Ϊ�߳������Ĺ�����������û�в��������ز���ϵͳҪ���thread_return_t���͵�ֵ��
 * \param o T��Ķ��󣬱��������ô˶����ָ��������Ϊʵ�ʵ��̺߳���
 */
template<class T, thread_return_t(T::*proc)()>
thread_return_t OSAPI thread_proc(void *o) {
	return ((reinterpret_cast<T*>(o))->*proc)();
}
/**
 * ��Ա������Ϊ�̺߳����Ĺ���.
 * \tparam T ������
 * \tparam proc T��ĳ�Ա����������T�������Ϊ�߳������Ĺ�����������û�в��������ز���ϵͳҪ���thread_return_t���͵�ֵ��
 * \param o Ҫ�����̵߳�T�����
 * \param tid ��������������id�����
 * \param priority ������������ȼ�
 * \param t_name ��������֣����Ժ��Ź�ʱ�ǳ�����
 * \param size WIN32��ʹ�ã������߳�ʱ����Ķ�ջ��С����Ϊ0����ΪĬ�ϴ�С
 */
template<class T, thread_return_t(T::*proc)()>
void CreateThreadWithMember(T *o, thread_id_t &tid, int priority = DEFAULT_THREAD_PRIORITY, const char *t_name = NULL,int size = 0) {
	thread_return_t(OSAPI *thproc)(void *) = &thread_proc<T,proc>;
#ifdef __vxworks
	int stackSize = 1024 * 1024 * 1;
	if (size != 0)
	{
		stackSize = size;
	}
	tid = taskSpawn(
	         const_cast<char*>(t_name),           /* name of new task (stored at pStackBase) */
	         priority,       /* priority of new task */
	         VX_FP_TASK,     /* task option word */
	         stackSize,      /* size (bytes) of stack needed plus name */
	         reinterpret_cast<int (*)(...)>(thproc),   /* entry point of new task */
	         reinterpret_cast<int>(o),          /* 1st of 10 req'd task args to pass to func */
	         0, 0, 0, 0, 0, 0, 0, 0, 0 /* we only use 1 arg -- the object itself */
	         );
	if ( tid == ERROR )
		throw os_error(errno);
#elif defined(WIN32)
	if (size != 0)
	{
		tid = CreateThread(NULL, size, thproc, (LPVOID)o,STACK_SIZE_PARAM_IS_A_RESERVATION | CREATE_SUSPENDED, 0);
	}else	
	{
		tid = CreateThread(NULL, 0, thproc, (LPVOID)o, CREATE_SUSPENDED, 0);
	}
	//tid = CreateThread(NULL, 1*1024*1024, thproc, (LPVOID)o,STACK_SIZE_PARAM_IS_A_RESERVATION, 0);
	if ( tid == NULL )
		throw os_error(GetLastError());
	if ( !SetThreadPriority(tid, priority) )
		throw os_error(GetLastError());
	if ( !ResumeThread(tid) )
		throw os_error(GetLastError());
	
#else
	int e = pthread_create(&tid, NULL, thproc, o);
	if ( e != 0 )
		throw os_error(e);
#endif
}

/**
 * ����/�̵߳ĳ���.
 * �ṩ����������ֹͣ��ͬ�����ƵĻ��ơ�
 */
class Thread {
protected:
	/// ����ϵͳ���ص��̱߳�ʶ
	thread_id_t thread_id;
	/// �̱߳�ʶ���߳�����ֹͣ��thread_id�ᱻ����������ס���ȴ����߳�JOIN
	thread_id_t join_id; // Linux must join, even when the thread has stopped!
	/// �߳����ȼ�
	const int priority;
	/// �߳����ƣ������Ź�ʱ���ָ�����ͬ���߳�
	std::string thread_name;
	/// �߳�״̬��ע������������ģ�
	volatile enum {
		WS_INIT, ///< ����ոմ���ʱ�ĳ�ʼ״̬
		WS_RUN, ///< �߳�������/�����߳����е�״̬
		WS_STOP, ///< ���߳�ֹͣʱ���õ�״̬
		WS_ERROR, ///< �߳��쳣
		WS_EXIT ///< �߳����˳�
	} work_stat;

public:
	/**
	 * ���캯��
	 * \param pri ��������ʱʹ�õ����ȼ�
	 */
	Thread(int pri = DEFAULT_THREAD_PRIORITY) :
		thread_id(0), join_id(0), priority(pri), work_stat(WS_INIT) {
	}
	/**
	 * ��������
	 */
	virtual ~Thread() {
#ifdef __linux__
		sync();
#elif defined(WIN32)
		if ( thread_id )
			CloseHandle(thread_id);
#endif
	}
	/**
	 * ��������
	 * \param thread_name ����ϵͳ�����������
	 * \param size �����̷߳���Ķ�ջ��С����Ϊ0����ΪĬ�ϴ�С
	 */
	void start(const char *display_name = NULL,int size = 0) {
#ifdef _TRACE
		std::cerr << "thread start [" << display_name << "]" << std::endl;
#endif
		if ( thread_id /*&& taskIdVerify(thread_id) == OK*/ ) // �߳�������
			return;
		if ( display_name )
			thread_name = display_name;
		work_stat = WS_RUN;
		CreateThreadWithMember<Thread, &Thread::run0> (this, thread_id, priority, display_name,size);
		join_id = thread_id;
	}
	/**
	 * ����ͬ��.
	 * �ȴ�������ɡ�
	 * ע�⣺Linuxϵͳ����ÿ�������˵��̣߳��������sync()��
	 * \param retval_ptr ������񷵻�ֵ��ָ�롣NULL��ʾ����Ҫ����ֵ��
	 * \return ����ϵͳ����ֵ�������Ƿ�ɹ���
	 */
	int sync(void **retval_ptr = NULL) {
		if ( !join_id )
			return 0;
		int r;
#ifdef __vxworks
		r = -1;
#elif defined(WIN32)
		r = WaitForSingleObject(join_id, INFINITE);
#else
		r = pthread_join(join_id, retval_ptr);
#endif
		join_id = 0;
		return r;
	}
	/**
	 * ֹͣ����.
	 * ��������ֹͣ�źš�
	 */
	void stop() {
		if ( work_stat == WS_RUN )
			work_stat = WS_STOP;
	}
	/**
	 * (ǿ��)ֹͣ����.
	 * �ȸ�������ֹͣ�źš����100ms��������ֹͣ����ǿ��ֹͣ����
	 */
	int exit() {
		if ( thread_id == 0 && join_id == 0 )
			return 0;
		stop();
		usleep(100000);
		if ( thread_id == 0 ) // �ٴμ���Ƿ��˳�
			return 0;
#ifdef __vxworks
		int ret = taskIdVerify(thread_id) == OK ? taskDelete(thread_id) : OK;
#elif defined(WIN32)
		int ret = TerminateThread(thread_id, -1) ? 0 : GetLastError();
		CloseHandle(thread_id);
		join_id = 0;
#else
		int ret = pthread_cancel(thread_id);
#endif
		work_stat = WS_EXIT;
		thread_id = 0;
		return ret;
	}
private:
	/*
	 * ʹrun�����ɱ��������ض���
	 */
	thread_return_t run0()
	{
		return run();
	}
protected:
	/**
	 * ����Ĺ�������.
	 * Ĭ�ϵĹ���������������do_job()��ֱ������ֹͣ�źű����á���ѭ�������������������쳣��
	 * ֹͣ�źſ��ܱ�ֹͣ����ķ���stop()��exit()���ã�Ҳ���ܸ���ҵ���߼��й�������do_job()���á�
	 * \p���Ƽ���������д�˷�����
	 * \return ����ϵͳָ�������񷵻�ֵ��
	 */
	virtual thread_return_t run() {
		while ( work_stat == WS_RUN )
			try {
				do_job();
			}
			catch ( std::exception &x ) {
				deal_error(std::string("std::exception caught: ") + x.what());
				usleep(100000);
			}
			catch ( ... ) {
				deal_error("unknown exception caught");
				usleep(100000);
			}
#if defined(WIN32)
		CloseHandle(thread_id);
		join_id = 0;
#endif
		thread_id = 0;
		return 0;
	}
	/**
	 * ������.
	 * ���������Ĺ�����������ɡ��������ᱻ�������ã������ظ��Ĺ�����Ӧ��������ѭ����
	 * \pĬ�ϵ�ʵ��ʲôҲ���ɡ�������Ӧ������ʵ�ִ˷�����
	 */
	virtual void do_job() {
		usleep(100000);
	}
	/**
	 * �쳣������.
	 * �������з����쳣ʱ���ô˷����������
	 * Ĭ�ϵĴ��������������Ϣ��ֹͣ����������������Ҫ����ʵ�ִ˷�����
	 */
	virtual void deal_error(const std::string &msg) {
		std::cerr << "thread " << thread_name << " <" << thread_id << "> " << msg.c_str() << std::endl;
		work_stat = WS_ERROR;
	}
};

//__thread bool lock_locked;
/**
 * \class MXLock
 * ��������
 */

class MXLock {
#ifdef __vxworks
	SEM_ID mySem; // �������ݽṹ
#elif defined(WIN32)
	CRITICAL_SECTION sync_lock;
#else
	pthread_mutex_t mutex;
#endif
	bool lock_inited; // CRITICAL_SECTION��Ҫ�ӳ�(���ǵȵ�һ������ʱ)��ʼ�������Լ�¼�Ƿ��ʼ��
public:
	void lock(); ///< ����������������ʱ���ݽṹ�İ�ȫ��
	void unlock(); ///< �������

	MXLock();
	~MXLock();
#if !defined( __vxworks ) && !defined(WIN32)
	pthread_mutex_t *get_mutex() { return &mutex; }
#endif
private:
	MXLock(const MXLock &s);
	MXLock &operator=(const MXLock &s);
};

inline MXLock::~MXLock() {
	if ( lock_inited ) {
#ifdef __vxworks
		semDelete (mySem);
#elif defined(WIN32)
		DeleteCriticalSection(&sync_lock);
#else /* __vxworks */
		pthread_mutex_destroy(&mutex);
#endif /* __vxworks */
		lock_inited = false;
	}
}

inline void MXLock::lock() {
	if ( !lock_inited ) {
#ifdef __vxworks
		mySem = semMCreate (SEM_Q_PRIORITY);
#elif defined(WIN32)
		InitializeCriticalSection(&sync_lock);
#else /* __vxworks */
		class MutexAutoRecursiveAttr {
		public:
			pthread_mutexattr_t mutex_attr;
			MutexAutoRecursiveAttr() {
				pthread_mutexattr_init(&mutex_attr);
				pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
			}
			~MutexAutoRecursiveAttr() {
				pthread_mutexattr_destroy(&mutex_attr);
			}
		};
		static MutexAutoRecursiveAttr recursive;
		pthread_mutex_init(&mutex, &recursive.mutex_attr);
#endif /* __vxworks */
		lock_inited = true;
	}
#if _TRACE > 8
	std::cout << this << "\tMXLock::lock() begin..." << std::endl;
#endif
#ifdef __vxworks
	int err = S_objLib_OBJ_TIMEOUT;
	while ( err == S_objLib_OBJ_TIMEOUT ) {
		int s = semTake (mySem, 100);
		if ( s == OK )
			return;
		err = errno;
	}
	throw os_error(err);
#elif defined(WIN32)
	EnterCriticalSection(&sync_lock);
#else /* __vxworks */
	pthread_mutex_lock(&mutex);
#endif /* __vxworks */
#if _TRACE > 8
	std::cout << this << "\tMXLock::lock() ...success" << std::endl;
#endif
}

inline void MXLock::unlock() {
#ifdef __vxworks
	semGive (mySem);
#elif defined(WIN32)
	LeaveCriticalSection(&sync_lock);
#else /* __vxworks */
	pthread_mutex_unlock(&mutex);
#endif /* __vxworks */
#if _TRACE > 8
	std::cout << this << "\tMXLock::unlock() ... released!" << std::endl;
#endif
}

inline MXLock::MXLock() : lock_inited(false) {
}

inline MXLock::MXLock(const MXLock &) : lock_inited(false) { // ͬ�������ܿ�����
}
inline MXLock& MXLock::operator=(const MXLock &) {
	return *this; // �����Լ���ͬ�����󣬲�Ҫ������
}

/// �Զ��ͷŵ�����
/// \note ע��ʹ��AutoLock�ı���Ҫ��ȷ����������
/// ����ע�ⲻҪ������ʱ(����)��������Ϊ��ʱ����������ǰ����(����)�������𲻵���ϣ�������á�
class AutoLock {
	MXLock &lock_obj; // ע�������ǡ����á�����û�д����µ�������
public:
	/**
	 * ���캯��������ָ������
	 * \param lock ָ����
	 */
	AutoLock(MXLock &lock) :
		lock_obj(lock) {
		lock_obj.lock();
	}
	/**
	 * �����������ͷ���������������
	 */
	~AutoLock() {
		lock_obj.unlock();
	}
};

/// ����������������ʱ����
class CondEvent {
#ifdef __vxworks
	SEM_ID ev;
#elif defined(WIN32)
	HANDLE ev;
#else /* __vxworks */
	pthread_cond_t ev;
#endif
public:
	/// ���캯��
	/// \param initial_signal �����ĳ�ʼֵ
	CondEvent(bool initial_signal = false) {
#ifdef __vxworks
		ev = semBCreate (SEM_Q_PRIORITY, initial_signal ? SEM_FULL : SEM_EMPTY);
#elif defined(WIN32)
		ev = CreateEvent(NULL, false, initial_signal, NULL);
#else /* __vxworks */
		pthread_cond_init(&ev, NULL);
		if ( initial_signal )
			pthread_cond_signal(&ev);
#endif
	}

	~CondEvent() {
#ifdef __vxworks
		semDelete (ev);
#elif defined(WIN32)
		CloseHandle(ev);
#else /* __vxworks */
		pthread_cond_destroy(&ev);
#endif /* __vxworks */
	}

	/**
	 * ��������.
	 *
	 * ��������ֱ���������㡣
	 * �������ʱ����Ϊ�棬��ֱ������ָ�����������򣬹����������
	 * �ȴ�������Ϊ��(�ȴ�ʱ�������)��Ϊ�˱������޵ȴ����������ó�ʱֵ��
	 * ����ڹ涨��ʱ����������Ϊ�棬����ָ������������ֱ�ӷ��ء�
	 *
	 * \param mutex ָ������������ʱ�������Ϊ�棬�����������������߿�ֱ�Ӳ���������������
	 * \param time �ȵ����ʱ�䣬��λ΢��
	 * \return �ȴ��������Ƿ�Ϊ�档����Ϊ��ʱ������ָ��������Ϊ��ʱ��������
	 * \exception os_error �ײ����ϵͳ�����׳����쳣�������˴���ԭ��
	 */
	bool wait(MXLock *mutex, unsigned int time_out) {
#ifdef __vxworks
		if ( mutex )
			mutex->unlock();
		// time_out is in microseconds, so...
		int ticks = static_cast<int>(static_cast<double>(time_out) * sysClkRateGet() / 1000000);
		int status = semTake(ev, time_out ? ticks < 1 ? 1 : ticks : WAIT_FOREVER);
		if ( mutex )
			mutex->lock();
		return status != ERROR;
#elif defined(WIN32)
		if ( mutex )
			mutex->unlock();
		DWORD wait_ret = WaitForSingleObject(ev, (time_out + 999) / 1000);
		if ( mutex )
			mutex->lock();
		return wait_ret == WAIT_OBJECT_0;
#elif defined(__linux__)
		if ( time_out > 0 ) {
			timespec ts;
			::clock_gettime(CLOCK_REALTIME, &ts);
			ts.tv_sec += time_out / 1000000;
			unsigned long nsec = ts.tv_nsec + (time_out % 1000000) * 1000;
			ts.tv_nsec = nsec % 1000000000;
			ts.tv_sec += nsec / 1000000000;
			if ( pthread_cond_timedwait(&ev, mutex->get_mutex(), &ts) )
				return errno != ETIMEDOUT;
			return true;
		} else {
			pthread_cond_wait(&ev, mutex->get_mutex());
			return true;
		}

#else /* __vxworks */
		if ( time_out > 0 ) {
			timespec ts = {0, time_out * 1000};
			if ( pthread_cond_timedwait(&ev, mutex->get_mutex(), &ts) )
				return errno != ETIMEDOUT;
			return true;
		} else {
			pthread_cond_wait(&ev, mutex->get_mutex());
			return true;
		}
#endif /* __vxworks */
	}

	/**
	 * \fn signal
	 * ��������Ϊ�棬���ͷ����ڵȴ�����������
	 */
#ifdef __vxworks
	void signal() {
		semGive(ev);
	}
#elif defined(WIN32)
	void signal() {
		SetEvent(ev);
	}
#else /* __vxworks */
	void signal() {
		pthread_cond_signal(&ev);
	}

	/// ��������Ϊ�棬���ͷ��������ڵȴ�����������
	void broadcast() {
		pthread_cond_broadcast(&ev);
	}
#endif /* __vxworks */
};

/**
 * \class SyncEvent
 * ͬ���¼�
 *
 * \fn SyncEvent::wait(unsigned int)
 * �ȴ�ͬ��.
 * �ȴ�ͬ���¼���ɡ�
 * \param time_out �ȴ����ʱ�䣬��λ�����롣0��ʾ��Զ�ȴ���
 * \return �Ƿ�ͬ���������ʱָ��ʱ�䣬��ͬ�����ء�
 *
 * \fn SyncEvent::signal
 * ͬ��.
 * ͬ��������ɣ�֪ͨҪ��ͬ�����������������
 */
#ifdef __vxworks
class SyncEvent {
	SEM_ID ev;
public:
	SyncEvent() : ev(semBCreate(SEM_Q_PRIORITY, SEM_EMPTY)) {}
	bool wait(unsigned int time_out/*microsecond*/ = 0) {
		if ( time_out == 0 )
			return semTake(ev, WAIT_FOREVER) != ERROR;
		int ticks = static_cast<int>(static_cast<double>(time_out) * sysClkRateGet() / 1000000);
		return semTake(ev, ticks < 1 ? 1 : ticks) != ERROR;
	}

	void signal() {
		semGive(ev);
	}

	~SyncEvent() {
		semDelete (ev);
	}
};
#elif defined(WIN32)
class SyncEvent {
	HANDLE ev;
public:
	SyncEvent() {
		ev = CreateEvent(NULL, false, false, NULL);
	}

	bool wait(unsigned int time_out/*microsecond*/ = 0) {
		return WaitForSingleObject(ev, time_out == 0 ? INFINITE : (time_out + 999) / 1000) == WAIT_OBJECT_0;
	}

	void signal() {
		SetEvent(ev);
	}

	~SyncEvent() {
		CloseHandle(ev);
	}
};
#else
class SyncEvent : protected CondEvent {
	MXLock mutex;
public:
	SyncEvent() : CondEvent(false) {}

	bool wait(unsigned int time_out/*microsecond*/ = 0) {
		AutoLock al(mutex);
		return CondEvent::wait(&mutex, time_out);
	}

	void signal() {
		return CondEvent::signal();
	}
};
#endif

#endif // __THREAD_SYNC_UTIL_H_
