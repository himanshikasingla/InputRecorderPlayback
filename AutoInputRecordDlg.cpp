
// AutoInputRecordDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "AutoInputRecord.h"
#include "AutoInputRecordDlg.h"
#include "HookLib/HookLib.h"
#include "afxdialogex.h"
#include <windows.h>
#include <atlconv.h> // For CT2A
#include <fstream>
#include <vector>
#include <chrono>
#include <string>
#include <winuser.h>
#include <atlstr.h> 
#include <iostream>
#include <sstream>
#include <algorithm>




#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HHOOK hKeyboardHook = NULL;
HHOOK hMouseHook = NULL;
std::vector<std::string> recordedEvents;
auto startTime = std::chrono::steady_clock::now();
bool isRecording = false;

// CAboutDlg dialog used for App About


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAutoInputRecordDlg dialog



CAutoInputRecordDlg::CAutoInputRecordDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_AUTOINPUTRECORD_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAutoInputRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_EDIT1, m_editRepeatCount);
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAutoInputRecordDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CAutoInputRecordDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_PLAYBACKFILE, &CAutoInputRecordDlg::OnBnClickedPlaybackfile)
	ON_EN_CHANGE(IDC_LOG_BOX, &CAutoInputRecordDlg::OnEnChangeLogBox)
END_MESSAGE_MAP()


// CAutoInputRecordDlg message handlers

BOOL CAutoInputRecordDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetDlgItemText(IDC_LOG_BOX, L"Click Start Recording.");
	SetDlgItemText(IDC_EDIT2, L"Loop");
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAutoInputRecordDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAutoInputRecordDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAutoInputRecordDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;
		auto now = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

		recordedEvents.push_back("KBD " + std::to_string(pKey->vkCode) + " " + std::to_string(wParam) + " " + std::to_string(duration));
	}
	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		auto now = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

		recordedEvents.push_back("MOUSE " + std::to_string(wParam) + " " + std::to_string(duration));
	}
	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}
void CAutoInputRecordDlg::OnBnClickedStart()
{
	typedef void (*InstallHooksFunc)(HWND, const wchar_t*);
	typedef void (*UninstallHooksFunc)();

	static HMODULE hHookDLL = nullptr;
	static InstallHooksFunc InstallHooks = nullptr;
	static UninstallHooksFunc UninstallHooks = nullptr;
	static CString filePath;

	if (!isRecording)
	{
		CFileDialog dlgSaveFile(FALSE, L"txt", L"recorded_input",
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			L"Text Files (*.txt)|*.txt||");

		if (dlgSaveFile.DoModal() == IDOK) {
			filePath = dlgSaveFile.GetPathName();

			hHookDLL = LoadLibrary(L"HookLib.dll");
			if (!hHookDLL) {
				AfxMessageBox(L"Failed to load HookLib.dll");
				return;
			}

			InstallHooks = (InstallHooksFunc)GetProcAddress(hHookDLL, "InstallHooks");
			if (!InstallHooks) {
				AfxMessageBox(L"InstallHooks not found.");
				return;
			}

			InstallHooks(m_hWnd, filePath);
			GetDlgItem(IDC_START)->SetWindowTextW(L"Stop Recording");
			GetDlgItem(IDC_LOG_BOX)->SetWindowTextW(L"Recording Started...");
			isRecording = true;
		}
	}
	else
	{
		if (!UninstallHooks && hHookDLL) {
			UninstallHooks = (UninstallHooksFunc)GetProcAddress(hHookDLL, "UninstallHooks");
		}
		if (UninstallHooks) UninstallHooks();
		if (hHookDLL) { FreeLibrary(hHookDLL); hHookDLL = nullptr; }

		GetDlgItem(IDC_START)->SetWindowTextW(L"Start Recording");
		GetDlgItem(IDC_LOG_BOX)->SetWindowTextW(L"Recording Stopped");
		isRecording = false;
	}
}

void CAutoInputRecordDlg::OnBnClickedPlaybackfile()
{
	UpdateData(TRUE); // Get the latest value from UI controls

	int repeatCount = _ttoi(m_editRepeatCount);

	if (repeatCount < 1 || repeatCount > 10)
		repeatCount = 1; // Default to 1 if invalid input
	std::string abc;

	CFileDialog dlgOpenFile(TRUE, L"txt", nullptr,
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		L"Text Files (*.txt)|*.txt||");

	if (dlgOpenFile.DoModal() != IDOK)
		return;

	CString filePath = dlgOpenFile.GetPathName();
	std::string narrowPath = CW2A(filePath);

	for (int r = 0; r < repeatCount; ++r)
	{
		CString msg;
		msg.Format(L"Playing iteration %d", r + 1);
		AfxMessageBox(msg);

		std::ifstream in(narrowPath);
		if (!in.is_open()) {
			AfxMessageBox(L"Failed to open the selected file.");
			return;
		}

		std::string line;
		long lastTimestamp = -1;

		while (std::getline(in, line))
		{
			std::istringstream iss(line);
			std::string type;
			iss >> type;

			if (type == "MOUSE") {
				int message, x, y;
				long timestamp;
				iss >> message >> x >> y >> timestamp;

				if (lastTimestamp != -1)
					Sleep(timestamp - lastTimestamp);
				lastTimestamp = timestamp;

				SetCursorPos(x, y);

				if (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP ||
					message == WM_RBUTTONDOWN || message == WM_RBUTTONUP)
				{
					INPUT input = { 0 };
					input.type = INPUT_MOUSE;

					switch (message) {
					case WM_LBUTTONDOWN: input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN; break;
					case WM_LBUTTONUP:   input.mi.dwFlags = MOUSEEVENTF_LEFTUP; break;
					case WM_RBUTTONDOWN: input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN; break;
					case WM_RBUTTONUP:   input.mi.dwFlags = MOUSEEVENTF_RIGHTUP; break;
					}

					SendInput(1, &input, sizeof(INPUT));
				}
			}
			else if (type == "KEY") {
				int vkCode;
				std::string state;
				long timestamp;
				iss >> vkCode >> state >> timestamp;

				if (lastTimestamp != -1)
					Sleep(timestamp - lastTimestamp);
				lastTimestamp = timestamp;

				INPUT input = { 0 };
				input.type = INPUT_KEYBOARD;
				input.ki.wVk = vkCode;
				input.ki.dwFlags = (state == "UP") ? KEYEVENTF_KEYUP : 0;
				SendInput(1, &input, sizeof(INPUT));
			}
		}

		in.close();
	}

	GetDlgItem(IDC_LOG_BOX)->SetWindowTextW(L"Playback Complete.");
	GetDlgItem(IDC_EDIT1)->SetWindowTextW(L"");
}






void CAutoInputRecordDlg::OnEnChangeLogBox()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
