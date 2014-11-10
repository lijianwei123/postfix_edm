/*
 * ImPduClient.h
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#ifndef IMPDUCLIENT_H_
#define IMPDUCLIENT_H_

#include "ImPduBase.h"

#define IM_PDU_TYPE_HEARTBEAT						1
#define IM_PDU_TYPE_TEST_DATA						2
#define IM_PDU_TYPE_SERVER_COMMAND					3
#define IM_PDU_TYPE_ADJUST_RATE						4



class DLL_MODIFIER CImPduAdjustRate : public CImPdu
{
public:
	CImPduAdjustRate(uchar_t* buf, uint32_t len);
	CImPduAdjustRate(uint16_t servCommand);
	virtual ~CImPduAdjustRate() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_ADJUST_RATE; }

	uint16_t GetDelay() { return delay; }
	uint16_t GetMulti() { return multi; }
private:
	uint16_t delay;
	uint16_t multi;
};


class DLL_MODIFIER CImPduServerCommand : public CImPdu
{
public:
	CImPduServerCommand(uchar_t* buf, uint32_t len);
	CImPduServerCommand(uint16_t servCommand);
	virtual ~CImPduServerCommand() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_SERVER_COMMAND; }

	uint16_t GetServCommand() { return servCommand; }
private:
	uint16_t  servCommand;
};


class DLL_MODIFIER CImPduHeartbeat : public CImPdu
{
public:
	CImPduHeartbeat(uchar_t* buf, uint32_t len) {}
	CImPduHeartbeat();
	virtual ~CImPduHeartbeat() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_HEARTBEAT; }
};

class DLL_MODIFIER CImPduClientTestData : public CImPdu
{
public:
	CImPduClientTestData(uchar_t* buf, uint32_t len);
	CImPduClientTestData(char *data, uint32_t data_len);
	virtual ~CImPduClientTestData() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_TEST_DATA; }


	char* GetData() { return data; }
private:
	char*		data;
	uint32_t  data_len;
};




#endif /* IMPDUCLIENT_H_ */
