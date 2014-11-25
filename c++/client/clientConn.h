/*
 * clientConn.h
 *
 *  Created on: 2014-11-4
 *  Author: Administrator
 */

#ifndef CLIENTCONN_H_
#define CLIENTCONN_H_

#include "imconn.h"

extern char *serverIp;
extern char *serverPort;

class cClientConn : public CImConn
{
public:
	cClientConn();
	virtual ~cClientConn();

	void connect(const char *serverIp, uint16_t serverPort);
	virtual void close();

	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	virtual void HandlePdu(CImPdu *pdu);
private:
	int _HandleServerCommand(CImPduServerCommand *pdu);
	int _HandleServerAdjustRate(CImPduAdjustRate *pdu);
	int _HandleServerResponse(CImPduResponse *pdu);
	int _HandleServerStatusInfo(CImPduServerStatusInfo *pPdu);
private:
	bool m_bOpen;
};

void connect_server();
void init_client_conn();

#endif /* CLIENTCONN_H_ */
