#ifndef _TIMER_H_
#define  _TIMER_H_
#include <thread>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <map>
#include <deque>
#include <memory>
#include "loop.h"

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

typedef std::function<void(void)> timercb;

class uvtimer:public std::enable_shared_from_this<uvtimer>
{
public:
	uvtimer();
	~uvtimer();
	void SetTimer(timercb ticb, unsigned long long delay = 0, unsigned long long repeat = 0);
	unsigned int RegisterTimer(std::shared_ptr<uvloop> ploop);
	bool startTimer(bool bForceRestart = false);
	bool StopTimer();

	bool m_bTimerStarted;
	bool m_bRestartTimer;
	std::shared_ptr<uvloop> m_ploop;
	timercb m_pTimercb;
private:
	std::shared_ptr<uv_timer_t> m_pTimer;
	void timerStart();
	void timerStop();

	unsigned long long m_delay;
	unsigned long long m_repeat;
};

#endif