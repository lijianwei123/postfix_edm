/*
 * test_singleton.h
 *
 *  Created on: 2014-11-10
 *      Author: Administrator
 */

#ifndef TEST_SINGLETON_H_
#define TEST_SINGLETON_H_

class test_singleton
{
public:
	test_singleton();
	virtual ~test_singleton();

	void setStatus(uint16_t status) { _status = status;}
	uint16_t getStatus() { return _status;}
public:
	static test_singleton *instance;
private:
	uint16_t _status;
};

#endif /* TEST_SINGLETON_H_ */
