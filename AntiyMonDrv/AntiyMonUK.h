#pragma once
//Kernel & user mode

#ifndef __ANTIYMONUK_H__
#define __ANTIYMONUK_H__


//
//  Name of port used to communicate
//

#define ATPortName L"\\ATPort"

#define AntiyMFilter_READ_BUFFER_SIZE   1024
#define MAX_PATH 260

#define DRIVER_NAME L"AntiyMonDrv"
#define DRIVER_PATH L".\\AntiyMonDrv.sys"
#define DRIVER_ALTITUDE L"370020"

//内核给应用发来的事件类型，比如创建进程、创建线程、文件操作、加载镜像、加载驱动、管道操作等事件类型
typedef enum _ANTIYMFILTER_EVENT_TYPE
{

	ATMFilter_CreateProcess = 0,
	ATMFilter_CloseProcess,
	ATMFilter_CreateRemoteThread,
	ATMFilter_CloseThread,
	ATMFilter_OpenFile,
	ATMFilter_ReadFile,
	ATMFilter_WriteFile,
	ATMFilter_CloseFile,
	ATMFilter_DeleteFile,
	

}ANTIYMFILTER_EVENT_TYPE;


typedef struct _ANTIYMFILTER_PROCESSINFO
{
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
	LIST_ENTRY List;
} ANTIYMFILTER_PROCESSINFO, *PANTIYMFILTER_PROCESSINFO;

typedef struct _ANTIYMFILTER_TERMINATEPROCINFO
{
	ULONG dwUnknow;
	ULONG dwSidLen;
	ULONG dwProcessId;
	LARGE_INTEGER tmSystemTime;
	ULONG dwMaxCount;
	SID sidValue;
	unsigned char PlaceBuffer[1];

}ANTIYMFILTER_TERMINATE_PROCESSINFO,*PANTIYMFILTER_TERMINATE_PROCESSINFO;

/************************************************************************/
/* 
	给R3发送的最终数据
	Type:区分捕获的事件类型
	Notification:最终的数据
*/
/************************************************************************/
typedef struct _ANTIYMFILTER_REPORT_R3DATA
{
	ANTIYMFILTER_EVENT_TYPE Type;
	ULONG dwReportDataLength;
	union _ReportData
	{
		ANTIYMFILTER_PROCESSINFO ProcInfo;
		ANTIYMFILTER_TERMINATE_PROCESSINFO TerMinateProcInfo;
	}ReportData;
	

}ANTIYMFILTER_REPORT_R3DATA,*PANTIYMFILTER_REPORT_R3DATA;

typedef struct _AntiyMFilter_REPLY {

	BOOLEAN SafeToOpen;

} AntiyMFilter_REPLY, *PAntiyMFilter_REPLY;

#endif //  __ANTIYMONUK_H__


