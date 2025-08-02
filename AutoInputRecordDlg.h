
// AutoInputRecordDlg.h : header file
//

#pragma once
#include <string>

enum class EventType {
	Mouse,
	Key
};

struct InputEvent {
	EventType type;
	int message = 0;
	int x = 0, y = 0;
	int vkCode = 0;
	bool isKeyUp = false;
	long timestamp = 0;
};
// CAutoInputRecordDlg dialog
class CAutoInputRecordDlg : public CDialogEx
{
// Construction
public:
	CAutoInputRecordDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AUTOINPUTRECORD_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support



// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedPlaybackfile();
	afx_msg void OnEnChangeLogBox();
	CString m_editRepeatCount;
};
