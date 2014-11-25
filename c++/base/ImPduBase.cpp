/*
 * ImPduBase.cpp
 *
 *  Created on: 2013-8-27
 *      Author: ziteng@mogujie.com
 */

#include "ImPduBase.h"
#include "ImPduClient.h"



CImPdu::CImPdu()
{
	m_incoming_buf = NULL;
	m_incoming_len = 0;

	m_pdu_header.module_id = SID_OTHER;
	m_pdu_header.command_id = 0;
    m_pdu_header.version = IM_PDU_VERSION;
	m_pdu_header.reserved = 0;
}

uchar_t* CImPdu::GetBuffer()
{
	if (m_incoming_buf)
		return m_incoming_buf;
	else
		return m_buf.GetBuffer();
}

uint32_t CImPdu::GetLength()
{
	if (m_incoming_buf)
		return m_incoming_len;
	else
		return m_buf.GetWriteOffset();
}

void CImPdu::WriteHeader()
{
	uchar_t* buf = GetBuffer();

	CByteStream::WriteInt32(buf, GetLength());
	CByteStream::WriteUint16(buf + 4, m_pdu_header.module_id);
	CByteStream::WriteUint16(buf + 6, m_pdu_header.command_id);
    CByteStream::WriteUint16(buf + 8, m_pdu_header.version);
	CByteStream::WriteUint16(buf + 10, m_pdu_header.reserved);
}

void CImPdu::SetVersion(uint16_t version)
{
	uchar_t* buf = GetBuffer();
	CByteStream::WriteUint16(buf + 8, version);
}

void CImPdu::SetReserved(uint16_t reserved)
{
	uchar_t* buf = GetBuffer();
	CByteStream::WriteUint16(buf + 10, reserved);
}

int CImPdu::ReadPduHeader(uchar_t* buf, uint32_t len, PduHeader_t* header)
{
	int ret = -1;
	if (len >= IM_PDU_HEADER_LEN && buf && header) {
		CByteStream is(buf, len);

		is >> header->length;
		is >> header->module_id;
		is >> header->command_id;
        is >> header->version;
		is >> header->reserved;

		ret = 0;
	}

	return ret;
}

CImPdu* CImPdu::ReadPdu(uchar_t *buf, uint32_t len)
{
	uint32_t pdu_len = 0;
	if (!_IsPduAvailable(buf, len, pdu_len))
		return NULL;

	uint16_t service_id = CByteStream::ReadUint16(buf + 4);
	uint16_t command_id = CByteStream::ReadUint16(buf + 6);
	CImPdu* pPdu = NULL;

	switch (service_id)
	{
	case SID_COMMAND:
		pPdu = ReadPduCommand(command_id, buf, pdu_len);
		break;
	case SID_MSG:
		pPdu = ReadPduMsg(command_id, buf, pdu_len);
		break;
	case SID_OTHER:
		pPdu = ReadPduOther(command_id, buf, pdu_len);
		break;
	default:
		throw CPduException(service_id, command_id, ERROR_CODE_WRONG_SERVICE_ID, "wrong service id");
	}

	pPdu->_SetIncomingLen(pdu_len);
	pPdu->_SetIncomingBuf(buf);
	return pPdu;
}

CImPdu* CImPdu::ReadPduCommand(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;
	switch (command_id) {
	case CID_COMMAND_START:
	case CID_COMMAND_STOP:
	case CID_COMMAND_PAUSE:
	case CID_COMMAND_RESUME:
		pPdu = new CImPduServerCommand(pdu_buf, pdu_len);
		break;
	case CID_COMMAND_ADJUST_RATE:
		pPdu = new CImPduAdjustRate(pdu_buf, pdu_len);
		break;
	default:
		throw CPduException(SID_COMMAND, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong command id");
	}

	return pPdu;
}

CImPdu* CImPdu::ReadPduMsg(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;
	switch (command_id) {
	case CID_MSG_DATA:
		pPdu = new CImPduClientData(pdu_buf, pdu_len);
		break;
	case CID_MSG_SERVER_STATUS_INFO:
		pPdu = new CImPduServerStatusInfo(pdu_buf, pdu_len);
		break;
	case CID_MSG_ALL_CLIENT_STATUS_INFO:
		pPdu = new CImPduAllClientStatusInfo(pdu_buf, pdu_len);
		break;
	default:
		throw CPduException(SID_MSG, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong command id");
	}

	return pPdu;
}

CImPdu* CImPdu::ReadPduOther(uint16_t command_id, uchar_t* pdu_buf, uint32_t pdu_len)
{
	CImPdu* pPdu = NULL;
	switch (command_id)
	{
	case CID_OTHER_HEARTBEAT:
		pPdu = new CImPduHeartbeat(pdu_buf, pdu_len);
		break;
	case CID_OTHER_RESPONSE:
		pPdu = new CImPduResponse(pdu_buf, pdu_len);
		break;
	case CID_OTHER_REG_CLIENT_TYPE:
		pPdu = new CImPduRegClientType(pdu_buf, pdu_len);
		break;
	default:
		throw CPduException(SID_OTHER, command_id, ERROR_CODE_WRONG_COMMAND_ID, "wrong packet type");
		return NULL;
	}

	return pPdu;
}


bool CImPdu::_IsPduAvailable(uchar_t* buf, uint32_t len, uint32_t& pdu_len)
{
	if (len < IM_PDU_HEADER_LEN)
		return false;

	pdu_len = CByteStream::ReadUint32(buf);
	if (pdu_len > len)
	{
		//log("pdu_len=%d, len=%d\n", pdu_len, len);
		return false;
	}

	return true;
}

