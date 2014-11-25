/*
 * ImPduClient.cpp
 * Interactive Packet with client
 *
 *  Created on: 2013-8-27
 *  Author: ziteng@mogujie.com
 */

#include "ImPduClient.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctime>


//====================================SID_COMMAND  START=========================================================================

//解包
CImPduServerCommand::CImPduServerCommand(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	servCommand = 0;
	is >> servCommand;

	PARSE_PACKET_ASSERT
}

//组包
CImPduServerCommand::CImPduServerCommand(uint16_t servCommand)
{
	m_pdu_header.module_id = SID_COMMAND;
	m_pdu_header.command_id = servCommand;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << servCommand;

	WriteHeader();
}
//=====================================================================

//解包
CImPduAdjustRate::CImPduAdjustRate(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	delay = 0;
	multi = 0;
	is >> delay;
	is >> multi;

	PARSE_PACKET_ASSERT
}

//组包
CImPduAdjustRate::CImPduAdjustRate(uint16_t delay, uint16_t multi)
{
	m_pdu_header.module_id = SID_COMMAND;
	m_pdu_header.command_id = CID_COMMAND_ADJUST_RATE;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << delay;
	os << multi;

	WriteHeader();
}

//====================================SID_COMMAND  END=========================================================================


//====================================SID_MSG  START===========================================================================

//解包
CImPduClientData::CImPduClientData(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
	data = NULL;

	data = is.ReadString(data_len);

	PARSE_PACKET_ASSERT
}

//组包
CImPduClientData::CImPduClientData(char *data, uint32_t data_len)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_DATA;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(data, data_len);

	WriteHeader();
}

//=====================================================================

//解包client 发给  server 请求包   或者  server 发给client 的响应
CImPduServerStatusInfo::CImPduServerStatusInfo(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	if (len > IM_PDU_HEADER_LEN) {
		json_data = is.ReadString(json_data_len);
	}

	PARSE_PACKET_ASSERT
}

//组包client->server 请求
CImPduServerStatusInfo::CImPduServerStatusInfo()
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_SERVER_STATUS_INFO;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	WriteHeader();
}

//组包  server->client 响应
CImPduServerStatusInfo::CImPduServerStatusInfo(shared_ptr<server_status_info> server_status_ptr)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_SERVER_STATUS_INFO;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(server_status_ptr->serialize());

	WriteHeader();
}

//=====================================================================

//解包   所有客户端状态
CImPduAllClientStatusInfo::CImPduAllClientStatusInfo(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	PARSE_PACKET_ASSERT
}

//组包
CImPduAllClientStatusInfo::CImPduAllClientStatusInfo(char *json_data)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_ALL_CLIENT_STATUS_INFO;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(json_data);

	WriteHeader();
}

//====================================SID_MSG  END===========================================================================

//====================================SID_OTHER  START=======================================================================

//解包
CImPduHeartbeat::CImPduHeartbeat(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	if (len > IM_PDU_HEADER_LEN) {
		json_data = is.ReadString(json_data_len);
	}

	PARSE_PACKET_ASSERT
}

//组包
CImPduHeartbeat::CImPduHeartbeat()
{
	m_pdu_header.command_id = CID_OTHER_HEARTBEAT;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	WriteHeader();
}

//组包
CImPduHeartbeat::CImPduHeartbeat(shared_ptr<client_status_info> client_status_ptr)
{
	m_pdu_header.command_id = CID_OTHER_HEARTBEAT;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(client_status_ptr->serialize());

	WriteHeader();
}

//=====================================================================

//解包
CImPduResponse::CImPduResponse(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	status = 0;
	message = NULL;
	message_len = 0;

	is >> status;
	message = is.ReadString(message_len);

	PARSE_PACKET_ASSERT
}

//组包
CImPduResponse::CImPduResponse(uint16_t status, char *message)
{
	m_pdu_header.command_id = CID_OTHER_RESPONSE;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << status;
	os.WriteString(message);

	WriteHeader();
}

//=====================================================================

//解包
CImPduRegClientType::CImPduRegClientType(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);

	client_type = 0;
	is >> client_type;

	PARSE_PACKET_ASSERT
}

//组包
CImPduRegClientType::CImPduRegClientType(uint16_t client_type)
{
	m_pdu_header.command_id = CID_OTHER_REG_CLIENT_TYPE;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << client_type;

	WriteHeader();
}

//====================================SID_OTHER  END=====================================================================

