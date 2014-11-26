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

extern mysql_connect_info_t  mysql_connect_info;
extern int changeProcessName;
extern int isDaemon;

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


	uint16_t GetStatus() { return _status; }
	uint16_t GetDelay() { return _delay; }
	uint16_t GetMulti() { return _multi; }

	cMonitoring *GetMonitor() { return _Monitor; }

public:
	static cManager *instance;

	client_config_map_t clientConfigMaps;

private:
	int _cleanHistoryData();
	uint64_t _getTotalNum();
	int _GetAllClientConfig();
private:
	uint16_t _status;
	uint16_t _delay;
	uint16_t _multi;

	cLoadBalance *_loadBlance;
	cMonitoring *_Monitor;


};


#endif /* MANAGER_H_ */
