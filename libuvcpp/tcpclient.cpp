#include "tcpClient.h"


TcpClient::TcpClient()
: m_pConn(new uvconnect)
, m_pLoop(new uvloop)
{

}

TcpClient::~TcpClient()
{

}

bool TcpClient::ConnectToServer(std::string ip, unsigned int port, eventcb evHandler)
{
	if (!m_pLoop->IsRunning())
	{
		m_pLoop->run();
	}

	m_pConn->RegisterConnect(m_pLoop);
	m_pConn->SetEventHandler(evHandler);
	return m_pConn->ConnectToServer();
}

bool TcpClient::WriteData(char *pbuf, unsigned long len)
{
	return  m_pConn->wirteData(pbuf, len);
}

