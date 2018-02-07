#include "tcpServer.h"


TcpServer::TcpServer()
		: m_pLoop(new uvloop)
		, m_pListner(new uvaccept)
{

}


bool TcpServer::tcpListen(std::string ip, unsigned int port, NewConnectcb ConnectCallback)
{
	if (!m_pLoop->IsRunning())
	{
		//run loop first
		tcpRun();
	}
	m_ip = ip;
	m_port = port;
	m_ConCB = ConnectCallback;
	m_pListner->RegisterAccept(m_pLoop);
	m_pListner->SetIPAndPort((char *)m_ip.c_str(), port);
	m_pListner->SetNewConnectCallback(ConnectCallback);
	m_pListner->listen();
	return true;
}

bool TcpServer::tcpRun()
{
	m_pLoop->run();
	return true;
}

bool TcpServer::tcpStop()
{
	m_pLoop->stop();
	return true;
}

TcpServer::~TcpServer()
{
	m_pLoop->stop();
}

