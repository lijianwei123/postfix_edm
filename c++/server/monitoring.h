/*
 * monitoring.h
 *
 *  Created on: 2014-11-6
 *      Author: Administrator
 */

#ifndef MONITORING_H_
#define MONITORING_H_

#include "util.h"
#include "impdu.h"


class cMonitoring : public CRefObject
{
public:
	cMonitoring();
	virtual ~cMonitoring();

	void setTotalNum(uint64_t totalNum){ _totalNum = totalNum; }
	uint64_t getTotalNum(){ return _totalNum; }
	uint16_t getOnlineClientNum() { return _online_client_num;}
	void setOnlineClientNum(uint16_t online_client_num) { _online_client_num = online_client_num; }

	client_status_map_t GetClientStatusMap()  { return _client_status_maps; }
	shared_ptr<client_status_info> GetClientStatus(char *ip);
	int UpdateClientStatus(shared_ptr<client_status_info> client_status_ptr);
	bool removeClientStauts(const char *ip);


public:
		static cMonitoring *instance;
private:
	//总的发送量
	uint64_t _totalNum;

	//已发送数量
	uint64_t _sendedNum;

	uint16_t _online_client_num;

	client_status_map_t _client_status_maps;
};

#endif /* MONITORING_H_ */
