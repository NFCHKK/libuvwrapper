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

std::shared_ptr<uvloop> pLoop(new uvloop);
std::shared_ptr<uvaccept> pAcceptor(new uvaccept);

std::vector<std::shared_ptr<uvconnect>> vReceivedCon;


void OnReadCallback(ssize_t nread, char *buf)
{
	std::cout << buf << std::endl;
}

void OnNewConnection(std::shared_ptr<uvconnect> pConn)
{
	{
		//libuv use single thread for connection 
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
}

