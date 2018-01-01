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
# sample code:
~~~c++
#include "stdafx.h"
#include <memory>
#include <iostream>
#include <vector>
#include "loop.h"
#include "accept.h"
#include "connect.h"
#include "timer.h"

std::shared_ptr<uvloop> pLoop(new uvloop);
std::shared_ptr<uvaccept> pAcceptor(new uvaccept);
std::vector<std::shared_ptr<uvconnect>> vReceivedCon;

//read callback, directly print all the 
//received data
void OnReadCallback(ssize_t nread, char *buf)
{
	std::cout << buf << std::endl;
}

//New connection callback, store connection object and
//start reading...
void OnNewConnection(std::shared_ptr<uvconnect> pConn)
{
	{
		//libuv use single thread for connection
		//so there is no need to protect the vector
		vReceivedCon.push_back(pConn);
	}
	pConn->SetReadCallback(OnReadCallback);
	pConn->StartReadData();
}

int _tmain(int argc, _TCHAR* argv[])
{
	pLoop->run();
	pAcceptor->SetIPAndPort("0.0.0.0", 12396);
	pAcceptor->SetConnectCallback(OnNewConnection);
	pAcceptor->RegisterAccept(pLoop);
	pAcceptor->listen();
	
	system("pause");
	return 0;
}'
~~~
