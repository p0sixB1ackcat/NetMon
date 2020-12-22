#include "AntiyProc.h"
#include "globalvar.h"
#include "AntiyMiniFilterCallback.h"
#include "AntiyMonUK.h"
#include <ntddk.h>

extern FAST_MUTEX g_ListHeaderMutex;
extern LIST_ENTRY g_ListHeader;

extern FAST_MUTEX g_ProcessRelevanceListLock;
extern LIST_ENTRY g_ProcessRelevanceListHeader;

extern FAST_MUTEX g_TerminateProcessListLock;
extern LIST_ENTRY g_TerminateProcessListHeader;

TOKEN_USER * __fastcall GetProcessToken(HANDLE TokenHandle, TOKEN_STATISTICS *pTokenStatics, ULONG *pTokenGroup, ULONG *pTokenVirtualEnable, ULONG *ResultLength, ULONG *pids)
{
	PVOID pResult;
	HANDLE hToken;
	ULONG *LengthAddr;
	TOKEN_USER *P = NULL;
	PVOID pTokenGroups = NULL;
	PVOID pTokenImpersonationLevel = NULL;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	
	hToken = TokenHandle;
	if (pTokenGroup)
		*pTokenGroup = 0;
	LengthAddr = ResultLength;
	if (ResultLength)
		*ResultLength = 0;
	if (pids)
		*(ULONG *)pids = 0;

	ntStatus = ZwQueryInformationToken(TokenHandle, TokenUser, 0, 0, &ResultLength);
	if (ntStatus != STATUS_BUFFER_TOO_SMALL)
		return 0;

	P = ExAllocatePoolWithTag(NonPagedPool, ResultLength, 'EKOT');
	if (!P)
		return 0;
	
	ntStatus = ZwQueryInformationToken(hToken, TokenUser, P, ResultLength, &ResultLength);
	if (!NT_SUCCESS(ntStatus))
	{
		ExFreePoolWithTag(P, 0);
		P = NULL;
	}

	while(pTokenGroup)
	{
		ntStatus = ZwQueryInformationToken(hToken, TokenGroups, 0, 0, &ResultLength);
		if (ntStatus != STATUS_BUFFER_TOO_SMALL)
			break;

		pTokenGroups = ExAllocatePoolWithTag(NonPagedPool, *ResultLength, 'PGOT');

		*pTokenGroup = pTokenGroups;
		if (!pTokenGroups)
			break;

		ntStatus = ZwQueryInformationToken(hToken, TokenGroups, pTokenGroups, ResultLength, &ResultLength);
		if (!NT_SUCCESS(ntStatus))
		{
			ExFreePoolWithTag(*pTokenGroup, 0);
			*pTokenGroup = 0;
		}
		break;
	} ;
	
	if (pTokenStatics)
	{
		ResultLength = sizeof(TOKEN_STATISTICS);
		ZwQueryInformationToken(hToken, TokenStatistics, pTokenStatics, ResultLength, &ResultLength);
	}

	if (pTokenVirtualEnable)
	{
		ntStatus = ZwQueryInformationToken(hToken, TokenVirtualizationEnabled, pTokenVirtualEnable, sizeof(ULONG), &ResultLength);
		if (!NT_SUCCESS(ntStatus))
		{
			*(ULONG *)pTokenVirtualEnable = -1;
		}
	}

	if (LengthAddr)
	{
		ntStatus = ZwQueryInformationToken(hToken, TokenImpersonationLevel | TokenAuditPolicy, 0, 0, &ResultLength);
		if (ntStatus == STATUS_BUFFER_TOO_SMALL)
		{
			pTokenImpersonationLevel = ExAllocatePoolWithTag(NonPagedPool, ResultLength, 'asyS');
			if (pTokenImpersonationLevel)
			{
				*LengthAddr = pTokenImpersonationLevel;
				ntStatus = ZwQueryInformationToken(hToken, TokenImpersonationLevel | TokenAuditPolicy, pTokenImpersonationLevel, ResultLength, &ResultLength);
				if (!NT_SUCCESS(ntStatus))
				{
					ExFreePoolWithTag(pTokenImpersonationLevel, 0);
					pTokenImpersonationLevel = NULL;
				}
			}
		}
	}

	if (pids)
	{
		ZwQueryInformationToken(hToken, TokenSessionId | TokenUser | TokenAuditPolicy, pids, sizeof(ULONG), &ResultLength);
	}
	
	return P;
}

HANDLE __fastcall GetProcessHandle(HANDLE ProcessId, PVOID *pTokenStatics,TOKEN_USER *pTokenUser, ULONG *pTokenGroup, ULONG *ResultLength, PVOID ProcessInformation, ULONG *pids)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES ObjectAttributes;
	LARGE_INTEGER TimeOut;
	CLIENT_ID ClinentId;
	HANDLE TokenHandle;
	HANDLE ProcessHandle;
	ULONG VirtualEnable = -1;
	PVOID lpTokenUser = NULL;

	ProcessHandle = 0;
	TimeOut.QuadPart = 0;
	if (ProcessId)
	{
		ClinentId.UniqueThread = 0;
		ClinentId.UniqueProcess = ProcessId;
		InitializeObjectAttributes(&ObjectAttributes,0,512,0,0);
		
		ntStatus = ZwOpenProcess(&ProcessHandle, 0, &ObjectAttributes, &ClinentId);
		if (!NT_SUCCESS(ntStatus))
			return 0;
		
		if (g_BuildNumber < WIN_BUILDNUM_VISTA)
		{
			ntStatus = ZwWaitForSingleObject(ProcessHandle, 0, &TimeOut);
			if (ntStatus != STATUS_TIMEOUT)
			{
				ZwClose(ProcessHandle);
				return 0;
			}
			
		}
		
		ZwQueryInformationProcess(ProcessHandle, ProcessSessionInformation, ProcessInformation, sizeof(ULONG), NULL);
		if (g_ZwOpenProcessTokenEx)
			ntStatus = g_ZwOpenProcessTokenEx(ProcessHandle, READ_CONTROL | TOKEN_QUERY, 512, &TokenHandle);
		else
			ntStatus = ZwOpenProcessToken(ProcessHandle, READ_CONTROL | TOKEN_QUERY, &TokenHandle); 
		if (NT_SUCCESS(ntStatus))
		{
			lpTokenUser = GetProcessToken(TokenHandle, pTokenStatics, pTokenGroup, &VirtualEnable, ResultLength, pids);
			*(DWORD *)pTokenUser = lpTokenUser;
			ZwClose(TokenHandle);
		}
	}
	
	return ProcessHandle;
}

NTSTATUS GetNameInformationLength(PVOID Dst, int offset, WCHAR *szNameInfoBuffer, ULONG dwNameInfoBuffSize, PULONG pResultLength)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	return ntStatus;
}

VOID InsertTerminateProcessList(HANDLE ProcessId)
{
	ANTIYMFILTER_PROCLIST *pList = NULL;

	pList = ExAllocatePoolWithTag(PagedPool, sizeof(ANTIYMFILTER_PROCLIST), '0syS');
	if (pList)
	{
		pList->ProcessId = ProcessId;
		ExAcquireFastMutex(&g_TerminateProcessListLock);
		InsertHeadList(&g_TerminateProcessListHeader, &pList->List);
		ExReleaseFastMutex(&g_TerminateProcessListLock);
	}
}

VOID FillNotificationDataWithR3(ANTIYMFILTER_PROCESSINFO *pDesNotification, PROCESS_BASIC_INFORMATION *pBasicInformation, PROCESS_SESSION_INFORMATION *pSessionInformation, UNICODE_STRING *szImagePath, UNICODE_STRING *szCommandLine, UNICODE_STRING *szCurrentDirtory, LUID *pLogonId,LARGE_INTEGER *pTimeStamp)
{
	KIRQL CurrentIrql = KeGetCurrentIrql();
	if (CurrentIrql > APC_LEVEL)
	{
		DbgPrint("Current IRQL Bigger Than APC!\n");
		return;
	}
	if (!pDesNotification->CommandLine)
	{
		DbgPrint("Des CommandLine is NULL!\n");
		return;
	}
	if (!pDesNotification->ImagePath)
	{
		DbgPrint("Des ImagePath is Null!\n");
		return;
	}
	if (!pDesNotification->CurrentDirtory)
	{
		DbgPrint("Des CurrentDirtory is NULL!\n");
		return;
	}

	pDesNotification->ProcessId = pBasicInformation->UniqueProcessId;
	pDesNotification->ParentId = pBasicInformation->InheritedFromUniqueProcessId;
	pDesNotification->LogonId.LowPart = pLogonId->LowPart;
	pDesNotification->LogonId.HighPart = pLogonId->HighPart;
	pDesNotification->TimeStamp.QuadPart = pTimeStamp->QuadPart;
	pDesNotification->TerminalSessionId = pSessionInformation->SessionId;
	
	RtlCopyMemory(pDesNotification->CommandLine, szCommandLine->Buffer, min(sizeof(pDesNotification->CommandLine), szCommandLine->Length));
	RtlCopyMemory(pDesNotification->ImagePath, szImagePath->Buffer, min(sizeof(pDesNotification->ImagePath), szImagePath->Length));
	RtlCopyMemory(pDesNotification->CurrentDirtory, szCurrentDirtory->Buffer, min(sizeof(pDesNotification->CurrentDirtory), szCurrentDirtory->Length));
}

NTSTATUS  __fastcall GetProcessInfoDataByPid(HANDLE ProcessId, BOOL IsIrp, PIRP MasterIrp, PIRP pIrp,int Parent)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UNICODE_STRING *pFileName = NULL;
	UNICODE_STRING *puReceiveDataBufferUnicodeString = NULL;
	UNICODE_STRING uImagePathName = {0x00};
	TOKEN_STATISTICS TokenStics;
	TOKEN_USER *pTokenUser = NULL;
	ULONG *dwResultLen;
	HANDLE ProcessHandle;
	PROCESS_BASIC_INFORMATION ProcessInformation = {0x00};
	PROCESS_SESSION_INFORMATION ProcessSessionInfo;
	ULONG ProcessInformationLength;
	__int64 dqProcessTime;
	LARGE_INTEGER Subtrahend;
	UINT_PTR ResultLength;
	size_t MaxCount = 0;
	ULONG dwTokenUser = 0;
	ULONG dwTokenUserDataLen = 0;
	ULONG dwSessionLength = 0;
	PVOID P = NULL;
	POBJECT_NAME_INFORMATION pObjectNameInfo = NULL;
	UNICODE_STRING uCommlineStr;
	UNICODE_STRING uCurrentDirtoryStr;
	ANTIYMFILTER_PROCESSINFO *pNotification = NULL;
	KIRQL CurrentIrql;

	dwResultLen = NULL;
	
	ProcessHandle = GetProcessHandle(ProcessId, &TokenStics, &pTokenUser, 0, (ULONG)&dwResultLen, &ProcessSessionInfo,&dwSessionLength);

	if (!ProcessHandle)
		return STATUS_INVALID_PARAMETER; 

	ntStatus = ZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &ProcessInformation, sizeof(PROCESS_BASIC_INFORMATION), 0);
	
	__try
	{
		if (NT_SUCCESS(ntStatus))
		{
			if (!GetProcessParamtersByPeb(ProcessHandle, ProcessInformation.PebBaseAddress, (int)&uImagePathName, &uCommlineStr, &uCurrentDirtoryStr))
				GetUnicodeStringByAnsiString(ProcessHandle, &uImagePathName);

			
			if (MasterIrp)
				//InsertTerminateProcessList(ProcessId);

			//获取父进程信息
			if (Parent == 0)
				GetProcessInfoDataByPid((HANDLE)ProcessInformation.InheritedFromUniqueProcessId, 0, 1, pIrp, 1);
			

			dqProcessTime = 0;
			ZwQueryInformationProcess(ProcessHandle, ProcessTimes, &dqProcessTime, 0x20, 0);
			
			if (!dqProcessTime)
			{
				ResultLength = 48;
				ntStatus = ZwQuerySystemInformation(SystemTimeOfDayInformation, &dqProcessTime, ResultLength, &ResultLength);
				if (NT_SUCCESS(ntStatus))
				{
					Subtrahend.QuadPart = dqProcessTime & 0xffff0000;
					dqProcessTime -= Subtrahend.QuadPart;
				}
			}

			Subtrahend.QuadPart = dqProcessTime;

			if (pTokenUser)
			{
				MaxCount = RtlLengthSid(*(PSID *)pTokenUser);
				dwTokenUser = *(ULONG *)pTokenUser;
				if (MaxCount)
					P = MaxCount;
				else
					dwTokenUser = 0;
			}
			if (dwResultLen)
			{
				dwTokenUserDataLen = RtlLengthSid(*(PSID *)dwResultLen);
				if (dwTokenUserDataLen)
				{
					if ((unsigned int)P + dwTokenUserDataLen < dwTokenUserDataLen)
					{
						ntStatus = STATUS_BUFFER_OVERFLOW;
						puReceiveDataBufferUnicodeString = NULL;
						__leave;
					}
					P = (char *)P + dwTokenUserDataLen;
				}
				else
				{

				}
			}

			ProcessInformationLength = (MAX_PATH + 4) * sizeof(WCHAR);
			puReceiveDataBufferUnicodeString = (UNICODE_STRING *)ExAllocatePoolWithTag(NonPagedPool, ProcessInformationLength, 'FFUB');
			if (!puReceiveDataBufferUnicodeString)
			{
				ntStatus = STATUS_NO_MEMORY;
				__leave;
			}
			
			ntStatus = ZwQueryInformationProcess(ProcessHandle, ProcessImageFileName, puReceiveDataBufferUnicodeString, ProcessInformationLength, &ProcessInformationLength);
			if (ntStatus = STATUS_INFO_LENGTH_MISMATCH)
			{
				ExFreePoolWithTag(puReceiveDataBufferUnicodeString, 0);
				puReceiveDataBufferUnicodeString = (UNICODE_STRING *)ExAllocatePoolWithTag(NonPagedPool, ProcessInformationLength, 'FFUB');
				if (!puReceiveDataBufferUnicodeString)
				{
					ntStatus = STATUS_NO_MEMORY;
					__leave;
				}

				ntStatus = ZwQueryInformationProcess(ProcessHandle, ProcessImageFileName, puReceiveDataBufferUnicodeString, ProcessInformationLength, &ProcessInformationLength);

				if (!NT_SUCCESS(ntStatus) || (ntStatus = GetDosNameByImageName(puReceiveDataBufferUnicodeString, &pObjectNameInfo), !NT_SUCCESS(ntStatus)))
				{
					pFileName = &uImagePathName;
				}
				else
				{
					pFileName = &pObjectNameInfo->Name;
					if (!pObjectNameInfo)
					{
					LABEL_36:
						if (uCommlineStr.Length)
						{

						}
						if (uCurrentDirtoryStr.Length)
						{

						}
						if (!IsIrp)
						{
							QueryProcessFileInfo(puReceiveDataBufferUnicodeString, 1, NULL);
						}

					}
				}
				ntStatus = GetNameInformationLength(NULL, 2, pFileName->Buffer, pFileName->Length, (PULONG)&P);
				if (NT_SUCCESS(ntStatus))
				{
					
				}
			}
			
			pNotification = (PANTIYMFILTER_PROCESSINFO)ExAllocatePoolWithTag(PagedPool,sizeof(ANTIYMFILTER_PROCESSINFO),'2syS');

			if (!pNotification)
			{
				__leave;
			}

			
			memset(pNotification, 0, sizeof(ANTIYMFILTER_PROCESSINFO));
			FillNotificationDataWithR3(pNotification, &ProcessInformation, &ProcessSessionInfo, &uImagePathName, &uCommlineStr, &uCurrentDirtoryStr, &TokenStics.AuthenticationId, &Subtrahend);
			
			CurrentIrql = KeGetCurrentIrql();
			if (CurrentIrql >= DISPATCH_LEVEL)
			{
				DbgPrint("Current IRQL is Too Heigh!\n");
				__leave;
			}
			
			ExAcquireFastMutex(&g_ProcessRelevanceListLock);
			if (!pNotification || !&pNotification->List || !&g_ProcessRelevanceListHeader)
			{
				DbgPrint("have null!\n");
			}
			InsertHeadList(&g_ProcessRelevanceListHeader, &pNotification->List);
			ExReleaseFastMutex(&g_ProcessRelevanceListLock);
			if (!Parent)
			{
				ReportDataToR3(ATMFilter_CreateProcess,NULL);
			}
			
			__leave;
		}
	}
	
	__finally 
	{
		if (pTokenUser)
			ExFreePoolWithTag(pTokenUser, 0);
		if (puReceiveDataBufferUnicodeString)
			ExFreePoolWithTag(puReceiveDataBufferUnicodeString, 0);
		if (uImagePathName.Length)
			ExFreePoolWithTag(uImagePathName.Buffer, 0);
		if (uCommlineStr.Length)
			ExFreePoolWithTag(uCommlineStr.Buffer, 0);
		if (uCurrentDirtoryStr.Length)
			ExFreePoolWithTag(uCurrentDirtoryStr.Buffer, 0);
		if (dwResultLen)
			ExFreePoolWithTag(dwResultLen,0);
		if(ProcessHandle != INVALID_KERNEL_HANDLE)
			ZwClose(ProcessHandle);
	}
	

	return ntStatus;
}


VOID CreateProcessCallback(
	 HANDLE ParentId,
	 HANDLE ProcessId,
	 BOOLEAN Create
)
{
	KIRQL kCurrentIrql;
	ANTIYMFILTER_PROCLIST *pList = NULL;
	LIST_ENTRY *pEntry;
	BOOL bFound;
	TOKEN_USER *pUserToken = NULL;
	HANDLE hProcess = NULL;
	ULONG dwSidLength;
	ANTIYMFILTER_REPORT_R3DATA *pReport = NULL;

	kCurrentIrql = KeGetCurrentIrql();
	if (kCurrentIrql >= DISPATCH_LEVEL)
	{
		//如果当前irql大于等于DISPATCH_LEVEL时，直接返回
		return;
	}
	
	if (Create)
	{
		pList = (ANTIYMFILTER_PROCLIST *)ExAllocatePoolWithTag(PagedPool, sizeof(ANTIYMFILTER_PROCLIST), '1syS');
		if (pList)
		{
			pList->ProcessId = ProcessId;
			ExAcquireFastMutex(&g_ListHeaderMutex);
			InsertHeadList(&g_ListHeader,&pList->List);
			ExReleaseFastMutex(&g_ListHeaderMutex);
		}
	}
	else
	{
		bFound = FALSE;
		ExAcquireFastMutex(&g_TerminateProcessListLock);
		if (!IsListEmpty(&g_TerminateProcessListHeader))
		{
			pEntry = g_TerminateProcessListHeader.Flink;
			while (pEntry != &g_TerminateProcessListHeader)
			{
				pList = CONTAINING_RECORD(pEntry,ANTIYMFILTER_PROCLIST,List);
				if (pList || MmIsAddressValid(pList))
				{
					if (pList->ProcessId == ProcessId)
					{
						bFound = TRUE;
						break;
					}
				}
				pEntry = pEntry->Flink;
			}
			if (bFound)
			{
				RemoveEntryList(pEntry);
				ExFreePoolWithTag(pList, 0);
			}
				
		}

		ExReleaseFastMutex(&g_TerminateProcessListLock);

		hProcess = GetProcessHandle(ProcessId, NULL, &pUserToken, NULL, NULL, NULL,NULL);
		if (hProcess && pUserToken)
		{
			dwSidLength = RtlLengthSid(*(PSID *)pUserToken);
			if (dwSidLength + 832 <= dwSidLength)
			{
				DbgPrint("*****************************\n");
			}
			else
			{
				pReport = (PANTIYMFILTER_REPORT_R3DATA)ExAllocatePoolWithTag(PagedPool, sizeof(ANTIYMFILTER_REPORT_R3DATA), 'AsyS');
				if (pReport)
				{
					memset(&pReport->ReportData.TerMinateProcInfo, 0, dwSidLength + 832);
					pReport->ReportData.TerMinateProcInfo.dwUnknow = 3;
					pReport->ReportData.TerMinateProcInfo.dwSidLen = dwSidLength + 832;
					pReport->ReportData.TerMinateProcInfo.dwProcessId = ProcessId;
					KeQuerySystemTime(&pReport->ReportData.TerMinateProcInfo.tmSystemTime);
					
					pReport->Type = ATMFilter_CloseProcess;
					pReport->dwReportDataLength = sizeof(ANTIYMFILTER_REPORT_R3DATA);
					ReportDataToR3(pReport->Type, pReport);
					ExFreePoolWithTag(pReport, 0);
				}
			}
			if (hProcess)
				ZwClose(hProcess);
			if (pUserToken)
				ExFreePoolWithTag(pUserToken,0);
		}

	}

}

typedef struct _TARGET_PROCESS_INFO
{
	PEB_LDR_DATA m_LdrData;
	PRTL_USER_PROCESS_PARAMETERS m_ProcessParameters;
	ULONG m_Value1;
	ULONG m_Value2;
	ULONG m_Value3;
	ULONG m_Value4;
	PROCESS_BASIC_INFORMATION m_ProcBasicInfo;

}TARGET_PROCESS_INFO, * PTARGET_PROCESS_INFO;

CHAR SearchMemoryOrderModuleList(PPEB_LDR_DATA pPebLdr, LIST_ENTRY *pMemoryOrderModuleList, ULONG* pThreadStartAddress, ULONG* Dest)
{
	CHAR result;
	LIST_ENTRY* pList = NULL;
	ULONG dwIndex = 0;
	PULONG pReserved = NULL;
	UCHAR Buffer[0x34] = { 0x00 };

	pList = pPebLdr->InMemoryOrderModuleList.Flink;
	if (!pList || !pPebLdr->InMemoryOrderModuleList.Blink)
		return 0;

	__try
	{
		while (pList != pMemoryOrderModuleList && dwIndex < 0x200)
		{
			ProbeForRead(pList, sizeof(LIST_ENTRY), 4);
			pReserved = CONTAINING_RECORD(pList,PEB_LDR_DATA,InMemoryOrderModuleList);
			ProbeForRead(pReserved, 0x34, 4);
			memcpy(Buffer,pReserved,0x34);
			
			if (pThreadStartAddress >= (*(ULONG*)& Buffer[24] + *(ULONG*)& Buffer[32]) || (unsigned int)pThreadStartAddress <= *(ULONG*)& Buffer[24])
			{
				result = 0;
			}
			else
			{
				ProbeForRead(*(CONST void**) & Buffer[40], 2 * *(unsigned __int16*)& Buffer[36], 2);
				Dest[132] = *(ULONG *)&Buffer[32];
				Dest[131] = *(ULONG *)& Buffer[24];
				*((CHAR*)Dest + 532) = 0;
				Dest[134] = pThreadStartAddress;
				if (*(ULONG*)& Buffer[36] > 0x208)
					memcpy(Dest, *(const void**)& Buffer[40], 0x208);
				else
					memcpy(Dest, *(const void**)& Buffer[40], 2 * *(unsigned __int16*)& Buffer[36]);
				result = 1;

			}

			++dwIndex;
			pList = pList->Flink;
		}
	}
	__finally
	{

	}

	return result;
}

NTSTATUS GetTargetProcessInfo(PEPROCESS pEprocess, ULONG pThreadAddress, PVOID pOutputBuffer)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	KAPC_STATE kApc;
	HANDLE ProcessHandle;
	PEB PebInfo;
	TARGET_PROCESS_INFO TargetProcInfo = {0x00};

	if(pOutputBuffer)
		memset(pOutputBuffer, 0, 0x21c);

	ntStatus = ObOpenObjectByPointer(pEprocess, 512, 0, 0x2000000, *PsProcessType, 0, &ProcessHandle);
	if (NT_SUCCESS(ntStatus))
	{
		ntStatus = ZwQueryInformationProcess(ProcessHandle, ProcessBasicInformation, &TargetProcInfo.m_ProcBasicInfo, sizeof(PROCESS_BASIC_INFORMATION), NULL);

		ZwClose(ProcessHandle);
		if (NT_SUCCESS(ntStatus))
		{
			KeStackAttachProcess(pEprocess, &kApc);
			__try
			{
				ProbeForRead(TargetProcInfo.m_ProcBasicInfo.PebBaseAddress, sizeof(PEB), 8);
				memcpy(&PebInfo,TargetProcInfo.m_ProcBasicInfo.PebBaseAddress,sizeof(PebInfo));
				ProbeForRead(TargetProcInfo.m_ProcBasicInfo.PebBaseAddress->Ldr, 0x30, 4);
				memcpy(&TargetProcInfo,TargetProcInfo.m_ProcBasicInfo.PebBaseAddress->Ldr,0x30);
				ntStatus = SearchMemoryOrderModuleList(&TargetProcInfo.m_LdrData,&TargetProcInfo.m_LdrData.InMemoryOrderModuleList,(PULONG)pThreadAddress,(PULONG)pOutputBuffer);
				if (!NT_SUCCESS(ntStatus))
				{
					*((PULONG)pOutputBuffer + 131) = 0;
					*((PULONG)pOutputBuffer + 132) = 0;
				}
			}
			__finally
			{

			}

			KeUnstackDetachProcess(&kApc);
		}

	}

	return ntStatus;
}

VOID CreateThreadCallback(
	HANDLE ProcessId,
	HANDLE ThreadId,
	BOOLEAN Create
)
{
	LIST_ENTRY *pEntry = NULL;
	BOOL bIsCreateProc = FALSE;
	ANTIYMFILTER_PROCLIST *pList = NULL;
	ULONG dwCurrentProcessId;
	ULONG dwCurrentThreadId;
	NTSTATUS ntStatus;
	PEPROCESS pEprocess;
	PETHREAD pEthread = NULL;
	HANDLE hThread;
	ULONG dwThreadStartAddress = 0;
	unsigned char Buffer[0x21c] = {0};

	if (Create)
	{
		ExAcquireFastMutex(&g_ListHeaderMutex);
		pEntry = g_ListHeader.Flink;
		while (pEntry != &g_ListHeader)
		{
			pList = CONTAINING_RECORD(pEntry, ANTIYMFILTER_PROCLIST, List);
			if (pList)
			{
				if (pList->ProcessId == ProcessId)
				{
					bIsCreateProc = TRUE;
					RemoveEntryList(pEntry);
					ExFreePoolWithTag(pList, 0);
					break;
				}
			}
			pEntry = pEntry->Flink;
		}
		
		ExReleaseFastMutex(&g_ListHeaderMutex);
		if (bIsCreateProc)
		{
			GetProcessInfoDataByPid(ProcessId, 0, 1, 0, 0);
			return;
		}
		
		dwCurrentProcessId = PsGetCurrentProcessId();
		if (dwCurrentProcessId != ProcessId && ProcessId != (HANDLE)4)
		{
			dwCurrentThreadId = KeGetCurrentThread();
			if (!(unsigned __int8)PsIsSystemThread(dwCurrentThreadId))
			{
				ntStatus = PsLookupThreadByThreadId(ThreadId, &pEthread);
				if (NT_SUCCESS(ntStatus))
				{
					ntStatus = ObOpenObjectByPointer(pEthread , OBJ_KERNEL_HANDLE, NULL, MAXIMUM_ALLOWED, *PsThreadType, 0, &hThread);
					if (NT_SUCCESS(ntStatus))
					{
						ZwQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &dwThreadStartAddress, sizeof(ULONG), NULL);
						
						ZwClose(hThread);
					}
					else
					{
						DbgPrint("ObOpenObjectByPointer fail:0x%x!\n", ntStatus);
					}
					ObfDereferenceObject(pEthread);
				}
				if (dwThreadStartAddress && NT_SUCCESS(PsLookupProcessByProcessId(ProcessId, &pEprocess)))
				{
					if (NT_SUCCESS(GetTargetProcessInfo(pEprocess, dwThreadStartAddress, Buffer)))
					{

					}
				}
				

			}
		}
		
	}
}

NTSTATUS __fastcall QueryProcessFileInfo(PUNICODE_STRING Source, ULONG Config_flag1, ULONG *Dst)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	WCHAR Sourcea[2] = {0x00};
	UNICODE_STRING Destination = {0x00};
	PUNICODE_STRING uObjectName = NULL;
	OBJECT_ATTRIBUTES ObjectAttributes = {0x00};
	HANDLE FileHandle = NULL;
	HANDLE SectionHandle = NULL;
	IO_STATUS_BLOCK IoStatusBlock = {0};
	FILE_ALL_INFORMATION *pFileAllInformation = NULL;
	PFILE_OBJECT FileObject = NULL;
	PVOID pNotAlphaSectionPoint = NULL;
	PVOID pSectionBaseAddress = NULL;
	__int64 dwSectionSize = 0;
	UINT_PTR ViewSize = 0;
	ULONG dwFileInFormationNameLength = 0;

	Destination.Length = 0;
	Destination.MaximumLength = 0;
	Destination.Buffer = NULL;
	uObjectName = Source;
	*(DWORD *)Sourcea = '?\0\\';
	
	do 
	{
		if (Source->Length > 4 && Source->Length < 65527 && Source->Buffer[1] == ':')
		{
			Destination.Length = 0;
			Destination.MaximumLength = Source->Length + 8;
			Destination.Buffer = ExAllocatePoolWithTag(NonPagedPool, Destination.MaximumLength, 'DSYS');
			if (!Destination.Buffer)
			{
				ntStatus = STATUS_NO_MEMORY;
				break;
			}

			ntStatus = RtlAppendUnicodeToString(&Destination, Sourcea);
			if (!NT_SUCCESS(ntStatus))
				break;
			ntStatus = RtlAppendUnicodeStringToString(&Destination, Source);
			if (!NT_SUCCESS(ntStatus))
				break;
			uObjectName = &Destination;
		}

		InitializeObjectAttributes(&ObjectAttributes, uObjectName, 0x240, 0, 0);

		ntStatus = ZwCreateFile(&FileHandle
			, FILE_READ_ATTRIBUTES | FILE_READ_DATA
			, &ObjectAttributes
			, &IoStatusBlock
			, 0
			, FILE_READ_ATTRIBUTES
			, FILE_OVERWRITE_IF
			, FILE_DIRECTORY_FILE
			, FILE_NON_DIRECTORY_FILE | FILE_SEQUENTIAL_ONLY
			, 0
			, 0);

		if (!NT_SUCCESS(ntStatus))
			break;
		pFileAllInformation = ExAllocatePoolWithTag(NonPagedPool, 0x270, 'AsyS');
		if (!pFileAllInformation)
		{
			ntStatus = STATUS_NO_MEMORY;
			break;
		}
		ntStatus = ZwQueryInformationFile(FileHandle, &IoStatusBlock, pFileAllInformation, 0x270, FileAllInformation);
		if (ntStatus == STATUS_BUFFER_OVERFLOW)
		{
			dwFileInFormationNameLength = pFileAllInformation->NameInformation.FileNameLength + 104;
			ExFreePoolWithTag(pFileAllInformation, 0);
			pFileAllInformation = NULL;

			if (dwFileInFormationNameLength < 0x68)
			{
				ntStatus = STATUS_INTEGER_OVERFLOW;
				break;
			}

			pFileAllInformation = ExAllocatePoolWithTag(NonPagedPool, dwFileInFormationNameLength, 'AsyS');
			if (!pFileAllInformation)
			{
				ntStatus = STATUS_NO_MEMORY;
				break;
			}

			ntStatus = ZwQueryInformationFile(FileHandle, &IoStatusBlock, pFileAllInformation, dwFileInFormationNameLength, FileAllInformation);
		}
		
		if (NT_SUCCESS(ntStatus))
		{
			if (pFileAllInformation->NameInformation.FileNameLength > 65535)
			{
				ntStatus = STATUS_BUFFER_OVERFLOW;
				break;
			}
		}
		
		ntStatus = ObReferenceObjectByHandle(FileHandle, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_NORMAL, IoFileObjectType, KernelMode, &FileObject, NULL);
		if (NT_SUCCESS(ntStatus))
		{
			InitializeObjectAttributes(&ObjectAttributes, NULL, 0x200, 0, 0);
			ntStatus = FsRtlCreateSectionForDataScan(&SectionHandle, &pNotAlphaSectionPoint, &dwSectionSize, FileObject, SECTION_MAP_READ | SECTION_QUERY, &ObjectAttributes, 0, PAGE_READONLY, SEC_COMMIT, 0);
			if (NT_SUCCESS(ntStatus))
			{
				if (dwSectionSize <= 0xffffffff)
				{
					pSectionBaseAddress = NULL;
					ViewSize = 0;
					ntStatus = ZwMapViewOfSection(SectionHandle, (HANDLE)0xffffffff, pSectionBaseAddress, 0, 0, 0, &ViewSize, ViewUnmap, 0, 2);
					if (!NT_SUCCESS(ntStatus))
					{
						__try
						{

						}
						__except (EXCEPTION_EXECUTE_HANDLER)
						{

						}
					}
				}
			}
		}

	} while (0);


	
	if (Destination.Buffer)
		ExFreePoolWithTag(Destination.Buffer, 0);
	if (FileHandle)
		ZwClose(FileHandle);
	
	return ntStatus;
}

PVOID __fastcall AllocatePoolWithMagicTag(POOL_TYPE PoolType, SIZE_T NumberOfBytes, int Magic)
{
	return ExAllocatePoolWithTag(PoolType, NumberOfBytes, (Magic << 24) | 'syS');
}

BOOL CopyProcessParameters(PPEB PebAddress, RTL_USER_PROCESS_PARAMETERS *pProcessParameters, PUNICODE_STRING DescString, PUNICODE_STRING SourceString, ULONG Magic)
{
	BOOL bResult;
	PWSTR pwStr_v6;
	USHORT SourceStringLen;
	bResult = FALSE;
	DescString->Length = 0;
	DescString->Buffer = NULL;
	if (pProcessParameters->Flags & 1)
		pwStr_v6 = SourceString->Buffer;
	else
		pwStr_v6 = (PWSTR)((char *)SourceString->Buffer + (unsigned int)PebAddress->ProcessParameters);
	SourceStringLen = SourceString->Length;
	if (SourceStringLen >= 2)
	{
		if (SourceStringLen & 1)
			--SourceStringLen;
		ProbeForRead(pwStr_v6, SourceStringLen, 1);
		DescString->Buffer = AllocatePoolWithMagicTag(NonPagedPool, SourceStringLen + 2, Magic);
		if (DescString->Buffer)
		{
			memmove(DescString->Buffer, pwStr_v6, SourceStringLen);
			DescString->Buffer[SourceStringLen >> 1] = 0;
			DescString->Length = SourceStringLen + 2;
			bResult = TRUE;
		}
	}

	return bResult;

}

WCHAR *__fastcall GetUnicodeStringByAnsiString(HANDLE Handle, PUNICODE_STRING DestinationString)
{
	WCHAR *pResult = NULL;

	return pResult;
}

BOOL GetProcessParamtersByPeb(HANDLE Handle, PPEB *PebAddress, PUNICODE_STRING DestinationString, PUNICODE_STRING szCommlineStr, PUNICODE_STRING szCurrentDirtoryStr)
{
	BOOL bResult = FALSE;
	PROCESSPARAMDATA pProcessParamterDatas;
	ULONG dwProcessParamtersLength;
	PEB peb = {0x00};
	PEPROCESS Object = NULL;
	KAPC_STATE ApcState;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	
	if (szCommlineStr)
	{
		szCommlineStr->Buffer = NULL;
		szCommlineStr->Length = 0;
	}
	if (szCurrentDirtoryStr)
	{
		szCurrentDirtoryStr->Buffer = NULL;
		szCurrentDirtoryStr->Length = 0;
	}
	dwProcessParamtersLength = sizeof(PROCESSPARAMDATA);
	
	if (Handle && PebAddress)
	{
		ntStatus = ObReferenceObjectByHandle(Handle, 0, 0, 0, &Object, 0);
		if (NT_SUCCESS(ntStatus))
		{
			KeStackAttachProcess(Object, &ApcState);
			__try
			{
				ProbeForRead(PebAddress, 0x250, 4);
				memcpy(&peb, PebAddress, sizeof(PEB));
				if (peb.ProcessParameters)
				{
					ProbeForRead(peb.ProcessParameters, 0x2a8, 4);
					memcpy(&pProcessParamterDatas, peb.ProcessParameters, dwProcessParamtersLength);
					
					bResult = CopyProcessParameters(&peb, &pProcessParamterDatas.ProcessParameters, DestinationString, &pProcessParamterDatas.ProcessParameters
						.ImagePathName, 53);
					
					
					if (szCommlineStr)
						CopyProcessParameters(&peb, &pProcessParamterDatas.ProcessParameters, szCommlineStr, &pProcessParamterDatas.ProcessParameters.CommandLine, 54);
					if (szCurrentDirtoryStr)
						CopyProcessParameters(&peb, &pProcessParamterDatas, szCurrentDirtoryStr, &pProcessParamterDatas.ProcessParameters.CurrentDirectoryPath, 55);
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				KeUnstackDetachProcess(&ApcState);
				ObfDereferenceObject(Object);
			}

			KeUnstackDetachProcess(&ApcState);
			ObfDereferenceObject(Object);
		}
		else
		{
			bResult = FALSE;
		}
	}
	return bResult;
}

NTSTATUS __fastcall GetDosNameByImageName(PUNICODE_STRING ObjectName, POBJECT_NAME_INFORMATION *Output)
{
	NTSTATUS ntStatus;
	PDEVICE_OBJECT pDeviceObject;
	PFILE_OBJECT pFileObject;
	BOOL b2;
	BOOL b3;
	PWSTR Magic;

	b2 = ObjectName->Length < 4;
	b3 = ObjectName->Length == 4;
	pDeviceObject = NULL;
	pFileObject = NULL;

	if (!b2 && !b3)
	{
		Magic = ObjectName->Buffer;
		if (*Magic == '\\' && Magic[1] == '\\')
		{
			return STATUS_INVALID_PARAMETER;
		}
		if (Magic[1] == ':')
		{
			return STATUS_INVALID_PARAMETER;
		}
	}
	ntStatus = IoGetDeviceObjectPointer(ObjectName, 0x80, &pFileObject, &pDeviceObject);
	if (NT_SUCCESS(ntStatus))
		ntStatus = IoQueryFileDosDeviceName(pFileObject,Output);
	if (pFileObject)
		ObDereferenceObject(pFileObject);
	
	return ntStatus;
}

