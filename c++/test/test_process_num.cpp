//g++ -g test_process_num.cpp -Wno-deprecated  -o test_process_num
/*
 * test_process_num.cpp  测试进程调整
 *
 *  Created on: 2014-11-25
 *  Author: Administrator
 */
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdint.h>
#include <sys/wait.h>
#include <string.h>


#include <ext/hash_map>
using namespace __gnu_cxx;


//use boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

using namespace boost;

using namespace std;

class cChildProcess;

int child_quit = 0;
hash_map<pid_t, shared_ptr<cChildProcess> > childProcessMap;
uint16_t childNum = 0;


static void sigUsr1Handler(int signalNo)
{
	//printf("signal no usr1:%d\n", signalNo);
	switch (signalNo) {
	case SIGUSR1:
		child_quit = true;
		break;
	}
}

class CProcess
{
public:
	CProcess()
	{

	}
	~CProcess()
	{

	}
	pid_t CreateChild()
	{
		pid_t pid = 0;
		pid = fork();
		if (pid < 0) {
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

	virtual void OnChildRun() = 0;

	pid_t getCid(){return _cid;}
private:
	pid_t _cid;
};



class cChildProcess : public CProcess
{
public:
	cChildProcess()
	{
		//cout << __FUNCTION__ << endl;
	}

	~cChildProcess()
	{
	}

	void OnChildRun()
	{
		signal(SIGUSR1, sigUsr1Handler);


		while (true) {
			if (child_quit || getppid() == 1) {
				exit(SIGUSR1);
			}

			//cout << __FUNCTION__ << endl;
			sleep(10);
		}
	}
};



void sigChldHandler(int signalNo)
{
	//printf("signal no:%d\n", signalNo);

	pid_t chldPid = 0;
	int status = 0;

	chldPid = waitpid(-1, &status, WNOHANG | WUNTRACED);

	if (chldPid < 0) {
		//printf("waitpid error: %s\n", strerror(errno));
	} else {
		//printf("WIFEXITED:%d, WEXITSTATUS: %d, WIFSIGNALED: %d, WTERMSIG: %d\n", WIFEXITED(status), WEXITSTATUS(status), WIFSIGNALED(status), WTERMSIG(status));

		if (WIFEXITED(status) && WEXITSTATUS(status) == SIGUSR1) {
			//这个就不减了
		} else {
			childProcessMap.erase(chldPid);
			--childNum;
		}
	}
}



int main()
{

		uint16_t processNum = 0, incProcessNum = 0, reduceProcessNum = 0;
		int i = 0;
		hash_map<pid_t, shared_ptr<cChildProcess> >::iterator it;

		signal(SIGCHLD, sigChldHandler);

		while (true) {
				cout << "please input process num:" << endl;
				cin >> processNum;
				cout << "child num " << childNum << endl;
				
				cout << "adjust child process num start" << endl;
				//进程调整
				if (childNum > processNum) {
					reduceProcessNum = childNum - processNum;
					cout << "reduce num" << reduceProcessNum << endl;
					if (reduceProcessNum) {
						//发送信号
						for(i = 0, it = childProcessMap.begin(); it != childProcessMap.end() && i < reduceProcessNum; ++i) {
							hash_map<pid_t, shared_ptr<cChildProcess> >::iterator it_old = it;
							++it;
							cout << "reduce pid" << it_old->first << endl;
							//子进程你可以退休了
							kill(it_old->first, SIGUSR1);
							--childNum;
							childProcessMap.erase(it_old->first);
						}
				    	}
				} else {
					incProcessNum = processNum - childNum;
					cout << "inc process num" << incProcessNum << endl;
					
					if (incProcessNum) {

						shared_ptr<cChildProcess> childProcess;

						int i = 0;
						for (i = 0; i < incProcessNum; ++i) {
							childProcess = make_shared<cChildProcess>();
							childProcess->CreateChild();

							childProcessMap.insert(make_pair(childProcess->getCid(), childProcess));
							++childNum;
						}
					}
				}

				cout << "adjust child process num success" << endl;
				cout << "childProcessMap size" << childProcessMap.size() << endl;
				cout << "child pid" << endl;
				for (it = childProcessMap.begin(); it != childProcessMap.end(); ++it) {
					cout << it->first << endl;
				} 				
		
				sleep(10);
		}

}
