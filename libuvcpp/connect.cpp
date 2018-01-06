#include "connect.h"
#include <assert.h>

uvconnect::uvconnect()
	: m_bConnectSuccess(false)
	, m_pStream(nullptr)
	, m_ploop(nullptr)
	, m_bReconect(false)
	, m_EvnentHandler(nullptr)
{
	m_pAddr.reset((sockaddr_in *)malloc(sizeof(sockaddr_in)));
}

uvconnect::~uvconnect()
{

}

void uvconnect::SetIPAndAddr(const char *phost, unsigned int port)
{
	unsigned int uir = uv_ip4_addr(phost, port, m_pAddr.get());
	assert(0 == uir);
}

void uvconnect::Connect()
{
	m_pConn.reset((uv_connect_t *)malloc(sizeof(uv_connect_t)));
	m_ptcp.reset((uv_tcp_t *)malloc(sizeof(uv_tcp_t)));
	m_pConn->data = this;

	uv_tcp_init(m_ploop->m_pUVLoop.get(), m_ptcp.get());
	uv_tcp_connect(m_pConn.get(), m_ptcp.get(), (sockaddr *)m_pAddr.get(), [](uv_connect_t *req, int status)
	{
		
		uvconnect *pConn = (uvconnect *)req->data;
		pConn->m_pStream.reset(req->handle);

		if (status == 0)
		{
			pConn->m_bConnectSuccess = true;
		}
		else
		{
			pConn->m_bConnectSuccess = false;
		}
		if (pConn->m_EvnentHandler)
		{
			pConn->m_EvnentHandler(EV_CONNECT, 0, status);
		}
	});
}

bool uvconnect::RegisterConnect(std::shared_ptr<uvloop> ploop)
{
	m_ploop = ploop;
	return true;
}


bool uvconnect::wirteData(char *pbuf, unsigned long len)
{
	if (!m_bConnectSuccess)
	{
		return false;
	}

	if (m_ploop == nullptr)
	{
		return false;
	}
	
	std::shared_ptr<stSend> pNewSendst;
	{
		std::unique_lock<std::mutex> lck(m_writelock);
		if (m_dSend.empty())
		{
			pNewSendst.reset(new stSend);
		}
		else
		{
			pNewSendst = m_dSend.front();
			m_dSend.pop_front();
			pNewSendst->tsafebuf.ClearMem();
		}
		m_mapSend.insert(make_pair(&pNewSendst->tWrite, pNewSendst));
	}
	pNewSendst->tsafebuf.readmem(pbuf, len);

 	unsigned int uiErr = m_ploop->AddReq(E_TCP_WRITE, std::bind(&uvconnect::tcpWrite, shared_from_this(), pNewSendst));
 	return (uiErr == 0);
}

void uvconnect::CloseConnect()
{
	m_pStream->data = this;
	//will close connect, stop writing or reading
	m_bConnectSuccess = false;
	uv_close((uv_handle_t *)m_pStream.get(), [](uv_handle_t* handle)
	{
		uvconnect *pConn = (uvconnect *)handle->data;
		if (pConn->m_bReconect)
		{
			pConn->m_bReconect = false;
			pConn->ConnectToServer();
		}
	});
}

bool uvconnect::Reconnnect()
{
	m_bReconect = true;
	int uiErr = m_ploop->AddReq(E_TCP_CLOSE, std::bind(&uvconnect::CloseConnect, shared_from_this()));
	return (uiErr == 0);
}

bool uvconnect::ConnectToServer()
{
	unsigned int uiErr = m_ploop->AddReq(E_TCP_CONNECT, std::bind(&uvconnect::Connect, shared_from_this()));
	return (uiErr == 0);
}

bool uvconnect::CloseConnection()
{
	int uiErr = m_ploop->AddReq(E_TCP_CLOSE, std::bind(&uvconnect::CloseConnect, shared_from_this()));
	return (uiErr == 0);
}



bool uvconnect::StartReadData()
{
	if (m_bConnectSuccess)
	{
		int iErr = m_ploop->AddReq(E_TCP_READ_START, std::bind(&uvconnect::ReadStart, shared_from_this()));
		return (iErr == 0);
	}
	else
	{
		return false;
	}
}

bool uvconnect::StopReadData()
{
	int iErr = m_ploop->AddReq(E_TCP_READ_STOP, std::bind(&uvconnect::ReadStop, shared_from_this()));
	return (iErr == 0);
}

void uvconnect::ReadStart()
{
	if (!m_bConnectSuccess)
	{
		return;
	}

	m_pStream->data = this;
	uv_read_start(m_pStream.get(),
		//alloc callback
		[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
	{
		static char slab[65536];
		buf->base = slab;
		buf->len = sizeof(slab);
	},
		//read callback
		[](uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
	{
		uvconnect *pConn = (uvconnect *)tcp->data;
		if (nread >= 0)
		{
			if (pConn->m_EvnentHandler)
			{
				pConn->m_EvnentHandler(EV_READ, buf->base, nread);
			}
		}
		else
		{
			//stop read
			uv_read_stop(pConn->m_pStream.get());
			pConn->m_bConnectSuccess = false;
			//change status ready to reconnect
			if (pConn->m_EvnentHandler)
			{
				pConn->m_EvnentHandler(EV_CLOSE, 0, 0);
			}
		}
	});
}

void uvconnect::ReadStop()
{
	int iErr = uv_read_stop(m_pStream.get());
	assert(iErr == 0);
}


void uvconnect::tcpWrite(std::shared_ptr<stSend> pNewSend)
{
	if (!m_bConnectSuccess)
	{
		return;
	}
	pNewSend->tWrite.data = this;
	pNewSend->tbuf = uv_buf_init(pNewSend->tsafebuf.GetMemory(), pNewSend->tsafebuf.getDataLength());
	//
	int uiErr = uv_write(&(pNewSend->tWrite), m_pStream.get(), &pNewSend->tbuf, 1, [](uv_write_t *req, int status)
	{
		uvconnect *pConn = (uvconnect *)req->data;
		{
			std::unique_lock<std::mutex> lck(pConn->m_writelock);
			std::map<uv_write_t *, std::shared_ptr<stSend>>::iterator it = pConn->m_mapSend.find(req);
			if (it == pConn->m_mapSend.end())
			{
				//error, anyway put handle into deque to reuse
				pConn->m_dSend.push_back(it->second);
			}
			else
			{
				pConn->m_dSend.push_back(it->second);
				pConn->m_mapSend.erase(it);
			}
		}
		if (pConn->m_EvnentHandler)
		{
			pConn->m_EvnentHandler(EV_WRITE, 0, status);
		}
	});

	assert(uiErr == 0);
}

bool uvconnect::RegisterloopAndStream(std::shared_ptr<uvloop> ploop, uv_tcp_t* pStream)
{
	m_bConnectSuccess = true;
	m_ploop = ploop;
	m_pStream.reset((uv_stream_t *)pStream);
	return true;
}

void uvconnect::SetEventHandler(eventcb evcb)
{
	m_EvnentHandler = evcb;
}

bool uvconnect::IsConnected()
{
	return m_bConnectSuccess;
}
