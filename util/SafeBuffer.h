#ifndef _SAFE_BUFFER_
#define _SAFE_BUFFER_
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
/*
@safebuffer
	
*/
#define DEFAULT_BUFFER_LEN 1024
class safebuffer
{
public:

	safebuffer(unsigned long bufflen = DEFAULT_BUFFER_LEN);
	safebuffer(safebuffer &other);
	safebuffer(char* pMem, unsigned long len);
	virtual ~safebuffer();
	safebuffer & operator=(safebuffer &other);
	safebuffer & swap(safebuffer &other);

	bool resize(unsigned long len);
	void ClearMem();
	bool readmem(char* pMem, unsigned long len);

	unsigned long getMemLength()  ;
	unsigned long getDataLength()  ;
	/*
	@NOTE: very dangerous operation
	*/
	char* GetMemory()  ;
	/*
	@NOTE: very dangerous operation
	*/
	char* GetoffsetMemory(unsigned long offset);

	/*
	@
	*/
	unsigned long GetCurrentReadOffset();

	/*
	@
	*/
	void StepOffset(unsigned long step);

private:
	char *m_pbuffer;
	unsigned long m_bufferlen;
	unsigned long m_datalen;
	unsigned long m_offset;
};
#endif