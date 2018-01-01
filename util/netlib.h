#ifndef _TBP_CLASS_H_
#define _TBP_CLASS_H_

#include <iostream>
#include <functional>
#include <mutex>
#include <memory>
#include <thread>
#include <deque>
#include <map>
#include <condition_variable>
#include "SafeBuffer.h"

#include "../include/uv.h"
#ifdef WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#ifdef _DEBUG
#pragma comment(lib, "libuvd.lib")
#else
#pragma comment(lib, "libuv.lib")
#endif

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")
#else
#define _stdcall 
#endif

//用于外部发送数据的handle数量
#define  ASYNC_HANDLE_NUM     5
typedef struct tagSendst
{
	uv_write_t pWrite;
	uv_buf_t uv_buf;
	safebuffer buffer;
} Sendst;

typedef struct tagAsyncSendst
{
	uv_async_t pAsync;
	safebuffer buffer;
} AsyncSendst;

typedef std::function<void(char *buf, int len)> readcb;
typedef std::function<void(int status)> writecb;
typedef std::function<void(int status)> connetcb;
typedef std::function<void(void)> timercb;

typedef enum enlinkType
{
	LINK_TMP,
	LINK_TBP,
} linkType;

template<linkType T>
class netlibuv :public std::enable_shared_from_this<netlibuv<T>>
{
public:
	netlibuv();
	netlibuv(const char *phost, unsigned int port);
	virtual ~netlibuv();
	void net_SetIpAndPort(const char *phost, unsigned int port);
	bool net_Connect();
	bool net_closeConnection();
	bool net_reConnect();
	bool net_Send(char *pBuffer, unsigned int len);
	bool net_AsyncSend(char *pBuffer, unsigned int len);

	void net_StartTimer(unsigned int time, unsigned int repeat, timercb timerCallback);
	void net_stopTimer();

	void net_StartTcpTimer(unsigned int timer, unsigned int repeat, timercb tcptimerCallback);
	void net_stopTcpTimer();
	void net_Run();
	void net_Stop();
	static netlibuv<T>* net_GetInstance();

	static unsigned int _stdcall threadRun(void *pVoid);

	uv_loop_t *net_GetLoop();

	static inline void net_SetReadCallBack(readcb pRead){m_pReadCallback = pRead;};
	static inline void net_SetWriteCallBack(writecb pWrite){m_pWriteCallback = pWrite;};
	static inline void net_SetConnectCallBack(connetcb pConn){m_pConnectCallBack = pConn;};
	
private:
	static void connect_cb(uv_connect_t* req, int status);
	static void write_cb(uv_write_t* req, int status);
	static void read_cb(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf);
	static void time_cb(uv_timer_t* timer);
	static void tcptimer_cb(uv_timer_t* timer);


	std::mutex m_lockWriteBuffer;
	std::deque<std::shared_ptr<Sendst>> m_dSend;
	std::map<uv_write_t*, std::shared_ptr<Sendst>> m_mapSend;

	std::mutex m_lockAsyncWriteBuffer;
	std::deque<std::shared_ptr<AsyncSendst>> m_dAsyncSend;
	std::map<uv_async_t *, std::shared_ptr<AsyncSendst>> m_mapAsyncSend;

	static void close_cb(uv_handle_t* handle);
	static void shutdown_cb(uv_shutdown_t* req, int status);

	static inline void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
	{
		static char slab[65536];
		buf->base = slab;
		buf->len = sizeof(slab);
	};

	static void async_cb(uv_async_t *handle);

private:
	static readcb m_pReadCallback;
	static writecb m_pWriteCallback;
	static connetcb m_pConnectCallBack;
	static timercb m_pTimerCallback;
	static timercb m_pTcpTimerCallback;

	static uv_stream_t *m_ptcpHandle;
	static netlibuv<T>* m_pNetlibInst;
	static bool m_btcpClosed;


	uv_loop_t* m_ploop;
	uv_tcp_t* m_psocktcp;
	uv_connect_t m_connect;
	struct sockaddr_in m_adder;
	
	unsigned int m_uiError;
	uv_timer_t m_timer;
	uv_timer_t m_tcpReconnectTimer;

	static uv_write_t* m_puvWright;
	uv_shutdown_t* m_pshutdown;
};

template<linkType T>
void netlibuv<T>::async_cb(uv_async_t *handle)
{
	{
		std::unique_lock<std::mutex> lck(m_pNetlibInst->m_lockAsyncWriteBuffer);
		std::map<uv_async_t *, std::shared_ptr<AsyncSendst>>::iterator it = m_pNetlibInst->m_mapAsyncSend.find(handle);
		if (it == m_pNetlibInst->m_mapAsyncSend.end())
		{
			//error
		}
		else
		{
			m_pNetlibInst->net_Send(it->second->buffer.GetMemory(), it->second->buffer.getDataLength());
		}
	}
}

template<linkType T>
bool netlibuv<T>::net_AsyncSend(char *pBuffer, unsigned int len)
{
	
	{
		std::unique_lock<std::mutex> lck(m_lockAsyncWriteBuffer);
		if (m_dAsyncSend.empty())
		{
			return false;
		}
		else
		{
			std::shared_ptr<AsyncSendst> pNew = m_dAsyncSend.front();
			//deque 中取出 handle
			m_dAsyncSend.pop_front();
			// 插入到map中
			m_mapAsyncSend.insert(make_pair(&pNew->pAsync, pNew));
			//清理，并拷贝数据
			pNew->buffer.ClearMem();
			pNew->buffer.readmem(pBuffer, len);
			//出发回调，使用loop 线程发送数据
			uv_async_send(&pNew->pAsync);
			return true;
		}

	}

}

template<linkType T>
uv_stream_t* netlibuv<T>::m_ptcpHandle = NULL;

template<linkType T>
netlibuv<T>* netlibuv<T>::m_pNetlibInst = NULL;

template<linkType T>
bool netlibuv<T>::m_btcpClosed = false;

template<linkType T>
readcb netlibuv<T>::m_pReadCallback = [](char*, unsigned int){return; };
template<linkType T>
writecb netlibuv<T>::m_pWriteCallback = [](int){return; };
template<linkType T>
connetcb netlibuv<T>::m_pConnectCallBack = [](int){return; };

template<linkType T>
timercb netlibuv<T>::m_pTimerCallback = [](){return; };

template<linkType T>
timercb netlibuv<T>::m_pTcpTimerCallback = [](){return; };

template<linkType T>
uv_write_t * netlibuv<T>::m_puvWright = new uv_write_t;

template<linkType T>
netlibuv<T>::netlibuv()
	:m_pshutdown(new uv_shutdown_t)
{
	m_ploop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
	assert(0 == uv_loop_init(m_ploop));
	memset(m_pshutdown, 0x00, sizeof(uv_shutdown_t));

	m_psocktcp = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	memset(m_psocktcp, 0x00, sizeof(uv_tcp_t));

	unsigned int uir = uv_tcp_init(m_ploop, m_psocktcp);
	assert(0 == uir);
	uir = uv_timer_init(m_ploop, &m_timer);
	assert(0 == uir);
	uir = uv_timer_init(m_ploop, &m_tcpReconnectTimer);

	for (int i = 0; i < ASYNC_HANDLE_NUM; i++)
	{
		std::shared_ptr<AsyncSendst> pNewAsyncSendst(new AsyncSendst);
		uir = uv_async_init(m_ploop, &pNewAsyncSendst->pAsync, &netlibuv<T>::async_cb);
		assert(0 == uir);
		m_dAsyncSend.push_back(pNewAsyncSendst);
	}
}
template<linkType T>
netlibuv<T>::netlibuv(const char *phost, unsigned int port)
{
	unsigned int uir = uv_ip4_addr(phost, port, &m_adder);
	assert(0 == uir);
}

template<linkType T>
netlibuv<T>::~netlibuv()
{
	net_Stop();

	//uv_close((uv_handle_t*)(netlibuv::net_GetInstance()->net_GetLoop()), &netlibuv::close_cb);
	if (m_pNetlibInst)
	{
		delete m_pNetlibInst;
		m_pNetlibInst = NULL;
	}

	if (m_pshutdown)
	{
		delete m_pshutdown;
		m_pshutdown = NULL;
	}

	if (m_puvWright)
	{
		delete m_puvWright;
		m_puvWright = NULL;
	}
}

template<linkType T>
void netlibuv<T>::net_SetIpAndPort(const char *phost, unsigned int port)
{
	unsigned int uir = uv_ip4_addr(phost, port, &m_adder);
	assert(0 == uir);
}

template<linkType T>
bool netlibuv<T>::net_Connect()
{
	m_uiError = uv_tcp_connect(&m_connect, m_psocktcp, (const sockaddr *)&m_adder, &netlibuv<T>::connect_cb);
	assert(m_uiError == 0);

	return true;
}

template<linkType T>
void netlibuv<T>::connect_cb(uv_connect_t* req, int status)
{

	m_ptcpHandle = req->handle;
	if (!m_ptcpHandle)
	{
		m_pConnectCallBack(-1);
	}

	if (status != 0)
	{
		m_pConnectCallBack(status);
		return;
	}

	m_btcpClosed = false;
	unsigned int uiError = uv_read_start(m_ptcpHandle, &netlibuv<T>::alloc_cb, &netlibuv<T>::read_cb);
	assert(uiError == 0);

	m_pConnectCallBack(status || uiError);
}

template<linkType T>
void netlibuv<T>::write_cb(uv_write_t* req, int status)
{
	//将写的结果传出去，由外部决定如何处理
	{
		std::unique_lock<std::mutex> lck(m_pNetlibInst->m_lockWriteBuffer);
		std::map<uv_write_t *, std::shared_ptr<Sendst>>::iterator it = m_pNetlibInst->m_mapSend.find(req);
		if (it == m_pNetlibInst->m_mapSend.end())
		{
			//ERROR, 丢弃
			//std::cout << "Can not find right stream" << std::endl;
			
		}
		else
		{
			m_pNetlibInst->m_dSend.push_back(it->second);
			m_pNetlibInst->m_mapSend.erase(it);
		}
	}
}

template<linkType T>
void netlibuv<T>::read_cb(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf)
{
	if (nread < 0)
	{
		//std::cout << "nread: " << nread << std::endl;
		//数据读取错误，将nread 传出去，由外部考虑如何处理
		// 如果网络断开，心跳包会发现错误
		//std::cout << "read failed!" << std::endl;
		uv_read_stop(m_ptcpHandle);
		m_pReadCallback(buf->base, -1);
		return;
	}
	// neread == 0, 并非错误，忽略消息
	//追加消息
	m_pReadCallback(buf->base, nread);
}

template<linkType T>
bool netlibuv<T>::net_Send(char *pBuffer, unsigned int len)
{
	std::shared_ptr<Sendst> pNewSend;
	{
		std::unique_lock<std::mutex> lck(m_lockWriteBuffer);
		if (m_dSend.empty())
		{
			pNewSend.reset(new Sendst);
			m_mapSend.insert(make_pair(&(pNewSend->pWrite), pNewSend));
		}
		else
		{
			pNewSend = m_dSend.front();
			m_dSend.pop_front();
			pNewSend->buffer.ClearMem();
			m_mapSend.insert(make_pair(&(pNewSend->pWrite), pNewSend));
		}
	}

	pNewSend->buffer.readmem(pBuffer, len);
	pNewSend->uv_buf = uv_buf_init(pNewSend->buffer.GetMemory(), pNewSend->buffer.getDataLength());
	unsigned int uiError = uv_write(&(pNewSend->pWrite), netlibuv<T>::m_ptcpHandle, &pNewSend->uv_buf, 1, &netlibuv<T>::write_cb);
	assert(uiError == 0);
	return true;

}

template<linkType T>
netlibuv<T>* netlibuv<T>::net_GetInstance()
{
	if (m_pNetlibInst == NULL)
	{
		m_pNetlibInst = new netlibuv < T > ;
	}

	return m_pNetlibInst;
}

template<linkType T>
unsigned int netlibuv<T>::threadRun(void *pVoid)
{
	uv_run(m_pNetlibInst->net_GetLoop(), UV_RUN_DEFAULT);
	return 0;
}

template<linkType T>
void netlibuv<T>::net_Run()
{
	std::thread tRun(&netlibuv<T>::threadRun, m_pNetlibInst);
	//允许线程独立运行
	tRun.detach();
	//_beginthreadex(NULL, 0, &netlibuv<T>::threadRun, NULL, 0, 0);
}

template<linkType T>
void netlibuv<T>::net_Stop()
{
	net_stopTimer();
}

template<linkType T>
uv_loop_t * netlibuv<T>::net_GetLoop()
{
	return m_ploop;
}

template<linkType T>
void netlibuv<T>::net_StartTimer(unsigned int time, unsigned int repeat, timercb timerCallback)
{
	//LOG_TRACE("start timer!");
	m_pTimerCallback = timerCallback;
	m_uiError = uv_timer_start(&m_timer, time_cb, time, repeat);
	assert(m_uiError == 0);
}

template<linkType T>
void netlibuv<T>::time_cb(uv_timer_t* timer)
{
	m_pTimerCallback();
}

template<linkType T>
void netlibuv<T>::net_stopTimer()
{
	m_uiError = uv_timer_stop(&m_timer);
	assert(m_uiError == 0);
}

template<linkType T>
void netlibuv<T>::net_stopTcpTimer()
{
	m_uiError = uv_timer_stop(&m_tcpReconnectTimer);
	assert(m_uiError == 0);
}

template<linkType T>
void netlibuv<T>::net_StartTcpTimer(unsigned int time, unsigned int repeat, timercb tcptimerCallback)
{
	m_pTcpTimerCallback = tcptimerCallback;
	m_uiError = uv_timer_start(&m_tcpReconnectTimer, tcptimer_cb, time, repeat);
	assert(m_uiError == 0);
}

template<linkType T>
void netlibuv<T>::tcptimer_cb(uv_timer_t* timer)
{
	m_pTcpTimerCallback();
}

template<linkType T>
bool netlibuv<T>::net_reConnect()
{
	if (!m_btcpClosed)
	{
		net_closeConnection();
		return false;
	}

	//std::cout << "reconnect  server!" << std::endl;

	uv_tcp_init(m_ploop, m_psocktcp);
	m_uiError = uv_tcp_connect(&m_connect, m_psocktcp, (const sockaddr *)&m_adder, &netlibuv<T>::connect_cb);
	if (m_uiError != 0)
	{
		return false;
	}
	return true;
}

template<linkType T>
bool netlibuv<T>::net_closeConnection()
{
	uv_close((uv_handle_t*)m_ptcpHandle, &netlibuv<T>::close_cb);
	//std::cout << "call uv_close: " << __FILE__ << "  " << __LINE__ << std::endl;

	return true;
}

template<linkType T>
void netlibuv<T>::shutdown_cb(uv_shutdown_t* req, int status)
{

};

template<linkType T>
void netlibuv<T>::close_cb(uv_handle_t* handle)
{
	assert(1 == uv_is_closing((uv_handle_t *)m_ptcpHandle));
	//std::cout << "Close tcp network" << __FILE__ << "  " << __LINE__ << std::endl;
	{
		m_btcpClosed = true;
	}
};
#endif