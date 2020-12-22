#pragma once
#include "Common.h"
#include "CLock.h"

typedef VOID(*ETWCALLBACK)(void);

class CAntiyEventTraceData
{
public:
	static CAntiyEventTraceData *ShareInstance(void);
	void Clear(void);
	
	~CAntiyEventTraceData();
public:
	USHORT m_LocalPort;
	USHORT m_RemotePort;
	WCHAR m_ProcessId[0x10];
	WCHAR m_OperType[0x10];
	WCHAR m_RemoteIpAddress[0x30];
	WCHAR m_LocalIpAddress[0x30];
	WCHAR m_OperaResult[0x30];
	WCHAR m_OperationPath[MAX_PATH];
	WCHAR m_ProcessName[MAX_PATH];
	WCHAR m_Time[MAX_PATH];
	ETWCALLBACK m_EtwCallback;
	STARTTRACEW m_StartTraceFunc;
	CONTROLTRACEW m_ControlTraceFunc;
	OPENTRACEW m_OpenTraceFunc;
	PROCESSTRACE m_ProcessTraceFunc;
private:
	CAntiyEventTraceData();
private:
	static CAntiyEventTraceData *m_This;
	static CLock m_Lock;
};

