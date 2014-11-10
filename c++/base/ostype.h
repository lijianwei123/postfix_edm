// OS dependant type definition
#ifndef __OS_TYPE_H__
#define __OS_TYPE_H__

#include <sys/epoll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>		// define int8_t ...
#include <signal.h>
#define closesocket close
#define ioctlsocket ioctl


typedef int	SOCKET;
typedef int BOOL;
const int TRUE = 1;
const int FALSE = 0;
const int SOCKET_ERROR	= -1;
const int INVALID_SOCKET = -1;

typedef unsigned char	uchar_t;
typedef int				net_handle_t;
typedef int				conn_handle_t;

enum {
	NETLIB_OK		= 0,
	NETLIB_ERROR	= -1
};

#define NETLIB_INVALID_HANDLE	-1

enum
{
	NETLIB_MSG_CONNECT = 1,
	NETLIB_MSG_CONFIRM,
	NETLIB_MSG_READ,
	NETLIB_MSG_WRITE,
	NETLIB_MSG_CLOSE,
	NETLIB_MSG_TIMER
};

typedef void (*callback_t)(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);

#endif
