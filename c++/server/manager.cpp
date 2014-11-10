/*
 * manager.cpp
 *
 *  Created on: 2014-11-6
 *  Author: Administrator
 */

#include "util.h"
#include "manager.h"
#include "servConn.h"

cManager *cManager::instance = getInstance<cManager>();

cManager::cManager()
{
	_status = 0;
	_Monitor = cMonitoring::instance;
}

cManager::~cManager()
{
}


//开始
int cManager::start()
{
	uint64_t totalNum = 0;

	//清除旧的一些数据
	_cleanHistoryData();

	//获取总量
	totalNum = _getTotalNum();
	_Monitor->setTotalNum(totalNum);

	//向每个client发送命令
	CImPduServerCommand pdu(COMMAND_TO_ENUM(START));
	send_all_client_pdu(&pdu);

	_status = COMMAND_TO_ENUM(START);

	return 0;
}

int cManager::stop()
{
	//向每个client发送命令
	CImPduServerCommand pdu(COMMAND_TO_ENUM(STOP));
	send_all_client_pdu(&pdu);

	_status = COMMAND_TO_ENUM(STOP);

	return 0;
}

int cManager::pause()
{
	//向每个client发送命令
	CImPduServerCommand pdu(COMMAND_TO_ENUM(PAUSE));
	send_all_client_pdu(&pdu);

	_status = COMMAND_TO_ENUM(PAUSE);

	return 0;
}

int cManager::resume()
{
	//向每个client发送命令
	CImPduServerCommand pdu(COMMAND_TO_ENUM(RESUME));
	send_all_client_pdu(&pdu);

	_status = COMMAND_TO_ENUM(RESUME);

	return 0;
}

//调整速率
int cManager::adjustSendRate(uint16_t delay, uint16_t multi)
{
	CImPduAdjustRate pdu(delay, multi);
	send_all_client_pdu(&pdu);

	return 0;
}

int cManager::_cleanHistoryData()
{
	return 0;
}

uint64_t cManager::_getTotalNum()
{
	return 0;
}

