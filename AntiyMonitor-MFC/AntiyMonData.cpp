#include "AntiyEtw.h"


ULONG CAntiyMonData::Len(void)
{
	if (m_Data)
	{
		__try
		{
			return SysStringLen(m_Data);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{

		}
	}
	return 0;

}

BOOL CAntiyMonData::Compare(BSTR bstrString)
{
	if (bstrString && m_Data)
	{
		return wcscmp(m_Data, bstrString) == 0;
	}
	return FALSE;
}

BOOL CAntiyMonData::CompareN(BSTR bstrString, ULONG nSize)
{
	if (bstrString && m_Data)
	{
		return _wcsnicmp(m_Data, bstrString, nSize) == 0;
	}
	return FALSE;
}

CAntiyMonData::CAntiyMonData()
{
}


CAntiyMonData::~CAntiyMonData()
{
}
