#ifndef _CONNECT_H_
#define _CONNECT_H_
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <map>
#include <deque>
#include <memory>
#include "loop.h"
#include "SafeBuffer.h"

#include "../include/uv.h"
#ifdef WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#ifdef _DEBUG
#pragma comment(lib, "libuv.lib")
#else
#pragma comment(lib, "libuv.lib")
#endif

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Userenv.lib")
#else
#define _stdcall 
#endif

typedef struct tagSendst
{
	uv_write_t tWrite;
	safebuffer tsafebuf;
	uv_buf_t tbuf;
}stSend;

typedef enum
{
	EV_CONNECT,
	EV_READ,
	EV_WRITE,
	EV_CLOSE,
}evType;

class  uvaccept;

typedef std::function<void(evType, char *, ssize_t nread)>  eventcb;
typedef std::function<void(ssize_t nread, char *buf)> readcb;
typedef std::function<void(int status)> writecb;
typedef std::function<void(int status)> conncb;

class uvconnect:public std::enable_shared_from_this<uvconnect>
{
public:
	uvconnect();
	~uvconnect();
	void SetIPAndAddr(const char *phost, unsigned int port);
	bool RegisterConnect(std::shared_ptr<uvloop> ploop);
	

	bool ConnectToServer();

	bool CloseConnection();

	void SetEventHandler(eventcb evcb);

	bool StartReadData();
	bool StopReadData();
	bool wirteData(char *pbuf, unsigned long len);

	bool IsConnected();
	bool Reconnnect();

	bool RegisterloopAndStream(std::shared_ptr<uvloop> ploop, uv_tcp_t*  pStream);
	
	bool m_bConnectSuccess;

	bool m_bReconect;

	eventcb m_EvnentHandler;

	std::shared_ptr<uvloop> m_ploop;

	std::mutex m_writelock;
	std::map<uv_write_t *, std::shared_ptr<stSend>> m_mapSend;
	std::deque<std::shared_ptr<stSend>> m_dSend;

private:

	void ReadStart();
	void ReadStop();

	void tcpWrite(std::shared_ptr<stSend> pNewSend);

	void Connect();

	void CloseConnect();


	

	std::shared_ptr<sockaddr_in> m_pAddr;
	std::shared_ptr<uv_connect_t> m_pConn;
	std::shared_ptr<uv_tcp_t> m_ptcp;

	std::shared_ptr<uv_stream_t> m_pStream;

};
#endif