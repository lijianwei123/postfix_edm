/*
 * servConn.cpp
 *
 *  Created on: 2014-11-4
 *  Author: lijianwei
 */
#include "servConn.h"
#include "manager.h"

//客户端连接
static ConnMap_t g_client_conn_map;

static void server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	log("g_client_conn_map size: %d\n", g_client_conn_map.size());
	uint64_t curr_time = get_tick_count();
	for (ConnMap_t::iterator it = g_client_conn_map.begin(); it != g_client_conn_map.end(); ) {
		ConnMap_t::iterator it_old = it;
		it++;

		cServConn *pConn = (cServConn *)it_old->second;
		if (pConn && pConn->getClientType() != CLIENT_PHP) {
			pConn->OnTimer(curr_time);
		}
	}
	//统计最新的online_client_num
	uint16_t online_client_num = 0;
	for (ConnMap_t::iterator it = g_client_conn_map.begin(); it != g_client_conn_map.end();) {
		ConnMap_t::iterator it_old = it;
		it++;


		cServConn *pConn = (cServConn *)it_old->second;
		if (pConn && pConn->getClientType() != CLIENT_PHP) {
			online_client_num++;
		}
	}
	cManager::instance->GetMonitor()->setOnlineClientNum(online_client_num);
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
		if (pConn && pConn->getClientType() != CLIENT_PHP) {
			pConn->SendPdu(pdu);
		}
	}
	return 0;
}



//构造函数
cServConn::cServConn()
{
}

cServConn::~cServConn()
{
	log("cServConn die\n");
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
		CBaseSocket* pSocket = FindBaseSocket(m_handle);
		const char *ip = pSocket->GetRemoteIP();
		pSocket->ReleaseRef();

		//删除client status
		cManager::instance->GetMonitor()->removeClientStauts(ip);

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

		close();
	}
}


void cServConn::HandlePdu(CImPdu *pPdu)
{
	switch (pPdu->GetPduType()) {
	//心跳
	case IM_PDU_TYPE_HEARTBEAT:
		_HandleHeartBeat((CImPduHeartbeat *)pPdu);
		break;
	//发送data
	case IM_PDU_TYPE_DATA:
			_HandleClientData((CImPduClientData *)pPdu);
		break;
	//处理CImPduServerCommand
	case IM_PDU_TYPE_SERVER_COMMAND:
			_HandleServerCommand((CImPduServerCommand *)pPdu);
		break;
	case IM_PDU_TYPE_ADJUST_RATE:
			_HandleServerAdjustRate((CImPduAdjustRate *)pPdu);
		break;
	//注册客户端类型
	case IM_PDU_TYPE_REG_CLIENT_TYPE:
			_HandleRegClientType((CImPduRegClientType *)pPdu);
		break;
	case IM_PDU_TYPE_SERVER_STATUS_INFO:
			_HandleServerStatusInfo((CImPduServerStatusInfo *)pPdu);
		break;
	case IM_PDU_TYPE_ALL_CLIENT_STATUS_INFO:
			_HandleAllClientStatusInfo((CImPduAllClientStatusInfo *)pPdu);
		break;
	//有一个小弟干完活
	case IM_PDU_TYPE_CLIENT_DOEN:
			_HandleClientDone((CImPduClientDone *)pPdu);
		break;
	default:
		log("no such pdu_type: %u\n", pPdu->GetPduType());
		break;
	}
}

//孩儿们的情况
int cServConn::_HandleHeartBeat(CImPduHeartbeat *pdu)
{
	int ret = -1;
	string json_data(pdu->GetJsonData(), pdu->GetJsonDataLen());

	shared_ptr<client_status_info> status_info_ptr = make_shared<client_status_info>();
	ret = status_info_ptr->unserialize(json_data.c_str());

	//统一报到参谋本部
	if (ret == 0) {
		cManager::instance->GetMonitor()->UpdateClientStatus(status_info_ptr);
	}

	return 0;
}

int cServConn::_HandleClientData(CImPduClientData* pdu)
{
	string data(pdu->GetData(), pdu->GetDataLen());
	cout << "from client data:" << data << endl;

	CImPduResponse respPdu(0, const_cast<char *>("操作成功"));
	SendPdu(&respPdu);

	return 0;
}

int cServConn::_HandleServerCommand(CImPduServerCommand* pdu)
{
	uint16_t command = pdu->GetServCommand();

	int ret = -1;

	switch (command) {
	case CID_COMMAND_START:
		ret = cManager::instance->start();
		break;
	case CID_COMMAND_STOP:
		ret = cManager::instance->stop();
		break;
	case CID_COMMAND_PAUSE:
		ret = cManager::instance->pause();
		break;
	case CID_COMMAND_RESUME:
		ret = cManager::instance->resume();
		break;
	}


	if (!ret) {
		CImPduResponse respPdu(0, const_cast<char *>("操作成功"));
		SendPdu(&respPdu);
	} else {
		CImPduResponse respPdu(-1, const_cast<char *>("操作失败"));
		SendPdu(&respPdu);
	}

	return 0;
}

//调整发送速率
int cServConn::_HandleServerAdjustRate(CImPduAdjustRate *pPdu)
{
	int ret = -1;
	uint16_t delay = pPdu->GetDelay();
	uint16_t multi = pPdu->GetMulti();

	ret = cManager::instance->adjustSendRate(delay, multi);

	if (!ret) {
		CImPduResponse respPdu(0, const_cast<char *>("操作成功"));
		SendPdu(&respPdu);
	} else {
		CImPduResponse respPdu(-1, const_cast<char *>("操作失败"));
		SendPdu(&respPdu);
	}

	return 0;
}

//设置客户端类型
int cServConn::_HandleRegClientType(CImPduRegClientType *pPdu)
{
	uint16_t type = pPdu->GetClientType();
	setClientType(type);

	return 0;
}

//server status
int cServConn::_HandleServerStatusInfo(CImPduServerStatusInfo *pPdu)
{
	//直接返回server status
	shared_ptr<server_status_info> server_status_ptr = make_shared<server_status_info>();
	server_status_ptr->status = cManager::instance->GetStatus();
	server_status_ptr->delay = cManager::instance->GetDelay();
	server_status_ptr->multi = cManager::instance->GetMulti();
	server_status_ptr->totalNum = cManager::instance->GetMonitor()->getTotalNum();
	server_status_ptr->online_client_num = cManager::instance->GetMonitor()->getOnlineClientNum();

	CImPduServerStatusInfo pdu(server_status_ptr);
	SendPdu(&pdu);

	return 0;
}


int cServConn::_HandleAllClientStatusInfo(CImPduAllClientStatusInfo *pPdu)
{
	client_status_map_t client_status_maps = cManager::instance->GetMonitor()->GetClientStatusMap();
	char *json_data = NULL;

	cJSON *root = NULL;
	root = cJSON_CreateObject();

	for (client_status_map_t::iterator it = client_status_maps.begin(); it != client_status_maps.end();) {
		client_status_map_t::iterator it_old = it;
		it++;

		cJSON_AddItemToObject(root, it_old->first.c_str(), it_old->second->getJsonObject());
	}
	json_data = cJSON_Print(root);
	cJSON_Delete(root);

	CImPduAllClientStatusInfo  pdu(json_data);
	SendPdu(&pdu);

	free(json_data);

	return 0;
}

int cServConn::_HandleClientDone(CImPduClientDone *pPdu)
{
	int64_t remainSentNum = cManager::instance->GetMonitor()->getSentEmailNum();

	//如果都没有数据了，就先暂停吧
	if (remainSentNum == 0) {
		cManager::instance->pause();
	}

	return 0;
}

