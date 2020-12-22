#include "AntiyMiniFilterCallback.h"
#include "AntiyMonUK.h"
#include "AntiyMonDrv.h"

extern ATKMFDATA g_KMfData;
//lock
extern ERESOURCE g_RuleLock;
//保存R3下发的规则
extern LIST_ENTRY g_RuleEntry;

extern FAST_MUTEX g_ProcessRelevanceListLock;
extern LIST_ENTRY g_ProcessRelevanceListHeader;

const UNICODE_STRING ScannerExtensionsToScan[] =
{ RTL_CONSTANT_STRING(L"doc"),
RTL_CONSTANT_STRING(L"txt"),
RTL_CONSTANT_STRING(L"bat"),
RTL_CONSTANT_STRING(L"cmd"),
RTL_CONSTANT_STRING(L"inf"),
//RTL_CONSTANT_STRING( L"ini"),   Removed, to much usage
{ 0, 0, NULL }
};



VOID LockRule(PERESOURCE lock)
{
	KeEnterCriticalRegion();
	
	ExAcquireResourceExclusive(lock, TRUE);
}

VOID UnlockRule(PERESOURCE lock)
{
	ExReleaseResource(lock);

	KeLeaveCriticalRegion();
}


NTSTATUS AntiyMFilterNotifyMessage(PVOID PortCookie
	, PVOID InputBuffer
	, ULONG InputBufferLength
	, PVOID OutputBuffer
	, ULONG OutputBufferLength
	, ULONG *ReturnOutputLength)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UNICODE_STRING uR3Data = { 0x00 };

	//ntStatus = AddRuleToListEntry(InputBuffer, InputBufferLength);

	//ConvertRuleListEntry(&g_RuleEntry);

	DbgPrint("Recive Data With Ring3:%ws\n",InputBuffer);

	return ntStatus;
}

NTSTATUS
AntiyMFilterPortConnect(
	__in PFLT_PORT ClientPort,
	__in_opt PVOID ServerPortCookie,
	__in_bcount_opt(SizeOfContext) PVOID ConnectionContext,
	__in ULONG SizeOfContext,
	__deref_out_opt PVOID *ConnectionCookie
)
{
	PAGED_CODE();

	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionContext);
	UNREFERENCED_PARAMETER(SizeOfContext);
	UNREFERENCED_PARAMETER(ConnectionCookie);

	ASSERT(g_KMfData.ClientPort == NULL);
	ASSERT(g_KMfData.UserProcess == NULL);

	//
	//  Set the user process and port.
	//

	g_KMfData.UserProcess = PsGetCurrentProcess();
	g_KMfData.ClientPort = ClientPort;

	DbgPrint("!!! AntiyMonDrv.sys --- connected, port=0x%p\n", ClientPort);

	return STATUS_SUCCESS;
}

VOID
AntiyMFilterPortDisconnect(
	__in_opt PVOID ConnectionCookie
)
{
	UNREFERENCED_PARAMETER(ConnectionCookie);

	PAGED_CODE();

	DbgPrint("!!! AntiyMonDrv.sys --- disconnected, port=0x%p\n", g_KMfData.ClientPort);

	FltCloseClientPort(g_KMfData.Filter, &g_KMfData.ClientPort);

	g_KMfData.UserProcess = NULL;

}

NTSTATUS
AntiyMFilterInstanceSetup(
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in FLT_INSTANCE_SETUP_FLAGS Flags,
	__in DEVICE_TYPE VolumeDeviceType,
	__in FLT_FILESYSTEM_TYPE VolumeFilesystemType
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(VolumeFilesystemType);

	PAGED_CODE();

	ASSERT(FltObjects->Filter == g_KMfData.Filter);

	//
	//  Don't attach to network volumes.
	//

	if (VolumeDeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM) {

		return STATUS_FLT_DO_NOT_ATTACH;
	}

	return STATUS_SUCCESS;
}

NTSTATUS
AntiyMFilterQueryTeardown(
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	return STATUS_SUCCESS;
}


FLT_PREOP_CALLBACK_STATUS
AntiyMFilterPreCreate(
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__deref_out_opt PVOID *CompletionContext
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	PAGED_CODE();

	//
	//  See if this create is being done by our user process.
	//

	if (IoThreadToProcess(Data->Thread) == g_KMfData.UserProcess) {

		DbgPrint("!!! AntiyMonDrv.sys -- allowing create for trusted process \n");

		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

BOOLEAN
AntiyMFilterpCheckExtension(
	__in PUNICODE_STRING Extension
)
{
	const UNICODE_STRING *ext;

	if (Extension->Length == 0) {

		return FALSE;
	}

	//
	//  Check if it matches any one of our static extension list
	//

	ext = ScannerExtensionsToScan;

	while (ext->Buffer != NULL) {

		if (RtlCompareUnicodeString(Extension, ext, TRUE) == 0) {

			//
			//  A match. We are interested in this file
			//

			return TRUE;
		}
		ext++;
	}

	return FALSE;
}


FLT_POSTOP_CALLBACK_STATUS
AntiyMFilterPostCreate(
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in_opt PVOID CompletionContext,
	__in FLT_POST_OPERATION_FLAGS Flags
)
{
	return FLT_POSTOP_FINISHED_PROCESSING;
}


FLT_PREOP_CALLBACK_STATUS
AntiyMFilterPreCleanup(
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__deref_out_opt PVOID *CompletionContext
)
{
	NTSTATUS status;
	PAntiyMFilter_STREAM_HANDLE_CONTEXT context;
	BOOLEAN safe;

	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(CompletionContext);

	status = FltGetStreamHandleContext(FltObjects->Instance,
		FltObjects->FileObject,
		&context);

	if (NT_SUCCESS(status)) {

		if (context->RescanRequired) {//写关闭

			(VOID)AntiyMFilterpScanFileInUserMode(FltObjects->Instance,
				FltObjects->FileObject,
				&safe);

			if (!safe) {

				DbgPrint("!!! AntiyMonDrv.sys -- foul language detected in precleanup !!!\n");
			}
		}

		FltReleaseContext(context);
	}


	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS
AntiyMFilterPreWrite(
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__deref_out_opt PVOID *CompletionContext
)
{
	FLT_PREOP_CALLBACK_STATUS returnStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
	NTSTATUS status;
	PANTIYMFILTER_REPORT_R3DATA notification = NULL;
	PAntiyMFilter_STREAM_HANDLE_CONTEXT context = NULL;
	ULONG replyLength;
	BOOLEAN safe = TRUE;
	PUCHAR buffer;

	UNREFERENCED_PARAMETER(CompletionContext);

	//
	//  If not client port just ignore this write.
	//

	if (g_KMfData.ClientPort == NULL) {

		return FLT_PREOP_SUCCESS_NO_CALLBACK;
	}

	status = FltGetStreamHandleContext(FltObjects->Instance,
		FltObjects->FileObject,
		&context);

	if (!NT_SUCCESS(status))
	{

		//
		//  We are not interested in this file
		//

		return FLT_PREOP_SUCCESS_NO_CALLBACK;

	}

	//
	//  Use try-finally to cleanup
	//

	__try
	{
		if (Data->Iopb->Parameters.Write.Length != 0)
		{
			if (Data->Iopb->Parameters.Write.MdlAddress != NULL) {

				buffer = MmGetSystemAddressForMdlSafe(Data->Iopb->Parameters.Write.MdlAddress,
					NormalPagePriority);

				if (buffer == NULL) {

					Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
					Data->IoStatus.Information = 0;
					returnStatus = FLT_PREOP_COMPLETE;
					__leave;
				}

			}
			else 
			{
				buffer = Data->Iopb->Parameters.Write.WriteBuffer;
			}

			notification = ExAllocatePoolWithTag(NonPagedPool,
				sizeof(ANTIYMFILTER_PROCESSINFO),
				'nacS');
			if (notification == NULL) 
			{

				Data->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
				Data->IoStatus.Information = 0;
				returnStatus = FLT_PREOP_COMPLETE;
				__leave;
			}

			

			__try
			{

				

			}
			__except (EXCEPTION_EXECUTE_HANDLER) {

				//
				//  Error accessing buffer. Complete i/o with failure
				//

				Data->IoStatus.Status = GetExceptionCode();
				Data->IoStatus.Information = 0;
				returnStatus = FLT_PREOP_COMPLETE;
				__leave;
			}
			
			replyLength = sizeof(AntiyMFilter_REPLY);

			status = FltSendMessage(g_KMfData.Filter,
				&g_KMfData.ClientPort,
				notification,
				sizeof(ANTIYMFILTER_PROCESSINFO),
				notification,
				&replyLength,
				NULL);

			if (STATUS_SUCCESS == status) 
			{

				safe = ((PAntiyMFilter_REPLY)notification)->SafeToOpen;

			}
			else 
			{

				//
				//  Couldn't send message. This sample will let the i/o through.
				//

				DbgPrint("!!! AntiyMonDrv.sys --- couldn't send message to user-mode to scan file, status 0x%X\n", status);
			}
		}

		if (!safe) {

			DbgPrint("!!! AntiyMonDrv.sys -- foul language detected in write !!!\n");

			if (!FlagOn(Data->Iopb->IrpFlags, IRP_PAGING_IO)) {

				DbgPrint("!!! AntiyMonDrv.sys -- blocking the write !!!\n");

				Data->IoStatus.Status = STATUS_ACCESS_DENIED;
				Data->IoStatus.Information = 0;
				returnStatus = FLT_PREOP_COMPLETE;
			}
		}

	}
	__finally {

		if (notification != NULL) {

			ExFreePoolWithTag(notification, 'nacS');
		}

		if (context) {

			FltReleaseContext(context);
		}
	}

	return returnStatus;
}

NTSTATUS
AntiyMFilterpScanFileInUserMode(
	__in PFLT_INSTANCE Instance,
	__in PFILE_OBJECT FileObject,
	__out PBOOLEAN SafeToOpen
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PVOID buffer = NULL;
	ULONG bytesRead;
	PANTIYMFILTER_PROCESSINFO notification = NULL;
	FLT_VOLUME_PROPERTIES volumeProps;
	LARGE_INTEGER offset;
	ULONG replyLength, length;
	PFLT_VOLUME volume = NULL;

	*SafeToOpen = TRUE;

	//
	//  If not client port just return.
	//

	if (g_KMfData.ClientPort == NULL) {

		return STATUS_SUCCESS;
	}

	__try {

		//
		//  Obtain the volume object .
		//

		status = FltGetVolumeFromInstance(Instance, &volume);

		if (!NT_SUCCESS(status)) {

			__leave;
		}

		//
		//  Determine sector size. Noncached I/O can only be done at sector size offsets, and in lengths which are
		//  multiples of sector size. A more efficient way is to make this call once and remember the sector size in the
		//  instance setup routine and setup an instance context where we can cache it.
		//

		status = FltGetVolumeProperties(volume,
			&volumeProps,
			sizeof(volumeProps),
			&length);
		//
		//  STATUS_BUFFER_OVERFLOW can be returned - however we only need the properties, not the names
		//  hence we only check for error status.
		//

		if (NT_ERROR(status)) {

			__leave;
		}

		length = max(AntiyMFilter_READ_BUFFER_SIZE, volumeProps.SectorSize);

		//
		//  Use non-buffered i/o, so allocate aligned pool
		//

		buffer = FltAllocatePoolAlignedWithTag(Instance,
			NonPagedPool,
			length,
			'nacS');

		if (NULL == buffer) {

			status = STATUS_INSUFFICIENT_RESOURCES;
			__leave;
		}

		notification = ExAllocatePoolWithTag(NonPagedPool,
			sizeof(ANTIYMFILTER_PROCESSINFO),
			'nacS');

		if (NULL == notification) {

			status = STATUS_INSUFFICIENT_RESOURCES;
			__leave;
		}

		//
		//  Read the beginning of the file and pass the contents to user mode.
		//

		offset.QuadPart = bytesRead = 0;
		status = FltReadFile(Instance,
			FileObject,
			&offset,
			length,
			buffer,
			FLTFL_IO_OPERATION_NON_CACHED |
			FLTFL_IO_OPERATION_DO_NOT_UPDATE_BYTE_OFFSET,
			&bytesRead,
			NULL,
			NULL);

		if (NT_SUCCESS(status) && (0 != bytesRead)) 
		{

			replyLength = sizeof(AntiyMFilter_REPLY);

			status = FltSendMessage(g_KMfData.Filter,
				&g_KMfData.ClientPort,
				notification,//request
				sizeof(ANTIYMFILTER_PROCESSINFO),
				notification,//reply
				&replyLength,
				NULL);

			if (STATUS_SUCCESS == status) 
			{
				*SafeToOpen = ((PAntiyMFilter_REPLY)notification)->SafeToOpen;
			}
			else {

				//
				//  Couldn't send message
				//

				DbgPrint("!!! AntiyMonDrv.sys --- couldn't send message to user-mode to scan file, status 0x%X\n", status);
			}
		}

	}
	__finally {

		if (NULL != buffer) {

			FltFreePoolAlignedWithTag(Instance, buffer, 'nacS');
		}

		if (NULL != notification) {

			ExFreePoolWithTag(notification, 'nacS');
		}

		if (NULL != volume) {

			FltObjectDereference(volume);
		}
	}

	return status;
}

NTSTATUS ReportCreateProcessEvent(VOID)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	ANTIYMFILTER_PROCESSINFO *pNotification = NULL;
	ANTIYMFILTER_PROCESSINFO *pParentProcessInfo = NULL;
	LIST_ENTRY *pList = NULL;
	LIST_ENTRY *pParentList = NULL;
	LIST_ENTRY *pTmpList = NULL;
	ULONG dwReplyLen = 0;
	ANTIYMFILTER_REPORT_R3DATA *pReport = NULL;
	ANTIYMFILTER_PROCESSINFO *pProcInfo = NULL;

	ExAcquireFastMutex(&g_ProcessRelevanceListLock);
	pList = g_ProcessRelevanceListHeader.Flink;

	while (pList != &g_ProcessRelevanceListHeader)
	{
		pNotification = CONTAINING_RECORD(pList, ANTIYMFILTER_PROCESSINFO, List);
		if (!pNotification || !MmIsAddressValid(pNotification))
			break;

		pParentList = pList->Flink;
		while (pParentList != &g_ProcessRelevanceListHeader)
		{
			pParentProcessInfo = CONTAINING_RECORD(pParentList, ANTIYMFILTER_PROCESSINFO, List);
			if (!pParentProcessInfo || !MmIsAddressValid(pParentProcessInfo))
			{
				ntStatus = STATUS_NO_MEMORY;
				break;
			}

			if (pParentProcessInfo->ProcessId == pNotification->ParentId)
			{
				__try
				{
					RtlCopyMemory(pNotification->ParentCommandLine, pParentProcessInfo->CommandLine, sizeof(pNotification->ParentCommandLine));
					RtlCopyMemory(pNotification->ParentCurrentDirtyory, pParentProcessInfo->CurrentDirtory, sizeof(pNotification->ParentCurrentDirtyory));
					RtlCopyMemory(pNotification->ParentImagePath, pParentProcessInfo->ImagePath, sizeof(pNotification->ParentImagePath));
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{
					DbgPrint("Copy ParentProcess Info fail!\n");
				}

			}

			pParentList = pParentList->Flink;
			dwReplyLen = sizeof(ANTIYMFILTER_REPORT_R3DATA);
			pReport = (PANTIYMFILTER_REPORT_R3DATA)ExAllocatePoolWithTag(PagedPool, dwReplyLen, '7syS');
			memset(pReport,0,dwReplyLen);
			if (!pReport)
			{
				ntStatus = STATUS_MEMORY_NOT_ALLOCATED;
				break;
			}
			pReport->Type = ATMFilter_CreateProcess;
			pProcInfo = &pReport->ReportData.ProcInfo;
			__try
			{
				RtlCopyMemory(pProcInfo, pNotification, sizeof(ANTIYMFILTER_PROCESSINFO));
				pReport->dwReportDataLength = dwReplyLen;
				ntStatus = FltSendMessage(g_KMfData.Filter, &g_KMfData.ClientPort, pReport, sizeof(ANTIYMFILTER_REPORT_R3DATA), pReport, &dwReplyLen, NULL);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				DbgPrint("Copy Process Info fail!\n");
			}
			
			ExFreePoolWithTag(pReport, 0);
		}


		pTmpList = pList;
		pList = pList->Flink;
		RemoveEntryList(pTmpList);
		ExFreePoolWithTag(pNotification, 0);
		pNotification = NULL;
	}

	ExReleaseFastMutex(&g_ProcessRelevanceListLock);

	return ntStatus;
}

NTSTATUS ReportTerminateProcessEvent(PVOID pData)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	ULONG dwRetLen = 0;
	ANTIYMFILTER_REPORT_R3DATA *pReport = (ANTIYMFILTER_REPORT_R3DATA *)pData;
	do 
	{
		if (!pReport || !MmIsAddressValid(pReport))
		{
			ntStatus = STATUS_INVALID_PARAMETER;
			break;
		}

		dwRetLen = pReport->dwReportDataLength;
		ntStatus = FltSendMessage(g_KMfData.Filter, &g_KMfData.ClientPort, pReport, pReport->dwReportDataLength, pReport, &dwRetLen, NULL);

	} while (0);
	
	
	return ntStatus;
}

NTSTATUS ReportDataToR3(ULONG ReportType,PVOID pData)
{
	ANTIYMFILTER_EVENT_TYPE Type = ReportType;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	switch (Type)
	{
		case ATMFilter_CreateProcess:
		{
			ntStatus = ReportCreateProcessEvent();
		}
		break;
		case ATMFilter_CloseProcess:
		{
			ntStatus = ReportTerminateProcessEvent(pData);
		}
		break;
		case ATMFilter_CreateRemoteThread:
		{

		}
		break;
	}

	return ntStatus;
}