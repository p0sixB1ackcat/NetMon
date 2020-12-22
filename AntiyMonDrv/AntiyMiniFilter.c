#include "AntiyMiniFilter.h"
#include "AntiyMiniFilterCallback.h"
#include "AntiyMonUK.h"
#include "AntiyMonDrv.h"

NTSTATUS AntiyMFilterUnload(__in FLT_FILTER_UNLOAD_FLAGS Flags);

const FLT_CONTEXT_REGISTRATION ContextRegistration[] = {

	{ FLT_STREAMHANDLE_CONTEXT,
	0,
	NULL,
	sizeof(AntiyMFilter_STREAM_HANDLE_CONTEXT),
	'chBS' },

	{ FLT_CONTEXT_END }
};

const FLT_OPERATION_REGISTRATION Callbacks[] = {

	{ IRP_MJ_CREATE,
	0,
	AntiyMFilterPreCreate,
	AntiyMFilterPostCreate },

	{ IRP_MJ_CLEANUP,
	0,
	AntiyMFilterPreCleanup,
	NULL },

	{ IRP_MJ_WRITE,
	0,
	AntiyMFilterPreWrite,
	NULL },

	{ IRP_MJ_OPERATION_END }
};

const FLT_REGISTRATION FilterRegistration = {

	sizeof(FLT_REGISTRATION),         //  Size
	FLT_REGISTRATION_VERSION,           //  Version
	0,                                  //  Flags
	ContextRegistration,                //  Context Registration.
	Callbacks,                          //  Operation callbacks
	AntiyMFilterUnload,                      //  FilterUnload
	AntiyMFilterInstanceSetup,               //  InstanceSetup
	AntiyMFilterQueryTeardown,               //  InstanceQueryTeardown
	NULL,                               //  InstanceTeardownStart
	NULL,                               //  InstanceTeardownComplete
	NULL,                               //  GenerateFileName
	NULL,                               //  GenerateDestinationFileName
	NULL                                //  NormalizeNameComponent
};

NTSTATUS
AntiyMFilterUnload(
	__in FLT_FILTER_UNLOAD_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(Flags);

	FltCloseCommunicationPort(g_KMfData.ServerPort);

	FltUnregisterFilter(g_KMfData.Filter);

	return STATUS_SUCCESS;
}

NTSTATUS InitMiniFilter(PDRIVER_OBJECT pDriverObject,PUNICODE_STRING pRegPath)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	UNICODE_STRING uniString = {0x00};
	OBJECT_ATTRIBUTES oa;
	PSECURITY_DESCRIPTOR sd;

	ntStatus = FltRegisterFilter(pDriverObject,
		&FilterRegistration,
		&g_KMfData.Filter);

	if (!NT_SUCCESS(ntStatus))
	{
		return ntStatus;
	}

	RtlInitUnicodeString(&uniString,ATPortName);

	ntStatus = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);

	if (NT_SUCCESS(ntStatus))
	{
		InitializeObjectAttributes(&oa,
			&uniString,
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
			NULL,
			sd);

		ntStatus = FltCreateCommunicationPort(g_KMfData.Filter,
			&g_KMfData.ServerPort,
			&oa,
			NULL,
			AntiyMFilterPortConnect,
			AntiyMFilterPortDisconnect,
			AntiyMFilterNotifyMessage,
			1);

		FltFreeSecurityDescriptor(sd);

		if (NT_SUCCESS(ntStatus))
		{

			ntStatus = FltStartFiltering(g_KMfData.Filter);

			if (NT_SUCCESS(ntStatus)) {

				return STATUS_SUCCESS;
			}
			DbgPrint("FltStart fail:%d!\n", ntStatus);
			FltCloseCommunicationPort(g_KMfData.ServerPort);
		}
	}
	FltUnregisterFilter(g_KMfData.Filter);
	return ntStatus;
}
