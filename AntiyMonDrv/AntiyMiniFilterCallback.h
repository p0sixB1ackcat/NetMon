#pragma once
#include <FltKernel.h>

NTSTATUS
AntiyMFilterQueryTeardown(
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
AntiyMFilterPreCreate(
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__deref_out_opt PVOID *CompletionContext
);

FLT_POSTOP_CALLBACK_STATUS
AntiyMFilterPostCreate(
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in_opt PVOID CompletionContext,
	__in FLT_POST_OPERATION_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
AntiyMFilterPreCleanup(
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__deref_out_opt PVOID *CompletionContext
);

FLT_PREOP_CALLBACK_STATUS
AntiyMFilterPreWrite(
	__inout PFLT_CALLBACK_DATA Data,
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__deref_out_opt PVOID *CompletionContext
);

NTSTATUS
AntiyMFilterInstanceSetup(
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in FLT_INSTANCE_SETUP_FLAGS Flags,
	__in DEVICE_TYPE VolumeDeviceType,
	__in FLT_FILESYSTEM_TYPE VolumeFilesystemType
);


NTSTATUS
AntiyMFilterPortConnect(
	__in PFLT_PORT ClientPort,
	__in_opt PVOID ServerPortCookie,
	__in_bcount_opt(SizeOfContext) PVOID ConnectionContext,
	__in ULONG SizeOfContext,
	__deref_out_opt PVOID *ConnectionCookie
);

VOID
AntiyMFilterPortDisconnect(
	__in_opt PVOID ConnectionCookie
);

NTSTATUS AntiyMFilterNotifyMessage(PVOID PortCookie
	, PVOID InputBuffer
	, ULONG InputBufferLength
	, PVOID OutputBuffer
	, ULONG OutputBufferLength
	, ULONG *ReturnOutputLength);

NTSTATUS
AntiyMFilterpScanFileInUserMode(
	__in PFLT_INSTANCE Instance,
	__in PFILE_OBJECT FileObject,
	__out PBOOLEAN SafeToOpen
);

BOOLEAN
AntiyMFilterpCheckExtension(
	__in PUNICODE_STRING Extension
);

NTSTATUS ReportDataToR3(ULONG ReportType,PVOID pData);