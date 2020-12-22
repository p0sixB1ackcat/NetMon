/*++

Module Name:

    AntiyMonDrv.c

Abstract:

    This is the main module of the AntiyMonDrv miniFilter driver.

Environment:

    Kernel mode

--*/

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include "AntiyMonUK.h"
#include "AntiyMonDrv.h"
#include "AntiyMiniFilter.h"
#include "globalvar.h"
#include "AntiyProc.h"
#include "AntiyCancelSafeIrpQueue.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

//�豸��Ϣ
ATKMFDATA g_KMfData;

ULONG g_BuildNumber = 0;
ZWOPENPROCESSTOKENEX g_ZwOpenProcessTokenEx = NULL;
PSGETPROCESSPEB g_PsGetProcessPeb = NULL;
PSISPROTECTEDPROCESS g_PsIsProtectedProcess = NULL;

FAST_MUTEX g_ListHeaderMutex;
LIST_ENTRY g_ListHeader;//��������ʱ��������id���浽��������

FAST_MUTEX g_ProcessRelevanceListLock;
LIST_ENTRY g_ProcessRelevanceListHeader;//�������̹���������ǰ���̺͸�����

FAST_MUTEX g_TerminateProcessListLock;
LIST_ENTRY g_TerminateProcessListHeader;//��������ʱͬ�����뵽�ñ��У���������ʱ���ҵ��ñ��еı�����ͷ�

#define DeviceName L"\\Device\\AntiyMonDrv"
#define SymbolicLinkName L"\\DosDevices\\AntiyMonDrv"


VOID Initialize(VOID)
{
	UNICODE_STRING uFuncName = { 0x00 };

	if (g_BuildNumber >= 0xece)
	{
		RtlInitUnicodeString(&uFuncName, L"ZwOpenProcessTokenEx");
		g_ZwOpenProcessTokenEx = (ZWOPENPROCESSTOKENEX)MmGetSystemRoutineAddress(&uFuncName);
	}

	ExInitializeFastMutex(&g_ListHeaderMutex);
	InitializeListHead(&g_ListHeader);

	ExInitializeFastMutex(&g_ProcessRelevanceListLock);
	InitializeListHead(&g_ProcessRelevanceListHeader);

	ExInitializeFastMutex(&g_TerminateProcessListLock);
	InitializeListHead(&g_TerminateProcessListHeader);

}

VOID CreateCallBack(VOID)
{
	PsSetCreateProcessNotifyRoutine(CreateProcessCallback, FALSE);
	PsSetCreateThreadNotifyRoutine(CreateThreadCallback);
}

VOID GetNotExportFunctions(VOID)
{
	UNICODE_STRING uFuncsName = { 0x00 };
	RtlInitUnicodeString(&uFuncsName, PsGetProcessPebName);
	g_PsGetProcessPeb = (PSGETPROCESSPEB)MmGetSystemRoutineAddress(&uFuncsName);

	RtlInitUnicodeString(&uFuncsName, PsIsProtectedProcessName);
	g_PsIsProtectedProcess = (PSISPROTECTEDPROCESS)MmGetSystemRoutineAddress(&uFuncsName);
}

VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING uSymbolicLinkName = { 0x00 };
	RtlInitUnicodeString(&uSymbolicLinkName, SymbolicLinkName);
	IoDeleteSymbolicLink(&uSymbolicLinkName);
	IoDeleteDevice(pDriverObject->DeviceObject);
	PsSetCreateProcessNotifyRoutine(CreateProcessCallback, TRUE);
	PsRemoveCreateThreadNotifyRoutine(CreateThreadCallback);
	DbgPrint
	("Unload ProcInfo!\n");
}

NTSTATUS DispatchCommon(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	IO_STACK_LOCATION *pIrpStack = NULL;
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = ntStatus;
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return ntStatus;
}

NTSTATUS
DriverEntry(
	 PDRIVER_OBJECT pDriverObject,
	 PUNICODE_STRING pRegPath
)
{
	
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UNICODE_STRING uDeviceName = { 0x00 };
	UNICODE_STRING uSymbolicLinkName = { 0x00 };
	PDEVICE_OBJECT pDeviceObject = NULL;
	ULONG i;
	PDEVICE_EXTENSION pDex = NULL;

	UNREFERENCED_PARAMETER(pRegPath);

	RtlInitUnicodeString(&uDeviceName, DeviceName);
	ntStatus = IoCreateDevice(pDriverObject
		, sizeof(DEVICE_EXTENSION)
		, &uDeviceName
		, 0
		, 0
		, FALSE
		, &pDeviceObject
	);
	if (!NT_SUCCESS(ntStatus))
	{
		DbgPrint("IoCreateDevice fail:%d!\n", ntStatus);
		return ntStatus;
	}
	
	RtlInitUnicodeString(&uSymbolicLinkName, SymbolicLinkName);
	ntStatus = IoCreateSymbolicLink(&uSymbolicLinkName, &uDeviceName);
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; ++i)
	{
		pDriverObject->MajorFunction[i] = DispatchCommon;
	}
	
	pDriverObject->DriverUnload = DriverUnload;

	pDex = pDeviceObject->DeviceExtension;

	IoCsqInitialize(&pDex->CancelSafeQueue, CsqInsertIrp, CsqRemoveIrp, CsqPeekNextIrp, CsqAcquireLock, CsqReleaseLock, CsqCompleteCancelIrp);
	KeInitializeEvent(&pDex->KernelEvent, SynchronizationEvent, FALSE);
	pDeviceObject->Flags |= DO_BUFFERED_IO;
	GetNotExportFunctions();

	PsGetVersion(NULL, NULL, &g_BuildNumber, NULL);

	Initialize();

	InitMiniFilter(pDriverObject, pRegPath);

	CreateCallBack();

	DbgPrint("ProcInfo DriverEntry!\n");
	
	return ntStatus;
}






