#include "util.h"

CProcess::CProcess(){}

CProcess::~CProcess(){}


pid_t CProcess::CreateChild() throw(CProcessException)
{
	pid_t pid = 0;
	pid = fork();
	if (pid < 0) {
		throw CProcessException(strerror(errno));
		return -1;
	}

	if (pid > 0) {
		return _cid = pid;
	} else if (pid == 0) {
		//子进程
		OnChildRun();
	}
	return pid;
}


CThread::CThread()
{
	m_thread_id = 0;
}

CThread::~CThread()
{
	log("cthread die\n");
	//pthread_exit(NULL);
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

	pthread_create(&m_thread_id, &attr, StartRoutine, this);

	return m_thread_id;
}

bool CThread::checkThreadAlive(pthread_t tid)
{
	int kill_rc = pthread_kill(tid, 0);
	return kill_rc != ESRCH;
}

void create_thread_lock(struct CThreadLock *lock)
{
	pthread_mutexattr_init(&lock->m_mutexattr);
	//类似于 threading.RLock @see http://www.cnblogs.com/huxi/archive/2010/06/26/1765808.html
	//pthread_mutexattr_settype(&lock->m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
	//@see http://peng-wp.iteye.com/blog/1616637
	pthread_mutexattr_setpshared(&lock->m_mutexattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&lock->m_mutex, &lock->m_mutexattr);

	pthread_condattr_init(&lock->cond_attr);
	pthread_condattr_setpshared(&lock->cond_attr, PTHREAD_PROCESS_SHARED);

	pthread_cond_init(&lock->cond, &lock->cond_attr);
}

void destory_thread_lock(struct CThreadLock *lock)
{
	pthread_mutexattr_destroy(&lock->m_mutexattr);
	pthread_mutex_destroy(&lock->m_mutex);

	pthread_condattr_destroy(&lock->cond_attr);
	pthread_cond_destroy(&lock->cond);
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
	static int file_no = 0;
	static FILE* log_fp = NULL;
	if (log_fp == NULL)
	{
		log_fp = fopen(LOG_FILE_NAME, "w");
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
		char bak_log_name[200] = {0};
		LOG_APPEND_NAME(bak_log_name);
		rename(LOG_FILE_NAME, bak_log_name);
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

/*
 * 	@desc 剔除换行符
 *
 *	\r\n windows 换行
 *	\n 类unix 换行
 *	\r 其他
*/
char *stripLineSep(char *str)
{
	if (str == NULL) return str;

	int len = 0;
	while ((len = strlen(str))) {
		if (str[len - 1] == '\r' || str[len - 1] == '\n') {
			str[len - 1] = '\0';
		} else {
			break;
		}
	}
	return str;
}

//获取本机ip
char *getLocalIp()
{
	static char ip[20] = {0};
	if (strlen(ip)) return ip;

	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	//和服务端打个招呼
	int clientSocket;
	struct sockaddr_in serverAddr;

	//创建个socket
	if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return NULL;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(53);
	serverAddr.sin_addr.s_addr = inet_addr("114.114.114.114");

	//连接服务端
	if(connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		return NULL;
	}

	if (getsockname(clientSocket, (struct sockaddr *)&clientAddr, &clientLen) < 0) {
		return NULL;
	}
	inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));

	return ip;
}

//根据进程号获取父进程id
pid_t getParentProcessIdByChildId(pid_t childId)
{
	char filePath[100] = {0};
	char commandLine[100] = {0};
	FILE *fp = NULL;
	char buffer[100] = {0};

	sprintf(filePath, "/proc/%d/status", childId);

	//进程不存在
	if (access(filePath, F_OK))
			return -1;

	sprintf(commandLine, "ps -p %d -o ppid=", childId);
	fp = popen(commandLine, "r");
	if (fp == NULL)
		return -1;
	fgets(buffer, sizeof(buffer), fp);
	pclose(fp);
	return	atoi(buffer);
}

/**
 * 设置daemon
 * 也可以系统自带的daemon函数    @see http://man7.org/linux/man-pages/man3/daemon.3.html
 * daemon(0, 0);
 */
void setDaemon(char *log_file)
{
	pid_t pid;

	pid = fork();
	if (pid < 0) {
		log("%s\n", "fork error");
		exit(EXIT_FAILURE);
	}
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
	//子进程
	if (setsid() == -1) {
		log("setsid error:%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	int log_fd = open(log_file, O_RDWR|O_CREAT|O_APPEND, 00644);
	if (log_fd == -1) {
		log("open log file %s error:%s\n", log_file, strerror(errno));
		exit(EXIT_FAILURE);
	}
	dup2(log_fd, STDIN_FILENO);
	dup2(log_fd, STDOUT_FILENO);
	dup2(log_fd, STDERR_FILENO);
	close(log_fd);

	chdir("/");
}

bool is_dir(const char *filePath)
{
	struct stat buf;
	int ret = -1;
	ret = stat(filePath, &buf);
	if (ret < 0) {
		return false;
	}

	return S_ISDIR(buf.st_mode);
}

//获取扩展  只是指向 不需要free(返回指针)
char *getExtension(const char *path)
{
	char *last_period;
	last_period = const_cast<char *>(strrchr(path, '.'));
	if (!last_period || strchr(last_period, '/'))
		return NULL;
	return last_period + 1;
}
