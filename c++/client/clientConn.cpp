/*
 * clientConn.cpp
 *
 *  Created on: 2014-11-4
 *  Author: lijianwei
 */
#include "clientConn.h"
#include "worker.h"

static ConnMap_t g_serv_conn_map;

static void client_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	uint64_t curr_time = get_tick_count();
	for (ConnMap_t::iterator it = g_serv_conn_map.begin(); it != g_serv_conn_map.end(); ++it) {
		cClientConn *pConn = (cClientConn *)it->second;
		pConn->OnTimer(curr_time);
	}
}

void init_client_conn()
{
	netlib_register_timer(client_conn_timer_callback, NULL, 5000);
}

//向服务端发送数据包
int send_server_pdu(CImPdu *pdu)
{
	for (ConnMap_t::iterator it = g_serv_conn_map.begin(); it != g_serv_conn_map.end(); ++it) {
		cClientConn *pConn = (cClientConn *)it->second;
		if (pConn)
			pConn->SendPdu(&pdu);
	}
	return 0;
}

cClientConn::cClientConn()
{
	m_bOpen = false;
}

cClientConn::~cClientConn()
{
	log("cClientConn die");
}

void cClientConn::connect(const char *serverIp, uint16_t serverPort)
{
	log("Connecting to Server %s:%d\n", serverIp, serverPort);

	m_handle = netlib_connect(serverIp, serverPort, imconn_callback, &g_serv_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_serv_conn_map.insert(make_pair(m_handle, this));
	}
}

void cClientConn::close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_serv_conn_map.erase(m_handle);
	}
	ReleaseRef();
}


void cClientConn::OnClose()
{
	close();
}

void cClientConn::OnConfirm()
{
	//说明连接上了
	log("connect server success!");

	//发送测试数据
	char *data = const_cast<char *>("hello world");
	uint32_t data_len = strlen(data);

	CImPduClientTestData pdu(data, data_len);
	SendPdu(&pdu);
}

void cClientConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + CLIENT_HEARTBEAT_INTERVAL) {
		CImPduHeartbeat pdu;
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {

		CBaseSocket* pSocket = FindBaseSocket(m_handle);
		log("server connect %s:%d timeout\n", pSocket->GetRemoteIP(), pSocket->GetRemotePort());
		pSocket->ReleaseRef();

		Close();
	}
}


void cClientConn::HandlePdu(CImPdu *pdu)
{
	switch (pPdu->GetPduType()) {
	case IM_PDU_TYPE_HEARTBEAT:
		break;
	case IM_PDU_TYPE_SERVER_COMMAND:
		_HandleServerCommand((CImPduServerCommand *)pPdu);
		break;
	case IM_PDU_TYPE_ADJUST_RATE:
		_HandleServerAdjustRate((CImPduAdjustRate *)pPdu);
	default:
		log("no such pdu_type: %u\n", pPdu->GetPduType());
		break;
	}
}

int cClientConn::_HandleServerCommand(CImPduServerCommand *pdu)
{
	uint16_t serverCommand = pdu->GetServCommand();
	cWorker::instance->setStatus(serverCommand);

	return 0;
}

int cClientConn::_HandleServerAdjustRate(CImPduAdjustRate *pdu)
{
	uint16_t delay = pdu->GetDelay();
	uint16_t multi = pdu->GetMulti();

	cWorker::instance->setCont(multi);
	cWorker::instance->setDelay(delay);
}
