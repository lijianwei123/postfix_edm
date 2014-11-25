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
#include <libgen.h> //dirname

#include <iconv.h>  //编码转换

#include <stdarg.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>


#include <assert.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>


//为了获取本机ip
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

//解析json字符串
#include "json/json.h"

#include "mysql.h"
#include "cJSON.h"
#include "prename.h"

#include "base64.h"




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
	pid_t CreateChild() throw(CProcessException);
	virtual void OnChildRun() = 0;

	pid_t getCid(){return _cid;}
private:
	pid_t _cid;
};


class CThread
{
public:
	CThread();
	virtual ~CThread();

	static void* StartRoutine(void* arg);
	virtual pthread_t CreateThread(void);
	virtual void OnThreadRun(void) = 0;

public:
	//检测线程是否还活着   @see http://blog.csdn.net/echoisland/article/details/6398081
	static bool checkThreadAlive(pthread_t tid);

protected:
	pthread_t	m_thread_id;
};

//@see http://www.cnblogs.com/zhangchaoyang/articles/2302085.html
//@see http://baike.baidu.com/view/5725833.htm?fr=aladdin
struct CThreadLock
{
	pthread_mutex_t 	m_mutex;
	pthread_mutexattr_t	m_mutexattr;

	pthread_cond_t cond;
	pthread_condattr_t cond_attr;

	void Lock(void)
	{
		pthread_mutex_lock(&m_mutex);
	}

	void Unlock(void)
	{
		pthread_mutex_unlock(&m_mutex);
	}

	void wait()
	{
		pthread_cond_wait(&cond, &m_mutex);
	}

	void timedWait(struct timespec *outtime)
	{
		if (outtime == NULL) {
			struct timespec spec;
			spec.tv_sec = time(NULL) + 1;
			spec.tv_nsec = 0;
			outtime = &spec;
		}
		pthread_cond_timedwait(&cond, &m_mutex, outtime);
	}

	void notify()
	{
		pthread_cond_signal(&cond);
	}

	void notifyAll()
	{
		pthread_cond_broadcast(&cond);
	}
};


void create_thread_lock(struct CThreadLock *lock);

void destory_thread_lock(struct CThreadLock *lock);

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

extern const char *LOG_FILE_NAME;

#define LOG_APPEND_NAME(new_log_name)	\
	char *clone1 = strdup(LOG_FILE_NAME);	\
	char *clone2 = strdup(LOG_FILE_NAME);	\
	char *p1 = strrchr(clone1, '.');	\
	char *p2 = strrchr(clone2, '.');	\
	*p1 = '\0';	\
	sprintf(new_log_name, "%s_%d.%s", clone1, file_no, ++p2); \
	free(clone1);	\
	free(clone2);

#define log(fmt, args...)  logger("<%s>|<%d>|<%s>," fmt, __FILE__, __LINE__, __FUNCTION__, ##args)

void logger(const char* fmt, ...);


uint64_t get_tick_count();
void util_sleep(uint32_t millisecond);


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


//编码转换  @see http://jazka.blog.51cto.com/809003/231917
class CodeConverter
{
private:
	iconv_t cd;
public:
	// 构造
	CodeConverter(const char *from_charset,const char *to_charset)
	{
		cd = iconv_open(to_charset,from_charset);
	}

	// 析构
	~CodeConverter()
	{
		iconv_close(cd);
	}

	//转换输出
	int convert(char *inbuf, int inlen, char *outbuf, int outlen)
	{
		char **pin = &inbuf;
		char **pout = &outbuf;

		memset(outbuf, 0, outlen);
		return iconv(cd, pin, (size_t *)&inlen, pout, (size_t *)&outlen);
	}
};

//剔除换行符
char *stripLineSep(char *str);

//获取本机ip
char *getLocalIp();

pid_t getParentProcessIdByChildId(pid_t childId);

//设置daemon
void setDaemon(char *log_file);

bool is_dir(const char *filePath);

char *getExtension(const char *path);

#endif
