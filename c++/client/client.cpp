/*
 * client.cpp
 *
 *  Created on: 2014-11-4
 *  Author: Administrator
 */
#include "netlib.h"
#include "ConfigFileReader.h"
#include "clientConn.h"
#include "worker.h"

#define CONF_FILE "client.conf"

//检查线程是否存活
static void worker_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	cWorker::instance->checkThreadAlive();
}

//恩，干活了
static void init_worker()
{
	cWorker::instance->run();

	netlib_register_timer(worker_timer_callback, 5000);
}

int main(int argc, char **argv)
{
	cout << "starting client..." << endl;
	CConfigFileReader  clientConf(CONF_FILE);
	char *serverIp = clientConf.GetConfigName("serverIp");
	char *serverPort = clientConf.GetConfigName("serverPort");

	mysql_connect_info.host = clientConf.GetConfigName("mysqlHost");
	if (clientConf.GetConfigName("mysqlPort")) {
		mysql_connect_info.port =  (unsigned int)atoi(clientConf.GetConfigName("mysqlPort"));
	}
	mysql_connect_info.db = clientConf.GetConfigName("mysqlDb");
	mysql_connect_info.user = clientConf.GetConfigName("mysqlUser");
	mysql_connect_info.pwd = clientConf.GetConfigName("mysqlPwd");

	//shared_ptr<cClientConn> clientConn = make_shared<cClientConn>();
	//自带了一套自动清除内存方式,不需要shared_ptr了
	cClientConn *clientConn = new cClientConn();
	clientConn->connect(serverIp, (uint16_t)atoi(serverPort));

	init_client_conn();

	init_worker();

	printf("now enter the event loop...\n");
	netlib_eventloop();

	return 0;
}
