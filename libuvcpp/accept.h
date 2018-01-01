#ifndef _ACCEPT_H_
#define _ACCEPT_H_
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <map>
#include <deque>
#include <memory>
#include "loop.h"
#include "SafeBuffer.h"
#include "connect.h"

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

typedef std::function<void(std::shared_ptr<uvconnect>)> connectcb;
class uvaccept:public std::enable_shared_from_this<uvaccept>
{
public:
	uvaccept();
	~uvaccept();
	void SetIPAndPort(char *phost = "0.0.0.0", unsigned int port = 12396);
	bool RegisterAccept(std::shared_ptr<uvloop> ploop);

	void SetConnectCallback(connectcb cocb);

	bool listen();

	std::shared_ptr<uvloop> m_ploop;

	connectcb m_pConnectcb;

	std::shared_ptr<uv_tcp_t> m_pServer;
private:
	sockaddr_in m_addr;
	void ServerListenStart();
	
};
#endif // !_ACCEPT_H_
