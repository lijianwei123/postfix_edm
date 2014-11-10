#ifndef __UTIL_H__
#define __UTIL_H__

#define _CRT_SECURE_NO_DEPRECATE	// remove warning C4996, 

//use boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
using namespace boost;

#include "ostype.h"
#include "UtilPdu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdarg.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>


#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#include "mysql.h"




#include <ext/hash_map>
using namespace __gnu_cxx;

#define MAX_LOG_FILE_SIZE	0x4000000	// 64MB
#define NOTUSED_ARG(v) ((void)v)		// used this to remove warning C4100, unreferenced parameter

class CProcessException
{
public:
	CProcessException(const char *message)
	{
		_message = message;
	}
	const char *getMessage() const { return _message;}
private:
	const char *_message;
};

//进程类
class CProcess
{
public:
	CProcess();
	virtual ~CProcess();
	pid_t forkChild();
	virtual void child() = 0;

	pid_t getCid(){return _cid;}
private:
	pid_t _cid;
};

//检测线程是否还活着   @see http://blog.csdn.net/echoisland/article/details/6398081
bool checkThreadAlive(pthread_t tid);

class CThread
{
public:
	CThread();
	virtual ~CThread();

	static void* StartRoutine(void* arg);
	virtual pthread_t CreateThread(void);
	virtual void OnThreadRun(void) = 0;

protected:
	pthread_t	m_thread_id;
};

class CThreadLock
{
public:
	CThreadLock();
	~CThreadLock();
	void Lock(void);
	void Unlock(void);
protected:
	pthread_mutex_t 	m_mutex;
private:
	pthread_mutexattr_t	m_mutexattr;
};

//@see http://www.cnblogs.com/zhangchaoyang/articles/2302085.html
//@see http://baike.baidu.com/view/5725833.htm?fr=aladdin
class CThreadCondLock : public CThreadLock
{
public:
	CThreadCondLock();
	~CThreadCondLock();

	void wait();
	void timedWait(struct timespec *outtime);
	void notify();
	void notifyAll();


private:
	pthread_cond_t cond;
};

class CFuncLock
{
public:
	CFuncLock(CThreadLock* lock) 
	{ 
		m_lock = lock; 
		if (m_lock)
			m_lock->Lock(); 
	}

	~CFuncLock() 
	{ 
		if (m_lock)
			m_lock->Unlock(); 
	}
private:
	CThreadLock*	m_lock;
};

class CRefObject
{
public:
	CRefObject();
	virtual ~CRefObject();

	void SetLock(CThreadLock* lock) { m_lock = lock; }
	void AddRef();
	void ReleaseRef();
private:
	int				m_refCount;
	CThreadLock*	m_lock;
};

//单例
template<class T>
T *getInstance()
{
	static T instance;
	return &instance;
}


#define log(fmt, args...)  logger("<%s>|<%d>|<%s>," fmt, __FILE__, __LINE__, __FUNCTION__, ##args)

void logger(const char* fmt, ...);


uint64_t get_tick_count();
void util_sleep(uint32_t millisecond);


#ifdef __cplusplus
extern "C" {
#endif

//rtrim
char *strtrimr(char *pstr);
//ltrim
char *strtriml(char *pstr);
//trim
char *strtrim(char *pstr);

#ifdef __cplusplus
}
#endif

class CStrExplode
{
public:
	CStrExplode(char* str, char seperator);
	virtual ~CStrExplode();

	uint32_t GetItemCnt() { return m_item_cnt; }
	char* GetItem(uint32_t idx) { return m_item_list[idx]; }
private:
	uint32_t	m_item_cnt;
	char** 		m_item_list;
};

#endif
