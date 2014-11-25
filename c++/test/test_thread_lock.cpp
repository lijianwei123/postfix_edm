//g++ -g -lpthread -Wno-deprecated  -I../base test_thread_lock.cpp ../base/libbase.a -o test_thread_lock
/*
 * test_thread_lock.cpp
 *
 *  Created on: 2014-11-21
 *      Author: Administrator
 */

#include <iostream>

#include "util.h"

struct ThreadLock
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

void init(struct ThreadLock *lock)
{
	pthread_mutexattr_init(&lock->m_mutexattr);
	pthread_mutexattr_setpshared(&lock->m_mutexattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&lock->m_mutex, &lock->m_mutexattr);

	pthread_condattr_init(&lock->cond_attr);
	pthread_condattr_setpshared(&lock->cond_attr, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&lock->cond, &lock->cond_attr);
}


struct test_shared_t {
	ThreadLock commandLock;
	int i;
};

int shm_id = 0;
void handler(int signo)
{
	shmctl(shm_id, IPC_RMID, NULL);
}

int main()
{
	signal(SIGTERM, handler);
	//创建共享内存
	srand((unsigned int)getpid());
	key_t ipc_shm_key = ftok("/dev/null", (int)'a');
	shm_id = shmget(ipc_shm_key, sizeof(test_shared_t), IPC_CREAT|0666);

	//初始化共享内存
	test_shared_t *shm_ptr = (test_shared_t *)shmat(shm_id, NULL, 0);

	memset(shm_ptr, 0, sizeof(test_shared_t));

	//初始化锁
	init(&shm_ptr->commandLock);
	shm_ptr->i = 1;

	pid_t pid = fork();

	if (pid == 0) {
		while (true) {
			shm_ptr->commandLock.Lock();
				shm_ptr->i++;
				cout << "child i:" << shm_ptr->i << endl;
				if (shm_ptr->i > 10)
					shm_ptr->commandLock.wait();
				else
					shm_ptr->commandLock.notify();
			shm_ptr->commandLock.Unlock();

			sleep(1);
		}
	} else if (pid > 0) {
		while (true) {
			shm_ptr->commandLock.Lock();
				shm_ptr->i--;
				cout << "parent i:" << shm_ptr->i << endl;
				if (shm_ptr->i < 0)
					shm_ptr->commandLock.notify();
				else
					shm_ptr->commandLock.wait();
			shm_ptr->commandLock.Unlock();
			sleep(1);
		}

	}

	return 0;

}



