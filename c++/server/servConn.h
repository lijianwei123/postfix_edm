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

private:
	int _HandleClientTestData(CImPduClientTestData* pdu);
};

void init_server_conn();
int send_all_client_pdu(CImPdu *pdu);

#endif /* SERVCONN_H_ */
