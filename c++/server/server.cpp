/*
 * index.cpp
 *
 *  Created on: 2014-11-4
 *  Author: lijianwei
 */

#include "netlib.h"
#include "ConfigFileReader.h"
#include "servConn.h"

#define CONF_FILE "server.conf"

void serverCallback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	if (msg == NETLIB_MSG_CONNECT)
	{
		cServConn *pConn = new cServConn();
		pConn->onConnect2(handle);
	}
	else
	{
		log("!!!error msg: %d\n", msg);
	}
}


int main(int argc, char *argv[])
{
	CConfigFileReader  serverConf(CONF_FILE);
	char *serverIp = serverConf.GetConfigName("serverIp");
	char *serverPort = serverConf.GetConfigName("serverPort");

	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;

	ret = netlib_listen(serverIp, (uint16_t)atoi(serverPort), serverCallback, NULL);

	printf("now enter the event loop...\n");


	init_server_conn();

	netlib_eventloop();

	return 0;
}
