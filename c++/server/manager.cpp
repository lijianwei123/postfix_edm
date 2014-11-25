/*
 * manager.cpp
 *
 *  Created on: 2014-11-6
 *  Author: Administrator
 */

#include "util.h"
#include "manager.h"
#include "servConn.h"
#include "mysql.h"

cManager *cManager::instance = getInstance<cManager>();

//数据库配置
mysql_connect_info_t  mysql_connect_info;
int isDaemon = 0;
int changeProcessName = 0;

cManager::cManager()
{
	_status = COMMAND_TO_ENUM(STOP);
	_delay = 0;
	_multi = 1;
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
	//_cleanHistoryData();

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
	_delay = delay;
	_multi = multi;

	CImPduAdjustRate pdu(delay, multi);
	send_all_client_pdu(&pdu);

	return 0;
}

int cManager::_cleanHistoryData()
{
	MYSQL conn;
	int retCode = -1;
	unsigned long result = 0l;
	char delete_sql[200] = {0};


	retCode = mysql_user_connect(&conn, &mysql_connect_info);
	assert(retCode == 0);

	retCode = mysql_select_db(&conn, mysql_connect_info.db);
	assert(retCode == 0);

	snprintf(delete_sql, sizeof(delete_sql), "delete from `send_email`");
	retCode = mysql_execute(&conn, delete_sql, &result);

	#ifdef DEBUG
		printf("delete affected rows: %lu\n", result);
	#endif

	mysql_close(&conn);

	return retCode;
}

uint64_t cManager::_getTotalNum()
{
	MYSQL conn;
	result_data_t result_data;
	mysql_field_value_t *pointer = NULL;

	int retCode = -1;
	char select_sql[200] = {0};

	retCode = mysql_user_connect(&conn, &mysql_connect_info);
	assert(retCode == 0);

	retCode = mysql_select_db(&conn, mysql_connect_info.db);
	assert(retCode == 0);

	snprintf(select_sql, sizeof(select_sql), "SELECT count(DISTINCT `to`) as totalnum FROM `send_email` limit 1");
	retCode = mysql_select(&conn, select_sql, &result_data);

	#ifdef DEBUG
		printf("rows:%d, cols:%d\n", result_data.rows, result_data.columns);
	#endif

	mysql_close(&conn);

	if (result_data.rows > 0) {
		pointer = *(result_data.data);
		return static_cast<uint16_t>(atoi(pointer->next->fieldValue));
	} else {
		return 0;
	}
}

