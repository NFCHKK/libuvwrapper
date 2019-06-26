// netTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "uvcpp.h"

using namespace uv;
int _tmain(int argc, _TCHAR* argv[])
{

// 	netlibuv netIns;
// 	netIns.net_Connect("127.0.0.1", 50000);
// 	netIns.net_Run();
// 
// 	netIns.net_AsyncSend("hello", strlen("hello"));
// 	Sleep(100);
// 	netIns.net_Stop();
// 	//int i = 0;
// 	//std::cin >> i;
// 	std::cout << "hello world!" << std::endl;
// 	netIns.net_CloseConnection();

	uvcpp uvInst;
	uvInst.connect(std::string("127.0.0.1"), 50000);
	uvInst.setAutoReconnect(true);
	uvInst.run();

	int i = 0;
	while (i ++ < 5000)
	{
		uvInst.send("hello", 5);
		Sleep(1);
	}
	

	Sleep(10000);
	uvInst.close();
	system("pause");
	i = 0;
	while (i++ < 5000)
	{
		uvInst.send("hello", 5);
		Sleep(1);
	}
	uvInst.stop();
	system("pause");
	return 0;
}

