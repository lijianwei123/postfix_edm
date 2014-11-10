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
	SID_COMMAND			= 0x0001,
	SID_MSG 			= 0x0002,
	SID_OTHER 			= 0x0003,
};

//command id for command
enum {
	CID_COMMAND_START			= 1 << 0, //start
	CID_COMMAND_STOP			= 1 << 1, //stop
	CID_COMMAND_PAUSE			= 1 << 2, //pause
	CID_COMMAND_RESUME			= 1 << 3, //resume
};

//command id for msg
enum {
	CID_MSG_TEST 			= 1,
	CID_MSG_DATA			= 2,
	CID_MSG_ADJUST_RATE  	= 3,
};


#define PARSE_PACKET_ASSERT if (is.GetPos() != (len - IM_PDU_HEADER_LEN)) { \
		throw CPduException(m_pdu_header.module_id, m_pdu_header.command_id, ERROR_CODE_PARSE_FAILED, "parse packet failed"); \
	}

#define ALLOC_FAIL_ASSERT(p) if (p == NULL) { \
		throw CPduException(m_pdu_header.module_id, m_pdu_header.command_id, ERROR_CODE_ALLOC_FAILED, "allocate failed"); \
	}

#define DLL_MODIFIER

typedef struct {
	uint32_t 	from_user_id;
	uint32_t	from_name_len;
	char*	 	from_name;
	uint32_t	from_nick_name_len;
	char*		from_nick_name;
	uint32_t	from_avatar_len;
	char*		from_avatar_url;

	uint32_t 	create_time;
	uint8_t	 	msg_type;
	uint32_t 	msg_len;
	uchar_t* 	msg_data;
} server_msg_t;

typedef struct {
	uint32_t from_user_id;
	uint32_t unread_msg_cnt;
} UserUnreadMsgCnt_t;

typedef struct ip_addr_t{
    uint32_t ip_len;
    char*    ip;
    uint16_t port;
} ip_addr;

typedef struct svr_ip_addr_t{
	std::string ip;
	uint16_t port;
	
	svr_ip_addr_t() {
		port = 0;
	}

	svr_ip_addr_t (const char* addr, uint16_t p) {
		ip = addr;
		port = p;
	}
}svr_ip_addr;

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
	static CImPdu* ReadPduMsg(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len);


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
