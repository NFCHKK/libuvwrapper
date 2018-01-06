#ifndef _LOOP_H_
#define _LOOP_H_
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <map>
#include <deque>
#include <memory>

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

typedef enum 
{
	E_LOOP_STOP,
	E_TCP_CONNECT,
	E_TCP_CLOSE,
	E_TCP_WRITE,
	E_TCP_LISTEN, 
	E_TIMER_START,
	E_TIMER_STOP,
	E_TCP_READ_START,
	E_TCP_READ_STOP,
} eType;

typedef enum 
{
	ERR_LOOP_NO_ERR,
	ERR_LOOP_BUSY,
	ERR_LOOP_STOP,

}elooperr;

class  uvloop;

typedef std::function<void()> reqcb;

typedef struct tagAsync
{
	uv_async_t tAasync;
	eType eEvent;
	uvloop *ploop;
	reqcb tcb;
}stAsync;



class uvloop:public std::enable_shared_from_this<uvloop>
{
public:
	uvloop();
	~uvloop();
	void run();
	void stop();
	bool IsRunning();

	unsigned int AddReq(eType eEeventType, reqcb pConn);
	
	bool m_bRunning;

	std::shared_ptr<uv_loop_t> m_pUVLoop;
	std::mutex m_lockAsync;
	std::map<uv_async_t *, std::shared_ptr<stAsync>> m_mapAsync;
	std::deque<std::shared_ptr<stAsync>> m_dAsync;

	std::mutex m_RunningLock;
	std::condition_variable m_StopRunNotify;

private:
};

#endif