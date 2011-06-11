/*

 Copyright (c) 2004-2006 Ladislav Prosek.

 The use and distribution terms for this software are contained in the file named License.txt, 
 which can be found in the root of the Phalanger distribution. By using this software 
 in any fashion, you are agreeing to be bound by the terms of this license.
 
 You must not remove this notice from this software.

*/

// BootstrapperDlg.cpp : implementation file
//

#include "stdafx.h"
#include <atlpath.h>
#include "Installer.Bootstrapper.h"
#include "BootstrapperDlg.h"
#include ".\bootstrapperdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBootstrapperDlg dialog
BEGIN_DHTML_EVENT_MAP(CBootstrapperDlg)
	DHTML_EVENT_ONCLICK(_T("Exit"), OnLinkExit)
	DHTML_EVENT_ONCLICK(_T("InstallFw"), OnLinkFw)
	DHTML_EVENT_ONCLICK(_T("InstallCore"), OnLinkCore)
	//DHTML_EVENT_ONCLICK(_T("InstallVsip"), OnLinkVsip)
	DHTML_EVENT_ONCLICK(_T("InstallIntegration"), OnLinkIntegration)
END_DHTML_EVENT_MAP()


CBootstrapperDlg::CBootstrapperDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CBootstrapperDlg::IDD, CBootstrapperDlg::IDH, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBootstrapperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBootstrapperDlg, CDHtmlDialog)
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CBootstrapperDlg message handlers
BOOL CBootstrapperDlg::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SendMessage(DM_SETDEFID, -1, 0);
	SetHostFlags(DOCHOSTUIFLAG_DIALOG | DOCHOSTUIFLAG_SCROLL_NO);

	m_Timer = SetTimer(1, 2000, NULL);
	//PostMessage(WM_TIMER);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBootstrapperDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CDHtmlDialog::OnDocumentComplete(pDisp, szUrl);

	SetElementHtml("Donate", GetResourceAsBSTR(IDS_IMGDONATE));

	PostMessage(WM_TIMER);	
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBootstrapperDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HRESULT CBootstrapperDlg::OnLinkExit(IHTMLElement* pElement)
{
	OnCancel();
	return -1;
}

HRESULT CBootstrapperDlg::OnLinkFw(IHTMLElement* pElement)
{
	Launch("Setup\\dotNetFx40_Full_setup.exe");
	return -1;
}

HRESULT CBootstrapperDlg::OnLinkCore(IHTMLElement* pElement)
{
	Launch("Setup\\setup.exe");
	return -1;
}

HRESULT CBootstrapperDlg::OnLinkIntegration(IHTMLElement* pElement)
{
	Launch("Setup\\Phalanger.VS2010.vsix");
	return -1;
}

// Launches a file in the same directory where this application's executable lives
void CBootstrapperDlg::Launch(CString fileName)
{
	char buf[1024];

	// build absolute path to the given file
	CString filePath;
	if (GetModuleFileName(NULL, buf, sizeof(buf)) > 0)
	{
		CPath path = CPath(buf);
		path.RemoveFileSpec();
		path.AddBackslash();
		filePath = (CString)path + fileName;
	}
	else filePath = fileName;

	// call ShellExecute
	SHELLEXECUTEINFO info;
	ZeroMemory(&info, sizeof(SHELLEXECUTEINFO));
	info.cbSize = sizeof(SHELLEXECUTEINFO);
	info.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
	info.lpVerb = "open";
	info.lpFile = filePath;
	info.lpParameters = NULL;
	info.lpDirectory = NULL;
	info.nShow = SW_SHOW;
	
	if (ShellExecuteEx(&info) == FALSE)
	{
		int count = sprintf_s(buf, sizeof(buf), "Could not launch '%s'.\n", fileName);

		DWORD err = GetLastError();

		if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf + count, sizeof(buf) - count, NULL)) return;

		MessageBox(buf, "Could not launch the installer", MB_OK | MB_ICONSTOP);

		UpdateState();
		return;
	}

	// wait until the process exits
	if (info.hProcess != NULL)
	{
		ShowWindow(SW_HIDE);
		WaitForSingleObject(info.hProcess, INFINITE);
		ShowWindow(SW_SHOW);

		UpdateState();
	}
}

bool CBootstrapperDlg::FwInstalled()
{
	CRegKey key;
	return (key.Open(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full", KEY_QUERY_VALUE) == ERROR_SUCCESS);
}

bool CBootstrapperDlg::CoreInstalled()
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Phalanger\\v2.1", KEY_QUERY_VALUE) != ERROR_SUCCESS) return false;

	char buf[1024];
	ULONG count = sizeof(buf);
	return (key.QueryStringValue("InstallDir", buf, &count) == ERROR_SUCCESS && strlen(buf) > 0);
}

bool CBootstrapperDlg::VsipInstalled()
{
	CRegKey key;
	return (key.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\VSIP\\8.0", KEY_QUERY_VALUE) == ERROR_SUCCESS);
}

bool CBootstrapperDlg::VsNetInstalled()
{
	CRegKey key;
	return (key.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\VisualStudio\\10.0", KEY_QUERY_VALUE) == ERROR_SUCCESS);
}

bool CBootstrapperDlg::IntegrationInstalled()
{
	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Phalanger\\v2.1", KEY_QUERY_VALUE) != ERROR_SUCCESS) return false;
	
	DWORD value;
	return (key.QueryDWORDValue("IntegrationInstalled", value) == ERROR_SUCCESS && value != 0);
}

void CBootstrapperDlg::SetFw(UINT img, UINT text)
{
	SetElementHtml("Img1", GetResourceAsBSTR(img));
	SetElementHtml("Text1", GetResourceAsBSTR(text));
}

void CBootstrapperDlg::SetCore(UINT img, UINT text)
{
	SetElementHtml("Img2", GetResourceAsBSTR(img));
	SetElementHtml("Text2", GetResourceAsBSTR(text));
}

//void CBootstrapperDlg::SetVsip(UINT img, UINT text)
//{
//	SetElementHtml("Img3", GetResourceAsBSTR(img));
//	SetElementHtml("Text3", GetResourceAsBSTR(text));
//}

void CBootstrapperDlg::SetIntegration(UINT img, UINT text)
{
	SetElementHtml("Img3", GetResourceAsBSTR(img));
	SetElementHtml("Text3", GetResourceAsBSTR(text));
}

// Not thread-safe!
BSTR CBootstrapperDlg::GetResourceAsBSTR(UINT resourceId)
{
	const int MAX_LENGTH = 1024;

	static OLECHAR buffer[MAX_LENGTH];
	LoadStringW(::GetModuleHandle(NULL), resourceId, buffer, MAX_LENGTH);

	return buffer;
}


// Updates the information displayed in this dialog
void CBootstrapperDlg::UpdateState()
{
	static bool _first_time = true;
	static bool _fw_installed, _core_installed, /*_vsip_installed,*/ _vsnet_installed, _integration_installed;
	bool fw_installed, core_installed, /*vsip_installed,*/ vsnet_installed, integration_installed;

	fw_installed = FwInstalled();
	core_installed = CoreInstalled();
	//vsip_installed = VsipInstalled();
	vsnet_installed = VsNetInstalled();
	integration_installed = IntegrationInstalled();

	if (!_first_time && fw_installed == _fw_installed && core_installed == _core_installed &&
		/*vsip_installed == _vsip_installed &&*/ vsnet_installed == _vsnet_installed &&
		integration_installed == _integration_installed) return;

	_first_time = false;
	_fw_installed = fw_installed;
	_core_installed = core_installed;
	//_vsip_installed = vsip_installed;
	_vsnet_installed = vsnet_installed;
	_integration_installed = integration_installed;

	// Framework
	if (fw_installed) SetFw(IDS_IMG1A, IDS_TEXT1B);
	else
	{
		SetFw(IDS_IMG1A, IDS_TEXT1A);
		SetCore(IDS_IMG2B, IDS_TEXT2C);
		//SetVsip(IDS_IMG3B, IDS_TEXT3C);
		SetIntegration(IDS_IMG3B, IDS_TEXT3C);
		return;
	}

	// Phalanger
	if (core_installed) SetCore(IDS_IMG2A, IDS_TEXT2B);
	else
	{
		SetCore(IDS_IMG2A, IDS_TEXT2A);
		//SetVsip(IDS_IMG3B, IDS_TEXT3C);
		SetIntegration(IDS_IMG3B, IDS_TEXT3C);
		return;
	}

	// VS.NET installed?
	if (vsnet_installed)
	{
		// VSIP
		//if (vsip_installed) SetVsip(IDS_IMG3A, IDS_TEXT3B);
		//else SetVsip(IDS_IMG3A, IDS_TEXT3A);

		// Integration
		if (integration_installed) SetIntegration(IDS_IMG3A, IDS_TEXT3B);
		else SetIntegration(IDS_IMG3A, IDS_TEXT3A);
	}
	else
	{
		// VSIP
		//if (vsip_installed) SetVsip(IDS_IMG3A, IDS_TEXT3B);
		//else SetVsip(IDS_IMG3B, IDS_TEXT3D);
		
		// Integration
		if (integration_installed) SetIntegration(IDS_IMG3A, IDS_TEXT3B);
		else SetIntegration(IDS_IMG3B, IDS_TEXT3D);
	}
}

// Calls UpdateState
void CBootstrapperDlg::OnTimer(UINT nIDEvent)
{
	UpdateState();
	CDHtmlDialog::OnTimer(nIDEvent);
}
