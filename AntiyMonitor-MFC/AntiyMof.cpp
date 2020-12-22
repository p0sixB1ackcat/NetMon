
#define INITGUID

#include <windows.h>
#include <stdio.h>
#include <comutil.h>
#include <wbemidl.h>
#include <wmistr.h>
#include <evntrace.h>
#include <in6addr.h>
#include <Psapi.h>
#include <emmintrin.h>
#include <winternl.h>
#include <iostream>
#include "AntiyEtw.h"

using namespace std;

extern IWbemServices  *g_pProxy;

extern CAntiyMonLinkList *g_MofLinkListHeader;
ULONG g_MofEventListCount = 0;

LARGE_INTEGER g_PerformanceCounter = { 0x00 };

extern LARGE_INTEGER g_PerformanceCount;
extern LARGE_INTEGER g_Frequency;
extern FILETIME g_SystemFileTime;

//#pragma comment(lib, "comsupp.lib") 
#pragma comment(lib, "ws2_32.lib") 

typedef struct _propertyList
{
	BSTR Name;
	LONG CimType;
	IWbemQualifierSet* pQualifiers;
} PROPERTY_LIST;

USHORT g_PointerSize = 0;

CAntiyMonDataEntry *__stdcall _getdata(IWbemClassObject *lpClass, PEVENT_TRACE lpTrace);

typedef LPTSTR(NTAPI *PIPV6ADDRTOSTRING)(
	const IN6_ADDR *Addr,
	LPTSTR S
	);

typedef
__kernel_entry NTSTATUS
(NTAPI *NTQUERYSYSTEMTIME)(
	OUT PLARGE_INTEGER SystemTime
	);

BOOLEAN  IsListEmpty(PLIST_ENTRY ListHead)
{
	return (BOOLEAN)(ListHead == ListHead->Flink);
}

ULONG InitMofProxy()
{
	IWbemLocator *ppv = NULL;
	HRESULT hInstance;

	OLECHAR *lpServiceName = NULL;

	lpServiceName = SysAllocString(L"root\\wmi");

	if (g_pProxy)
		return S_OK;

	CoInitializeEx(0, 0);
	hInstance = CoCreateInstance(rclsid, 0, 1, riid, (LPVOID *)&ppv);

	if (!hInstance)
	{
		hInstance = ppv->ConnectServer(lpServiceName, NULL, NULL, NULL, 0L, NULL, NULL, &g_pProxy);

		if (!hInstance)
		{
			hInstance = CoSetProxyBlanket((IUnknown *)g_pProxy, 0xa, 0, 0, 4, 3, 0, 0);
		}
	}

	SysFreeString(lpServiceName);

	if (ppv)
	{
		ppv->Release();
	}

	return hInstance;
}

CAntiyMonDataEntry *GetEventCategoryClass(PEVENT_TRACE pEvent)
{
	HRESULT hr;
	IEnumWbemClassObject *lpEnumClass = NULL;
	IEnumWbemClassObject *lpEnumClassClass = NULL;

	IWbemClassObject *lpClass = NULL;
	//IWbemClassObject *lpClass_v15 = NULL;
	IWbemClassObject *lpClass_v9 = NULL;

	IWbemQualifierSet *lpQualifierSet = NULL;

	ULONG dwCnt = 1;
	ULONG dwCnt_v37;

	SHORT wVersion;
	VARIANT varClass;
	VARIANT varVersion;
	VARIANT varGuid;

	IWbemClassObject* pEventCategoryClass = NULL;
	IWbemClassObject* pEventClass = NULL;
	PBYTE pEventData = NULL;
	PBYTE pEndOfEventData = NULL;
	PROPERTY_LIST* pProperties = NULL;
	DWORD PropertyCount = 0;
	LONG* pPropertyIndex = NULL;
	CAntiyMonDataEntry *lpMofData = NULL;

	VariantInit(&varClass);
	VariantInit(&varVersion);
	VariantInit(&varGuid);

	do
	{
		if (!g_pProxy && InitMofProxy())
		{
			break;
		}

		hr = g_pProxy->CreateClassEnum(_bstr_t(L"EventTrace"), WBEM_FLAG_DEEP | WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_USE_AMENDED_QUALIFIERS, NULL, &lpEnumClass);
		if (hr != S_OK)
			break;

		try
		{
			do
			{
				if (lpClass)
				{
					lpClass->Release();
					lpClass = NULL;
				}

				if (lpEnumClass->Next(0x1388, 1, &lpClass, &dwCnt))
					continue;
				if (dwCnt != 1)
					break;
				if (lpClass->Get(L"__CLASS", 0, &varClass, NULL, NULL))
					continue;
				g_pProxy->CreateClassEnum(varClass.bstrVal, 0x20001, NULL, &lpEnumClassClass);
				VariantClear(&varClass);
				dwCnt_v37 = 1;

				do
				{
					lpClass_v9 = NULL;
					if (lpEnumClassClass)
					{
						if (lpEnumClassClass->Next(0x1388, 1, &lpClass_v9, &dwCnt_v37))
							continue;
						if (dwCnt_v37 != 1)
							break;
					}
					else
					{
						lpClass_v9 = lpClass;
						dwCnt_v37 = 1;
					}

					hr = lpClass_v9->Get(L"__CLASS", 0, &varClass, NULL, NULL);
					VariantClear(&varClass);
					if (hr == S_OK)
					{
						if (lpQualifierSet)
						{
							lpQualifierSet->Release();
							lpQualifierSet = NULL;
						}
						lpClass_v9->GetQualifierSet(&lpQualifierSet);
						if (lpQualifierSet->Get(L"Guid", 0, &varGuid, NULL) == S_OK)
						{
							WCHAR lpBuffer[0x104] = { 0x00 };
							WCHAR wGuidStr[50] = { 0x00 };
							wcsncpy_s(lpBuffer, 0x104, varGuid.bstrVal, 0xffffffff);
							VariantClear(&varGuid);
							//StringFromGUID2(pEvent->Header.Guid, wGuidStr, sizeof(wGuidStr));
							swprintf_s(wGuidStr, L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", pEvent->Header.Guid.Data1, pEvent->Header.Guid.Data2, pEvent->Header.Guid.Data3, pEvent->Header.Guid.Data4[0], pEvent->Header.Guid.Data4[1], pEvent->Header.Guid.Data4[2], pEvent->Header.Guid.Data4[3], pEvent->Header.Guid.Data4[4], pEvent->Header.Guid.Data4[5], pEvent->Header.Guid.Data4[6], pEvent->Header.Guid.Data4[7]);

							if (_wcsicmp(wGuidStr, lpBuffer) == 0)
							{
								if (lpQualifierSet->Get(L"EventVersion", 0, &varVersion, NULL))
								{
									lpMofData = _getdata(lpClass_v9, pEvent);
								}
								else
								{
									VariantChangeType(&varVersion, &varVersion, 0, 2u);
									wVersion = varVersion.iVal;
									VariantClear(&varVersion);
									if (wVersion != pEvent->Header.Class.Version)
										continue;
									lpMofData = _getdata(lpClass_v9, pEvent);

								}

								break;
							}

						}
					}

				} while (dwCnt_v37 == 1);
				if (lpEnumClassClass)
				{
					lpEnumClassClass->Release();
					lpEnumClassClass = NULL;
				}

			} while (dwCnt == 1);
		}
		catch (const std::exception&)
		{

		}
		if (lpEnumClass)
		{
			lpEnumClass->Release();
			lpEnumClass = NULL;
		}

	} while (FALSE);
	VariantClear(&varVersion);
	VariantClear(&varGuid);

	if (lpClass)
	{
		lpClass->Release();
		lpClass = NULL;
	}
	if (lpClass_v9)
	{
		lpClass_v9->Release();
		lpClass_v9 = NULL;
	}
	if (lpQualifierSet)
	{
		lpQualifierSet->Release();
		lpQualifierSet = NULL;
	}

	return lpMofData;

}

void TraverLinklist(LIST_ENTRY *Head)
{
	LIST_ENTRY *pEntry = NULL;
	ULONG i = 0;
	if (!Head)
		return;
	pEntry = Head->Flink;
	while (pEntry != Head)
	{
		pEntry = pEntry->Flink;
		++i;
	}
	//printf("List Count is %d\n", i);

}

CAntiyMonDataEntry *WINAPI ResolveEventTrace(PEVENT_TRACE pEvent)
{
	CAntiyMonDataEntry *lpMofData = NULL;
	WCHAR ClassGuid[50];

	//清空上一次保存的信息
	CAntiyEventTraceData::ShareInstance()->Clear();

	StringFromGUID2(pEvent->Header.Guid, ClassGuid, sizeof(ClassGuid));
	

	//printf("Time is %ws\n", CAntiyEventTraceData::ShareInstance()->m_Time);
	do
	{
		if (!g_MofEventListCount)
		{
			CAntiyMonDataEntry *pDataEntry = new CAntiyMonDataEntry((GUID *)&EventTraceGuid, -1, 0, 0);
			if (pDataEntry)
			{
				pDataEntry->InitData(&pDataEntry->m_ClassData, _bstr_t(L"EventTrace"));
				pDataEntry->InitData(&pDataEntry->m_TypeData, _bstr_t(L"Header"));

				CAntiyMonLinkList *pNewList = g_MofLinkListHeader->InsertHeader(g_MofLinkListHeader->m_Entry.Blink, pDataEntry);

				if ((unsigned int)(MAX_MOF_LIST_COUNT - g_MofEventListCount) < 1)
				{
					printf("List too long!\n");
				}

				++g_MofEventListCount;

				g_MofLinkListHeader->m_Entry.Blink = &pNewList->m_Entry;
				pNewList->m_Entry.Blink->Flink = &pNewList->m_Entry;

			}
		}

		if (!IsListEmpty(&g_MofLinkListHeader->m_Entry))
		{
			TraverLinklist(&g_MofLinkListHeader->m_Entry);
			LIST_ENTRY *pEntry = g_MofLinkListHeader->m_Entry.Flink;
			CAntiyMonDataEntry *pDataEntry_2 = NULL;
			CAntiyMonDataEntry *pDataEntry_3 = NULL;

			while (TRUE)
			{
				CAntiyMonLinkList *pList = CONTAINING_RECORD(pEntry, CAntiyMonLinkList, m_Entry);
				CAntiyMonDataEntry *pDataEntry_1 = (CAntiyMonDataEntry *)pList->m_pDataEvent;

				int dwGuidLen = 12;
				if (pDataEntry_1)
				{
					LONG *pl1 = (LONG *)&pEvent->Header.Guid;
					LONG *pl2 = (LONG *)&pDataEntry_1->m_Guid;
					while (*pl1 == *pl2)
					{
						pl1++;
						pl2++;

						BOOL isEnd = dwGuidLen < 4;
						dwGuidLen -= 4;
						if (isEnd)
						{
							if (pDataEntry_1->m_Type == pEvent->Header.Class.Type && pDataEntry_1->m_Version == pEvent->Header.Class.Version)
							{
								lpMofData = pDataEntry_1;
								return lpMofData;
							}


							break;
						}
					}

				}
				pEntry = pEntry->Flink;
				if (pEntry != &g_MofLinkListHeader->m_Entry)
					continue;

				break;

			}
		}

		if (!lpMofData)
		{
			lpMofData = GetEventCategoryClass(pEvent);
			if (!lpMofData)
			{
				lpMofData = new CAntiyMonDataEntry(&pEvent->Header.Guid, pEvent->Header.Class.Type, pEvent->Header.Class.Version, pEvent->Header.Class.Level);
				if (lpMofData)
				{
					lpMofData->m_ClassData = NULL;
					lpMofData->m_TypeData = NULL;
					lpMofData->m_PropertyList = NULL;
					lpMofData->m_PropertyCount = 0;
				}

			}
		}

	} while (FALSE);

	return lpMofData;

}


//创建Property链表
void InsertProperty(CAntiyMonLinkList *pList, BSTR bName, LONG ArraySize, LONG Length)
{
	LIST_ENTRY *pEntry = NULL;
	
	if (pList)
	{
		pEntry = pList->m_Entry.Flink;
		while (pEntry != &pList->m_Entry)
		{
			CAntiyMonLinkList *pLinkList = CONTAINING_RECORD(pEntry, CAntiyMonLinkList, m_Entry);
			if (pLinkList)
			{
				CAntiyMonDataEntry *pDataEntry = (CAntiyMonDataEntry *)pLinkList->m_pDataEvent;
				if (pDataEntry)
				{
					CAntiyMonProperty *pProperty = new CAntiyMonProperty(bName, ArraySize, Length);
					pDataEntry->m_PropertyCount++;
					CAntiyMonLinkList *NewList = pDataEntry->m_PropertyList->InsertHeader(pDataEntry->m_PropertyList->m_Entry.Blink, pProperty);
					NewList->m_Entry.Blink->Flink = &NewList->m_Entry;
					pDataEntry->m_PropertyList->m_Entry.Blink = &NewList->m_Entry;
				}
			}
			pEntry = pEntry->Flink;
		}
	}
}

//将每次循环生成的链表，插入到全局链表中，并且将局部链表的结点释放
CAntiyMonLinkList *InsertEvent(CAntiyMonLinkList *pTmpList)
{
	CAntiyMonLinkList *pRetList = pTmpList;
	LIST_ENTRY *pEntry = NULL;

	if (pTmpList)
	{
		pEntry = pTmpList->m_Entry.Flink;
		while (pEntry != &pTmpList->m_Entry)
		{
			CAntiyMonLinkList *pList = CONTAINING_RECORD(pEntry, CAntiyMonLinkList, m_Entry);
			if (pList)
			{
				CAntiyMonDataEntry *pDataEntry = (CAntiyMonDataEntry *)pList->m_pDataEvent;
				CAntiyMonLinkList *NewList = g_MofLinkListHeader->InsertHeader(g_MofLinkListHeader->m_Entry.Blink, pDataEntry);
				g_MofLinkListHeader->m_Entry.Blink = &NewList->m_Entry;
				NewList->m_Entry.Blink->Flink = &NewList->m_Entry;

				g_MofEventListCount++;
			}
			pEntry = pEntry->Flink;
		}

		pEntry = pTmpList->m_Entry.Flink;

		InitListEntry(&pTmpList->m_Entry);

		while (pEntry != &pTmpList->m_Entry)
		{
			CAntiyMonLinkList *pList = CONTAINING_RECORD(pEntry, CAntiyMonLinkList, m_Entry);
			pRetList = pList;
			pEntry = pEntry->Flink;

			if (pList)
			{
				delete pList;
				pList = NULL;
			}
		}

		return pRetList;

	}
	return NULL;
}

//这里的函数名暂时这么命名，实际用途可能不是
ULONG GetDataLocationByCimtype(int CimType, IWbemQualifierSet *lpWbemQualifierSet)
{
	ULONG result = 0;
	HRESULT hr;
	VARIANT varg;
	wchar_t mofData[0x0A];
	wchar_t lpTerminateBuffer[0x1e];
	wchar_t lpExtensionBuffer[0x1e];

	BOOL bIsPointer = FALSE;

	if (!lpWbemQualifierSet)
		return 0;
	hr = lpWbemQualifierSet->Get(L"format", 0, &varg, NULL);
	if (hr == S_OK && varg.lVal)
		wcscpy_s(mofData, 0xau, varg.bstrVal);
	VariantClear(&varg);
	hr = lpWbemQualifierSet->Get(L"StringTermination", 0, &varg, NULL);
	if (hr == S_OK && varg.lVal)
		wcscpy_s(lpTerminateBuffer, 0x1e, varg.bstrVal);
	VariantClear(&varg);
	hr = lpWbemQualifierSet->Get(L"pointer", 0, &varg, NULL);
	if (hr == S_OK)
		bIsPointer = TRUE;
	VariantClear(&varg);

	hr = lpWbemQualifierSet->Get(L"extension", 0, &varg, NULL);
	if (hr == S_OK && varg.lVal)
		wcscpy_s(lpExtensionBuffer, 0x1e, varg.bstrVal);
	VariantClear(&varg);

	switch (CimType & (~CIM_FLAG_ARRAY))
	{
	case CIM_SINT8:
	{
		result = 3;
		if (!_wcsicmp(mofData, L"c"))
			result = 0;
		goto LABEL_52;

	}
	break;
	case CIM_UINT8:
	{
		result = 2;
		goto LABEL_52;

	}
	break;
	case CIM_SINT16:
	{

		result = 4;
		goto LABEL_52;

	}
	break;
	case 0x12u:
		result = 5;
		goto LABEL_52;
	case 3u:
		goto LABEL_20;
	case 0x13u:
		result = 7;
		if (!_wcsicmp(mofData, L"x"))
			result = 8;
		goto LABEL_52;
	case 0x14u:
		result = 9;
		goto LABEL_52;
	case 0x15u:
		result = 10;
		goto LABEL_52;
	case 4u:
		result = 11;
		goto LABEL_52;
	case 5u:
		result = 12;
		goto LABEL_52;
	case 0xBu:
		result = 26;
		goto LABEL_52;
	case 8u:
		if (_wcsicmp(lpTerminateBuffer, L"NullTerminated"))
		{
			if (_wcsicmp(lpTerminateBuffer, L"Counted"))
			{
				if (_wcsicmp(lpTerminateBuffer, L"ReverseCounted"))
					result = _wcsicmp(lpTerminateBuffer, L"NotCounted") != 0 ? 13 : 23;
				else
					result = 18 - (_wcsicmp(mofData, L"w") != 0);
			}
			else
			{
				result = 16 - (_wcsicmp(mofData, L"w") != 0);
			}
		}
		else
		{
			result = 14 - (_wcsicmp(mofData, L"w") != 0);
		}
		goto LABEL_52;
	case 0x67u:
		result = 1;
		goto LABEL_52;
	case CIM_OBJECT:
		if (!_wcsicmp(lpExtensionBuffer, L"Port"))
		{
			result = 21;
			goto LABEL_52;
		}
		if (!_wcsicmp(lpExtensionBuffer, L"IPAddr"))
			goto LABEL_47;
		if (_wcsicmp(lpExtensionBuffer, L"Sid"))
		{
			if (_wcsicmp(lpExtensionBuffer, L"Guid"))
			{
				if (!_wcsicmp(lpExtensionBuffer, L"SizeT"))
				{
				LABEL_20:
					result = 6;
					goto LABEL_52;
				}
				if (_wcsicmp(lpExtensionBuffer, L"IPAddrV6"))
				{
					if (_wcsicmp(lpExtensionBuffer, L"IPAddrV4"))
					{
						if (_wcsicmp(lpExtensionBuffer, L"WmiTime"))
							result = bIsPointer ? 1 : 0;
						else
							result = 28;
					}
					else
					{
					LABEL_47:
						result = 20;
					}
				}
				else
				{
					result = 27;
				}
			}
			else
			{
				result = 25;
			}
		}
		else
		{
			result = 19;
		}
	LABEL_52:
		if (bIsPointer)
			result = 24;
		break;
	default:
		result = 29;
	}


	return result;
}

ULONG GetMax(IWbemQualifierSet *lpQualifierSet)
{
	ULONG result = 1;
	VARIANT varg;
	HRESULT hr = NULL;

	if (lpQualifierSet)
	{
		VariantInit(&varg);
		hr = lpQualifierSet->Get(L"MAX", 0, &varg, NULL);
		if (hr == S_OK && varg.vt == 3)
			result = varg.lVal;
		VariantClear(&varg);
	}

	return result;

}

void InsertHeader(CAntiyMonLinkList *listNode, void *pData, ULONG *Reference)
{
	CAntiyMonLinkList *NewLinkListNode = listNode->InsertHeader(listNode->m_Entry.Blink, pData);
	if (0x15555554 - (*Reference) < 1)
	{
		printf("List too long!\n");
	}

	(*Reference)++;
	NewLinkListNode->m_Entry.Blink->Flink = &NewLinkListNode->m_Entry;
	listNode->m_Entry.Blink = &NewLinkListNode->m_Entry;
}



CAntiyMonDataEntry * __stdcall _getdata(IWbemClassObject *lpClass, PEVENT_TRACE lpTrace)
{
	WCHAR lpEventName[MAX_PATH] = { 0x00 };
	WCHAR lpClassName[MAX_PATH] = { 0x00 };
	HRESULT hr;
	IEnumWbemClassObject *lpEnumClass = NULL;

	IWbemQualifierSet *lpQualifierSet = NULL;
	IWbemQualifierSet *lpQualifierClassSet = NULL;

	IWbemClassObject *lpClass_class = NULL;
	VARIANT varClass;
	VARIANT varDisplayName;
	VARIANT varEventType;
	VARIANT varEventName;

	SAFEARRAY *lpWmiDatas = NULL;

	ULONG dwCnt;

	LONG dwLNameBound = 0;
	LONG dwUNameBound = 0;

	PVOID lpEventTypeData = NULL;
	PVOID lpEventNameData = NULL;
	PVOID lpEventTypeBuffer = NULL;

	CAntiyMonLinkList *pEventLinkList = new CAntiyMonLinkList(0, 0);
	CAntiyMonLinkList *pEventLinkList_2 = NULL;
	ULONG dwReference = 1;

	CAntiyMonDataEntry* lpMofData = NULL;
	CAntiyMonDataEntry *lpOldMofData;
	CAntiyMonDataEntry *lpMofData_3 = NULL;

	VariantInit(&varEventType);
	VariantInit(&varEventName);

	if (lpClass->Get(L"__CLASS", 0, &varClass, NULL, NULL) == S_OK)
	{
		wcsncpy_s(lpClassName, MAX_PATH, varClass.bstrVal, 0xffffffff);
		if (lpQualifierSet)
		{
			lpQualifierSet->Release();
			lpQualifierSet = NULL;
		}
		lpClass->GetQualifierSet(&lpQualifierSet);
		if (lpQualifierSet->Get(L"DisplayName", 0, &varDisplayName, NULL) == S_OK && varDisplayName.lVal)
		{
			wcscpy_s(lpClassName, MAX_PATH, varDisplayName.bstrVal);
		}

		lpMofData = new CAntiyMonDataEntry(&lpTrace->Header.Guid, -1, -1, -1);
		lpOldMofData = lpMofData_3 = lpMofData;
		if (lpMofData)
		{
			lpMofData->InitData(&lpMofData->m_ClassData, lpClassName);
			pEventLinkList_2 = pEventLinkList->InsertHeader(pEventLinkList->m_Entry.Blink, lpMofData);

			pEventLinkList_2->m_Entry.Blink->Flink = &pEventLinkList_2->m_Entry;
			pEventLinkList->m_Entry.Blink = &pEventLinkList_2->m_Entry;

			hr = g_pProxy->CreateClassEnum(varClass.bstrVal, 0x20001, 0, &lpEnumClass);
			if (hr == S_OK)
			{
				dwCnt = 1;
				while (1)
				{
					lpClass_class = NULL;
					if (lpEnumClass->Next(5000, 1, &lpClass_class, &dwCnt) != S_OK)
					{
						lpClass_class = lpClass;
						lpClass_class->AddRef();
					}
					if (lpQualifierClassSet)
					{
						lpQualifierClassSet->Release();
						lpQualifierClassSet = NULL;
					}
					lpClass_class->GetQualifierSet(&lpQualifierClassSet);
					VariantClear(&varEventType);
					if (lpQualifierClassSet->Get(L"EventType", 0, &varEventType, NULL) == S_OK)
					{
						if (varEventType.vt & 0x2000)
						{
							SAFEARRAY *lpSafeEventTypes = NULL;
							SAFEARRAY *lpSafeEventNames = NULL;
							LONG dwUbound = 0;
							LONG dwSafeEventTypeCnt = 0;
							LONG dwLbound = 0;
							lpSafeEventTypes = varEventType.parray;
							dwSafeEventTypeCnt = varEventType.lVal;

							VariantClear(&varEventName);
							if (lpQualifierClassSet->Get(L"EventTypeName", 0, &varEventName, NULL) == S_OK && varEventName.vt & 0x2000)
								lpSafeEventNames = varEventName.parray;
							if (!lpSafeEventTypes
								||
								SafeArrayGetUBound(lpSafeEventTypes, 1u, &dwUbound)
								||
								SafeArrayGetLBound(lpSafeEventTypes, 1u, &dwLbound)
								||
								dwUbound < 0)
							{
							LABEL_80:
								lpMofData = lpMofData_3;
								break;
							}
							SafeArrayAccessData(lpSafeEventTypes, (void **)&lpEventTypeData);
							if (lpSafeEventNames)
							{
								if (SafeArrayGetLBound(lpSafeEventNames, 1u, &dwLNameBound)
									||
									SafeArrayGetUBound(lpSafeEventNames, 1u, &dwUNameBound)
									||
									dwUNameBound < 0)
									goto LABEL_80;
								SafeArrayAccessData(lpSafeEventNames, (void **)&lpEventNameData);
							}

							if (dwLbound <= dwUbound)
							{
								LONG rgsIndex = dwLbound;
								while (1)
								{
									do
									{
										USHORT ucDataType = *((USHORT *)((LONG *)lpEventTypeData + rgsIndex));

										try
										{
											CAntiyMonDataEntry *pDataEntry_3 = new CAntiyMonDataEntry(&lpTrace->Header.Guid, ucDataType, lpTrace->Header.Class.Version, lpTrace->Header.Class.Level);
											if (pDataEntry_3)
											{
												InsertHeader(pEventLinkList, pDataEntry_3, &dwReference);
												if (lpOldMofData->GetClassDataLen())
												{
													*pDataEntry_3 = lpOldMofData;
												}

												if (ucDataType == lpTrace->Header.Class.Type)
												{
													lpMofData_3 = pDataEntry_3;
												}

												if (lpSafeEventNames)
												{
													if (rgsIndex < dwLNameBound || rgsIndex > dwUNameBound)
													{
														break;
													}
													pDataEntry_3->InitData(&pDataEntry_3->m_TypeData, ((BSTR *)lpEventNameData)[rgsIndex]);
												}

											}
										}
										catch (...)
										{

										}

									} while (FALSE);

									++rgsIndex;
									if (rgsIndex > dwUbound)
									{
										lpSafeEventTypes = varEventType.parray;
										break;
									}
								}
							}

							SafeArrayUnaccessData(lpSafeEventTypes);
							SafeArrayDestroy(lpSafeEventTypes);
							VariantInit(&varEventType);

							if (lpSafeEventNames)
							{
								SafeArrayUnaccessData(lpSafeEventNames);
								SafeArrayDestroy(lpSafeEventNames);
								VariantInit(&varEventName);
							}

						}
						else
						{
							VariantChangeType(&varEventType, &varEventType, 0, 2u);
							USHORT iEventType = varEventType.iVal;
							VariantClear(&varEventName);
							if (lpQualifierClassSet->Get(L"EventTypeName", 0, &varEventName, NULL) != S_OK)
							{
								lpEventName[0] = 0;
							}
							else
							{
								wcscpy_s(lpEventName, 0x104, varEventName.bstrVal);
							}

							CAntiyMonDataEntry *pDataEntry_3 = new CAntiyMonDataEntry(&lpTrace->Header.Guid, iEventType, lpTrace->Header.Class.Version, lpTrace->Header.Class.Level);
							if (pDataEntry_3)
							{
								InsertHeader(pEventLinkList, pDataEntry_3, &dwReference);
								if (lpOldMofData->GetClassDataLen())
								{
									*pDataEntry_3 = lpOldMofData;
								}
								if (lpTrace->Header.Class.Type == iEventType)
								{
									lpMofData_3 = pDataEntry_3;
								}
								pDataEntry_3->InitData(&pDataEntry_3->m_TypeData, lpEventName);
							}

						}
					}

					ULONG i = 1;
					VARIANT varWmiDataId;
					varWmiDataId.vt = 3;
					varWmiDataId.lVal = 1;
					LONG dwWmiLbound = 0;
					LONG dwWmiUbound = 0;
					SAFEARRAY *lpWmiDatas = NULL;
					BSTR bWmiString = NULL;

					while ((hr = lpClass_class->GetNames(L"WmiDataId", 3, &varWmiDataId, &lpWmiDatas)) == S_OK)
					{
						if (SafeArrayGetLBound(lpWmiDatas, 1u, &dwWmiLbound) || SafeArrayGetUBound(lpWmiDatas, 1u, &dwWmiUbound) || dwWmiUbound < 0)
							break;

						CIMTYPE CimType = 0;
						ULONG dwDataOffset = 0;
						ULONG dwMaxOffset = 0;

						if (dwWmiLbound <= dwWmiUbound)
						{
							LONG rgIndices = dwWmiLbound;
							do
							{
								if (SafeArrayGetElement(lpWmiDatas, &rgIndices, &bWmiString) || lpClass_class->Get(bWmiString, 0, 0, &CimType, 0))
									break;
								if (lpQualifierSet)
								{
									lpQualifierSet->Release();
									lpQualifierSet = NULL;
								}
								if (lpClass_class->GetPropertyQualifierSet(bWmiString, &lpQualifierSet))
									break;

								//*(*pPropertyIndex + varWmiDataId.intVal) = rgIndices;

								dwDataOffset = GetDataLocationByCimtype(CimType, lpQualifierSet);

								if (CimType & CIM_FLAG_ARRAY)
									dwMaxOffset = GetMax(lpQualifierSet);
								else
									dwMaxOffset = 1;
								//printf("ArrayValue is %d,Max is %d,Name is %ws\n",dwDataOffset,dwMaxOffset,pProperty->Name);

								InsertProperty(pEventLinkList, bWmiString, dwMaxOffset, dwDataOffset);

								++rgIndices;

							} while (rgIndices <= dwWmiUbound);

						}
						SafeArrayDestroy(lpWmiDatas);
						lpWmiDatas = NULL;
						//VariantClear(&varWmiDataId);
						//lpWmiDatas = NULL;
						varWmiDataId.lVal++;
					}

					InsertEvent(pEventLinkList);

					if (dwCnt != 1)
						break;
				}
			}
		}
	}

	VariantClear(&varClass);
	VariantClear(&varDisplayName);
	VariantClear(&varEventType);
	VariantClear(&varEventName);

	InsertEvent(pEventLinkList);

	LIST_ENTRY *pEntry = pEventLinkList->m_Entry.Flink;

	InitListEntry(&pEventLinkList->m_Entry);

	while (pEntry != &pEventLinkList->m_Entry)
	{
		CAntiyMonLinkList *pList = CONTAINING_RECORD(pEntry, CAntiyMonLinkList, m_Entry);

		pEntry = pEntry->Flink;

		if (pList)
		{
			delete pList;
			pList = NULL;
		}
	}

	return lpMofData;
}


BOOL wcstrstr(WCHAR *source, ULONG sLen, WCHAR *des, ULONG *index)
{
	ULONG i = 0;
	if (!source || !des)
		return FALSE;

	while (i < sLen)
	{
		if (_wcsnicmp(source + i, des, 2) == 0)
		{
			*index = i;
			return TRUE;
		}
		++i;
	}

	return FALSE;
}


