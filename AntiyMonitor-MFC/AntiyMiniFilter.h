#pragma once
#include "AntiyEtw.h"

typedef void(*ANTIYMFILTERCALLBACK)(void *p);

typedef struct _AntiyMFilter_Thread_Context
{
	HANDLE hPort;
	HANDLE hCompletion;

}AntiyMFilter_Thread_Context,*PAntiyMFilter_Thread_Context;


class CAntiyMiniFilter
{
public:
	static CAntiyMiniFilter *ShareInstance(void);
	
	BOOL InitMiniFilter(void);

	BOOL CreatePort(PCWSTR PortName);

	BOOL InstallMiniFilter(void);
	BOOL UnInstallMiniFilter(void);
	BOOL StartFilter(void);
	BOOL StopFilter(void);

	~CAntiyMiniFilter();
private:
	static DWORD AntiyMFilterWait(PVOID lpContext);
	static DWORD AntiyMFilterWork(AntiyMFilter_Thread_Context *Context);
public:
	DWORD m_DefaultThreadCount;
	DWORD m_DefaultRequestCount;
	DWORD m_MaxThreadCount;
	ANTIYMFILTERCALLBACK m_Callback;//给上层回调
private:
	CAntiyMiniFilter();
	
private:
	static CAntiyMiniFilter *m_ShareInstance;
	static CLock m_Clock;
};