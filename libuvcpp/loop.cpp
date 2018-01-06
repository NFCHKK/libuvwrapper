#include "loop.h"
#include "connect.h"

#define  UV_ASYNC_HANDE 5

uvloop::uvloop()
	:m_bRunning(false)
{
	m_pUVLoop.reset(uv_loop_new());
	for (int i = 0; i < UV_ASYNC_HANDE; i ++)
	{
		std::shared_ptr<stAsync> pNewAsync(new stAsync);
		// register a callback lambda for every async_send handle
		uv_async_init(m_pUVLoop.get(), &pNewAsync->tAasync, [](uv_async_t *pAsync)
		{
			stAsync * pSt((stAsync *)pAsync);
			switch (pSt->eEvent)
			{
			case E_TCP_WRITE:
			case E_TCP_CONNECT:
			case E_TCP_CLOSE:
			case E_TIMER_START:
			case E_TIMER_STOP:
			case E_TCP_LISTEN:
			case E_TCP_READ_STOP:
			case E_TCP_READ_START:
				// callback processing
				pSt->tcb();
				pSt->tcb = nullptr;
				break;
			case E_LOOP_STOP:
				//stop loop
				uv_stop(pSt->ploop->m_pUVLoop.get());
				pSt->tcb = nullptr;
				break;
			default:
				pSt->tcb = nullptr;
				assert(0);
				break;
			}

			{
				std::unique_lock<std::mutex> lck(pSt->ploop->m_lockAsync);
				std::map<uv_async_t *, std::shared_ptr<stAsync>>::iterator it = pSt->ploop->m_mapAsync.find(pAsync);
				if (it == pSt->ploop->m_mapAsync.end())
				{
					//error may be lost, anyway, put it into deque to reuse
					pSt->ploop->m_dAsync.push_back(it->second);
				}
				else
				{
					pSt->ploop->m_dAsync.push_back(it->second);
					pSt->ploop->m_mapAsync.erase(it);
				}
			}
		});
		m_dAsync.push_back(pNewAsync);
	}
}

uvloop::~uvloop()
{
	stop();
	{
		//waiting for uv_loop stop
		std::unique_lock<std::mutex> lck(m_RunningLock);
		m_StopRunNotify.wait(lck, [&](){return !this->m_bRunning; });
	}
}

void theadRun(uvloop* para)
{
	uv_run(para->m_pUVLoop.get(), UV_RUN_DEFAULT);
	uv_loop_close(para->m_pUVLoop.get());
	para->m_bRunning = false;
	para->m_StopRunNotify.notify_all();

}

void uvloop::run()
{
	if (!m_bRunning)
	{
		std::thread t1(theadRun, this);
		t1.detach();
		m_bRunning = true;
	}
	
}

void uvloop::stop()
{
	if (!m_bRunning)
	{
		return;
	}
	{
		std::unique_lock<std::mutex> lck(m_lockAsync);
		if (m_dAsync.empty())
		{
			// loop busy, wait later, need to define error code
			return ;
		}
		else
		{
			std::shared_ptr<stAsync> pAsync = m_dAsync.front();
			m_dAsync.pop_front();
			pAsync->eEvent = E_LOOP_STOP;
			pAsync->ploop = this;
			m_mapAsync.insert(make_pair(&pAsync->tAasync, pAsync));
			//
			uv_async_send(&pAsync->tAasync);
		}
	}
}

unsigned int uvloop::AddReq(eType eEeventType, reqcb pCallback)
{
	if (!m_bRunning)
	{
		// loop stop
		return ERR_LOOP_STOP;
	}
	{
		std::unique_lock<std::mutex> lck(m_lockAsync);
		if (m_dAsync.empty())
		{
			// loop busy, wait later, need to define error code
			return ERR_LOOP_BUSY;
		}
		else
		{
			std::shared_ptr<stAsync> pAsync = m_dAsync.front();
			m_dAsync.pop_front();
			pAsync->eEvent = eEeventType;
			pAsync->ploop = this;
			pAsync->tcb = pCallback;
			m_mapAsync.insert(make_pair(&pAsync->tAasync, pAsync));
			//
			uv_async_send(&pAsync->tAasync);
			return ERR_LOOP_NO_ERR;
		}
	}
}

bool uvloop::IsRunning()
{
	return m_bRunning;
}

