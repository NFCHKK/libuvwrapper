#pragma once
#include "loop.h"
#include "accept.h"
#include <memory>
#include <functional>

class TcpClient: public std::enable_shared_from_this<TcpClient>
{
public:
	TcpClient();
	~TcpClient();

	bool ConnectToServer(std::string ip, unsigned int port, eventcb evHandler);

	bool WriteData(char *pbuf, unsigned long len);

private:
	std::shared_ptr<uvloop> m_pLoop;
	std::shared_ptr<uvconnect> m_pConn;
};
