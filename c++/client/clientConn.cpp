/*
 * clientConn.cpp
 *
 *  Created on: 2014-11-4
 *  Author: lijianwei
 */
#include "util.h"
#include "clientConn.h"
#include "worker.h"

char *serverIp = NULL;
char *serverPort = NULL;

static ConnMap_t g_serv_conn_map;

static void client_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	uint64_t curr_time = get_tick_count();
	for (ConnMap_t::iterator it = g_serv_conn_map.begin(); it != g_serv_conn_map.end();) {
		ConnMap_t::iterator it_old = it;
		it++;

		cClientConn *pConn = (cClientConn *)it_old->second;
		pConn->OnTimer(curr_time);
	}
}

//向服务端发送数据包
int send_server_pdu(CImPdu *pdu)
{
	for (ConnMap_t::iterator it = g_serv_conn_map.begin(); it != g_serv_conn_map.end(); ++it) {
		cClientConn *pConn = (cClientConn *)it->second;
		if (pConn)
			pConn->SendPdu(pdu);
	}
	return 0;
}

void connect_server()
{
	//shared_ptr<cClientConn> clientConn = make_shared<cClientConn>();
	//自带了一套自动清除内存方式,不需要shared_ptr了
	cClientConn *clientConn = new cClientConn();
	clientConn->connect(serverIp, (uint16_t)atoi(serverPort));
}

void init_client_conn()
{
	log("init_client_conn...\n");
	//获取服务器status
	CImPduServerStatusInfo pdu;
	send_server_pdu(&pdu);

	netlib_register_timer(client_conn_timer_callback, NULL, 5000);
}


cClientConn::cClientConn()
{
	m_bOpen = false;
}

cClientConn::~cClientConn()
{
	log("cClientConn die\n");
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
	log("connect server success!\n");

	//注册客户端类型
	CImPduRegClientType regPdu((uint16_t)CLIENT_CPP);
	SendPdu(&regPdu);

	//发送测试数据
	char *data = const_cast<char *>("hello world");
	uint32_t data_len = strlen(data);

	CImPduClientData pdu(data, data_len);
	SendPdu(&pdu);
}

void cClientConn::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + CLIENT_HEARTBEAT_INTERVAL) {
		//发回client_status_info
		shared_ptr<client_status_info> client_status_ptr = make_shared<client_status_info>();

		client_status_ptr->ip = strdup(getLocalIp());
		client_status_ptr->emailFromAddr = strdup(emailFromAddr);
		client_status_ptr->havePushNum = shm_ptr->havePushNum;
		client_status_ptr->mailqNum = cWorker::instance->GetMailqNum();

		CImPduHeartbeat pdu(client_status_ptr);
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {

		CBaseSocket* pSocket = FindBaseSocket(m_handle);
		log("server connect %s:%d timeout\n", pSocket->GetRemoteIP(), pSocket->GetRemotePort());
		pSocket->ReleaseRef();

		close();

		//确保一直有连接
		if (g_serv_conn_map.size() < 1) {
			connect_server();
		}
	}
}


void cClientConn::HandlePdu(CImPdu *pPdu)
{
	switch (pPdu->GetPduType()) {
	case IM_PDU_TYPE_HEARTBEAT:
		break;
	case IM_PDU_TYPE_RESPONSE:
		_HandleServerResponse((CImPduResponse *)pPdu);
		break;
	case IM_PDU_TYPE_SERVER_COMMAND:
		_HandleServerCommand((CImPduServerCommand *)pPdu);
		break;
	case IM_PDU_TYPE_ADJUST_RATE:
		_HandleServerAdjustRate((CImPduAdjustRate *)pPdu);
		break;
	case IM_PDU_TYPE_SERVER_STATUS_INFO:
		_HandleServerStatusInfo((CImPduServerStatusInfo *)pPdu);
		break;
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

	cWorker::instance->setMulti(multi);
	cWorker::instance->setDelay(delay);

	return 0;
}

int cClientConn::_HandleServerResponse(CImPduResponse *pdu)
{
	uint16_t status = pdu->GetStatus();
	string message(pdu->GetMessage(), pdu->GetMessageLen());

	log("server response status: %u, message: %s\n", status, message.c_str());

	return 0;
}

//老大的命令 到了  attack
int cClientConn::_HandleServerStatusInfo(CImPduServerStatusInfo *pPdu)
{
	log("_HandleServerStatusInfo starting...\n");
	int ret = -1;
	string json_data(pPdu->GetJsonData(), pPdu->GetJsonDataLen());

	server_status_info status_info;
	ret = status_info.unserialize(json_data.c_str());
	//解析成功
	if (ret == 0) {
		cWorker::instance->setStatus(status_info.status);
		cWorker::instance->setMulti(status_info.multi);
		cWorker::instance->setDelay(status_info.delay);
	}
	return 0;
}

