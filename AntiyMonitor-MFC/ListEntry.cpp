#include "AntiyEtw.h"


void InitListEntry(LIST_ENTRY *pList)
{
	pList->Flink = pList->Blink = pList;
}

void InsertHeaderList(LIST_ENTRY *pHeader, LIST_ENTRY *pList)
{
	LIST_ENTRY *Flink;
	Flink = pHeader->Flink;
	pList->Blink = pHeader;
	pList->Flink = Flink;
	pHeader->Flink = pList;
	Flink->Blink = pList;
}

void RemoveHeaderList(LIST_ENTRY *pList)
{
	LIST_ENTRY *Flink;
	Flink = pList->Flink;
	pList->Blink->Flink = Flink;
	Flink->Blink = pList->Blink;
}

BOOLEAN IsEmptyList(LIST_ENTRY *pList)
{
	return (BOOLEAN)(pList == pList->Flink);
}