#pragma once
#include <windows.h>
#include <WbemCli.h>
#include <evntrace.h>

void InitWmi(void);
BOOL CreateWmiMon(void);
BOOL CreateEtwEventTrace(void);
BOOL CloseEtwTraceWmiEvent(void);

typedef struct _TRACE_PROPERTIES
{
	EVENT_TRACE_PROPERTIES m_Properties;
	WCHAR m_SessionName[128];
	WCHAR m_Buffer[1000];
}TRACE_PROPERTIES, *PTRACE_PROPERTIES;

typedef struct _EXTENSION
{
	TRACE_PROPERTIES Properties;
	ENABLE_TRACE_PARAMETERS Parameters;
	TRACEHANDLE hSession;
	TRACEHANDLE hTrace;
	BOOL bIsCreateGuid;
	GUID guid;
	GUID ProviderGuid;
	ULONGLONG MatchAndKeyword;
	HANDLE Event;

}EXTENSION, *PEXTENSION;

typedef struct _REPORT_WMI_EVENT
{
	ULONG dwProcessId;
	WCHAR *szServerName;
	WCHAR *szClassName;
	WCHAR *szEventName;
	WCHAR *szNamespace;
	WCHAR *szSuperClass;
	WCHAR *szName;
	WCHAR *szQuery;
	WCHAR *szQueryLanguage;
	WCHAR *szTime;
	WCHAR *szProcessFullPath;
	ULONG OperaCount;
	WCHAR *OperaList[MAX_PATH];
}REPORT_WMI_EVENT, *PREPORT_WMI_EVENT;

class CEtwWmiData
{
public:
	CEtwWmiData();
	void Add(LPWSTR Key, LPWSTR Value);
	void Add(LPWSTR Key, UINT32 Value);
	void Out(void);
	~CEtwWmiData();

public:
	USHORT m_id;
	UINT32 m_GroupOperationId;
	UINT32 m_OperationId;
	UINT32 m_ClientProcessid;
	WCHAR *m_Operation;
	WCHAR *m_ProcessFullPath;
	LIST_ENTRY m_List;
	LARGE_INTEGER m_TimeStamp;
};

typedef struct _UNICODE_STRING
{
	WCHAR *Buffer;
	ULONG Length;
	ULONG MaxLength;
}UNICODE_STRING, *PUNICODE_STRING;

typedef struct _WMI_EVENT_DATA
{
	UNICODE_STRING *szClassName;
	UNICODE_STRING *szEventName;
	UNICODE_STRING *szSuperClassName;
	UNICODE_STRING *szServerName;
	UNICODE_STRING *szNameSpace;
	UNICODE_STRING *szName;
	UNICODE_STRING *szQuery;
	UNICODE_STRING *szQueryLanguage;
	LIST_ENTRY List;
	LARGE_INTEGER llTimeCreate;
}WMI_EVENT_DATA, *PWMI_EVENT_DATA;

class EventSink :
	public IWbemObjectSink
{

	LONG m_lRef;
	bool bDone;
	RTL_CRITICAL_SECTION m_WmiEventLock;
public:
	IWbemServices *m_WbemService;
	LIST_ENTRY m_ListHeader;
public:
	EventSink();
	~EventSink();
	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT
		STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

	virtual HRESULT STDMETHODCALLTYPE Indicate(
		LONG lObjectCount,
		IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray
	);

	virtual HRESULT STDMETHODCALLTYPE SetStatus(
		/* [in] */ LONG lFlags,
		/* [in] */ HRESULT hResult,
		/* [in] */ BSTR strParam,
		/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
	);

	WMI_EVENT_DATA *EnumClassData(IWbemClassObject *pClass);
	void SetWmiEventData(WMI_EVENT_DATA *pWmiEventData, WCHAR *pKey, WCHAR *pValue);
	LARGE_INTEGER GetTimeStampValue(WCHAR *szTime);
	//void SetWmiEventData(WCHAR *pKey, ULONGLONG llValue);
	void FreeWmiData();
};

class CantiyWmiClass
{
public:
	static CantiyWmiClass *ShareInstance();
	~CantiyWmiClass();
	BOOL CreateWmiMon(void);
	BOOL CreateWmiEventMon(VOID);
	void ReportWmiData(REPORT_WMI_EVENT *pReportWmiData);
	static UINT32 WaitReceiveWmiThreadRoutine(void *pContext);
	static VOID WINAPI EtwEventTraceRecordCallback(PEVENT_RECORD EventRecord);
	void CreateGuidStr(WCHAR *szBuffer, size_t dwBufferLen);
	static ULONG WINAPI EventBufferCallbackRoutine(PEVENT_TRACE_LOGFILEW Logfile);
	BOOL CreateEtwEventTrace(void);
	static UINT32 __stdcall ProviderThreadRoutine(void *pContext);
	void FreeEtwWmiData(void);
	void UpdateData(void);
	void CloseTraceAndSession(void);
private:
	CantiyWmiClass();
public:
	PREPORT_WMI_EVENT m_pReport;
	static CantiyWmiClass *m_Instance;
	IWbemUnsecuredApartment *m_UnsecuredApartment = NULL;
	EventSink *m_WmiListernSink = NULL;
	IWbemServices *m_WbemService = NULL;
	LIST_ENTRY m_EtwWmiDataListHeader;
	RTL_CRITICAL_SECTION m_EtwWmiDataLock;
	EXTENSION m_Extension;
};

