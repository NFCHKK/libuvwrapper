#include "SafeBuffer.h"
#include <exception>
#include <iostream>
safebuffer::safebuffer(unsigned long bufflen /*= DEFAULT_BUFFER_LEN*/)
	: m_pbuffer(nullptr)
	, m_bufferlen(bufflen)
	, m_datalen(0)
	, m_offset(0)
{
	m_pbuffer = (char *)malloc(bufflen);
	assert(m_pbuffer != nullptr);
	memset(m_pbuffer, 0x00, bufflen);
}

safebuffer::safebuffer(safebuffer &other)
{
	m_pbuffer = nullptr;

	m_pbuffer = (char *)malloc(other.m_bufferlen);
	assert(m_pbuffer != nullptr);
	memset(m_pbuffer, 0x00, other.m_bufferlen);

	memcpy(m_pbuffer, other.m_pbuffer, other.m_datalen);
	m_bufferlen = other.m_bufferlen;
	m_datalen = other.m_datalen;

}

safebuffer::safebuffer(char* pMem, unsigned long len)
	: m_pbuffer(nullptr)
{
	m_bufferlen = (len / DEFAULT_BUFFER_LEN + 1) * DEFAULT_BUFFER_LEN;

	m_pbuffer = (char *)malloc(m_bufferlen);
	assert(m_pbuffer != nullptr);
	memset(m_pbuffer, 0x00, m_bufferlen);

	memcpy(m_pbuffer, pMem, len);
	m_datalen = len;
}

safebuffer::~safebuffer()
{
	if (m_pbuffer != nullptr)
	{
		free(m_pbuffer);
		m_pbuffer = nullptr;
	}
}


safebuffer & safebuffer::operator=(safebuffer &other)
{
	if (other.m_pbuffer != m_pbuffer)
	{
		//no memory to use
		if (m_pbuffer == nullptr)
		{
			m_pbuffer = (char *)malloc(other.m_bufferlen);
			assert(m_pbuffer != nullptr);
			m_datalen = 0;
			m_bufferlen = other.m_bufferlen;
		}
		// has memory and data
		else if (m_datalen != 0)
		{
			ClearMem();
			m_datalen = 0;
		}

		readmem(other.GetMemory(), other.getDataLength());
	}

	return *this;
}

unsigned long safebuffer::getMemLength()  
{
	return m_bufferlen;
}

 char* safebuffer::GetMemory()  
{
	return m_pbuffer;
}

unsigned long safebuffer::getDataLength()  
{
	return m_datalen;
}


void safebuffer::ClearMem()
{
	memset(m_pbuffer, 0x00, m_datalen);
	m_datalen = 0;
	m_offset = 0;
}

bool safebuffer::readmem(char *pMem, unsigned long len)
{
	// no memory to use
	if (m_pbuffer == nullptr)
	{
		// set datelen and bufferlen to 0
		m_datalen = 0;
		m_bufferlen = 0;
		// offset must be 0
		m_bufferlen = (len / DEFAULT_BUFFER_LEN + 1) * DEFAULT_BUFFER_LEN;
		//allocate memory
		m_pbuffer = (char *)malloc(m_bufferlen);
		assert(m_pbuffer != nullptr);
		memset(m_pbuffer, 0x00, m_bufferlen);
	}


	if (len >= m_bufferlen - m_datalen)
	{
		unsigned long tempBuflen = m_datalen + (len / DEFAULT_BUFFER_LEN + 1) * DEFAULT_BUFFER_LEN;
		if (!resize(tempBuflen))
		{
			return false;
		}
	}
	memcpy((char *)((unsigned long)m_pbuffer + m_datalen), pMem, len);
	m_datalen += len;
	return true;

}

safebuffer & safebuffer::swap(safebuffer &other)
{
	char *ptemp = other.m_pbuffer;
	other.m_pbuffer = m_pbuffer;
	m_pbuffer = ptemp;

	unsigned long temp = other.m_bufferlen;
	other.m_bufferlen = m_bufferlen;
	m_bufferlen = temp;

	temp = other.m_datalen;
	other.m_datalen = m_datalen;
	m_datalen = other.m_datalen;

	return *this;
}


char * safebuffer::GetoffsetMemory(unsigned long offset)
{
	return (char *)((unsigned long)m_pbuffer + offset);
}

bool safebuffer::resize(unsigned long len)
{
	if (len <= m_bufferlen)
	{
		return true;
	}

	m_bufferlen = len;
	m_pbuffer = (char *)realloc(m_pbuffer, m_bufferlen);
	if (m_pbuffer == nullptr)
	{
		m_bufferlen = 0;
		m_datalen = 0;
		return false;
	}
	memset((char *)((unsigned long)m_pbuffer + m_datalen), 0x00, m_bufferlen - m_datalen);
	return true;
}

unsigned long safebuffer::GetCurrentReadOffset()
{
	return m_offset;
}

void safebuffer::StepOffset(unsigned long step)
{
	m_offset += step;
}

