#include "accept.h"


bool uvaccept::RegisterAccept(std::shared_ptr<uvloop> ploop)
{
	m_ploop = ploop;
	return true;
}

void uvaccept::SetIPAndPort(char *phost /*= "0.0.0.0"*/, unsigned int port /*= 12396*/)
{
	int iErr = uv_ip4_addr(phost, port, &m_addr);
	assert(iErr == 0);
}

bool uvaccept::listen()
{
	int iErr = m_ploop->AddReq(E_TCP_LISTEN, std::bind(&uvaccept::ServerListenStart, shared_from_this()));
	return (iErr == 0);
}

uvaccept::uvaccept()
	: m_ploop(nullptr)
{
	

}

uvaccept::~uvaccept()
{

}

void uvaccept::ServerListenStart()
{
	m_pServer.reset((uv_tcp_t *)malloc(sizeof(uv_tcp_t)));
	int iErr = uv_tcp_init(m_ploop->m_pUVLoop.get(), (uv_tcp_t *)m_pServer.get());
	assert(iErr == 0);
	iErr = uv_tcp_bind(m_pServer.get(), (sockaddr *)&m_addr, 0);
	assert(iErr == 0);
	m_pServer->data = this;

	iErr = uv_listen((uv_stream_t *)m_pServer.get(), 1024, [](uv_stream_t *server, int status)
	{
		if (status < 0)
		{
			//err
			return;
		}
		else
		{
			uvaccept *pAcc = (uvaccept *)server->data;
			std::shared_ptr<uvconnect> pConn(new uvconnect);
			uv_tcp_t *pClient = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
			int iErr = uv_tcp_init(pAcc->m_ploop->m_pUVLoop.get(), pClient);
			assert(iErr == 0);
			iErr = uv_accept(server, (uv_stream_t *)pClient);
			if (iErr == 0)
			{
				pConn->RegisterloopAndStream(pAcc->m_ploop, pClient);
				pAcc->m_pConnectcb(pConn);
			}
		}
	});
}

void uvaccept::SetConnectCallback(connectcb cocb)
{
	m_pConnectcb = cocb;
}

