
// GISViewer.h : GISViewer ���� ���α׷��� ���� �� ��� ����
//
#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"       // �� ��ȣ�Դϴ�.


// CGISViewerApp:
// �� Ŭ������ ������ ���ؼ��� GISViewer.cpp�� �����Ͻʽÿ�.
//

class CGISViewerApp : public CWinApp
{
public:
	CGISViewerApp();


// �������Դϴ�.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// �����Դϴ�.
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CGISViewerApp theApp;
