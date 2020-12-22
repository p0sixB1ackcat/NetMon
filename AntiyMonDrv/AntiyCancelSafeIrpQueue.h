#pragma once
#include "AntiyCommon.h"

VOID __stdcall CsqInsertIrp(IO_CSQ *pCsq, PIRP pIrp);
VOID __stdcall CsqRemoveIrp(PIO_CSQ pCsq, PIRP pIrp);
NTSTATUS __stdcall CsqPeekNextIrp(PIO_CSQ pCsq, PIRP pIrp, PVOID PeekContext);
VOID __stdcall CsqAcquireLock(PIO_CSQ pCsq, PKIRQL pIrql);
VOID __stdcall CsqReleaseLock(PIO_CSQ pCsq, KIRQL Irql);
VOID __stdcall CsqCompleteCancelIrp(PIO_CSQ pCsq, PIRP pIrp);