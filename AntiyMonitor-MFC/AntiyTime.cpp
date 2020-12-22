#include "AntiyEtw.h"

HANDLE g_hProcessHeap = NULL;

extern LARGE_INTEGER g_PerformanceCount;
extern LARGE_INTEGER g_Frequency;
extern FILETIME g_SystemFileTime;

BOOL InitProcessHeap(VOID)
{
	g_hProcessHeap = GetProcessHeap();
	return g_hProcessHeap != 0;
}
BOOL InitTimeData(VOID)
{
	BOOL bResult = FALSE;

	GetSystemTimeAsFileTime(&g_SystemFileTime);
	printf("g_SystemFileTime.dwHighDataTime is 0x%x,g_SystemFileTime.dwLowDataTime is 0x%x\n!", g_SystemFileTime.dwHighDateTime, g_SystemFileTime.dwLowDateTime);
	QueryPerformanceCounter(&g_PerformanceCount);
	printf("g_PerformanceCount.highPart is 0x%x,g_PerformanceCount.lowPart is 0x%x!\n", g_PerformanceCount.HighPart, g_PerformanceCount.LowPart);
	bResult = QueryPerformanceFrequency(&g_Frequency);
	printf("g_Frequency.highPart is 0x%x,g_Frequency.lowPart is 0x%x!\n", g_Frequency.HighPart, g_Frequency.LowPart);

	return bResult;
}

LONGLONG CalcByTraceTimeLessPerformanceCount(FILETIME *a1, __int64 a2)
{
	LONGLONG result = 0;
	if (a2 >= 0)
	{
		(((a2 / a1->dwHighDateTime) * 10000000) + (a2 % a1->dwHighDateTime) * 10000000 / a1->dwHighDateTime);
	}
	else
		result = CalcByTraceTimeLessPerformanceCount(a1, -a2);

	return result;
}

LARGE_INTEGER CalcTime(LARGE_INTEGER TraceHeaderTimeStamp)
{
	LARGE_INTEGER qwResult = { 0x00 };
	LARGE_INTEGER llSpace = { 0x00 };
	LONGLONG magic = 0;
	if (TraceHeaderTimeStamp.QuadPart < 0x100000000000000i64)
	{
		llSpace.QuadPart = TraceHeaderTimeStamp.QuadPart - g_PerformanceCount.QuadPart;

		if (llSpace.HighPart >= 0)
		{
			magic = ((llSpace.QuadPart / g_Frequency.QuadPart) * 10000000) + (((llSpace.QuadPart % g_Frequency.QuadPart) * 10000000) / g_Frequency.QuadPart);

		}
		else
		{
			magic = -CalcByTraceTimeLessPerformanceCount(&g_SystemFileTime, -llSpace.QuadPart);
		}
		qwResult.QuadPart = magic + *(LONGLONG *)&g_SystemFileTime;
	}

	return qwResult;
}

int *GetPointerByFileTime(FILETIME * lpFileTime)
{
	FILETIME LocalFileTime = {0x00};
	SYSTEMTIME SystemTime = {0x00};
	WCHAR DataStr[0xA0] = {0x00};
	WCHAR FormatBuffer[0x0c] = {0x00};
	LARGE_INTEGER qwNanoSecond = { 0x00 };
	if (FileTimeToLocalFileTime(lpFileTime, &LocalFileTime) && FileTimeToSystemTime(&LocalFileTime, &SystemTime))
	{
		GetTimeFormatW(0x400u, NULL, &SystemTime, 0, DataStr, 80);
		qwNanoSecond.HighPart = lpFileTime->dwHighDateTime;
		qwNanoSecond.LowPart = lpFileTime->dwLowDateTime;
		qwNanoSecond.QuadPart = qwNanoSecond.QuadPart % 10000000;
		
		GetLocaleInfoW(LOCALE_SYSTEM_DEFAULT, 0x0e, FormatBuffer, 5);
		
		swprintf_s(CAntiyEventTraceData::ShareInstance()->m_Time, 0xc0 + sizeof(qwNanoSecond.QuadPart), L"%ws%ws%07d", DataStr, FormatBuffer, qwNanoSecond.QuadPart);
	}

	return NULL;
}

void GetLocalSystemTimeByFileTimeStamp(LARGE_INTEGER *pfileTimeStamp,SYSTEMTIME *st)
{
	FILETIME ft;
	SYSTEMTIME systime = {0x00};
	if (!pfileTimeStamp)
		return;
	
	ft.dwLowDateTime = pfileTimeStamp->LowPart;
	ft.dwHighDateTime = pfileTimeStamp->HighPart;
	FileTimeToSystemTime(&ft,&systime);
	SystemTimeToTzSpecificLocalTime(NULL, (const SYSTEMTIME *)&systime, st);


}