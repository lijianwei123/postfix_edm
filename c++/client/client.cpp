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
const char *LOG_FILE_NAME;

//检查线程是否存活
static void worker_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	cWorker::instance->checkThreadAlive();
}

//恩，干活了
static void init_worker()
{
	cWorker::instance->run();

	netlib_register_timer(worker_timer_callback, NULL, 5000);
}

static void init_log()
{
	LOG_FILE_NAME =  "./log/client.log";
	char *log_dir = dirname(const_cast<char *>(LOG_FILE_NAME));
	if (!is_dir(log_dir)) {
		 if(mkdir(log_dir, 0644) == -1) {
			 cout << "mkdir log dir error!" << endl;
			 exit(EXIT_FAILURE);
		 }
	}
}

int main(int argc, char **argv, char **envp)
{
	init_log();

	cout << "starting client..." << endl;
	CConfigFileReader  clientConf(CONF_FILE);
	serverIp = clientConf.GetConfigName("serverIp");
	serverPort = clientConf.GetConfigName("serverPort");
	emailFromAddr = stripLineSep(clientConf.GetConfigName("emailFromAddr"));

	mysql_connection_info_init(&mysql_connect_info);
	mysql_connect_info.host = stripLineSep(clientConf.GetConfigName("mysqlHost"));
	if (clientConf.GetConfigName("mysqlPort")) {
		mysql_connect_info.port =  (unsigned int)atoi(clientConf.GetConfigName("mysqlPort"));
	}
	mysql_connect_info.db = stripLineSep(clientConf.GetConfigName("mysqlDb"));
	mysql_connect_info.user = stripLineSep(clientConf.GetConfigName("mysqlUser"));
	mysql_connect_info.pwd = stripLineSep(clientConf.GetConfigName("mysqlPwd"));

	changeProcessName = atoi(clientConf.GetConfigName("changeProcessName"));
	int isDaemon = atoi(clientConf.GetConfigName("isDaemon"));

	if (changeProcessName) {
		//修改进程名称
		prename_setproctitle_init(argc, argv, envp);
		prename_setproctitle("%s", "postfix_edm_client");
	}

	if (isDaemon) {
		setDaemon(const_cast<char *>(LOG_FILE_NAME));
	}

	init_worker();
	connect_server();
	init_client_conn();

	printf("now enter the event loop...\n");
	netlib_eventloop();

	return 0;
}
