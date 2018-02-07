#pragma once
#include "loop.h"
#include "accept.h"
#include <memory>
#include <functional>


class TcpServer:public std::enable_shared_from_this<TcpServer>
{
public:
	TcpServer();
	~TcpServer();

	bool tcpListen(std::string ip, unsigned int port, NewConnectcb ConnectCallback);
	bool tcpRun();
	bool tcpStop();
	std::shared_ptr<uvloop> m_pLoop;
	std::shared_ptr<uvaccept> m_pListner;

private:
	std::string m_ip;
	unsigned int m_port;
	NewConnectcb m_ConCB;
};
