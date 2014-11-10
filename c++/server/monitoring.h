/*
 * monitoring.h
 *
 *  Created on: 2014-11-6
 *      Author: Administrator
 */

#ifndef MONITORING_H_
#define MONITORING_H_

#include "util.h"

class cMonitoring : public CRefObject
{
public:
	cMonitoring();
	virtual ~cMonitoring();

	void setTotalNum(uint64_t totalNum){ _totalNum = totalNum; }
	void getTotalNum(){ return _totalNum; }



public:
		static cMonitoring *instance;
private:
	//总的发送量
	uint64_t _totalNum;

	//已发送数量
	unit64_t _sendedNum;
};

#endif /* MONITORING_H_ */
