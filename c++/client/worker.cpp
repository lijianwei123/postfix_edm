/*
 * 	worker.cpp
 *
 * 	@see http://blog.csdn.net/wangshihui512/article/details/8930652
 * 	@see http://blog.csdn.net/liuxuejiang158blog/article/details/17301739
 *
 *  Created on: 2014-11-7
 *  Author: lijianwei
 */
#include "worker.h"

//共享内存
shm_shared_t *shm_ptr = NULL;
static int shm_id = 0;
static pid_t master_pid = 0;

//标识子进程是否退掉
static int child_quit = 0;

//数据库配置
mysql_connect_info_t  mysql_connect_info;
char *emailFromAddr = NULL;
int changeProcessName = 0;
int isDaemon = 0;


cWorker * cWorker::instance = getInstance<cWorker>();
cWorker::cWorker()
{
	_status = 0;
	_delay = 0;
	_multi = 1;
}
cWorker::~cWorker()
{
	shmctl(shm_id, IPC_RMID, NULL);
}

void cWorker::setStatus(uint16_t status)
{
	log("worker setStatus: %d\n", status);
	_status = status;

	shm_ptr->status = status;
	if (CHECK_CAN_START(shm_ptr)) {
		log("commandLock notifyAll....\n");

		COND_LOCK_NOTIFYALL(shm_ptr->commandLock);
	}
}

void cWorker::setDelay(uint16_t delay)
{
	_delay = delay;
	shm_ptr->delay = delay;
}


void cWorker::setMulti(uint16_t multi)
{
	_multi = multi;
	shm_ptr->multi = multi;
}

void cWorker::run()
{
	_createThread();
}

void cWorker::done()
{
	//给服务端发送个消息
	CImPduClientDone  pdu;
	send_server_pdu(&pdu);

}

void cWorker::checkThreadAlive()
{
	if (!CThread::checkThreadAlive(worker_thread_id)) {
		_createThread();
	}
}

int64_t cWorker::GetMailqNum()
{
	int ret = -1;
	long byteNum = 0;
	long mailqNum = 0;


	FILE *mailFp = NULL;
	const char *command = "/usr/bin/mailq 2>/dev/null |tail -n 1";
	char lastLineStr[1024] = {0};
	mailFp = popen(command, "r");
	if (mailFp == NULL) {
		return 0;
	}
	fgets(lastLineStr, 1024, mailFp);
	pclose(mailFp);

	if (strstr(lastLineStr, "empty")) {
		return 0;
	}

	ret = sscanf(lastLineStr, "-- %ld Kbytes in %ld Requests.", &byteNum, &mailqNum);
	if (ret == -1) {
		return 0;
	}

	return mailqNum;
}


void cWorker::_createThread()
{
	worker_thread_ptr = make_shared<cWorkerThread>();
	worker_thread_id = worker_thread_ptr->CreateThread();
}

//=============================================================================

cWorkerThread::cWorkerThread()
{
	_init_shm();
}
cWorkerThread::~cWorkerThread()
{

}

/**
 * 多进程 共享内存	@see http://blog.csdn.net/guoping16/article/details/6584058
 * 多进程同步  		@see http://peng-wp.iteye.com/blog/1616637
 */
void cWorkerThread::OnThreadRun(void)
{
	//第一次调用
	//或者第二次调用   master_pid 已经die了
	if (master_pid == 0 || (master_pid > 0 && getParentProcessIdByChildId(master_pid) != getpid())) {
		//产生主进程
		master_pid = _forkMasterProcess();
	}

	pid_t die_master_pid = 0;
	int status = 0;

	//notify
	while (true) {

		//监控主进程
		die_master_pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
		if (die_master_pid) {
			log("master pid: %d die\n", die_master_pid);
			_forkMasterProcess();
		}

		if (CHECK_CAN_START(shm_ptr)) {
			shm_ptr->productLock.Lock();
			if (_haveSendInfo()) {

				COND_LOCK_NOTIFYALL(shm_ptr->consumeLock)

				shm_ptr->productLock.wait();
			}
			shm_ptr->productLock.Unlock();

			sleep(CHECK_MAIL_DATA_INTERVAL);
		} else {
			log("%p\n", &shm_ptr->commandLock);
			shm_ptr->commandLock.Lock();
			shm_ptr->commandLock.wait();
			shm_ptr->commandLock.Unlock();
		}
	}
}

bool cWorkerThread::_haveSendInfo()
{
	bool haveSendInfo = false;

	result_data_t result_data;
	mysql_field_value_t *pointer = NULL;
	MYSQL conn;
	int retCode = -1;

	retCode = mysql_user_connect(&conn, &mysql_connect_info);
	assert(retCode == 0);

	retCode = mysql_select_db(&conn, mysql_connect_info.db);
	assert(retCode == 0);

	char select_sql[200] = {0};
	snprintf(select_sql, sizeof(select_sql), "select cout(`to`) from send_email where status = 1 and from = '%s' group by `to` limit %d", emailFromAddr, PULL_SENT_MAIL_UNIT);

	retCode = mysql_select(&conn, select_sql, &result_data);

	log("rows:%d, cols:%d\n", result_data.rows, result_data.columns);
	mysql_close(&conn);

	if (result_data.rows > 0) {
		pointer = *(result_data.data);
		haveSendInfo = pointer->next->fieldValue;
	}
	free_result_data(&result_data);

	return haveSendInfo;
}


void cWorkerThread::_init_shm()
{
	//创建共享内存
	srand((unsigned int)getpid());
	key_t ipc_shm_key = ftok("/dev/null", (int)'a');
	shm_id = shmget(ipc_shm_key, sizeof(shm_shared_t), IPC_CREAT|0666);
	if (shm_id < 0) {
		log("shmget error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//初始化共享内存
	shm_ptr = (shm_shared_t *)shmat(shm_id, NULL, 0);
	if (shm_ptr == (shm_shared_t *)-1) {
		log("work thread shmat error:%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(shm_ptr, 0, sizeof(shm_shared_t));
	shm_ptr->status = cWorker::instance->getStatus();
	shm_ptr->multi = cWorker::instance->getMulti();
	shm_ptr->delay = cWorker::instance->getDelay();
	shm_ptr->havePushNum = 0;

	//初始化锁
	create_thread_lock(&shm_ptr->childLock);
	create_thread_lock(&shm_ptr->commandLock);
	create_thread_lock(&shm_ptr->productLock);
	create_thread_lock(&shm_ptr->consumeLock);

	log("shm_ptr status: %d\n", shm_ptr->status);
}



//产生主进程
pid_t cWorkerThread::_forkMasterProcess()
{
	pid_t master_pid = 0;

	if (cMasterProcess::childProcessMap.size() > 0) {
		cMasterProcess::childProcessMap.clear();
		cMasterProcess::childNum = 0;
	}

	try {
		shared_ptr<cMasterProcess> masterProcess = make_shared<cMasterProcess>();
		master_pid = masterProcess->CreateChild();
	} catch(CProcessException &pe) {
		log("master fork fail: %s\n", pe.getMessage());
		exit(EXIT_FAILURE);
	}

	return master_pid;
}


void sigChldHandler(int signalNo)
{
	log("signal no:%d\n", signalNo);

	pid_t chldPid = 0;
	int status = 0;

	chldPid = waitpid(-1, &status, WNOHANG | WUNTRACED);

	if (chldPid < 0) {
		log("waitpid error: %s\n", strerror(errno));
	} else {
		log("WIFEXITED:%d, WEXITSTATUS: %d, WIFSIGNALED: %d, WTERMSIG: %d\n", WIFEXITED(status), WEXITSTATUS(status), WIFSIGNALED(status), WTERMSIG(status));

		if (WIFEXITED(status) && WEXITSTATUS(status) == SIGUSR1) {
			//这个就不减了
		} else {
			cMasterProcess::childProcessMap.erase(chldPid);
			--cMasterProcess::childNum;
		}
	}
}

uint16_t cMasterProcess::childNum = 0;
hash_map<pid_t, shared_ptr<cChildProcess> > cMasterProcess::childProcessMap;

cMasterProcess::cMasterProcess()
{
}
cMasterProcess::~cMasterProcess()
{
}

void cMasterProcess::OnChildRun()
{
	if (changeProcessName) prename_setproctitle("%s", "postfix_edm_client master");
	signal(SIGCHLD, sigChldHandler);

	//INIT_RDONLY_SHMAT

	uint16_t processNum = 0, incProcessNum = 0, reduceProcessNum = 0;
	int i = 0;
	hash_map<pid_t, shared_ptr<cChildProcess> >::iterator it, it_old;

	while (true) {
		if (CHECK_CAN_START(shm_ptr)) {
			log("shm_ptr status: %d\n", shm_ptr->status);
			//fork childs
			processNum = shm_ptr->multi;

			//进程调整
			if (childNum > processNum) {
				reduceProcessNum = childNum - processNum;
				if (reduceProcessNum) {
					//发送信号
					for(i = 0, it = childProcessMap.begin(); it != childProcessMap.end() && i < reduceProcessNum; ++i) {
						it_old = it;
						++it;
						//子进程你可以退休了
						kill(it_old->first, SIGUSR1);
						--childNum;
						childProcessMap.erase(it_old->first);
					}
				}
			} else {
				incProcessNum = processNum - childNum;

				if (incProcessNum) {
					try {
						shared_ptr<cChildProcess> childProcess;

						int i = 0;
						for (i = 0; i < incProcessNum; ++i) {
							childProcess = make_shared<cChildProcess>();
							childProcess->CreateChild();

							childProcessMap.insert(make_pair(childProcess->getCid(), childProcess));
							++childNum;
						}
					} catch (CProcessException &pe) {
						log("child fork fail: %s\n", pe.getMessage());
						exit(EXIT_FAILURE);
					}
				}
			}
			sleep(CHECK_PROCESSNUM_INTERVAL);

		} else {
			log("%p\n", &shm_ptr->commandLock);
			shm_ptr->commandLock.Lock();
				shm_ptr->commandLock.wait();
			shm_ptr->commandLock.Unlock();
		}

	}


}

cChildProcess::cChildProcess()
{
	_sendMail = cSendMail::instance;
	child_quit = 0;
}
cChildProcess::~cChildProcess()
{
}


static void sigUsr1Handler(int signalNo)
{
	log("signal no:%d\n", signalNo);
	switch (signalNo) {
	case SIGUSR1:
		child_quit = true;
		break;
	}
}


void cChildProcess::OnChildRun()
{
	if (changeProcessName) prename_setproctitle("%s", "postfix_edm_client child");
	//kill -s USR1  pid  或者  kill -n 12  pid  或者 kill -12 pid   或者   kill -USR1 pid  参看 kill
	signal(SIGUSR1, sigUsr1Handler);
	signal(SIGCHLD, SIG_DFL);

	//工作子进程
	//INIT_RDONLY_SHMAT

	uint16_t delay = 0;

	mail_info_t *mail_info_ptr = NULL;
	while (true) {

		if (child_quit || getppid() == 1) {
			exit(SIGUSR1);
		}

		if (CHECK_CAN_START(shm_ptr)) {

			shm_ptr->childLock.Lock();
			pullSentInfoFromMysql();
			shm_ptr->havePushNum += send_mail_queue.size();
			shm_ptr->childLock.Unlock();

			if (!send_mail_queue.empty()) {
				//有数据的话
				while (!send_mail_queue.empty()) {
					mail_info_ptr = send_mail_queue.front();
					send_mail_queue.pop();
					_sendMail->send(mail_info_ptr);
					free_mail_info(mail_info_ptr);
				}
				delay = shm_ptr->delay;
				if (delay) {
					sleep(delay);
				}
			} else {
				cWorker::instance->done();

				shm_ptr->consumeLock.Lock();
				COND_LOCK_NOTIFYALL(shm_ptr->productLock)
				shm_ptr->consumeLock.wait();
				shm_ptr->consumeLock.Unlock();


			}
		} else {
			shm_ptr->commandLock.Lock();
			shm_ptr->commandLock.wait();
			shm_ptr->commandLock.Unlock();

		}
	}
}

void cChildProcess::pullSentInfoFromMysql()
{
	result_data_t result_data;
	MYSQL conn;
	int retCode = -1;

	retCode = mysql_user_connect(&conn, &mysql_connect_info);
	assert(retCode == 0);

	retCode = mysql_select_db(&conn, mysql_connect_info.db);
	assert(retCode == 0);

	char select_sql[200] = {0};
	snprintf(select_sql, sizeof(select_sql), "select * from send_email where status = 1 and from = '%s' group by `to` limit %d", emailFromAddr, PULL_SENT_MAIL_UNIT);

	retCode = mysql_select(&conn, select_sql, &result_data);

	log("rows:%d, cols:%d\n", result_data.rows, result_data.columns);

	mysql_close(&conn);

	unsigned int i = 0, j = 0;
	mysql_field_value_t *pointer;
	mail_info_t *mail_info_ptr = NULL;

	for (i = 0; i < result_data.rows; i++) {
		mail_info_ptr = (mail_info_t *)calloc(1, sizeof(mail_info_t));
		pointer = *(result_data.data + i);
		for (j = 0; j < result_data.columns; j++) {
			if (!strncasecmp(pointer->next->fieldName, "to", sizeof("to")) && pointer->next->fieldValue) {
				mail_info_ptr->to = strdup(pointer->next->fieldValue);
			} else if (!strncasecmp(pointer->next->fieldName, "cc", sizeof("cc"))  && pointer->next->fieldValue) {
				mail_info_ptr->cc = strdup(pointer->next->fieldValue);
			} else if (!strncasecmp(pointer->next->fieldName, "subject", sizeof("subject"))  && pointer->next->fieldValue) {
				mail_info_ptr->subject = strdup(pointer->next->fieldValue);
			}else if (!strncasecmp(pointer->next->fieldName, "content", sizeof("content"))  && pointer->next->fieldValue) {
				mail_info_ptr->content = strdup(pointer->next->fieldValue);
			}else if (!strncasecmp(pointer->next->fieldName, "sender", sizeof("sender")) && pointer->next->fieldValue) {
				mail_info_ptr->sender = strdup(pointer->next->fieldValue);
			}else if (!strncasecmp(pointer->next->fieldName, "from", sizeof("from")) && pointer->next->fieldValue) {
				mail_info_ptr->from = strdup(pointer->next->fieldValue);
			}

			pointer = pointer->next;
		}
		send_mail_queue.push(mail_info_ptr);
	}
	free_result_data(&result_data);
}
