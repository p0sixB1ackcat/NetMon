#pragma once
#include <windows.h>

class CAntiyResource
{
public:
	CAntiyResource();

	static BOOL ReleasePeResourceFileToSystem(wchar_t *lpName, wchar_t *lpType);

	~CAntiyResource();
};

