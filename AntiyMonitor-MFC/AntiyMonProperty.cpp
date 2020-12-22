#include "AntiyEtw.h"


CAntiyMonProperty::CAntiyMonProperty(BSTR Name, LONG ArraySize, LONG Length):m_Data(NULL),m_ArraySize(ArraySize),m_Len(Length)
{
	if (Name)
	{
		InitData(&m_Data, Name);
	}
}

void CAntiyMonProperty::InitData(CAntiyMonData **ppData, BSTR bString)
{
	BSTR bOldString = NULL;
	if (ppData && *ppData)
	{
		bOldString = (*ppData)->m_Data;
	}

	if (!bString || bString != bOldString)
	{
		if (*ppData)
		{
			if (InterlockedDecrement((volatile LONG *)(*ppData)->m_Reference) == 0)
			{
				SysFreeString((*ppData)->m_Data);
				(*ppData)->m_Data = NULL;
			}

			if ((*ppData)->m_Alloc)
			{
				delete (*ppData)->m_Alloc;
				(*ppData)->m_Alloc = NULL;
			}

			delete *ppData;
		}
		*ppData = NULL;
	}

	CAntiyMonData *NewData = new CAntiyMonData();
	if (NewData)
	{
		NewData->m_Reference = 1;
		NewData->m_Alloc = NULL;
		NewData->m_Data = SysAllocString(bString);
	}
	else
	{

	}
	*ppData = NewData;

}

CAntiyMonProperty::CAntiyMonProperty()
{
}


CAntiyMonProperty::~CAntiyMonProperty()
{
}
