#include <emmintrin.h>
#include "AntiyEtw.h"

CAntiyMonDataEntry::CAntiyMonDataEntry(GUID *guid, USHORT Type, USHORT Version, USHORT Level)
{
	m_Type = Type;
	m_Version = Version;
	m_Level = Level;
	m_ClassData = NULL;
	m_TypeData = NULL;
	m_PropertyList = new CAntiyMonLinkList(NULL,NULL);
	m_PropertyCount = 0;
	_mm_storeu_si128((__m128i *)&m_Guid, _mm_loadu_si128((__m128i *)guid));
}

CAntiyMonDataEntry::CAntiyMonDataEntry()
{
}

void CAntiyMonDataEntry::operator= (CAntiyMonDataEntry *pDataEntry)
{
	if (this != pDataEntry)
	{
		m_ClassData = pDataEntry->m_ClassData;
		if (pDataEntry->m_ClassData)
			InterlockedIncrement((volatile ULONG *)&pDataEntry->m_ClassData->m_Reference);

	}
}

CAntiyMonDataEntry *CAntiyMonDataEntry::InitData(CAntiyMonData **ppData, BSTR bString)
{
	BSTR oldString = NULL;
	if (ppData && *ppData)
	{
		oldString = (*ppData)->m_Data;
	}

	if (!bString || oldString != bString)
	{
		if (*ppData)
		{
			if (!InterlockedDecrement((volatile ULONG *)(*ppData)->m_Reference))
			{
				if ((*ppData)->m_Data)
				{
					SysFreeString((*ppData)->m_Data);
					(*ppData)->m_Data = NULL;
				}
				if (m_ClassData->m_Alloc)
				{
					delete (*ppData)->m_Alloc;
					(*ppData)->m_Alloc = NULL;
				}

				delete (*ppData);
			}
			(*ppData) = NULL;
		}
	}

	CAntiyMonData *NewData = new CAntiyMonData();
	if (NewData)
	{
		NewData->m_Alloc = 0;
		NewData->m_Reference = 1;
		NewData->m_Data = SysAllocString(bString);
	}
	else
	{

	}

	(*ppData) = NewData;
	return this;
}

ULONG CAntiyMonDataEntry::GetClassDataLen()
{
	if (m_ClassData && m_ClassData->m_Data)
	{
		return SysStringLen(m_ClassData->m_Data);
	}
	return 0;
}

CAntiyMonDataEntry::~CAntiyMonDataEntry()
{
}
