/*
 * servConn.cpp
 *
 *  Created on: 2014-11-4
 *  Author: lijianwei
 */
#include "servConn.h"

//客户端连接
static ConnMap_t g_client_conn_map;

static void server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	uint64_t curr_time = get_tick_count();
	for (ConnMap_t::iterator it = g_client_conn_map.begin(); it != g_client_conn_map.end(); ++it) {
		cServConn *pConn = (cServConn *)it->second;
		pConn->OnTimer(curr_time);
	}
}

void init_server_conn()
{
	netlib_register_timer(server_conn_timer_callback, NULL, 5000);
}

//向所有客户端发送数据包
int send_all_client_pdu(CImPdu *pdu)
{
	for (ConnMap_t::iterator it = g_client_conn_map.begin(); it != g_client_conn_map.end(); ++it) {
		cServConn *pConn = (cServConn *)it->second;
		if (pConn)
			pConn->SendPdu(&pdu);
	}
	return 0;
}



//构造函数
cServConn::cServConn()
{
}

cServConn::~cServConn()
{
}

void cServConn::onConnect2(net_handle_t handle)
{
	m_handle = handle;
	ConnMap_t *connMap = &g_client_conn_map;

	connMap->insert(make_pair(handle, this));

	netlib_option(handle, NETLIB_OPT_SET_CALLBACK, (void *)imconn_callback);
	netlib_option(handle, NETLIB_OPT_SET_CALLBACK_DATA, (void *)connMap);
}


void cServConn::close()
{
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_client_conn_map.erase(m_handle);
	}
	ReleaseRef();
}


void cServConn::OnClose()
{
	close();
}


void cServConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
		CImPduHeartbeat pdu;
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + CLIENT_TIMEOUT) {

		CBaseSocket* pSocket = FindBaseSocket(m_handle);
		log("client %s:%d timeout\n", pSocket->GetRemoteIP(), pSocket->GetRemotePort());
		pSocket->ReleaseRef();

		Close();
	}
}


void cServConn::HandlePdu(CImPdu *pPdu)
{
	switch (pPdu->GetPduType()) {
	//心跳
	case IM_PDU_TYPE_HEARTBEAT:
		break;
	//测试发送
	case IM_PDU_TYPE_TEST_DATA:
			_HandleClientTestData((CImPduClientTestData *)pPdu);
		break;
	default:
		log("no such pdu_type: %u\n", pPdu->GetPduType());
		break;
	}
}

int cServConn::_HandleClientTestData(CImPduClientTestData* pdu)
{
	char *data = pdu->GetData();
	cout << "from client data:" << data << endl;

	return 0;
}

