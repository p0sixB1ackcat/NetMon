
// AntiyMonitor-MFC.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CAntiyMonitorMFCApp: 
// �йش����ʵ�֣������ AntiyMonitor-MFC.cpp
//

class CAntiyMonitorMFCApp : public CWinApp
{
public:
	CAntiyMonitorMFCApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CAntiyMonitorMFCApp theApp;