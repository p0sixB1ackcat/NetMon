#include "AntiyEtw.h"

CLock::CLock()
{
	InitializeCriticalSection(&m_CriticalSection);
}

void CLock::Lock(void)
{
	EnterCriticalSection(&m_CriticalSection);
}

void CLock::Unlock(void)
{
	LeaveCriticalSection(&m_CriticalSection);
}

CLock::~CLock()
{
	DeleteCriticalSection(&m_CriticalSection);
}

CAutoLock::CAutoLock(CLock *pLock) :m_pLock(pLock)
{
	m_pLock->Lock();
}

CAutoLock::~CAutoLock(void)
{
	m_pLock->Unlock();
}


