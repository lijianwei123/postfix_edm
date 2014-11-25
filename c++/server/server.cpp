/*
 * index.cpp
 *
 *  Created on: 2014-11-4
 *  Author: lijianwei
 */

#include "netlib.h"
#include "ConfigFileReader.h"
#include "servConn.h"
#include "manager.h"

#define CONF_FILE "server.conf"
const char *LOG_FILE_NAME;

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

void regSignal()
{
	signal(SIGPIPE, SIG_IGN);
}

static void init_log()
{
	LOG_FILE_NAME = "./log/server.log";
	char *log_dir = dirname(const_cast<char *>(LOG_FILE_NAME));
	if (!is_dir(log_dir)) {
		 if(mkdir(log_dir, 0644) == -1) {
			 cout << "mkdir log dir error!" << endl;
			 exit(EXIT_FAILURE);
		 }
	}
}

int main(int argc, char *argv[], char **envp)
{
	init_log();
	regSignal();

	CConfigFileReader  serverConf(CONF_FILE);
	char *serverIp = serverConf.GetConfigName("serverIp");
	char *serverPort = serverConf.GetConfigName("serverPort");

	//数据库
	mysql_connection_info_init(&mysql_connect_info);
	mysql_connect_info.host = stripLineSep(serverConf.GetConfigName("mysqlHost"));
	if (serverConf.GetConfigName("mysqlPort")) {
		mysql_connect_info.port =  (unsigned int)atoi(serverConf.GetConfigName("mysqlPort"));
	}
	mysql_connect_info.db = stripLineSep(serverConf.GetConfigName("mysqlDb"));
	mysql_connect_info.user = stripLineSep(serverConf.GetConfigName("mysqlUser"));
	mysql_connect_info.pwd = stripLineSep(serverConf.GetConfigName("mysqlPwd"));

	changeProcessName = atoi(serverConf.GetConfigName("changeProcessName"));
	isDaemon = atoi(serverConf.GetConfigName("isDaemon"));
	if (isDaemon) {
		setDaemon(const_cast<char *>(LOG_FILE_NAME));
	}

	if (changeProcessName) {
		//修改进程名称
		prename_setproctitle_init(argc, argv, envp);
		prename_setproctitle("%s", "postfix_edm_server");
	}


	int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;

	ret = netlib_listen(serverIp, (uint16_t)atoi(serverPort), serverCallback, NULL);

	printf("now enter the event loop...\n");


	init_server_conn();

	netlib_eventloop();

	return 0;
}
