#include <stdio.h>
#include <iostream>
#include <comutil.h>
#include <string.h>
#include <stdlib.h>
#include "AntiyEtw.h"
#include "AntiyWmi.h"

using namespace std;
EventSink::EventSink()
{
	InitListEntry(&m_ListHeader);
	InitializeCriticalSection(&m_WmiEventLock);

}


EventSink::~EventSink()
{
}

ULONG EventSink::AddRef()
{
	return InterlockedIncrement(&m_lRef);
}

ULONG EventSink::Release()
{
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT EventSink::QueryInterface(REFIID riid, void** ppv)
{
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
	{
		*ppv = (IWbemObjectSink *)this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else return E_NOINTERFACE;
}

void CopyUnicodeString(PUNICODE_STRING *Desc, WCHAR *Source)
{
	*Desc = (PUNICODE_STRING)malloc(sizeof(UNICODE_STRING));
	ULONG dwLen = wcslen(Source) + 1;
	(*Desc)->Buffer = (WCHAR *)malloc(dwLen * sizeof(WCHAR));
	memset((*Desc)->Buffer, 0, dwLen);
	(*Desc)->Length = dwLen;
	wcsncpy_s((*Desc)->Buffer, dwLen,Source, dwLen);
}

LARGE_INTEGER EventSink::GetTimeStampValue(WCHAR *szTime)
{
	LARGE_INTEGER llValue;
	WCHAR *p = NULL;
	
	llValue.QuadPart = 0;
	p = szTime;
	while (*p != '\0')
	{
		llValue.QuadPart *= 10;
		llValue.QuadPart += (*p);
		llValue.QuadPart -= '0';
		++p;
	}

	return llValue;
	
}

void EventSink::SetWmiEventData(WMI_EVENT_DATA *pWmiEventData ,WCHAR *pKey, WCHAR *pValue)
{
	
	if (_wcsicmp(pKey, L"__CLASS") == 0)
	{
		CopyUnicodeString(&pWmiEventData->szClassName, pValue);
	}
	else if (_wcsicmp(pKey, L"__SUPERCLASS") == 0)
	{
		CopyUnicodeString(&pWmiEventData->szSuperClassName, pValue);
	}
	else if (_wcsicmp(pKey, L"__SERVER") == 0)
	{
		CopyUnicodeString(&pWmiEventData->szServerName, pValue);
	}
	else if (_wcsicmp(pKey, L"__NAMESPACE") == 0)
	{
		CopyUnicodeString(&pWmiEventData->szNameSpace, pValue);
	}
	else if (_wcsicmp(pKey, L"TIME_CREATED") == 0)
	{
		pWmiEventData->llTimeCreate = GetTimeStampValue(pValue);
	}

}

WMI_EVENT_DATA *EventSink::EnumClassData(IWbemClassObject *pClass)
{
	VARIANT varWmiDatas;
	LONG Ubound = 0;
	LONG Lbound = 0;
	SAFEARRAY *pSafeArray = NULL;
	BSTR bWmiString = NULL;
	LONG rgIndices = 0;
	CIMTYPE CimType;
	IWbemQualifierSet *pQualifierSet = NULL;
	HRESULT hr;
	VARIANT varIant;
	VARIANT varValue;
	LIST_ENTRY *pList = NULL;
	WMI_EVENT_DATA *pWmiEventData = NULL;
	BOOL bSuccess = FALSE;

	if (!pClass)
		return pWmiEventData;

	pWmiEventData = (WMI_EVENT_DATA *)malloc(sizeof(WMI_EVENT_DATA));
	memset(pWmiEventData, 0, sizeof(WMI_EVENT_DATA));

	while (pClass->GetNames(NULL, WBEM_FLAG_ALWAYS, &varWmiDatas, &pSafeArray) == S_OK)
	{
		if(SafeArrayGetLBound(pSafeArray,1u,&Lbound) || SafeArrayGetUBound(pSafeArray, 1u, &Ubound) || Ubound < 0)
			break;
		if (Lbound <= Ubound)
		{
			rgIndices = Lbound;
		}
		do 
		{
			if(SafeArrayGetElement(pSafeArray,&rgIndices,&bWmiString) || pClass->Get(bWmiString,0,0,&CimType,NULL))
				break;
			
			VariantInit(&varIant);
			hr = pClass->Get(bWmiString,0,&varIant,NULL,NULL);
			
			do 
			{
				if(FAILED(hr))
					break;

				bSuccess = TRUE;
				VariantInit(&varValue);

				switch (varIant.vt)
				{
					case CIM_STRING:
					{
						//printf("%ws=%ws\n",bWmiString,varIant.bstrVal);
						hr = pClass->GetQualifierSet(&pQualifierSet);
						if (hr == S_OK)
						{
							SetWmiEventData(pWmiEventData,bWmiString, varIant.bstrVal);
						}

					}
					break;
					case CIM_UINT64:
					{
						printf("%ws=%d\n", bWmiString,varIant.llVal);
					}
				}
				VariantClear(&varIant);
				VariantClear(&varValue);
			} while (0);

			
 			++rgIndices;
		} while (rgIndices <= Ubound);

		VariantClear(&varWmiDatas);
		SafeArrayDestroy(pSafeArray);
		pSafeArray = NULL;
		varWmiDatas.lVal++;
		break;
	}

	if (!bSuccess)
	{
		free(pWmiEventData);
		return NULL;
	}

	EnterCriticalSection(&m_WmiEventLock);
	InsertHeaderList(&m_ListHeader, &pWmiEventData->List);
	LeaveCriticalSection(&m_WmiEventLock);
	return pWmiEventData;
}

BOOL GetWmiQueryInfo(PWMI_EVENT_DATA pWmiEventData, OLECHAR *szObjectText)
{
	BOOL bResult = FALSE;
	ULONG dwNameOffset = 0;
	ULONG dwNameLen;
	ULONG i = 0;
	ULONG dwEnd;
	OLECHAR *rp = NULL;//read pointer
	const OLECHAR *cps[4] = {L"Name = ",L"Query = ",L"QueryLanguage = ",L"TargetInstance = "};//compare pointer
	ULONG cpcount;
	const OLECHAR *cp = NULL;
	const OLECHAR *p = NULL;
	WCHAR *tmpstr = NULL;
	const OLECHAR *pName = NULL;

	do 
	{
		rp = szObjectText;
		for (cpcount = 0; cpcount < 4; ++cpcount)
		{
			cp = cps[cpcount];
			dwNameLen = wcslen(cp);
			dwEnd = 0;
			//while (rp[i] != '\0')
			{
				p = wcsstr(rp, cp);
				if (p)
				{
					pName = p + dwNameLen;
					while (pName[dwEnd] != '\0')
					{
						if (pName[dwEnd] == ';')
						{
							break;
						}
						++dwEnd;
					}

					if(dwEnd == 0)
						break;
					tmpstr = (WCHAR *)malloc((dwEnd + 1) * sizeof(WCHAR));
					memset(tmpstr, 0, (dwEnd + 1) * sizeof(WCHAR));
					CopyMemory(tmpstr, pName,dwEnd  * sizeof(WCHAR));
					if (_wcsicmp(cp, L"Name = ") == 0)
					{
						pWmiEventData->szName = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
						CopyUnicodeString(&pWmiEventData->szName, tmpstr);
					}
					else if (_wcsicmp(cp, L"Query = ") == 0)
					{
						pWmiEventData->szQuery = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
						CopyUnicodeString(&pWmiEventData->szQuery, tmpstr);
					}
					else if (_wcsicmp(cp, L"QueryLanguage = ") == 0)
					{
						pWmiEventData->szQueryLanguage = (UNICODE_STRING *)malloc(sizeof(UNICODE_STRING));
						CopyUnicodeString(&pWmiEventData->szQueryLanguage, tmpstr);
					}
					else if (_wcsicmp(cp, L"TargetInstance = ") == 0)
					{
						WCHAR *startstr = tmpstr;
						WCHAR *endstr = NULL;

						while (*startstr && *(startstr + 1))
						{
							//if (*startstr == '_' && *(startstr + 1) == '_')
							{
								startstr += wcslen(L"instance of ");
								endstr = startstr;
								while (*endstr)
								{
									if (*endstr == '{')
									{
										break;
									}
									++endstr;
								}
								break;
							}
							++startstr;
						}
						if (endstr)
						{
							WCHAR tmpBuffer[0x30] = { 0 };
							CopyMemory(tmpBuffer, startstr, (endstr - startstr) * sizeof(WCHAR));
							CopyUnicodeString(&pWmiEventData->szEventName, tmpBuffer);
						}
						
					}
					if (tmpstr)
					{
						free(tmpstr);
						tmpstr = NULL;
					}
				}

				++i;
			}
		}
		

	} while (0);

	return bResult;
}

HRESULT EventSink::Indicate(long lObjectCount,
	IWbemClassObject **apObjArray)
{
	ULONG dwResult = 0;
	IWbemClassObject *pClassObj = NULL;
	IWbemQualifierSet *pQualifierSet = NULL;
	SAFEARRAY *pNames = NULL;
	HRESULT hr;
	char pByteBuffer[0x1fe] = { 0 };
	void *pVBuffer_1;
	ULONG dwCurrentIndex = 0;
	VARIANT varClass;
	OLECHAR *bstr;
	OLECHAR **pBstr = NULL;
	void **pPAddress = NULL;
	ULONG ProcessId = 0;
	PWMI_EVENT_DATA pWmiEventData = NULL;

	if (lObjectCount <= 0)
		return WBEM_S_FALSE;

	do
	{
		pBstr = &bstr;
		bstr = NULL;
		pVBuffer_1 = NULL;

		pClassObj = *(apObjArray + dwCurrentIndex);
		do
		{
			if ( (hr = pClassObj->GetObjectText(0, &bstr)) < 0 || !bstr)
				break;

			pVBuffer_1 = NULL;
			pPAddress = (void **)&varClass;
			//printf("收到Wmi事件:%ws\n", bstr);

			pWmiEventData = EnumClassData(pClassObj);
			if(!pWmiEventData)
				break;
			GetWmiQueryInfo(pWmiEventData, bstr);
			
		}while (0);

		if (bstr)
			SysFreeString(bstr);

		
		++dwCurrentIndex;
		
	} while (FALSE);


	return WBEM_S_NO_ERROR;
}

HRESULT EventSink::SetStatus(
	/* [in] */ LONG lFlags,
	/* [in] */ HRESULT hResult,
	/* [in] */ BSTR strParam,
	/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
)
{
	if (lFlags == WBEM_STATUS_COMPLETE)
	{
		printf("Call complete. hResult = 0x%X\n", hResult);
	}
	else if (lFlags == WBEM_STATUS_PROGRESS)
	{
		printf("Call in progress.\n");
	}

	return WBEM_S_NO_ERROR;
}    // end of EventSink.cpp

void FreeUnicodeString(PUNICODE_STRING *uString)
{
	if (*uString == NULL || (*uString)->Buffer == NULL || (*uString)->Length == 0)
		return;
	free((*uString)->Buffer);
	(*uString)->Buffer = NULL;
	(*uString)->Length = 0;
	free(*uString);
	*uString = NULL;
}

void EventSink::FreeWmiData(void)
{
	LIST_ENTRY *pList = NULL;
	LIST_ENTRY *pFreeList = NULL;
	PWMI_EVENT_DATA pWmiEventData = NULL;

	if (IsEmptyList(&m_ListHeader))
		return;
	EnterCriticalSection(&m_WmiEventLock);
	pList = m_ListHeader.Flink;
	pFreeList = &m_ListHeader;
	while (pList != &m_ListHeader)
	{
		do 
		{
			pWmiEventData = CONTAINING_RECORD(pList,WMI_EVENT_DATA,List);
			if (!pWmiEventData)
			{
				pList = pList->Flink;
				break;
			}
			FreeUnicodeString(&pWmiEventData->szClassName);
			FreeUnicodeString(&pWmiEventData->szSuperClassName);
			FreeUnicodeString(&pWmiEventData->szNameSpace);
			FreeUnicodeString(&pWmiEventData->szServerName);
			FreeUnicodeString(&pWmiEventData->szName);
			FreeUnicodeString(&pWmiEventData->szQuery);
			FreeUnicodeString(&pWmiEventData->szQueryLanguage);
			FreeUnicodeString(&pWmiEventData->szEventName);
			
			pFreeList = pList;
			pList = pList->Flink;
			RemoveHeaderList(pFreeList);
			free(pWmiEventData);
			pWmiEventData = NULL;

		} while (0);
		

	}

	LeaveCriticalSection(&m_WmiEventLock);
}