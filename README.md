# libuvwrapper
    this is a very simple libuv wrapper using c++
#### loop.h, loop.cpp: 
    uvloop class
#### accept.h, accept.cpp:
    uvaccept class, used for listening and accepting clients' connections
#### connect.h, connect.cpp:
    uvconnect class, used for clients' connecting to server, or  server holding connections with clients
#### timer.h, timer.cpp:
    uvtimer class, also driven by uvloop
#### tcpserver.h, tcpserver.cpp; tcpclient.h tcpclient.cpp
	implementation of server and client
# sample code:

```c++
// uvwrapper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory>
#include <iostream>
#include <vector>
#include "loop.h"
#include "accept.h"
#include "connect.h"
#include "timer.h"
#include "tcpserver.h"
#include "tcpclient.h"


std::vector<std::shared_ptr<uvconnect>> vReceivedCon;


void OnHandlerCallback(evType eType, char *buf, ssize_t nread)
{
	switch (eType)
	{
	case EV_READ:
		std::cout << buf << std::endl;
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
	}
	pConn->SetEventHandler(OnHandlerCallback);
	pConn->StartReadData();
}

int _tmain(int argc, _TCHAR* argv[])
{
	//To create server
	std::shared_ptr<TcpServer> pTcp(new TcpServer);
	pTcp->tcpListen("0.0.0.0", 12396, OnNewConnection);
	
	//To create client
	std::shared_ptr<TcpClient> pTcpClient(new TcpClient);
	pTcpClient->ConnectToServer("127.0.0.1", 12396, OnHandlerCallback);
	pTcpClient->WriteData("hello, world!", strlen("hello, world!"));
	
	system("pause");
	return 0;
}
```
