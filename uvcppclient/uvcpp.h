#ifndef _UV_CPP_
#define _UV_CPP_

#include <iostream>
#include <functional>
#include <mutex>
#include <memory>
#include <thread>
#include <deque>
#include <map>
#include <condition_variable>

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

typedef std::function<void(char *buf, int len)> read_callback;
typedef std::function<void(int status)> write_callback;
typedef std::function<void(int status)> connect_callback;
typedef std::function<void(void)> timer_callback;

namespace uv{

#define  DEFAULT_BUFFER_LEN 1024
#define  BUFFER_INCREASE_FACTOR 2
	typedef enum {
		EV_SEND,
		EV_CLOSE_CONN,
		EV_STOP_LOOP,
		EV_NULL,
	} event_type;

	//ignore data
	typedef enum {
      HIGH = 0,
	  MEDIUM = 1,
	} priority_key;

	typedef struct event_tag {
		event_type ev = EV_NULL;
	} st_event;

	class uvcpp
	{
	public:
		uvcpp();
		~uvcpp();

		bool connect(std::string& host, unsigned short port);
		bool connect();

		// start loop
		void run();
		//stop loop
		void stop();
		// close connection
		void close();

		int send(char *buf, unsigned int len);

		void setAutoReconnect(bool bAuto);
		bool isAutoReconnect();
		bool isLoopStop();
		void setReadCallback(read_callback &rb){ _readcb = rb; };

		void setStatusNormal();
		void setStatusNonNormal();
		bool isStatusNormal();

	private:
		void inner_stop();
		void inner_close();
		void async_init();
		void async_deinit();
		void write();
		void allocSendBuf(unsigned int needSize);

	private:
		std::map<int, st_event> _works;
		std::mutex _workLock;
		std::shared_ptr<uv_async_t> _async;
		std::shared_ptr<uv_loop_t> _loop;
		std::shared_ptr<uv_tcp_t> _tcp;
		std::shared_ptr<uv_connect_t> _conn;
		std::shared_ptr<uv_stream_t> _send;

		uv_buf_t  _uv_buf;
		uv_stream_t *_tcpHandle;

		//external stop loop
		bool _exStopLoop;
		//conn status
		bool _statusNormal;
		// auto reconnect
		bool _autoReconnect;
		// keep addr
		struct sockaddr_in _addr;

		//send buffer
		std::mutex _sendBufLock;
		char * _sendBuf;
		unsigned int _sendLen;
		unsigned int _sendOffset;

		// callbacks
		read_callback _readcb ;
	};

};
#endif