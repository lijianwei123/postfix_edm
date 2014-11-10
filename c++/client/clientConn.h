/*
 * clientConn.h
 *
 *  Created on: 2014-11-4
 *  Author: Administrator
 */

#ifndef CLIENTCONN_H_
#define CLIENTCONN_H_

#include "imconn.h"

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
	int cClientConn::_HandleServerCommand(CImPduServerCommand *pdu);
	int cClientConn::_HandleServerAdjustRate(CImPduAdjustRate *pdu);
private:
	bool m_bOpen;
};

void init_client_conn();

#endif /* CLIENTCONN_H_ */
