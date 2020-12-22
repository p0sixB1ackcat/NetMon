#include "AntiyGetFileInfo.h"
#include <stdio.h>
#include <iostream>
#include <atlstr.h>
#include <string>
#include <imagehlp.h>


typedef DWORD (APIENTRY *GETFILEVERSIONINFOSIZEEXW)(_In_ DWORD dwFlags, _In_ LPCWSTR lpwstrFilename, _Out_ LPDWORD lpdwHandle);

typedef BOOL (APIENTRY *GETFILEVERSIONINFOEXW)(_In_ DWORD dwFlags,
	_In_ LPCWSTR lpwstrFilename,
	_Reserved_ DWORD dwHandle,
	_In_ DWORD dwLen,
	_Out_writes_bytes_(dwLen) LPVOID lpData);


GETFILEVERSIONINFOSIZEEXW g_GetFileVersionInfoSizeExW = NULL;
GETFILEVERSIONINFOEXW g_GetFileVersionInfoExW = NULL;

#define KeyCount 4
const WCHAR *szKeys[] = {
	L"CompanyName"
	,L"FileDescription"
	,L"FileVersion"
	,L"ProductName"};

using namespace std;
#pragma comment(lib,"Imagehlp.lib")
//#pragma comment (lib,"Mincore.lib")

class ATFileInfoClass
{
public:
	ATFileInfoClass();
	ATFileInfoClass(WCHAR *InputBuffer, size_t BufferLength);
	BOOL AtStrStr(WCHAR *s1, WCHAR *s2);
	ULONG ATScanStringBuffer(WCHAR *szBuffer, size_t dwBufferLength, WCHAR *keywords);
	~ATFileInfoClass();
public:
	CString m_FileVersion;
	CString m_Description;
	CString m_Company;
	CString m_ProductName;
	CString m_Hash;

};

ATFileInfoClass::ATFileInfoClass()
{
	
}

BOOL ATFileInfoClass::AtStrStr(WCHAR *s1, WCHAR *s2)
{
	WCHAR *p1, *p2;
	p1 = s1;
	p2 = s2;
	BOOL bResult = FALSE;
	if (!p1 || !p2)
		return bResult;
	if (!*p1 || !*p2)
		return bResult;
	
	bResult = TRUE;
	while (*p1 && *p2)
	{
		if (*p1 != *p2)
		{
			bResult = FALSE;
			break;
		}
		++p1;
		++p2;
	}

	return bResult;
}

ULONG ATFileInfoClass::ATScanStringBuffer(WCHAR *szBuffer, size_t dwBufferLength, WCHAR *keywords)
{
	ULONG i = 0;
	BOOL bResult = FALSE;

	while (i < dwBufferLength)
	{
		if (szBuffer[i] == *keywords)
		{
			if (AtStrStr(&(szBuffer[i]), keywords))
			{
				bResult = TRUE;
				break;
			}
		}
		++i;
	}

	if (bResult)
		return i;

	return 0;
}

ATFileInfoClass::ATFileInfoClass(WCHAR *InputBuffer, size_t BufferLength)
{
	if (!InputBuffer || !BufferLength)
		return;
	ULONG i,dwIndex;
	dwIndex = 0;
	memset(m_Company.GetBuffer(), 0x00, m_Company.GetLength());
	memset(m_FileVersion.GetBuffer(), 0x00, m_FileVersion.GetLength());
	memset(m_Description.GetBuffer(), 0x00, m_Description.GetLength());
	memset(m_ProductName.GetBuffer(), 0x00, m_ProductName.GetLength());

	for (i = 0; i < KeyCount; ++i)
	{
		dwIndex = ATScanStringBuffer(InputBuffer, BufferLength, (WCHAR *)szKeys[i]);
		switch (i)
		{
		case 0:
		{
			m_Company = InputBuffer + dwIndex + wcslen(szKeys[i]) + 2;
		}
		break;
		case 1:
		{
			m_Description = InputBuffer + dwIndex + wcslen(szKeys[i]) + 2;
		}
		break;
		case 2:
		{
			m_FileVersion = InputBuffer + dwIndex + wcslen(szKeys[i]) + 2;
		}
		break;
		case 3:
		{
			m_ProductName = InputBuffer + dwIndex + wcslen(szKeys[i]) + 2;
		}
		break;
		}
		
	}
	

}

ATFileInfoClass::~ATFileInfoClass()
{
// 	if (m_FileVersion && m_FileVersion.GetString())
// 	{
// 		delete m_FileVersion;
// 	}
// 	if (m_Description && m_Description.GetString())
// 	{
// 		delete m_Description;
// 	}
// 	if (m_Company && m_Company.GetString())
// 	{
// 		delete m_Company;
// 	}
}

void ATGetUserName(WCHAR *szfilePath)
{
	WCHAR *szNameBuffer[MAX_PATH] = {0x00};
	size_t dwNameLen = sizeof(szNameBuffer);
	
}

UINT ATGetFileVersion(WCHAR *szFilePath,LOCAL_FILE_INFO *pFileInfo)
{
	UINT uResult = ERROR_SUCCESS;
	WCHAR *VersionBuf = NULL;
	ULONG dwBufferSize = 0;
	

	dwBufferSize = g_GetFileVersionInfoSizeExW(0,szFilePath,NULL);
	if (!dwBufferSize)
	{
		printf("GetFileVersionInfoSize fail:%d!\n",GetLastError());
		return -1;
	}
	ATFileInfoClass *c1;
	VersionBuf = (WCHAR *)malloc(sizeof(WCHAR) * dwBufferSize);
	if (g_GetFileVersionInfoExW(FILE_VER_GET_LOCALISED,szFilePath, NULL, dwBufferSize, VersionBuf))
	{
		c1 = new ATFileInfoClass(VersionBuf, dwBufferSize);
		CopyMemory(pFileInfo->m_FileVersion,c1->m_FileVersion.GetString(),sizeof(pFileInfo->m_FileVersion));
		CopyMemory(pFileInfo->m_Description,c1->m_Description.GetString(),sizeof(pFileInfo->m_Description));
		CopyMemory(pFileInfo->m_Company,c1->m_Company.GetString(),sizeof(pFileInfo->m_Company));
		CopyMemory(pFileInfo->m_ProductName, c1->m_ProductName.GetString(), sizeof(pFileInfo->m_ProductName));
		
	}

	if (VersionBuf)
	{
		free(VersionBuf);
		VersionBuf = NULL;
	}
 	
	return uResult;
}


typedef enum _HASH_TYPE
{
	HASH_MD5 = 0,
	HASH_SHA1,
	HASH_SHA256

}HASH_TYPE;
// 计算文件的MD5字符串，支持大文件(large file)
BOOL GetFileHash_1(const TCHAR *szFilename,__out unsigned char *outPut,__out PULONG outputLen,HASH_TYPE ht)
{
	HCRYPTPROV hCryptProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE hMapFile = 0; 
	DWORD dwHashLen = 0;
	ALG_ID dwCalg = 0;
	BYTE *pBuf = NULL; 
	DWORD dwFileSize = 0, dwHighFileSize= 0; 
	BOOL bResult = FALSE;
	HANDLE hFile = CreateFile(szFilename, GENERIC_READ, 7, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE) 
		return bResult;  
	//System、其他盘符下的文件，getfilesize获取不完整
	dwFileSize= GetFileSize(hFile, &dwHighFileSize);
	hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL); 

	if (hMapFile == NULL || hMapFile == INVALID_HANDLE_VALUE)
		goto release;

	switch (ht)
	{
		case HASH_MD5:
		{
			dwHashLen = 16;
			dwCalg = CALG_MD5;
		}
		break;
		case HASH_SHA1:
		{
			dwHashLen = 20;
			dwCalg = CALG_SHA1;
		}
		break;
		case HASH_SHA256:
		{
			dwHashLen = 32;
			dwCalg = CALG_SHA_256;
		}
		break;
	}
	
	if (CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)
		||
		CryptAcquireContext(&hCryptProv,NULL,NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if (CryptCreateHash(hCryptProv, dwCalg, 0, 0, &hHash)) 
		{
			UINT64 filesize = (UINT64)dwHighFileSize << 32 | dwFileSize;
			//filesize = 0x42e00;
			for (UINT64 i = 0; i<= filesize; i += 1048576)//1048576
			{ 
				size_t bufsize = size_t(min(filesize-i, 1048576));
				pBuf = (BYTE*)MapViewOfFile(hMapFile,FILE_MAP_READ,0,(DWORD)0,dwFileSize); 
				if(pBuf ==NULL) 
					goto release; 
				if(!CryptHashData(hHash,pBuf,bufsize,0))
					goto release; 
				UnmapViewOfFile(pBuf); 
			}
			if(CryptGetHashParam(hHash,HP_HASHVAL,(BYTE *)outPut,&dwHashLen,0)) 
			{ 
				*outputLen = dwHashLen;
				bResult = TRUE;
			}
			else
			{
				printf("error is 0x%x\n", GetLastError());
			}
		}
		else
		{
			printf("error is 0x%x\n", GetLastError());
		}
	} 
	else
	{
		printf("error is 0x%x\n", GetLastError());
	}
release:
	if (!*outPut)
	{
		printf("error is 0x%x\n",GetLastError());
	}
	if (hHash)
	{
		CryptDestroyHash(hHash);
	}
	CryptReleaseContext(hCryptProv,0); 
	UnmapViewOfFile(pBuf); 
	CloseHandle(hMapFile); 
	CloseHandle(hFile); 
	return bResult;
}

BOOL GetfileHash(const TCHAR *szFilename, __out unsigned char *outPut, __out PULONG outputLen, HASH_TYPE ht)
{
	BOOL bResult = FALSE;
	HCRYPTPROV hCryptProv = 0;
	HCRYPTHASH hHash = 0;
	HANDLE FileHandle = 0;
	ULONG dwHighLength = 0;
	ULONG dwLowLength = 0;
	unsigned __int64 dqFileSize = 0;
	BYTE Buffer[0x1000] = {0x00};
	ULONG NumberOfBytesRead = 0;
	ALG_ID dwCalg = 0;
	ULONG dwHashLen = 0;

	switch (ht)
	{
	case HASH_MD5:
	{
		dwHashLen = 16;
		dwCalg = CALG_MD5;
	}
	break;
	case HASH_SHA1:
	{
		dwHashLen = 20;
		dwCalg = CALG_SHA1;
	}
	break;
	case HASH_SHA256:
	{
		dwHashLen = 32;
		dwCalg = CALG_SHA_256;
	}
	break;
	}

	if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
		goto error;
	CryptCreateHash(hCryptProv, dwCalg, 0,0, &hHash);
	
	FileHandle = CreateFile(szFilename,0x80000000,7,0,3,0,0);
	dwLowLength = GetFileSize(FileHandle, &dwHighLength);
	dqFileSize = dwLowLength | ((unsigned __int64)dwHighLength << 32);
	if (dqFileSize)
	{
		memset(Buffer, 0, sizeof(Buffer));
		NumberOfBytesRead = 0;
		if (ReadFile(FileHandle, Buffer, 0x1000, &NumberOfBytesRead, 0))
		{
			do 
			{
				if(!NumberOfBytesRead)
					break;
				
				CryptHashData(hHash, Buffer, NumberOfBytesRead, 0);

			} while (ReadFile(FileHandle,Buffer,0x1000,&NumberOfBytesRead,0));
		}
		if (CryptGetHashParam(hHash, 2, outPut, &dwHashLen, 0))
		{
			*outputLen = dwHashLen;
			bResult = TRUE;
		}
	}

error:
	{
		if (!bResult)
		{
			printf("CryptAcquireContext fail:%d!\n", GetLastError());
		}
	}
ret:
	{
		if (hHash)
			CryptDestroyHash(hHash);
		CryptReleaseContext(hCryptProv, 0);
		CloseHandle(FileHandle);
		return bResult;
	}

}

int ATGetFileAllInfo(WCHAR *szPath, LOCAL_FILE_INFO *pFileInfo)
{
	HRESULT hr = ERROR_SUCCESS;
	PCCERT_CONTEXT CertContext;
	BYTE bytes[MAX_PATH];
	ULONG byteLen;
	ULONG i;
	HMODULE hDll = 0;
	HMODULE hMineCore = 0;
	ULONG errorCode = 0;

	if (!pFileInfo)
	{
		return 0;
	}

	
	if (!g_GetFileVersionInfoExW || !g_GetFileVersionInfoSizeExW)
	{
		hDll = LoadLibrary(L"api-ms-win-core-version-l1-1-0.dll");
		if (!hDll)
		{
			errorCode = GetLastError();
			MessageBox(0, L"LoadLibrary fail!\n", L"message", 0);
			return 0;
		}

		hMineCore = GetModuleHandle(L"api-ms-win-core-version-l1-1-0.dll");
		if (!hMineCore)
		{
			errorCode = GetLastError();
			FreeLibrary(hDll);
			MessageBox(0, L"GetModuleHandle fail!", L"message", 0);
			return 0;
		}

		g_GetFileVersionInfoExW = (GETFILEVERSIONINFOEXW)GetProcAddress(hMineCore, "GetFileVersionInfoExW");
		if (!g_GetFileVersionInfoExW)
		{
			errorCode = GetLastError();
		}

		g_GetFileVersionInfoSizeExW = (GETFILEVERSIONINFOSIZEEXW)GetProcAddress(hMineCore, "GetFileVersionInfoSizeExW");

		if (!g_GetFileVersionInfoSizeExW)
		{
			errorCode = GetLastError();
		}
		if (!g_GetFileVersionInfoExW || !g_GetFileVersionInfoSizeExW)
		{
			MessageBox(0, L"GetProcAddress fail!", L"message", 0);
			return 0;
		}
	}

	ATGetFileVersion((WCHAR *)szPath, pFileInfo);

	typedef struct st
	{
		const WCHAR *hashname;
		HASH_TYPE ht;
	};

	int j;
	st s[3] = { {L"MD5",HASH_MD5},{L"SHA1",HASH_SHA1},{L"SHA256",HASH_SHA256} };
	BYTE bHash[0x7f] = { 0 };
	ULONG paddingLen = 0;
	WCHAR *szSource = NULL;
	char *aHexHash = NULL;
	ULONG dwUhexHashLen = 0;

	for (i = 0; i < sizeof(s) / sizeof(st); ++i)
	{
		memset(bHash, 0x00, 0x7f);
		if (GetfileHash(szPath, bHash, &paddingLen, s[i].ht))
		{
			printf("%ws:", s[i].hashname);
			switch (s[i].ht)
			{
			case HASH_MD5:
			{
				szSource = pFileInfo->m_MD5;
			}
			break;
			case HASH_SHA1:
			{
				szSource = pFileInfo->m_Sha1;
			}
			break;
			case HASH_SHA256:
			{
				szSource = pFileInfo->m_Sha256;
			}
			break;

			}

			if (aHexHash)
			{
				free(aHexHash);
				aHexHash = NULL;
			}
			aHexHash = (char *)malloc(paddingLen * 2 + 1);

			for (j = 0; j < paddingLen; ++j)
			{
				sprintf((char *)aHexHash + j * 2, "%02X", *((unsigned char *)(char *)bHash + j));
			}

			aHexHash[paddingLen * 2] = '\0';

			dwUhexHashLen = MultiByteToWideChar(0, 0, aHexHash, -1, NULL, 0);

			MultiByteToWideChar(0, 0, aHexHash, paddingLen * 2, szSource, dwUhexHashLen * sizeof(WCHAR));

			szSource[dwUhexHashLen-1] = '\0';
		}

	}

	if (aHexHash)
	{
		free(aHexHash);
		aHexHash = NULL;
	}

	return 1;
}
