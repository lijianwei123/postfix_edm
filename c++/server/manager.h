/*
 * manager.h
 *
 *  Created on: 2014-11-6
 *      Author: Administrator
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include "util.h"
#include "monitoring.h"

#define COMMAND_TO_ENUM(command) CID_COMMAND_##command

class cManager : public CRefObject
{
public:
	cManager();
	virtual ~cManager();

	//开始,会做一些清除工作
	int start();
	//停止
	int stop();
	//暂停
	int pause();
	//继续
	int resume();

	//调整发送速率  delay 延迟   multi 并发数
	int adjustSendRate(uint16_t delay, uint16_t multi);

public:
	static cManager *instance;

private:
	int _cleanHistoryData();
	int64_t _getTotalNum();
private:
	uint16_t _status;
	cMonitoring *_Monitor;
};


#endif /* MANAGER_H_ */
