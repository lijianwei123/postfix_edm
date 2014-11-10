/*
 * worker.cpp
 *
 *  Created on: 2014-11-7
 *  Author: Administrator
 */
#include "worker.h"

//条件锁
shared_ptr<CThreadCondLock> condLock;
shared_ptr<CThreadCondLock> threadLock;


//数据库配置
mysql_connect_info_t  mysql_connect_info;
mysql_connection_info_init(&mysql_connect_info);

#define PULL_SENT_MAIL_UNIT 1000;

//@see http://blog.csdn.net/wangshihui512/article/details/8930652
//@see http://blog.csdn.net/liuxuejiang158blog/article/details/17301739
static queue<mail_info_t *> send_mail_queue;

cWorker * cWorker::instance = getInstance<cWorker>();
cWorker::cWorker()
{
	_status = 0;
	_delay = 0;
	_cont = 1;
}
cWorker::~cWorker(){}

void cWorker::run()
{
	condLock = make_shared<CThreadCondLock>();
	threadLock = make_shared<CThreadCondLock>();
	_doProduct();
	_doConsume();
}

void cWorker::done()
{

}

void cWorker::checkThreadAlive()
{
	if (!checkThreadAlive(workerThread[0])) {
		_doProduct();
	}

	if (!checkThreadAlive(workerThread[1])) {
		_doConsume();
	}
}

void cWorker::_doProduct()
{
	cWorkerPullSentInfo producer;
	workerThread[0] =  producer.CreateThread();
}

void cWorker::_doConsume()
{
	cWorkerSendMail  consumer;
	workerThread[1] = consumer.CreateThread();
}


cWorkerPullSentInfo::cWorkerPullSentInfo(){}
cWorkerPullSentInfo:: ~cWorkerPullSentInfo(){}
//生产者
void cWorkerPullSentInfo::OnThreadRun(void)
{
	while(true) {
		threadLock->Lock();
		//可以运行
		if (cWorker::instance->getStatus() & COMMAND_TO_ENUM(START) || cWorker::instance->getStatus() & COMMAND_TO_ENUM(RESUME)) {
			pullSentInfoFromMysql();

			//有数据的话
			if (send_mail_queue != NULL && send_mail_queue.size() > 0) {
				if (send_mail_queue.size() > PULL_SENT_MAIL_UNIT * 10) {
					threadLock->wait();
				}
			} else {
				cWorker::instance->done();
			}
		}
		threadLock->Unlock();
	}
}

void cWorkerPullSentInfo::pullSentInfoFromMysql()
{
	result_data_t result_data;
	MYSQL conn;
	int retCode = -1;

	retCode = mysql_connect(&conn, &mysql_connect_info);
	assert(retCode == 0);

	retCode = mysql_select_db(&conn, mysql_connect_info.db);
	assert(retCode == 0);

	unsigned long result = 0l;
	char select_sql[200] = {0};
	snprintf(select_sql, sizeof(select_sql), "select * from df_monitor_collection limit %d", PULL_SENT_MAIL_UNIT);

	retCode = mysql_select(&conn, select_sql, &result_data);

#ifdef DEBUG
	printf("rows:%d, cols:%d\n", result_data.rows, result_data.columns);
#endif

	mysql_close(&conn);

	int i = 0, j = 0;
	mysql_field_value_t *pointer;
	mail_info_t *mail_info_ptr = NULL;

	for (i = 0; i < result_data.rows; i++) {
		mail_info_ptr = calloc(1, sizeof(mail_info_t));
		pointer = *(result_data.data + i);
		for (j = 0; j < result_data.columns; j++) {
			if (strncasecmp(pointer->next->fieldName, "to", sizeof("to"))) {
				mail_info_ptr->to = strdup(pointer->next->fieldValue);
			}
			if (strncasecmp(pointer->next->fieldName, "cc", sizeof("cc"))) {
				mail_info_ptr->cc = strdup(pointer->next->fieldValue);
			}
			if (strncasecmp(pointer->next->fieldName, "subject", sizeof("subject"))) {
				mail_info_ptr->subject = strdup(pointer->next->fieldValue);
			}
			if (strncasecmp(pointer->next->fieldName, "content", sizeof("content"))) {
				mail_info_ptr->content = strdup(pointer->next->fieldValue);
			}
			if (strncasecmp(pointer->next->fieldName, "sender", sizeof("sender"))) {
				mail_info_ptr->sender = strdup(pointer->next->fieldValue);
			}
			if (strncasecmp(pointer->next->fieldName, "from", sizeof("from"))) {
				mail_info_ptr->from = strdup(pointer->next->fieldValue);
			}

			pointer = pointer->next;
		}
		send_mail_queue.push(mail_info_ptr);
	}
	free_result_data(&result_data);
}


cWorkerSendMail::cWorkerSendMail(){}
cWorkerSendMail::~cWorkerSendMail(){}
//消费者
void cWorkerSendMail::OnThreadRun(void)
{
	//搞出多个进程发送  共享内存  @see http://blog.csdn.net/guoping16/article/details/6584058
	//多进程同步   http://peng-wp.iteye.com/blog/1616637
	//master worker

	//创建共享内存
	srand((unsigned int)getpid());
	key_t ipc_shm_key = ftok("/dev/null", (int)'a');
	int shm_id = 0;
	shm_id = shmget(ipc_shm_key, sizeof(shm_shared_t), IPC_CREAT|0666);
	if (shm_id < 0) {
		log("shmget error: %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//初始化共享内存
	shm_shared_t *shm_ptr = (shm_shared_t *)shmat(shm_id, NULL, 0);
	if ((int)shm_ptr == -1) {
		log("work thread shmat error:%s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	shm_ptr->email_queue_ptr = &send_mail_queue;
	shm_ptr->condLock = condLock;
	shm_ptr->worker = cWorker::instance;

	//产生主进程
	pid_t master_pid = _forkMasterProcess();

	int status = 0;

	//监控主进程、监控队列数据
	while (true) {
		master_pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
		if (master_pid) {
			log("master pid: %d die\n", master_pid);
			_forkMasterProcess();
		}

		threadLock->Lock();
		if (send_mail_queue.size() < PULL_SENT_MAIL_UNIT) {
			threadLock->notify();
		}
		threadLock->Unlock();

		sleep(10);
	}
}

//产生主进程
pid_t cWorkerSendMail::_forkMasterProcess()
{
	pid_t master_pid = 0;

	try {
		shared_ptr<cMasterProcess> masterProcess = make_shared<cMasterProcess>();
		master_pid = masterProcess->forkChild();
	} catch(CProcessException &pe) {
		log("master fork fail: %s", pe.getMessage());
		exit(EXIT_FAILURE);
	}

	return master_pid;
}


cMasterProcess::cMasterProcess()
{
	childNum = 0;
}
cMasterProcess::~cMasterProcess(){}
void cMasterProcess::child()
{
	signal(SIGCHLD, cMasterProcess::signalHandler);

	shm_shared_t *shm_ptr = (shm_shared_t *)shmat(shm_id, NULL, 0);
	if ((int)shm_ptr == -1) {
		log("cMasterProcess shmat error:%s", strerror(errno));
		exit(EXIT_FAILURE);
	}

	uint16_t processNum = 0, delay = 0, incProcessNum = 0, reduceProcessNum = 0;
	int i = 0;
	hash_map<pid_t, shared_ptr<cChildProcess>> childProcessMap;
	hash_map<pid_t, shared_ptr<cChildProcess>>::iterator it;

	while (true) {
		//fork childs
		processNum = shm_ptr->worker->getCont();
		delay = shm_ptr->worker->getDelay();


		//进程调整
		if (childNum > processNum) {
			reduceProcessNum = childNum - processNum;
			//发送信号
			for(i = 0, it = childProcessMap.begin(); it != childProcessMap.end() && i < reduceProcessNum; ++it, ++i) {
				//子进程你可以退休了
				kill(it->first, SIGUSR1);
				--childNum;
			}
		} else {
			incProcessNum = processNum - childNum;
		}

		try {
			shared_ptr<cChildProcess> childProcess;

			int i = 0;
			for (i = 0; i < incProcessNum; ++i) {
				childProcess = make_shared<cChildProcess>();
				childProcess->forkChild();

				childProcessMap.insert(make_pair(childProcess->getCid(), childProcess));
				++childNum;
			}
		} catch (CProcessException &pe) {
			log("child fork fail: %s", pe.getMessage());
			exit(EXIT_FAILURE);
		}
	}


}
void cMasterProcess::signalHandler(int signalNo)
{
	log("signal no:%d", signalNo);

	pid_t chldPid = 0;
	int status = 0;

	chldPid = waitpid(-1, &status, WNOHANG | WUNTRACED);

	if (chldPid < 0) {
		log("waitpid error: %s", strerror(errno));
	} else {
		log("WIFEXITED:%d, WEXITSTATUS: %d, WIFSIGNALED: %d\n", WIFEXITED(status), WEXITSTATUS(status), WIFSIGNALED(status));

		if (WIFEXITED(status) && WEXITSTATUS(status) == SIGUSR1) {
			//这个就不减了
		} else {
			--childNum;
		}
	}
}


cChildProcess::cChildProcess()
{
	_sendMail = cSendMail::instance;
	_quit = false;
}
cChildProcess::~cChildProcess(){}
void cChildProcess::child()
{
	signal(SIGUSR1, cChildProcess::signalHandler);
	//工作子进程
	shm_shared_t *shm_ptr = (shm_shared_t *)shmat(shm_id, NULL, SHM_RDONLY);
	if ((int)shm_ptr == -1) {
		log("child process shmat error:%s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	uint16_t delay = 0;


	mail_info_t *mail_info_ptr = NULL;
	while (true) {
		if (_quit) {exit(SIGUSR1);}
		shm_ptr->condLock->Lock();
		if (shm_ptr->worker->getStatus() & COMMAND_TO_ENUM(START) || shm_ptr->worker->getStatus() & COMMAND_TO_ENUM(RESUME)) {
			if (shm_ptr->email_queue_ptr->size()) {
				mail_info_ptr = shm_ptr->email_queue_ptr->front();
				shm_ptr->email_queue_ptr->pop();
				_sendMail->send(mail_info_ptr);
				free_mail_info(mail_info_ptr);
			}
		}
		shm_ptr->condLock->Unlock();
		if (delay = shm_ptr->worker->getDelay()) {
			sleep(delay);
		}

	}
}

void cChildProcess::signalHandler(int signalNo)
{
	log("signal no:%d", signalNo);
	switch (signalNo) {
	case SIGUSR1:
			_quit = true;
		break;
	}
}
