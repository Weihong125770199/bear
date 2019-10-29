// rcar3_downloadDlg.h : header file
//

#if !defined(AFX_RCAR3_DOWNLOADDLG_H__400C0FB9_EA9A_4991_A0E3_BC06D390431B__INCLUDED_)
#define AFX_RCAR3_DOWNLOADDLG_H__400C0FB9_EA9A_4991_A0E3_BC06D390431B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "CommUtils.h"
/////////////////////////////////////////////////////////////////////////////
// CRcar3_downloadDlg dialog
#import "msxml3.dll"
using namespace MSXML2;
struct serial_cmd
{
//unsigned char cmd[101];
//unsigned char response[101];
CString cmd;
CString response;
//CString next_cmd;
//BOOL next_cmd;
int type;
bool used;
};

struct str_cmd
{
//unsigned char cmd[101];
//unsigned char response[101];
char* cmd;
char* response;
//char* next_cmd;
//BOOL next_cmd;

int type;
bool used;
};


 struct serial_cmds 
{
serial_cmd cmds[200];
int size;
};

class CRcar3_downloadDlg : public CDialog
{
// Construction
public:
	BOOL usbDeviceFound;
	BOOL isDebug;
	BOOL isUSBdownloadIng;
	int global_timer_count;
	BOOL IsNeedSentCommand;
	CString show_read_string;
	BOOL init_global_cmds(serial_cmds &global_cmds);
	BOOL init_xml();
	BOOL G_StringToASII(CString  &strSrc, BYTE * &pRefStoreBuff, int &nBuffLen, int &nLenResult);
	int do_send(CommUtils *CommUtils ,int xfer_size, void *data2, unsigned sz2,CString FileName);
	int ProcessCmd(CString rec_cmd );
	int ProcessFastbootCmd(CString rec_cmd );
	serial_cmds gobal_cmds;
	void * load_data_file(char *file, unsigned *size);
	void * load_file(const char *file, unsigned *sz);
	void RefreshCom( BOOL add,CString comport );
	BOOL m_StartProcess;
	void AddCom(void);
	CRcar3_downloadDlg(CWnd* pParent = NULL);	// standard constructor
    static void ThreadFunc(void *p);
    static void ThreadUsbFunc(void *p);
	CommUtils  *pCommUtils ;
    HANDLE h1 ;                // 线程句柄
    DWORD pid ;                   // 保存cmd窗口的PID
    CString inputCmd, outputCmd;     // 保存cmd命令和返回的结果

//	str_cmd globalcmds[];

    
// Dialog Data
	//{{AFX_DATA(CRcar3_downloadDlg)
	enum { IDD = IDD_RCAR3_DOWNLOAD_DIALOG };
	CEdit	m_CmdEdit;
	CButton	m_usbButton;
	CEdit	m_EditShow;
	CProgressCtrl	m_ProgressBar;
	CButton	m_Start;
	CListBox	m_CombolPort;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRcar3_downloadDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
    HANDLE hThread;
	DWORD  ThreadID;
	int mPort;
	// Generated message map functions
	//{{AFX_MSG(CRcar3_downloadDlg)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnDeviceChange(UINT nEventType,DWORD dwData);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnOpenCom();
	afx_msg void OnSelchangeList1();
	afx_msg void OnButton1();
	afx_msg void OnClose();
	afx_msg void OnCancelMode();
	afx_msg void OnChangeEdit1();
	afx_msg void OnUpdateEdit1();
	afx_msg void OnButton3();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCaptureChanged(CWnd *pWnd);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
//	CWinThread * mReceive_Thread;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RCAR3_DOWNLOADDLG_H__400C0FB9_EA9A_4991_A0E3_BC06D390431B__INCLUDED_)
