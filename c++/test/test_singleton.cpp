/*
 * g++ -g -Wall -Wno-deprecated  -I../base test_singleton.cpp -o test_singleton
 *
 * test_singleton.cpp
 *
 *  Created on: 2014-11-10
 *  Author: Administrator
 */
#include <iostream>
#include <assert.h>
#include "util.h"
#include "test_singleton.h"

using namespace std;

test_singleton * test_singleton::instance = getInstance<test_singleton>();

test_singleton::test_singleton()
{
}

test_singleton::~test_singleton()
{
}


int main()
{
	//测试单例模式

	cout << "test_singleton status" << test_singleton::instance->getStatus() << endl;
	test_singleton::instance->setStatus(10);
	cout << "test_singleton status" << test_singleton::instance->getStatus() << endl;

	assert(10 == test_singleton::instance->getStatus());

	return 0;
}
