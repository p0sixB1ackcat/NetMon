#pragma once
#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <fltKernel.h>

typedef struct _ATKMFDATA {

	PDRIVER_OBJECT DriverObject;
	PFLT_FILTER Filter;
	PFLT_PORT ServerPort;
	PEPROCESS UserProcess;
	PFLT_PORT ClientPort;

} ATKMFDATA, *PATKMFDATA;

extern ATKMFDATA g_KMfData;

typedef struct _AntiyMFilter_STREAM_HANDLE_CONTEXT {

	BOOLEAN RescanRequired;

} AntiyMFilter_STREAM_HANDLE_CONTEXT, *PAntiyMFilter_STREAM_HANDLE_CONTEXT;

#pragma warning(push)
#pragma warning(disable:4200) // disable warnings for structures with zero length arrays.

typedef struct _AntiyMFilter_CREATE_PARAMS {

	WCHAR String[0];

} AntiyMFilter_CREATE_PARAMS, *PAntiyMFilter_CREATE_PARAMS;

#pragma warning(pop)



VOID Initialize(VOID);
#endif /* __SCANNER_H__ */


