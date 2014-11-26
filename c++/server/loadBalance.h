/*
 * loadBalance.h
 *
 *  Created on: 2014-11-26
 *  Author: Administrator
 */

#ifndef LOADBALANCE_H_
#define LOADBALANCE_H_


class cLoadBalance : public CRefObject
{
public:
	cLoadBalance();
	virtual ~cLoadBalance();

	void blanceAllGroupData();

public:
	static cLoadBalance *instance;

};



#endif /* LOADBALANCE_H_ */
