// AddToResourceTool.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <WinUser.h>



int main(int argc,char *argv[])
{
	WCHAR *pwstrFilePath = NULL;
	ULONG dwPstrFilePathLen = 0;
	HANDLE hFileModule = NULL;
	BOOL bResult = FALSE;
	ULONG data = 0;
	HANDLE hSys = NULL;
	BYTE pSysData[0x1000] = {0x00};
	WCHAR *pwstrSysPath = NULL;
	ULONG dwSysPathLen = 0;
	BYTE *pTotalSysData = NULL;
	ULONG dwSysDataSize = 0;
	ULONG dwRealReadLen = 0;
	BYTE *p;

	while (argc >= 3)
	{
		dwPstrFilePathLen = MultiByteToWideChar(0, 0, argv[1], strlen(argv[1]), NULL, 0);
		if (!dwPstrFilePathLen)
		{
			printf("MultiByteToWideChar fail:%d!\n", GetLastError());
			break;
		}

		pwstrFilePath = (WCHAR *)malloc((dwPstrFilePathLen + 1) * sizeof(WCHAR));
		if (!pwstrFilePath)
		{
			printf("Alloc WideChar fail:%d!\n", GetLastError());
			break;
		}
		memset(pwstrFilePath, 0, (dwPstrFilePathLen + 1) * sizeof(WCHAR));

		MultiByteToWideChar(0, 0, argv[1], strlen(argv[1]), pwstrFilePath, dwPstrFilePathLen);

		hFileModule = BeginUpdateResource(pwstrFilePath, FALSE);
		if (!hFileModule)
		{
			printf("LoadLibrary fail:%d!\n", GetLastError());
			break;
		}

		dwSysPathLen = MultiByteToWideChar(0, 0, argv[2], strlen(argv[2]), NULL, 0);
		++dwSysPathLen;
		dwSysPathLen *= sizeof(WCHAR);
		pwstrSysPath = (WCHAR *)malloc(dwSysPathLen);
		memset(pwstrSysPath, 0, dwSysPathLen);
		if (!pwstrSysPath)
		{
			printf("Alloc SysPath fail!\n");
			break;
		}

		MultiByteToWideChar(0, 0, argv[2], strlen(argv[2]), pwstrSysPath, (dwSysPathLen - 1) / sizeof(WCHAR));

		hSys = CreateFile(pwstrSysPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hSys == INVALID_HANDLE_VALUE)
			break;
		do 
		{
			if (!ReadFile(hSys, pSysData, sizeof(pSysData), &dwRealReadLen, NULL) || !dwRealReadLen)
			{
				break;
			}
			dwSysDataSize += dwRealReadLen;
		} while (1);
		

		pTotalSysData = (BYTE *)malloc(dwSysDataSize);
		if(!pTotalSysData)
			break;
		CloseHandle(hSys);
		hSys = CreateFile(pwstrSysPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hSys == INVALID_HANDLE_VALUE)
			break;

		p = pTotalSysData;
		while (ReadFile(hSys, pSysData, sizeof(pSysData),&dwRealReadLen, NULL))
		{
			if(!dwRealReadLen)
				break;
			memmove_s(p, sizeof(pSysData), pSysData, dwRealReadLen);
			p += dwRealReadLen;
		}

		bResult = UpdateResource(hFileModule, L"BIN", L"sys", 0, pTotalSysData, dwSysDataSize);
		if (!bResult)
		{
			printf("Write Resource fail:%d!\n", GetLastError());
		}
		

		break;
	}

	if (hFileModule)
		EndUpdateResource(hFileModule, FALSE);
	if (pwstrFilePath)
		free(pwstrFilePath);
	if (pTotalSysData)
		free(pTotalSysData);
	if(hSys)
		CloseHandle(hSys);

	printf("Execute Loging...!\n");
	
	getchar();
}

