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

extern shared_ptr<CThreadCondLock> condLock;
extern mysql_connect_info_t  mysql_connect_info;

typedef struct {
	queue<mail_info_t *> *email_queue_ptr;
	CThreadCondLock *condLock;
	cWorker *worker;

} shm_shared_t;

class cWorker
{
public:
	cWorker();
	~cWorker();
	void run();
	void done();
	void checkThreadAlive();

	void setStatus(uint16_t status) { _status = status;}
	uint16_t getStatus() { return _status;}

	void setDelay(uint16_t delay) { _delay = delay;}
	uint16_t getDelay() { return _delay;}

	void setCont(uint16_t cont) { _cont = cont;}
	uint16_t getCont() { return _cont;}


private:
	void _doProduct();
	void _doConsume();


public:
	thread_t workerThread[2];
	static cWorker *instance;

private:
	uint16_t _status; //状态   start  stop  pause  resume
	uint16_t _delay;  //延迟
	uint16_t _cont;  //并发
};

//取邮件   生产者
class cWorkerPullSentInfo : public CThread
{
public:
	cWorkerPullSentInfo();
	virtual ~cWorkerPullSentInfo();

	virtual void OnThreadRun(void);

protected:
	void pullSentInfoFromMysql();

};

//发送邮件 消费者
class cWorkerSendMail : public CThread
{
public:
	cWorkerSendMail();
	virtual ~cWorkerSendMail();

	virtual void OnThreadRun(void);

private:
	pid_t _forkMasterProcess();
};

//多进程发送邮件
class cMasterProcess : public CProcess
{
public:
	cMasterProcess();
	virtual ~cMasterProcess();
	virtual void child();
	virtual void signalHandler(int signalNo);

private:
	uint16_t childNum;
};

class cChildProcess : public CProcess
{
public:
	cChildProcess();
	virtual ~cChildProcess();
	virtual void child();
	virtual void signalHandler(int signalNo);
private:
	cSendMail *_sendMail;
	bool _quit;
};


#endif /* WORKER_H_ */
