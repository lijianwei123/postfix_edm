#include "util.h"

CProcess::CProcess(){}

CProcess::~CProcess(){}


pid_t CProcess::forkChild() throw(CProcessException)
{
	pid_t pid = 0;
	pid = fork();
	if (pid < 0) {
		throw CProcessException(strerror(errno));
	}

	if (pid > 0) {
		return _cid = pid;
	} else if (pid == 0) {
		//子进程
		child();
	}
}

bool checkThreadAlive(pthread_t tid)
{
	int kill_rc = pthread_kill(tid, 0);
	return kill_rc != ESRCH;
}

CThread::CThread()
{
	m_thread_id = 0;
}

CThread::~CThread()
{
	pthread_exit(NULL);
}

void* CThread::StartRoutine(void* arg)
{
	CThread* pThread = (CThread*)arg;
	
	pThread->OnThreadRun();

	return NULL;
}

pthread_t CThread::CreateThread()
{
	//防止僵尸线程
	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

	(void)pthread_create(&m_thread_id, &attr, StartRoutine, this);

	return m_thread_id;
}

/////////// CThreadLock ///////////
CThreadLock::CThreadLock()
{
	pthread_mutexattr_init(&m_mutexattr);
	//类似于 threading.RLock @see http://www.cnblogs.com/huxi/archive/2010/06/26/1765808.html
	pthread_mutexattr_settype(&m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
	//@see http://peng-wp.iteye.com/blog/1616637
	pthread_mutexattr_setpshared(&m_mutexattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&m_mutex, &m_mutexattr);
}

CThreadLock::~CThreadLock()
{
	pthread_mutexattr_destroy(&m_mutexattr);
	pthread_mutex_destroy(&m_mutex);
}

void CThreadLock::Lock(void)
{
	pthread_mutex_lock(&m_mutex);
}

void CThreadLock::Unlock(void)
{
	pthread_mutex_unlock(&m_mutex);
}

/////////// CThreadCondLock ///////////
CThreadCondLock::CThreadCondLock()
{
	cond = PTHREAD_COND_INITIALIZER;
}

CThreadCondLock::~CThreadCondLock()
{
	pthread_cond_destroy(&cond);
}

void CThreadCondLock::wait()
{
	pthread_cond_wait(&cond, &m_mutex);
}

void CThreadCondLock::timedWait(struct timespec *outtime)
{
	if (outtime == NULL) {
		struct timespec spec;
		spec.tv_sec = time(NULL) + 1;
		spec.tv_nsec = 0;
		outtime = &spec;
	}
	pthread_cond_timedwait(&cond, &m_mutex, outtime);
}

void CThreadCondLock::notify()
{
	pthread_cond_signal(&cond);
}

void CThreadCondLock::notifyAll()
{
	pthread_cond_broadcast(&cond);
}


CRefObject::CRefObject()
{
	m_lock = NULL;
	m_refCount = 1;
}

CRefObject::~CRefObject()
{
}

void CRefObject::AddRef()
{
	if (m_lock)
	{
		m_lock->Lock();
		m_refCount++;
		m_lock->Unlock();
	}
	else
	{
		m_refCount++;
	}
}

void CRefObject::ReleaseRef()
{
	if (m_lock)
	{
		m_lock->Lock();
		m_refCount--;
		if (m_refCount == 0)
		{
			delete this;
			return;
		}
		m_lock->Unlock();
	}
	else
	{
		m_refCount--;
		if (m_refCount == 0)
			delete this;
	}
}



static void print_format_time(FILE* fp)
{

	struct timeval tval;
	struct tm* tm;
	time_t currTime;

	time(&currTime);
	tm = localtime(&currTime);
	gettimeofday(&tval, NULL);
	fprintf(fp, "%04d-%02d-%02d, %02d:%02d:%02d.%03d, ", 1900 + tm->tm_year, 1 + tm->tm_mon,
		tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int)tval.tv_usec / 1000);
}

void logger(const char* fmt, ...)
{
	static int file_no = 1;
	static FILE* log_fp = NULL;
	if (log_fp == NULL)
	{
		char log_name[64];
		uint32_t pid = 0;

		pid = (uint32_t)getpid();

		snprintf(log_name, 64, "log_%d.txt", file_no);
		log_fp = fopen(log_name, "w");
		if (log_fp == NULL)
			return;
	}



	print_format_time(log_fp);

	va_list ap;
	va_start(ap, fmt);
	vfprintf(log_fp, fmt, ap);
	va_end(ap);
	fflush(log_fp);

	if (ftell(log_fp) > MAX_LOG_FILE_SIZE)
	{
		fclose(log_fp);
		log_fp = NULL;
		file_no++;
	}
}

uint64_t get_tick_count()
{

	struct timeval tval;
	uint64_t ret_tick;

	gettimeofday(&tval, NULL);

	ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
	return ret_tick;
}

void util_sleep(uint32_t millisecond)
{
	usleep(millisecond * 1000);
}


#ifdef __cplusplus
extern "C" {
#endif
//rtrim
char *strtrimr(char *pstr)
{
	int i;
	i = strlen(pstr) - 1;
	while((i >=0) && isspace(pstr[i])){
		pstr[i--] = '\0';
	}
	return pstr;
}

//ltrim
char *strtriml(char *pstr)
{
	int i=0,j;
	j = strlen(pstr) - 1;
	while(isspace(pstr[i]) && (i <= j))
		i++;
	if(0 < i)
		strcpy(pstr, &pstr[i]);
	return pstr;
}

//trim
char *strtrim(char *pstr)
{
	char *p;
	p = strtrimr(pstr);
	return strtriml(p);
}
#ifdef __cplusplus
}
#endif



CStrExplode::CStrExplode(char* str, char seperator)
{
	m_item_cnt = 1;
	char* pos = str;
	while (*pos) {
		if (*pos == seperator) {
			m_item_cnt++;
		}

		pos++;
	}

	m_item_list = new char* [m_item_cnt];

	int idx = 0;
	char* start = pos = str;
	while (*pos) {
		if (*pos == seperator) {
			uint32_t len = pos - start;
			m_item_list[idx] = new char [len + 1];
			strncpy(m_item_list[idx], start, len);
			m_item_list[idx][len]  = '\0';
			idx++;

			start = pos + 1;
		}

		pos++;
	}

	uint32_t len = pos - start;
	m_item_list[idx] = new char [len + 1];
	strncpy(m_item_list[idx], start, len);
	m_item_list[idx][len]  = '\0';
}

CStrExplode::~CStrExplode()
{
	for (uint32_t i = 0; i < m_item_cnt; i++) {
		delete [] m_item_list[i];
	}

	delete [] m_item_list;
}

