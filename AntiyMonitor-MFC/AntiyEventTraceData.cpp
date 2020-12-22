#include "AntiyEtw.h"

CAntiyEventTraceData *CAntiyEventTraceData::m_This = NULL;
CLock CAntiyEventTraceData::m_Lock = CLock();

CAntiyEventTraceData *CAntiyEventTraceData::ShareInstance(void)
{
	if (!m_This)
	{
		CAutoLock Aulock(&m_Lock);
		if (!m_This)
		{
			m_This = new CAntiyEventTraceData();
		}
	}
	return m_This;
}

CAntiyEventTraceData::CAntiyEventTraceData()
{
}

void CAntiyEventTraceData::Clear(void)
{
	m_LocalPort = 0;
	m_RemotePort = 0;
}

CAntiyEventTraceData::~CAntiyEventTraceData()
{
}
