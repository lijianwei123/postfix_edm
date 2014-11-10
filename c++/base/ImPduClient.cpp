/*
 * ImPduClient.cpp
 * Interactive Packet with client
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#include "ImPduClient.h"
#include <stdlib.h>
#include <ctime>


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
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_ADJUST_RATE;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << delay;
	os << multi;

	WriteHeader();
}


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
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = servCommand;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os << servCommand;

	WriteHeader();
}

//心跳
CImPduHeartbeat::CImPduHeartbeat()
{
	m_pdu_header.command_id = IM_PDU_TYPE_HEARTBEAT;
	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	WriteHeader();
}

//解包
CImPduClientTestData::CImPduClientTestData(uchar_t* buf, uint32_t len)
{
	ReadPduHeader(buf, IM_PDU_HEADER_LEN, &m_pdu_header);
	CByteStream is(buf + IM_PDU_HEADER_LEN, len - IM_PDU_HEADER_LEN);
	data = NULL;

	data = is.ReadString(data_len);

	PARSE_PACKET_ASSERT
}

//组包
CImPduClientTestData::CImPduClientTestData(char *data, uint32_t data_len)
{
	m_pdu_header.module_id = SID_MSG;
	m_pdu_header.command_id = CID_MSG_TEST;

	CByteStream os(&m_buf, IM_PDU_HEADER_LEN);
	m_buf.Write(NULL, IM_PDU_HEADER_LEN);

	os.WriteString(data, data_len);

	WriteHeader();

}

