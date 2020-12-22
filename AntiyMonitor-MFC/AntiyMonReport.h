#pragma once
#include "AntiyGetFileInfo.h"

//该枚举中的值要和UpdateMFCType中的一致
typedef enum _REPORT_DATA_TYPE
{
	Report_CreateProcess = 0,
	Report_TerminateProc,
	Report_CreateRemoteThread,

}REPORT_DATA_TYPE;

typedef struct _REPORT_MFC_DATA
{
	REPORT_DATA_TYPE ReportType;
	ULONG ProcessId;
	ULONG ParentId;
	ULONG TerminalSessionId;
	LARGE_INTEGER LogonId;
	LARGE_INTEGER TimeStamp;
	SID UserTokenSid;
	WCHAR CommandLine[MAX_PATH * 10];
	WCHAR ImagePath[MAX_PATH];
	WCHAR CurrentDirtory[MAX_PATH];
	WCHAR ParentCommandLine[MAX_PATH * 10];
	WCHAR ParentImagePath[MAX_PATH];
	WCHAR ParentCurrentDirtyory[MAX_PATH];
	LOCAL_FILE_INFO Local_File_Info;
}REPORT_MFC_DATA,*PREPORT_MFC_DATA;

typedef struct _REPORT_DATA_INFO
{
	REPORT_DATA_TYPE type; //
	void *ReportData;
};



class CAntiyMonReport
{
public:
	CAntiyMonReport();
	~CAntiyMonReport();
};

