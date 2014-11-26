/*
 * 	ImPduBase.h
 *
 *  Created on: 2013-8-27
 *  Author: ziteng@mogujie.com
 */
#ifndef IMPDUBASE_H_
#define IMPDUBASE_H_

#include "UtilPdu.h"
#include <string.h>
#include <stdlib.h>
#define IM_PDU_HEADER_LEN		12
#define IM_PDU_VERSION			1

// module id
enum {
	SID_COMMAND			= 1,
	SID_MSG 			= 2,
	SID_OTHER 			= 3,
};

//command id for command
enum {
	CID_COMMAND_START				= 1 << 0, //start
	CID_COMMAND_STOP				= 1 << 1, //stop
	CID_COMMAND_PAUSE				= 1 << 2, //pause
	CID_COMMAND_RESUME				= 1 << 3, //resume
	CID_COMMAND_ADJUST_RATE  		= 1 << 4, //adjust rate
};

//command id for msg
enum {
	CID_MSG_DATA					= 1,
	CID_MSG_SERVER_STATUS_INFO 		= 2,
	CID_MSG_ALL_CLIENT_STATUS_INFO	= 3,
};


//command id for other
enum {
	CID_OTHER_HEARTBEAT			= 1, //心跳
	CID_OTHER_RESPONSE			= 2, //返回响应
	CID_OTHER_REG_CLIENT_TYPE 	= 3, //注册客户端类型   如PHP管理端   c++客户端
	CID_OTHER_CLIENT_DONE 		= 4, //client 完成作业了(没有新的数据)
};



#define PARSE_PACKET_ASSERT if (is.GetPos() != (len - IM_PDU_HEADER_LEN)) { \
		throw CPduException(m_pdu_header.module_id, m_pdu_header.command_id, ERROR_CODE_PARSE_FAILED, "parse packet failed"); \
	}

#define ALLOC_FAIL_ASSERT(p) if (p == NULL) { \
		throw CPduException(m_pdu_header.module_id, m_pdu_header.command_id, ERROR_CODE_ALLOC_FAILED, "allocate failed"); \
	}

#define DLL_MODIFIER

//客户端配置
typedef struct {
	char *group;
	char *ip;
	char *emailFromAddr;
} client_config_t;

typedef map<string, shared_ptr<client_config_t> > client_config_map_t;


//////////////////////////////
typedef struct {
	uint32_t 	length;		// the whole pdu length
	uint16_t	module_id;	//
	uint16_t	command_id;	//
	uint16_t 	version;	// pdu version number
	uint16_t	reserved;	//
} PduHeader_t;

class DLL_MODIFIER CImPdu
{
public:
	CImPdu();
	virtual ~CImPdu() {}

	uchar_t* GetBuffer();
	uint32_t GetLength();

	uint16_t GetVersion() { return m_pdu_header.version; }
	uint16_t GetModuleId() { return m_pdu_header.module_id; }
	uint16_t GetCommandId() { return m_pdu_header.command_id; }
	uint16_t GetReserved() { return m_pdu_header.reserved; }

	void SetVersion(uint16_t version);
	void SetReserved(uint16_t reserved);

	void WriteHeader();
	virtual uint16_t GetPduType() { return 0; }

	static int ReadPduHeader(uchar_t* buf, uint32_t len, PduHeader_t* header);
	static CImPdu* ReadPdu(uchar_t* buf, uint32_t len);
private:
	static CImPdu* ReadPduCommand(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);
	static CImPdu* ReadPduMsg(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);
	static CImPdu* ReadPduOther(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);

	static bool _IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len);
	void _SetIncomingLen(uint32_t len) { m_incoming_len = len; }
	void _SetIncomingBuf(uchar_t* buf) { m_incoming_buf = buf; }

protected:
	CSimpleBuffer	m_buf;
	uchar_t*		m_incoming_buf;
	uint32_t		m_incoming_len;
	PduHeader_t		m_pdu_header;
};


#endif /* IMPDUBASE_H_ */
