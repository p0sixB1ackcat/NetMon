#include "AntiyResource.h"
#include <winbase.h>
#include <mbstring.h>
#include <stdio.h>
#include <tchar.h>
#include <mbstring.h>

CAntiyResource::CAntiyResource()
{
}


BOOL GetSystemRootPath(WCHAR *pbBuffer, ULONG cbBuffer)
{
	if (!pbBuffer || !cbBuffer)
		return FALSE;
	return ExpandEnvironmentStrings(L"%systemroot%",pbBuffer,cbBuffer) ? TRUE : FALSE;
}

 BOOL CAntiyResource::ReleasePeResourceFileToSystem(wchar_t *lpName, wchar_t *lpType)
{
	 BOOL bResult = FALSE;
	 HRSRC hr;
	 HGLOBAL hg;
	 ULONG dwSizeOfResource = 0;
	 void *pResource = NULL;
	 FILE *pFileSystem = NULL;
	 WCHAR pSystemPath[MAX_PATH] = {0x00};
	 

	 hr = FindResource(NULL,lpName,lpType);
	 while (hr)
	 {
		 hg = LoadResource(0, hr);
		 dwSizeOfResource = SizeofResource(0, hr);
		 pResource = LockResource(hg);

		 bResult = GetSystemRootPath(pSystemPath, sizeof(pSystemPath));
		 if(!bResult)
			 break;
		 
		 wcscat_s(pSystemPath, L"\\AntiyMonDrv.sys");
		 pFileSystem =  _wfopen(pSystemPath, L"wb");
		 if (!pFileSystem)
		 {
			 bResult = GetLastError();
			 break;
		 }

		 fwrite(pResource,1,dwSizeOfResource,pFileSystem);
		 fclose(pFileSystem);
		 break;
	 }

	 if (!hr)
	 {
		 bResult = GetLastError();
	 }
	 	 
	 return bResult;
}

CAntiyResource::~CAntiyResource()
{
}
