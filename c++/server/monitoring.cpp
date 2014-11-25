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


