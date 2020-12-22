#include "AntiyEtw.h"


CAntiyMonLinkList::CAntiyMonLinkList(LIST_ENTRY *Blink, LIST_ENTRY *Flink)
{
	LIST_ENTRY *lB = NULL;
	LIST_ENTRY *lF = NULL;

	lF = Flink;
	m_pDataEvent = 0;

	if (Flink)
	{
		lB = Blink;
	}
	else
	{
		lB = lF = &m_Entry;
	}

	m_Entry.Blink = lB;
	m_Entry.Flink = lF;

}

CAntiyMonLinkList::CAntiyMonLinkList()
{

}

CAntiyMonLinkList *CAntiyMonLinkList::InsertHeader(LIST_ENTRY *Blink, void *pDataEvent)
{
	CAntiyMonLinkList *ListNoe = new CAntiyMonLinkList(Blink,&m_Entry);
	
	if (ListNoe)
	{
		ListNoe->m_pDataEvent = pDataEvent;
	}

	return ListNoe;
}

CAntiyMonLinkList::~CAntiyMonLinkList()
{
}
