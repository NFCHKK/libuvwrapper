#include "stdafx.h"
#include "uvcpp.h"

uv::uvcpp::uvcpp() :_readcb([](char *, unsigned int){}),
_autoReconnect(true), _statusNormal(false), _exStopLoop(false), _tcpHandle(nullptr)
{
	_sendBuf = (char *)malloc(DEFAULT_BUFFER_LEN);
	_sendLen = DEFAULT_BUFFER_LEN;
	_sendOffset = 0;

	_loop.reset((uv_loop_t*)malloc(sizeof(uv_loop_t)));

	uv_loop_init(_loop.get());
	_async.reset((uv_async_t*)malloc(sizeof(uv_async_t)));
	
	_tcp.reset((uv_tcp_t*)malloc(sizeof(uv_tcp_t)));
	_conn.reset((uv_connect_t*)malloc(sizeof(uv_connect_t)));
	_send.reset((uv_stream_t*)malloc(sizeof(uv_stream_t)));

}
bool uv::uvcpp::connect(std::string& host, unsigned short port)
{
	uv_ip4_addr(host.c_str(), port, &_addr);
	return connect();
}

bool uv::uvcpp::connect()
{
	int err = uv_tcp_init(_loop.get(), _tcp.get());

	_conn.get()->data = this;
 
	err = uv_tcp_connect(_conn.get(), _tcp.get(), (const sockaddr *)&_addr,
		// connect
		[](uv_connect_t* req, int status){
		uvcpp *puv = (uvcpp *)req->data;
		puv->_tcpHandle = req->handle;
		if (!puv->_tcpHandle || status != 0)
		{
			// close ,maybe cause reconnect
			puv->inner_close();
			return;
		}
		puv->setStatusNormal();
		unsigned int uiError = uv_read_start(puv->_tcpHandle,
			// alloc 
			[](uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf){
			static char slab[65536];
			buf->base = slab;
			buf->len = sizeof(slab);
		    },
			// read
			[](uv_stream_t* tcp, ssize_t nread, const uv_buf_t* buf){
			uvcpp *puv = (uvcpp *)tcp->data;
			if (nread < 0)
			{
				uv_read_stop(puv->_tcpHandle);
				//close, maybe cause reconnect
				puv->inner_close();
				return;
			}
			if (nread > 0){
				puv->_readcb(buf->base, nread);
			}
		});
	});
	return true;
}

void uv::uvcpp::setStatusNormal()
{
	_statusNormal = true;
}

void uv::uvcpp::setStatusNonNormal()
{
	_statusNormal = false;
}

void uv::uvcpp::setAutoReconnect(bool bAuto)
{
	_autoReconnect = bAuto;
}

void uv::uvcpp::run()
{
	async_init();
	std::thread tRun([this](void *pVoid)
	{
		uv_run(_loop.get(), UV_RUN_DEFAULT);
		int err = uv_loop_close(_loop.get());
		return true;
	}, this);
	tRun.detach();
}

void uv::uvcpp::stop()
{
	{
		std::unique_lock<std::mutex> lock(_workLock);
		st_event st;
		st.ev = EV_STOP_LOOP;
		_works.insert(std::make_pair(HIGH, st));
	}
	uv_async_send(_async.get());
}

void uv::uvcpp::close()
{
	{
		std::unique_lock<std::mutex> lock(_workLock);
		st_event st;
		st.ev = EV_CLOSE_CONN;
		_works.insert(std::make_pair(MEDIUM, st));
	}
	uv_async_send(_async.get());
}

void uv::uvcpp::async_init()
{
	_async.reset((uv_async_t *)malloc(sizeof(uv_async_t)));
	_async->data = this;

	// async call back
	int err = uv_async_init(_loop.get(), _async.get(), 
		[](uv_async_t *handle){
		uvcpp *puv = (uvcpp *)handle->data;
		std::map<int, st_event> works;
		{
			std::unique_lock<std::mutex> lck(puv->_workLock);
			works.swap(puv->_works);
		}
		for (auto it = works.begin(); it != works.end(); it ++)
		{
			switch (it->second.ev)
			{
			case EV_SEND:
				puv->write();
				break;
			case EV_CLOSE_CONN:
				puv->inner_close();
				// no need to deal other event
				return;
			case EV_STOP_LOOP:
				// no need to deal other event
				puv->inner_stop();
				return;
			}
		}
			
	});
}

void uv::uvcpp::write()
{
	std::unique_lock<std::mutex> lock(_sendBufLock);
	if (_sendOffset == 0)
	{
		// already send all data
		return;
	}
	_uv_buf = uv_buf_init(_sendBuf, _sendOffset);
	int n = uv_try_write(_tcpHandle, &_uv_buf, 1);
	if (n > 0)
	{
		memmove(_sendBuf, _sendBuf + n, _sendOffset - n);	
		_sendOffset -= n;
	}
	// FIX ME : need to deal with error
}

int uv::uvcpp::send(char *buf, unsigned int len)
{
	{
		std::unique_lock<std::mutex> lock(_sendBufLock);
		if (len + _sendOffset > _sendLen)
		{
			allocSendBuf(_sendLen - len - _sendOffset);
		}

		memcpy(_sendBuf + _sendOffset, buf, len);
		_sendOffset += len;
	}

	{
	    std::unique_lock<std::mutex> lock(_workLock);
		st_event st;
		st.ev = EV_SEND;
		_works.insert(std::make_pair(_works.size(), st));
    }
	uv_async_send(_async.get());
	return len;
}

void uv::uvcpp::allocSendBuf(unsigned int needSize)
{
	unsigned int newSize = (_sendOffset + needSize) * BUFFER_INCREASE_FACTOR;
	if (!realloc(_sendBuf, newSize))
	{
		char *tmp = (char *)malloc(newSize);
		memcpy(tmp, _sendBuf, _sendOffset);
		free(_sendBuf);
		_sendBuf = tmp;
	}
	_sendLen = newSize;
}

bool uv::uvcpp::isAutoReconnect()
{
	return _autoReconnect;
}

bool uv::uvcpp::isLoopStop()
{
	return _exStopLoop;
}

bool uv::uvcpp::isStatusNormal()
{
	return _statusNormal;
}

void uv::uvcpp::async_deinit()
{
	if (!_async.get())
	{
		return;
	}
	_async.get()->data = this;
	uv_close((uv_handle_t *)_async.get(), [](uv_handle_t *handle){((uvcpp*)(handle->data))->_async.reset();});
}

uv::uvcpp::~uvcpp()
{

}

void uv::uvcpp::inner_stop()
{
	// close tcp connection
	inner_close();

	// deinit aysnc handle
	async_deinit();

	uv_stop(_loop.get());
}

void uv::uvcpp::inner_close()
{
	if (!isStatusNormal())
	{
		//already close handle
		return;
	}
	setStatusNonNormal();
	_tcpHandle->data = this;
	uv_close((uv_handle_t*)_tcpHandle,
		[](uv_handle_t* handle){
		uvcpp *puv = (uvcpp *)handle->data;

		if ((uv_handle_t *)puv->_tcpHandle == handle)
		{
			puv->_tcpHandle = nullptr;
			// re-connect unless user stop loop
			if (puv->isAutoReconnect() && !puv->isLoopStop())
			{
				puv->connect();
			}
			return;
		}
	});
}


