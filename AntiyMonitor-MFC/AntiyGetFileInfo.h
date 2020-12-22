#pragma once
#include <windows.h>

typedef struct _LOCAL_FILE_INFO
{
	WCHAR m_FileVersion[MAX_PATH];
	WCHAR m_Description[MAX_PATH];
	WCHAR m_Company[MAX_PATH];
	WCHAR m_ProductName[MAX_PATH];
	WCHAR m_MD5[64];
	WCHAR m_Sha1[64];
	WCHAR m_Sha256[256];

}LOCAL_FILE_INFO,*PLOCAL_FILE_INFO;

#define ImagePathName L"D:\\Sublime Text 3\\Sublime Text 3\\sublime_text.exe"
#ifdef __cplusplus
extern "C"
{
#endif
	UINT ATGetFileVersion(WCHAR *szFilePath, LOCAL_FILE_INFO *pFileInfo);
	int ATGetFileAllInfo(WCHAR *szPath, LOCAL_FILE_INFO *pFileInfo);
	
#ifdef __cplusplus
}
#endif




