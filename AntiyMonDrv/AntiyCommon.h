#pragma once
#include <ntifs.h>
#include <ntstrsafe.h>
#include <ntddk.h>
#include <WINDEF.H>
#include <process.h>

#define MAX_PATH 256
#define FLS_MAXIMUM_AVAILABLE 128
#define GDI_HANDLE_BUFFER_SIZE32 34
#define GDI_HANDLE_BUFFER_SIZE64 60

#define WIN_BUILDNUM_VISTA 6000

#ifndef WIN64
#define GDI_HANDLE_BUFFER_SIZE GDI_HANDLE_BUFFER_SIZE32
#else
#define GDI_HANDLE_BUFFER_SIZE GDI_HANDLE_BUFFER_SIZE64
#endif



typedef ULONG GDI_HANDLE_BUFFER[GDI_HANDLE_BUFFER_SIZE];

typedef struct _PEB_LDR_DATA {
	BYTE       Reserved1[8];
	PVOID      Reserved2[3];
	LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _RTL_DRIVE_LETTER_CURDIR {
	USHORT                  Flags;
	USHORT                  Length;
	ULONG                   TimeStamp;
	UNICODE_STRING          DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	ULONG                   MaximumLength;
	ULONG                   Length;
	ULONG                   Flags;
	ULONG                   DebugFlags;
	PVOID                   ConsoleHandle;
	ULONG                   ConsoleFlags;
	HANDLE                  StdInputHandle;
	HANDLE                  StdOutputHandle;
	HANDLE                  StdErrorHandle;
	UNICODE_STRING          CurrentDirectoryPath;
	HANDLE                  CurrentDirectoryHandle;
	UNICODE_STRING          DllPath;
	UNICODE_STRING          ImagePathName;
	UNICODE_STRING          CommandLine;
	PVOID                   Environment;
	ULONG                   StartingPositionLeft;
	ULONG                   StartingPositionTop;
	ULONG                   Width;
	ULONG                   Height;
	ULONG                   CharWidth;
	ULONG                   CharHeight;
	ULONG                   ConsoleTextAttributes;
	ULONG                   WindowFlags;
	ULONG                   ShowWindowFlags;
	UNICODE_STRING          WindowTitle;
	UNICODE_STRING          DesktopName;
	UNICODE_STRING          ShellInfo;
	UNICODE_STRING          RuntimeData;
	RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];

}RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _RTL_CRITICAL_SECTION *PRTL_CRITICAL_SECTION;

typedef struct _ProcessParamData
{
	RTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID SubSystemData;
	PVOID ProcessHeap;
	PRTL_CRITICAL_SECTION FastPebLock;
	PVOID AtlThunkSListPtr;
	PVOID IFEOKey;
	ULONG CrossProcessFlags;

}PROCESSPARAMDATA, *PPROCESSPARAMDATA;

typedef struct _PEB
{
	BOOLEAN InheritedAddressSpace;
	BOOLEAN ReadImageFileExecOptions;
	BOOLEAN BeingDebugged;
	union
	{
		BOOLEAN BitField;
		struct
		{
			BOOLEAN ImageUsesLargePages : 1;
			BOOLEAN IsProtectedProcess : 1;
			BOOLEAN IsImageDynamicallyRelocated : 1;
			BOOLEAN SkipPatchingUser32Forwarders : 1;
			BOOLEAN IsPackagedProcess : 1;
			BOOLEAN IsAppContainer : 1;
			BOOLEAN IsProtectedProcessLight : 1;
			BOOLEAN SpareBits : 1;
		};
	};
	HANDLE Mutant;

	PVOID ImageBaseAddress;
	PPEB_LDR_DATA Ldr;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID SubSystemData;
	PVOID ProcessHeap;
	PRTL_CRITICAL_SECTION FastPebLock;
	PVOID AtlThunkSListPtr;
	PVOID IFEOKey;
	union
	{
		ULONG CrossProcessFlags;
		struct
		{
			ULONG ProcessInJob : 1;
			ULONG ProcessInitializing : 1;
			ULONG ProcessUsingVEH : 1;
			ULONG ProcessUsingVCH : 1;
			ULONG ProcessUsingFTH : 1;
			ULONG ReservedBits0 : 27;
		};
		ULONG EnvironmentUpdateCount;
	};
	union
	{
		PVOID KernelCallbackTable;
		PVOID UserSharedInfoPtr;
	};
	ULONG SystemReserved[1];
	ULONG AtlThunkSListPtr32;
	PVOID ApiSetMap;
	ULONG TlsExpansionCounter;
	PVOID TlsBitmap;
	ULONG TlsBitmapBits[2];
	PVOID ReadOnlySharedMemoryBase;
	PVOID HotpatchInformation;
	PVOID *ReadOnlyStaticServerData;
	PVOID AnsiCodePageData;
	PVOID OemCodePageData;
	PVOID UnicodeCaseTableData;

	ULONG NumberOfProcessors;
	ULONG NtGlobalFlag;

	LARGE_INTEGER CriticalSectionTimeout;
	SIZE_T HeapSegmentReserve;
	SIZE_T HeapSegmentCommit;
	SIZE_T HeapDeCommitTotalFreeThreshold;
	SIZE_T HeapDeCommitFreeBlockThreshold;

	ULONG NumberOfHeaps;
	ULONG MaximumNumberOfHeaps;
	PVOID *ProcessHeaps;

	PVOID GdiSharedHandleTable;
	PVOID ProcessStarterHelper;
	ULONG GdiDCAttributeList;

	PRTL_CRITICAL_SECTION LoaderLock;

	ULONG OSMajorVersion;
	ULONG OSMinorVersion;
	USHORT OSBuildNumber;
	USHORT OSCSDVersion;
	ULONG OSPlatformId;
	ULONG ImageSubsystem;
	ULONG ImageSubsystemMajorVersion;
	ULONG ImageSubsystemMinorVersion;
	ULONG_PTR ImageProcessAffinityMask;
	GDI_HANDLE_BUFFER GdiHandleBuffer;
	PVOID PostProcessInitRoutine;

	PVOID TlsExpansionBitmap;
	ULONG TlsExpansionBitmapBits[32];

	ULONG SessionId;

	ULARGE_INTEGER AppCompatFlags;
	ULARGE_INTEGER AppCompatFlagsUser;
	PVOID pShimData;
	PVOID AppCompatInfo;

	UNICODE_STRING CSDVersion;

	PVOID ActivationContextData;
	PVOID ProcessAssemblyStorageMap;
	PVOID SystemDefaultActivationContextData;
	PVOID SystemAssemblyStorageMap;

	SIZE_T MinimumStackCommit;

	PVOID *FlsCallback;
	LIST_ENTRY FlsListHead;
	PVOID FlsBitmap;
	ULONG FlsBitmapBits[FLS_MAXIMUM_AVAILABLE / (sizeof(ULONG) * 8)];
	ULONG FlsHighIndex;

	PVOID WerRegistrationData;
	PVOID WerShipAssertPtr;
	PVOID pContextData;
	PVOID pImageHeaderHash;
	union
	{
		ULONG TracingFlags;
		struct
		{
			ULONG HeapTracingEnabled : 1;
			ULONG CritSecTracingEnabled : 1;
			ULONG LibLoaderTracingEnabled : 1;
			ULONG SpareTracingBits : 29;
		};
	};
	ULONGLONG CsrServerReadOnlySharedMemoryBase;
} PEB, *PPEB;


typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,                // 0 Y N
	SystemProcessorInformation,            // 1 Y N
	SystemPerformanceInformation,        // 2 Y N
	SystemTimeOfDayInformation,            // 3 Y N
	SystemNotImplemented1,                // 4 Y N
	SystemProcessesAndThreadsInformation, // 5 Y N
	SystemCallCounts,                    // 6 Y N
	SystemConfigurationInformation,        // 7 Y N
	SystemProcessorTimes,                // 8 Y N
	SystemGlobalFlag,                    // 9 Y Y
	SystemNotImplemented2,                // 10 Y N
	SystemModuleInformation,            // 11 Y N
	SystemLockInformation,                // 12 Y N
	SystemNotImplemented3,                // 13 Y N
	SystemNotImplemented4,                // 14 Y N
	SystemNotImplemented5,                // 15 Y N
	SystemHandleInformation,            // 16 Y N
	SystemObjectInformation,            // 17 Y N
	SystemPagefileInformation,            // 18 Y N
	SystemInstructionEmulationCounts,    // 19 Y N
	SystemInvalidInfoClass1,            // 20
	SystemCacheInformation,                // 21 Y Y
	SystemPoolTagInformation,            // 22 Y N
	SystemProcessorStatistics,            // 23 Y N
	SystemDpcInformation,                // 24 Y Y
	SystemNotImplemented6,                // 25 Y N
	SystemLoadImage,                    // 26 N Y
	SystemUnloadImage,                    // 27 N Y
	SystemTimeAdjustment,                // 28 Y Y
	SystemNotImplemented7,                // 29 Y N
	SystemNotImplemented8,                // 30 Y N
	SystemNotImplemented9,                // 31 Y N
	SystemCrashDumpInformation,            // 32 Y N
	SystemExceptionInformation,            // 33 Y N
	SystemCrashDumpStateInformation,    // 34 Y Y/N
	SystemKernelDebuggerInformation,    // 35 Y N
	SystemContextSwitchInformation,        // 36 Y N
	SystemRegistryQuotaInformation,        // 37 Y Y
	SystemLoadAndCallImage,                // 38 N Y
	SystemPrioritySeparation,            // 39 N Y
	SystemNotImplemented10,                // 40 Y N
	SystemNotImplemented11,                // 41 Y N
	SystemInvalidInfoClass2,            // 42
	SystemInvalidInfoClass3,            // 43
	SystemTimeZoneInformation,            // 44 Y N
	SystemLookasideInformation,            // 45 Y N
	SystemSetTimeSlipEvent,                // 46 N Y
	SystemCreateSession,                // 47 N Y
	SystemDeleteSession,                // 48 N Y
	SystemInvalidInfoClass4,            // 49
	SystemRangeStartInformation,        // 50 Y N
	SystemVerifierInformation,            // 51 Y Y
	SystemAddVerifier,                    // 52 N Y
	SystemSessionProcessesInformation    // 53 Y N
} SYSTEM_INFORMATION_CLASS;

typedef struct _DEVICE_EXTENSION
{
	struct _DEVICE_EXTENSION *pDeviceExtension;
	struct _DEVICE_EXTENSION *pNextDeviceExtension;
	LIST_ENTRY PendingCancelIrpQueue;
	ULONG *value3;
	FAST_MUTEX *FastMutex;
	KEVENT KernelEvent;
	IO_CSQ CancelSafeQueue;

}DEVICE_EXTENSION,*PDEVICE_EXTENSION;

extern NTSTATUS  ZwQueryInformationProcess(
	HANDLE           ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID            ProcessInformation,
	ULONG            ProcessInformationLength,
	PULONG           ReturnLength
);

extern NTSTATUS ZwQueryInformationThread(
	HANDLE          ThreadHandle,
	THREADINFOCLASS ThreadInformationClass,
	PVOID           ThreadInformation,
	ULONG           ThreadInformationLength,
	PULONG          ReturnLength
);

extern NTSTATUS __stdcall ZwOpenProcessToken(HANDLE ProcessHandle, ACCESS_MASK DesiredAccess, PHANDLE TokenHandle);

extern NTSTATUS __stdcall ZwQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, SIZE_T Length, PSIZE_T ResultLength);

#define PsGetProcessPebName L"PsGetProcessPeb"
#define PsIsProtectedProcessName L"PsIsProtectedProcess"

typedef PPEB(__stdcall *PSGETPROCESSPEB)(PEPROCESS pEprocess);
typedef BOOLEAN(__stdcall *PSISPROTECTEDPROCESS)(PEPROCESS pEprocess);
typedef int(__stdcall *ZWOPENPROCESSTOKENEX)(ULONG, ULONG, ULONG, ULONG);
