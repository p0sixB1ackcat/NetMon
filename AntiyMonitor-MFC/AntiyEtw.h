#pragma once
#include <stdio.h>
#define INITGUID
#include <windows.h>
#include <winsvc.h>
#include <evntrace.h>
#include <conio.h>
#include <wmistr.h>
#include <strsafe.h>
//#include <sysinfoapi.h>
#include <evntprov.h>
#include <evntcons.h>
#include <process.h>

#define MAX_MOF_LIST_COUNT 357913940

typedef enum _OperationType
{
	TCPSend = 0,
	TCPRecive,
	UDPSend,
	UDPRecive
}ProcOperType;

typedef VOID(*ETWCALLBACK)(void);

#ifdef __cplusplus
extern "C" {
#endif
	typedef ULONG (WINAPI *STARTTRACEW)(
			_Out_ PTRACEHANDLE TraceHandle,
			_In_ LPCWSTR InstanceName,
			_Inout_ PEVENT_TRACE_PROPERTIES Properties
			);
	typedef ULONG(WINAPI *CONTROLTRACEW)(
			_In_ TRACEHANDLE TraceHandle,
			_In_opt_ LPCWSTR InstanceName,
			_Inout_ PEVENT_TRACE_PROPERTIES Properties,
			_In_ ULONG ControlCode
			);
	typedef TRACEHANDLE
		(WINAPI *OPENTRACEW)(
			_Inout_ PEVENT_TRACE_LOGFILEW Logfile
			);
	typedef ULONG
		(WINAPI *PROCESSTRACE)(
			_In_reads_(HandleCount) PTRACEHANDLE HandleArray,
			_In_ ULONG HandleCount,
			_In_opt_ LPFILETIME StartTime,
			_In_opt_ LPFILETIME EndTime
			);
#ifdef __cplusplus
};
#endif

//该枚举中的值要和Report_Data_Type中的一致
typedef enum _UpDateMFCType
{
	UDDM_ETW = 0x1000,
	UDDM_WMI = 0x2000,
	UDDM_CREATEPROCESS = 0,
	UDDM_TERMINPROC = 1,
	UDDM_CREATEREMOTETHREAD = 2,
}UpDateMFCType;

#define WM_UPDATEETWDATA 10000 + 1

DEFINE_GUID(rclsid,
	0x4590F811, 0x1D3A, 0x11D0, 0x89, 0x1f, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24);

DEFINE_GUID(riid,
	0xDC12A687, 0x737F, 0x11cf, 0x88, 0x4d, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24);

void InitMonitorData(void);

UINT StartEventTrace(ULONG IsStart);

BOOL
WINAPI ProcessExitCallbackRoutine
(
	_In_ DWORD CtrlType
);

VOID LockMof(VOID);
VOID UnlockMof(VOID);

VOID LockEtw(VOID);
VOID UnlockEtw(VOID);

int *GetPointerByFileTime(FILETIME * lpFileTime);

BOOL InitProcessHeap(VOID);

BOOL InitTimeData(VOID);

LONGLONG CalcByTraceTimeLessPerformanceCount(FILETIME *a1, LONGLONG a2);

LARGE_INTEGER CalcTime(LARGE_INTEGER TraceHeaderTimeStamp);

void GetLocalSystemTimeByFileTimeStamp(LARGE_INTEGER *pfileTimeStamp, SYSTEMTIME *st);

void InitListEntry(LIST_ENTRY *pList);

void InsertHeaderList(LIST_ENTRY *pHeader, LIST_ENTRY *pList);

void RemoveHeaderList(LIST_ENTRY *pList);

BOOLEAN IsEmptyList(LIST_ENTRY *pList);

ULONG InitMofProxy(VOID);

void WINAPI ProcessEvent(PEVENT_TRACE pEvent);

ULONG
WINAPI
EventTraceLogfileBufferCallback
(
	PEVENT_TRACE_LOGFILEW lpLogfile
);

VOID WINAPI EventTraceLogfileEventRecordCallback(PEVENT_RECORD lpRecord);

VOID WINAPI EventTraceLogfileEventCallback(PEVENT_TRACE lpTrace);

VOID NTAPI RegEventCallBack(_In_ LPCGUID SourceId, _In_ ULONG IsEnabled, _In_ UCHAR Level, _In_ ULONGLONG MatchAnyKeyword, _In_ ULONGLONG MatchAllKeyword, _In_opt_ PEVENT_FILTER_DESCRIPTOR FilterData, _Inout_opt_ PVOID CallbackContext);

SERVICE_STATUS_HANDLE WINAPI StartServiceCtrlDispatchCallback(DWORD   dwNumServicesArgs, LPWSTR  *lpServiceArgVectors);

DWORD WINAPI RegServiceCtrlHandlerExWCallback(DWORD    dwControl, DWORD    dwEventType, LPVOID   lpEventData, LPVOID   lpContext);

class CLock
{
public:
	CLock();
	void Lock(void);
	void Unlock(void);
	~CLock();
private:
	CRITICAL_SECTION m_CriticalSection;
};

class CAutoLock
{
public:
	CAutoLock(CLock *pLock);
	~CAutoLock();

private:CLock *m_pLock;

};

class CAntiyMonData
{
	friend class CAntiyMonDataEntry;
	friend class CAntiyMonLinkList;
	friend class CAntiyMonProperty;
public:
	CAntiyMonData();
	~CAntiyMonData();

	BOOL Compare(BSTR bstrString);
	BOOL CompareN(BSTR bstrString, ULONG nSize);
	ULONG Len(void);

public:
	BSTR m_Data;
private:
	void *m_Alloc;
	ULONG m_Reference;
};

class CAntiyMonLinkList
{
public:
	CAntiyMonLinkList(LIST_ENTRY *Blink, LIST_ENTRY *Flink);
	CAntiyMonLinkList();
	CAntiyMonLinkList *InsertHeader(LIST_ENTRY *Blink, void *pDataEvent);
	~CAntiyMonLinkList();
public:
	LIST_ENTRY m_Entry;
	void *m_pDataEvent;
};

class CAntiyMonDataEntry
{
	friend class CAntiyMonLinkList;
public:
	CAntiyMonDataEntry(GUID *guid, USHORT Type, USHORT Version, USHORT Level);
	CAntiyMonDataEntry();
	void operator = (CAntiyMonDataEntry *pDataEntry);
	ULONG GetClassDataLen(void);
	CAntiyMonDataEntry *InitData(CAntiyMonData **ppData, BSTR bString);
	~CAntiyMonDataEntry();
public:
	CAntiyMonData *m_ClassData;
	CAntiyMonData *m_TypeData;
	GUID m_Guid;
	USHORT m_Type;
	USHORT m_Version;
	USHORT m_Level;
	CAntiyMonLinkList *m_PropertyList;
	ULONG m_PropertyCount;

};

CAntiyMonDataEntry *WINAPI ResolveEventTrace(PEVENT_TRACE pEvent);

class CAntiyMonProperty
{
public:
	CAntiyMonProperty(BSTR Name, LONG ArraySize, LONG Length);
	CAntiyMonProperty();
	void InitData(CAntiyMonData **ppData, BSTR bString);
	~CAntiyMonProperty();
	CAntiyMonData *m_Data;
	LONG m_ArraySize;
	LONG m_Len;

};

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