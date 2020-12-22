#include "AntiyCancelSafeIrpQueue.h"

VOID __stdcall CsqInsertIrp(IO_CSQ *pCsq, PIRP pIrp)
{
	LIST_ENTRY *IrpEntry;
	LIST_ENTRY *CsqReserveEntry;
	DEVICE_EXTENSION *pDevExten = NULL;

	pDevExten = CONTAINING_RECORD(pCsq,DEVICE_EXTENSION,CancelSafeQueue);
	ASSERT(pDevExten);
	InsertHeadList(&pDevExten->PendingCancelIrpQueue, &pIrp->Tail.Overlay.ListEntry);
}

VOID __stdcall CsqRemoveIrp(PIO_CSQ pCsq, PIRP pIrp)
{
	RemoveEntryList((LIST_ENTRY *)&pIrp->Tail.Overlay.ListEntry);
}

NTSTATUS __stdcall CsqPeekNextIrp(PIO_CSQ pCsq, PIRP pIrp, PVOID PeekContext)
{
	LIST_ENTRY *pIrpEntry;
	LIST_ENTRY *pCsqCompleteCancelIrp;
	DEVICE_EXTENSION *pDevExt = NULL;

	pDevExt = CONTAINING_RECORD(pCsq,DEVICE_EXTENSION,CancelSafeQueue);

	pIrpEntry = &pDevExt->PendingCancelIrpQueue;
	if (pIrp)
		pCsqCompleteCancelIrp = pIrp->Tail.Overlay.ListEntry.Flink;
	else
		pCsqCompleteCancelIrp = pIrpEntry->Flink;

	if (pIrpEntry == pCsqCompleteCancelIrp)
		return 0;
	while (PeekContext && pCsqCompleteCancelIrp->Flink->Flink != PeekContext)
	{
		pCsqCompleteCancelIrp = pCsqCompleteCancelIrp->Flink;
		if (pCsqCompleteCancelIrp == pIrpEntry)
			return 0;
	}
	return 1;
}

VOID __stdcall CsqAcquireLock(PIO_CSQ pCsq, PKIRQL pIrql)
{
	ExAcquireFastMutex(CONTAINING_RECORD(pCsq, DEVICE_EXTENSION, FastMutex));
}

VOID __stdcall CsqReleaseLock(PIO_CSQ pCsq, KIRQL Irql)
{
	ExReleaseFastMutex((CONTAINING_RECORD(pCsq, DEVICE_EXTENSION, FastMutex)));
}

VOID __stdcall CsqCompleteCancelIrp(PIO_CSQ pCsq, PIRP pIrp)
{
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_CANCELLED;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);
}