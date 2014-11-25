#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>



char *getLocalIp()
{
	static char ip[20] = {0};
	if (!ip) return ip;

	struct sockaddr_in clientAddr;
	int clientLen = sizeof(clientAddr);
	//和服务端打个招呼
	int clientSocket;
	struct sockaddr_in serverAddr;

	//创建个socket
	if((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return NULL;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(53);
	serverAddr.sin_addr.s_addr = inet_addr("114.114.114.114");

	//连接服务端
	if(connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		return NULL;
	}

	if (getsockname(clientSocket, (struct sockaddr *)&clientAddr, &clientLen) < 0) {
		return NULL;
	}
	inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip));

	return ip;
}

int main()
{
	printf("%s\n", getLocalIp());
	
	return 0;

}
