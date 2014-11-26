/*
 * monitoring.cpp
 *
 *  Created on: 2014-11-6
 *  Author: Administrator
 */
#include "monitoring.h"
#include "util.h"

cMonitoring *cMonitoring::instance = getInstance<cMonitoring>();

cMonitoring::cMonitoring()
{
	_totalNum = 0;
}

cMonitoring::~cMonitoring()
{
}

shared_ptr<client_status_info> cMonitoring::GetClientStatus(char *ip)
{
	shared_ptr<client_status_info> client_status;
	client_status.reset();

	client_status_map_t::iterator it = _client_status_maps.find(ip);
	if (it != _client_status_maps.end()) {
		client_status = it->second;
	}
	return client_status;

}

int cMonitoring::UpdateClientStatus(shared_ptr<client_status_info> client_status_ptr)
{
	if (GetClientStatus(client_status_ptr->ip) == NULL) {
		_client_status_maps.insert(make_pair(client_status_ptr->ip, client_status_ptr));
	} else {
		_client_status_maps.erase(client_status_ptr->ip);
		_client_status_maps.insert(make_pair(client_status_ptr->ip, client_status_ptr));
	}
	return 0;
}

bool cMonitoring::removeClientStauts(const char *ip)
{
	return !_client_status_maps.erase(ip);
}


int64_t cMonitoring::getSentEmailNum()
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

	snprintf(select_sql, sizeof(select_sql), "SELECT count(1) as sentNum FROM `send_email` where `status` = 1 limit 1");
	retCode = mysql_select(&conn, select_sql, &result_data);

	#ifdef DEBUG
		printf("rows:%d, cols:%d\n", result_data.rows, result_data.columns);
	#endif

	mysql_close(&conn);

	if (result_data.rows > 0) {
		pointer = *(result_data.data);
		return static_cast<uint64_t>(atoi(pointer->next->fieldValue));
	} else {
		return 0;
	}

}


