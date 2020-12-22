#pragma once
#include "AntiyCommon.h"


typedef struct _ANTIYMFILTER_PROCLIST
{
	ULONG ProcessId;
	LIST_ENTRY List;

}ANTIYMFILTER_PROCLIST,*PANTIYMFILTER_PROCLIST;


TOKEN_USER * __fastcall GetProcessToken(HANDLE TokenHandle, TOKEN_STATISTICS *pTokenStatics, ULONG *pTokenGroup, ULONG *pTokenVirtualEnable, ULONG *ResultLength, ULONG *pids);

HANDLE __fastcall GetProcessHandle(HANDLE ProcessId, PVOID *pTokenStatics, TOKEN_USER *pTokenUser, ULONG *pTokenGroup, ULONG *ResultLength, PVOID ProcessInformation, ULONG *pids);

NTSTATUS  __fastcall GetProcessInfoDataByPid(HANDLE ProcessId, BOOL IsIrp, PIRP MasterIrp, PIRP pIrp,int Parent);

VOID CreateProcessCallback(HANDLE ParentId,HANDLE ProcessId,BOOLEAN Create);

NTSTATUS __fastcall GetDosNameByImageName(PUNICODE_STRING ObjectName, POBJECT_NAME_INFORMATION *Output);

BOOL GetProcessParamtersByPeb(HANDLE Handle, PPEB *PebAddress, PUNICODE_STRING DestinationString, PUNICODE_STRING szCommlineStr, PUNICODE_STRING szCurrentDirtoryStr);
WCHAR *__fastcall GetUnicodeStringByAnsiString(HANDLE Handle, PUNICODE_STRING DestinationString);

NTSTATUS __fastcall QueryProcessFileInfo(PUNICODE_STRING Source, ULONG Config_flag1, ULONG *Dst);

VOID CreateProcessCallback(HANDLE ParentId,HANDLE ProcessId,BOOLEAN Create);

VOID CreateThreadCallback(HANDLE ProcessId,HANDLE ThreadId,BOOLEAN Create);

