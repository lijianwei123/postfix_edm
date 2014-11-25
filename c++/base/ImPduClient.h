/*
 * ImPduClient.h
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#ifndef IMPDUCLIENT_H_
#define IMPDUCLIENT_H_

#include "ImPduBase.h"
#include "util.h"

#define CLIENT_CPP 									1
#define CLIENT_PHP									2

//command
#define IM_PDU_TYPE_SERVER_COMMAND					1
#define IM_PDU_TYPE_ADJUST_RATE						2

//msg
#define IM_PDU_TYPE_DATA							1 << 8 | 1
#define IM_PDU_TYPE_SERVER_STATUS_INFO				1 << 8 | 2
#define IM_PDU_TYPE_ALL_CLIENT_STATUS_INFO			1 << 8 | 3

//other
#define IM_PDU_TYPE_HEARTBEAT						2 << 8 | 1
#define IM_PDU_TYPE_RESPONSE						2 << 8 | 2
#define IM_PDU_TYPE_REG_CLIENT_TYPE					2 << 8 | 3


typedef struct client_status_info
{
	char *ip;
	char *emailFromAddr;

	char *json_str;

	/**
	 * 序列化  注意释放返回值   free()
	 */
	cJSON *getJsonObject()
	{
		cJSON *root = NULL;
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "ip", ip);
		cJSON_AddStringToObject(root, "emailFromAddr", emailFromAddr);

		return root;
	}

	/**
	 * 序列化  注意释放返回值   free()
	 */
	char *serialize()
	{
		cJSON *root = NULL;
		root = cJSON_CreateObject();
		cJSON_AddStringToObject(root, "ip", ip);
		cJSON_AddStringToObject(root, "emailFromAddr", emailFromAddr);

		json_str = cJSON_Print(root);
		cJSON_Delete(root);

		return json_str;
	}

	/**
	 * 反序列化
	 */
	int unserialize(const char *json_str)
	{
		Json::Reader reader;
		Json::Value value;

		string json_string(json_str);

		if (reader.parse(json_string, value)) {
			ip= strdup(value["ip"].asString().c_str());
			emailFromAddr = strdup(value["emailFromAddr"].asString().c_str());
		}

		return 0;
	}

	client_status_info()
	{
		json_str = NULL;
		ip = NULL;
		emailFromAddr = NULL;
	}

	~client_status_info()
	{
		if (json_str)
			free(json_str);
		if (ip)
			free(ip);
		if (emailFromAddr)
			free(emailFromAddr);
	}

} client_status_info_t;


typedef map<string, shared_ptr<client_status_info> > client_status_map_t;

typedef struct server_status_info {
	uint16_t status;    //服务器的状态   start  stop

	uint16_t delay;
	uint16_t multi;

	uint64_t totalNum;
	uint16_t online_client_num;


	char *json_str;

	/**
	 * 序列化  注意释放返回值   free()
	 */
	char *serialize()
	{
		cJSON *root = NULL;
		root = cJSON_CreateObject();
		cJSON_AddNumberToObject(root, "status", status);
		cJSON_AddNumberToObject(root, "delay", delay);
		cJSON_AddNumberToObject(root, "multi", multi);
		cJSON_AddNumberToObject(root, "totalNum", totalNum);
		cJSON_AddNumberToObject(root, "online_client_num", online_client_num);
		json_str = cJSON_Print(root);
		cJSON_Delete(root);

		return json_str;
	}

	/**
	 * 反序列化
	 */
	int unserialize(const char *json_str)
	{
		Json::Reader reader;
		Json::Value value;

		string json_string(json_str);

		if (reader.parse(json_string, value)) {
			status = static_cast<uint16_t>(value["status"].asUInt());
			delay = static_cast<uint16_t>(value["delay"].asUInt());
			multi = static_cast<uint16_t>(value["multi"].asUInt());
			totalNum = static_cast<uint64_t>(value["totalNum"].asDouble());
			online_client_num = static_cast<uint16_t>(value["online_client_num"].asUInt());
		}
		return 0;
	}


	server_status_info()
	{
		status = CID_COMMAND_STOP;
		delay = 0;
		multi = 1;
		totalNum = 0;
		online_client_num = 0;

		json_str = NULL;
	}

	~server_status_info()
	{
		if (json_str)
			free(json_str);
	}

} server_status_info_t;

//====================================SID_COMMAND  START=========================================================================
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


class DLL_MODIFIER CImPduAdjustRate : public CImPdu
{
public:
	CImPduAdjustRate(uchar_t* buf, uint32_t len);
	CImPduAdjustRate(uint16_t delay, uint16_t multi);
	virtual ~CImPduAdjustRate() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_ADJUST_RATE; }

	uint16_t GetDelay() { return delay; }
	uint16_t GetMulti() { return multi; }
private:
	uint16_t delay;
	uint16_t multi;
};
//====================================SID_COMMAND  END===========================================================================


//====================================SID_MSG  START=============================================================================
class DLL_MODIFIER CImPduClientData : public CImPdu
{
public:
	CImPduClientData(uchar_t* buf, uint32_t len);
	CImPduClientData(char *data, uint32_t data_len);
	virtual ~CImPduClientData() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_DATA; }


	char* GetData() { return data; }
	uint32_t GetDataLen() { return data_len; }
private:
	char*		data;
	uint32_t  data_len;
};


//服务端状态信息
class DLL_MODIFIER CImPduServerStatusInfo : public CImPdu
{
public:
	//解包
	CImPduServerStatusInfo(uchar_t* buf, uint32_t len);
	//组包
	CImPduServerStatusInfo();
	//组包
	CImPduServerStatusInfo(shared_ptr<server_status_info> server_status_ptr);
	virtual ~CImPduServerStatusInfo() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_SERVER_STATUS_INFO; }
	char *GetJsonData() { return json_data; }
	uint32_t GetJsonDataLen() { return json_data_len; }

private:
	char *json_data;
	uint32_t json_data_len;
};


//所有客户端状态信息
class DLL_MODIFIER CImPduAllClientStatusInfo : public CImPdu
{
public:
	CImPduAllClientStatusInfo(uchar_t* buf, uint32_t len);
	CImPduAllClientStatusInfo(char *json_data);
	virtual ~CImPduAllClientStatusInfo() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_ALL_CLIENT_STATUS_INFO; }
};
//====================================SID_MSG  END=============================================================================

//====================================SID_OTHER  START=========================================================================

class DLL_MODIFIER CImPduHeartbeat : public CImPdu
{
public:
	CImPduHeartbeat(uchar_t* buf, uint32_t len);
	CImPduHeartbeat();
	//组包
	CImPduHeartbeat(shared_ptr<client_status_info> client_status_ptr);
	virtual ~CImPduHeartbeat() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_HEARTBEAT; }

	char *GetJsonData() { return json_data; }
	uint32_t GetJsonDataLen() { return json_data_len; }

private:
	char *json_data;
	uint32_t json_data_len;
};

class DLL_MODIFIER CImPduResponse : public CImPdu
{
public:
	CImPduResponse(uchar_t* buf, uint32_t len);
	CImPduResponse(uint16_t status, char *message);
	virtual ~CImPduResponse() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_RESPONSE; }

	char *GetMessage() { return message; }
	uint32_t GetMessageLen() { return message_len; }
	uint16_t GetStatus() { return status; }

private:
	char *message;
	uint32_t message_len;
	uint16_t status; // 0 表示成功  -1 表示失败
};

//注册客户端类型
class DLL_MODIFIER CImPduRegClientType : public CImPdu
{
public:
	CImPduRegClientType(uchar_t* buf, uint32_t len);
	CImPduRegClientType(uint16_t client_type);
	virtual ~CImPduRegClientType() {}

	virtual uint16_t GetPduType() { return IM_PDU_TYPE_REG_CLIENT_TYPE; }

	uint16_t GetClientType() { return client_type;}

private:
	uint16_t client_type;
};

//====================================SID_OTHER  END=========================================================================

#endif /* IMPDUCLIENT_H_ */
