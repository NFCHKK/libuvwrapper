
#include <memory>
#include <iostream>
#include <vector>
#include "./libuvcpp/loop.h"
#include "./libuvcpp/accept.h"
#include "./libuvcpp/connect.h"
#include "./libuvcpp/timer.h"
#include "./libuvcpp/tcpServer.h"
#include "./libuvcpp/tcpClient.h"


std::vector<std::shared_ptr<uvconnect>> vReceivedCon;


void OnHandlerCallback(evType eType, char *buf, ssize_t nread)
{
	switch (eType)
	{
	case EV_READ:
		//std::cout << "hello" << std::endl;
	case EV_WRITE:
		// ignore
		break;
	case EV_CLOSE:
		//need to add a new argument to identify which connection was closed
		break;
	default:
		break;
	}

}

void OnNewConnection(std::shared_ptr<uvconnect> pConn)
{
	{
		//libuv use single thread for connection 
		vReceivedCon.push_back(pConn);
		std::cout << "new connection" << std::endl;
	}
	pConn->SetEventHandler(OnHandlerCallback);
	pConn->StartReadData();
}

int main(int argc, char* argv[])
{
	//To create server
	std::shared_ptr<TcpServer> pTcp(new TcpServer);
	pTcp->tcpListen("0.0.0.0", 12396, OnNewConnection);

	//To create client
	//std::shared_ptr<TcpClient> pTcpClient(new TcpClient);
	//pTcpClient->ConnectToServer("127.0.0.1", 12396, OnHandlerCallback);
	//pTcpClient->WriteData("hello, world!", strlen("hello, world!"));

	//system("pause");
	std::cout << "Please waint input " << std::endl;
	int i ;
	std::cin >> i;
	return 0;
}

