#include "timer.h"
#include <assert.h>

uvtimer::uvtimer()
	: m_ploop(nullptr)
	, m_pTimercb(nullptr)
	, m_pTimer(nullptr)
	, m_delay(0)
	, m_repeat(0)
	, m_bTimerStarted(false)
	, m_bRestartTimer(false)
{
	m_pTimer.reset(new uv_timer_t);
}

uvtimer::~uvtimer()
{

}

unsigned int uvtimer::RegisterTimer(std::shared_ptr<uvloop>ploop)
{
	m_ploop = ploop;
	return 0;
}



void uvtimer::SetTimer(timercb ticb, unsigned long long delay /*= 0*/, unsigned long long repeat /*= 0*/)
{

	m_pTimercb = ticb;
	m_delay = delay;
	m_repeat = repeat;
}

void uvtimer::timerStart()
{
	uv_timer_init(m_ploop->m_pUVLoop.get(), m_pTimer.get());

	m_pTimer->data = this;
	int iErr = uv_timer_start(m_pTimer.get(), [](uv_timer_t *timer)
	{
		uvtimer *pTim = (uvtimer *)timer->data;
		pTim->m_pTimercb();
		//timer stop
		if (pTim->m_repeat == 0)
		{
			pTim->m_bTimerStarted = false;
		}
	}
		, m_delay
		, m_repeat);

	assert(iErr == 0);
	m_bTimerStarted = true;
}


bool uvtimer::startTimer(bool bForceRestart /*= false*/)
{
	if (!m_bTimerStarted)
	{
		unsigned int uiErr = m_ploop->AddReq(E_TIMER_START, std::bind(&uvtimer::timerStart, shared_from_this()));
		return (uiErr == 0);
	}
	else
	{
		if (bForceRestart)
		{
			m_bRestartTimer = true;
			StopTimer();
		}
		return true;
	}
	
}

bool uvtimer::StopTimer()
{
	unsigned int uiErr = m_ploop->AddReq(E_TIMER_STOP, std::bind(&uvtimer::timerStop, shared_from_this()));
	return (uiErr == 0);
}

void uvtimer::timerStop()
{
	int iErr = uv_timer_stop(m_pTimer.get());
	m_bTimerStarted = false;
	if (m_bRestartTimer)
	{
		m_bRestartTimer = false;
		//already stoped, restart
		startTimer();
	}
}

