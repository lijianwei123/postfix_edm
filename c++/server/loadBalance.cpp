/*
 * loadBalance.cpp  负载均衡
 *
 *  Created on: 2014-11-26
 *  Author: Administrator
 */
#include "util.h"
#include "loadBalance.h"
#include "manager.h"

cLoadBalance *cLoadBalance::instance = getInstance<cLoadBalance>();

cLoadBalance::cLoadBalance()
{
}

cLoadBalance::~cLoadBalance()
{
}

//把组内数据平均分给client
cLoadBalance::blanceAllGroupData()
{

}


