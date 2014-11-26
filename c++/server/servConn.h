/*
 * servConn.h
 *
 *  Created on: 2014-11-4
 *  Author: lijianwei
 */

#ifndef SERVCONN_H_
#define SERVCONN_H_

#include "imconn.h"

class cServConn : public CImConn
{
public:
	cServConn();
	virtual ~cServConn();

	virtual void close();

	void onConnect2(net_handle_t handle);
	virtual void OnClose();
	virtual void HandlePdu(CImPdu* pPdu);
	virtual void OnTimer(uint64_t curr_tick);

	uint16_t getClientType() { return client_type;}
	void setClientType(uint16_t type) { client_type = type;}

private:
	int _HandleHeartBeat(CImPduHeartbeat *pdu);
	int _HandleClientData(CImPduClientData* pdu);
	int _HandleServerCommand(CImPduServerCommand* pdu);
	int _HandleServerAdjustRate(CImPduAdjustRate *pPdu);
	int _HandleRegClientType(CImPduRegClientType *pPdu);
	int _HandleServerStatusInfo(CImPduServerStatusInfo *pPdu);
	int _HandleAllClientStatusInfo(CImPduAllClientStatusInfo *pPdu);
	int _HandleClientDone(CImPduClientDone *pPdu);

private:
	uint16_t client_type;
};

void init_server_conn();
int send_all_client_pdu(CImPdu *pdu);

#endif /* SERVCONN_H_ */
