/*
 *  g++ -g -I../client -I../base test_thread.cpp ../base/libbase.a -o test_thread
 *
 *  测试线程
 *
 *  Created on: 2014-11-17
 *  Author: Administrator
 */

#include <iostream>
#include "worker.h"

using namespace std;

int main()
{
	cWorkerPullSentInfo producer;
	workerThread[0] =  producer.CreateThread();

	return 0;
}
