/*
 * worker.h
 *
 *  Created on: 2014-11-7
 *  Author: Administrator
 */

#ifndef WORKER_H_
#define WORKER_H_

#include "util.h"
#include "sendMail.h"
#include "clientConn.h"

//拉取数量
#define PULL_SENT_MAIL_UNIT 1000
//子进程检查间隔
#define CHECK_PROCESSNUM_INTERVAL  30
//检测是否有新的邮件待发送
#define CHECK_MAIL_DATA_INTERVAL   30

#define COMMAND_TO_ENUM(command) CID_COMMAND_##command

#define INIT_RDONLY_SHMAT INIT_SHMAT(SHM_RDONLY)

#define INIT_SHMAT(mode)	\
shm_shared_t *shm_ptr = (shm_shared_t *)shmat(shm_id, NULL, mode); \
if ((int)shm_ptr == -1) { \
	log("shmat error:%s\n", strerror(errno)); \
	exit(EXIT_FAILURE); \
}

#define CHECK_CAN_START(shm_ptr) \
shm_ptr->status & COMMAND_TO_ENUM(START) || shm_ptr->status & COMMAND_TO_ENUM(RESUME)

#define COND_LOCK_NOTIFY(cond)	\
cond.Lock();	\
cond.notify();	\
cond.Unlock();

#define COND_LOCK_NOTIFYALL(cond)	\
cond.Lock();	\
cond.notifyAll();	\
cond.Unlock();

extern mysql_connect_info_t  mysql_connect_info;
extern char *emailFromAddr;
extern int changeProcessName;
extern int isDaemon;

class cChildProcess;

typedef struct {
	uint16_t status;
	uint16_t multi;
	uint16_t delay;
	uint64_t havePushNum;

	//条件锁
	struct CThreadLock childLock;
	struct CThreadLock commandLock;
	struct CThreadLock productLock;
	struct CThreadLock consumeLock;

} shm_shared_t;

extern 	shm_shared_t *shm_ptr;

class cWorkerThread : public CThread
{
public:
	cWorkerThread();
	virtual ~cWorkerThread();
	void OnThreadRun(void);

private:
	void _init_shm();
	bool _haveSendInfo();
	pid_t _forkMasterProcess();

};

class cWorker
{
public:
	cWorker();
	~cWorker();
	void run();
	void done();
	void checkThreadAlive();
	//获取待发队列数量
	int64_t GetMailqNum();


	void setStatus(uint16_t status);
	uint16_t getStatus() { return _status;}

	void setDelay(uint16_t delay);
	uint16_t getDelay() { return _delay;}

	void setMulti(uint16_t multi);
	uint16_t getMulti() { return _multi;}

private:
	void _createThread();
public:
	shared_ptr<cWorkerThread> worker_thread_ptr;
	pthread_t worker_thread_id;
	static cWorker *instance;

private:
	uint16_t _status; //状态   start  stop  pause  resume
	uint16_t _delay;  //延迟
	uint16_t _multi;  //并发
};



//多进程发送邮件
class cMasterProcess : public CProcess
{
public:
	cMasterProcess();
	virtual ~cMasterProcess();
	virtual void OnChildRun();
public:
	static hash_map<pid_t, shared_ptr<cChildProcess> > childProcessMap;
	static uint16_t childNum;
};

class cChildProcess : public CProcess
{
public:
	cChildProcess();
	virtual ~cChildProcess();
	virtual void OnChildRun();
	//自给自足
	void pullSentInfoFromMysql();
private:
	cSendMail *_sendMail;

	//队列
	queue<mail_info_t *> send_mail_queue;
};

#endif /* WORKER_H_ */
