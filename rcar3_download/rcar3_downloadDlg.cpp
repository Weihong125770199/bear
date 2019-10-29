// rcar3_downloadDlg.cpp : implementation file
//

#include "stdafx.h"
#include "rcar3_download.h"
#include "rcar3_downloadDlg.h"
#include "CommUtils.h"
#include <Dbt.h>
#include <windows.h>

#include <afx.h>
//#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <errno.h>
#include<afxtempl.h>
#include <iostream>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
using namespace std;
#import "msxml3.dll"
using namespace MSXML2;
#include "adb_api.h"
#include "usb.h"
#pragma comment(lib,"AdbWinApi.lib")
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void CRcar3_downloadDlg::ThreadUsbFunc(void *p )
{
    SECURITY_ATTRIBUTES sa;
    HANDLE hRead,hWrite;
	 CRcar3_downloadDlg *pDlg= (CRcar3_downloadDlg *)p ;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);    
    sa.lpSecurityDescriptor = NULL;        //使用系统默认的安全描述符    
    sa.bInheritHandle = TRUE;              //创建的进程继承句柄
	
    if (!CreatePipe(&hRead,&hWrite,&sa,1)) //创建匿名管道
    {        
        ::MessageBox(NULL,"CreatePipe Failed!","提示",MB_OK | MB_ICONWARNING);        
        return ;
    }
    
    STARTUPINFO si;    
    PROCESS_INFORMATION pi;
	
    ZeroMemory(&si,sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);    
    GetStartupInfo(&si);    
    si.hStdError = hWrite;    
    si.hStdOutput = hWrite;    //新创建进程的标准输出连在写管道一端
    si.wShowWindow = SW_HIDE;  //隐藏窗口    
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	
    
    char cmdline[400]; 
   // CString tmp,stredit2;
	 CString tmp;
    tmp.Format("cmd /C %s",pDlg->inputCmd);  // inputCmd为输入的cmd命令
    //tmp.Format("%s",pDlg->inputCmd);  // inputCmd为输入的cmd命令
	// pDlg->MessageBox(tmp);
    sprintf(cmdline,"%s",tmp);
	//CreateProcess(
    if (!CreateProcess(NULL,cmdline,NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi)) //创建子进程
    {
        ::MessageBox(NULL,"CreateProcess Failed!","提示",MB_OK | MB_ICONWARNING);        
        return ;
    }
 
	pDlg->pid = pi.dwProcessId;
 
    CloseHandle(hWrite); //关闭管道句柄
    
    char buffer[4096] = {0};
    DWORD bytesRead;
	
    while (true) 
    {
	
        if (ReadFile(hRead,buffer,1024,&bytesRead,NULL) == NULL) //读取管道
            break;
 
		CString temp;
        temp = buffer; // outpuuCmd为输出的结果
 
	//	temp += _T("\r\n\r\n");
 
	//	pDlg->outputCmd += temp;
		pDlg->show_read_string=pDlg->show_read_string +temp;
		//显示输出信息到编辑框,并刷新窗口
		pDlg->SetDlgItemText(IDC_EDIT3,pDlg->show_read_string);
		pDlg->ProcessFastbootCmd(temp);
	    pDlg->m_EditShow.LineScroll(pDlg->m_EditShow.GetLineCount() -1);
    }
    CloseHandle(hRead);
    	 //pDlg->MessageBox("退出进程");
	return ;

}
 void CRcar3_downloadDlg::ThreadFunc(void *p )
{

 
 unsigned char COMdata[500]; //接收的数据
 char szport[40];
 bool isOpen=false;
 bool ret;
 int result=0;
 CString show_string;
 CRcar3_downloadDlg *pDlg= (CRcar3_downloadDlg *)p ;
 CString temp_string;
 CString send_string;

 CommUtils mCommUtils ;
pDlg->pCommUtils = &mCommUtils; 

 int old_port = -1;

	while(1)
	{
       if(!pDlg->m_StartProcess)
	   {
		  //pDlg->MessageBox("sleep begin \n");
	  
          mCommUtils.CloseCom();
		  pDlg->GetDlgItemText(IDC_EDIT2,show_string);
		  if(show_string !=_T("未连接"))
                    // pDlg->SetDlgItemText(IDC_EDIT3,TEXT("未连接"));

		  old_port = -1;
		  Sleep(100);
		  continue;
	   }
	
	 
	   if(pDlg->mPort != old_port) /*port has changed*/
	   {
		 

            if(mCommUtils.OpenCom(pDlg->mPort,CBR_115200))//打开串口读串口数据：
			{
	         // pDlg->MessageBox("open COM successful  ---- \n");
			  sprintf(szport,"串口COM%d 打开成功", pDlg->mPort);
			  pDlg->SetDlgItemText(IDC_EDIT2,TEXT(szport));
              old_port = pDlg->mPort;
              isOpen=true;
			}else
			{
		      pDlg->MessageBox("open COM fail  ---- \n");
			  old_port = pDlg->mPort;
			
              sprintf(szport,"串口COM%d 无法打开", pDlg->mPort);
			 
			  pDlg->SetDlgItemText(IDC_EDIT2,TEXT(szport));
		      isOpen=false;
			}
	   } 
	   else
	   {  
		 if(old_port == -1)
		 {
	      pDlg->GetDlgItemText(IDC_EDIT2,show_string);
		  if(show_string !=_T("未连接"))
                     pDlg->SetDlgItemText(IDC_EDIT2,TEXT("未连接"));
		 }
		 
	   }



	   while( isOpen &&(pDlg->mPort == old_port) &&pDlg->m_StartProcess)

	   {
	       memset(COMdata,'\0',500);
           DWORD len = 300;

           // unsigned char SendArr[]="ls";
          if(pDlg->IsNeedSentCommand)
		  {
            

			pDlg->GetDlgItemText(IDC_EDIT1,send_string);
            send_string = send_string + "\r\n";
             if(!pDlg->pCommUtils->WriteCom((unsigned char*)send_string.GetBuffer(send_string.GetLength()), send_string.GetLength()))
			{
		      pDlg->MessageBox("write fail \n");
			}
             // pDlg->show_read_string = pDlg->show_read_string +_T("输入：") + send_string;
			 // pDlg->SetDlgItemText(IDC_EDIT3,pDlg->show_read_string);
			  pDlg->IsNeedSentCommand = false;
		  }
        
          ret=pDlg->pCommUtils->ReadCom(COMdata, len,result);//读数据：
		  if(ret)
		  {
			   temp_string=_T((char*)COMdata);
			   //temp_string.Replace(" ","");
		      // pDlg->MessageBox(temp_string);
			   //Sleep(50000);
			   pDlg->show_read_string = pDlg->show_read_string + temp_string ;
				pDlg->show_read_string = pDlg->show_read_string + "\r\n";
			   pDlg->SetDlgItemText(IDC_EDIT3,pDlg->show_read_string);
			   pDlg->m_EditShow.LineScroll(pDlg->m_EditShow.GetLineCount() -1);
               
               pDlg->ProcessCmd(temp_string);
			  // pDlg->SetDlgItemInt(IDC_EDIT3,len);
         

		/*	   
            if(!pDlg->pCommUtils->WriteCom(SendArr,2))
			{
		      pDlg->MessageBox("write fail \n");
			}
      */
			   
		  }else
		  {
		       //pDlg->SetDlgItemInt(IDC_EDIT3,result);
		  }

	   }
	
	  

	}

}

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRcar3_downloadDlg dialog

CRcar3_downloadDlg::CRcar3_downloadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRcar3_downloadDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRcar3_downloadDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRcar3_downloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRcar3_downloadDlg)
	DDX_Control(pDX, IDC_EDIT1, m_CmdEdit);
	DDX_Control(pDX, IDC_BUTTON1, m_usbButton);
	DDX_Control(pDX, IDC_EDIT3, m_EditShow);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressBar);
	DDX_Control(pDX, IDC_BUTTON2, m_Start);
	DDX_Control(pDX, IDC_LIST1, m_CombolPort);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRcar3_downloadDlg, CDialog)
	//{{AFX_MSG_MAP(CRcar3_downloadDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DEVICECHANGE()
	ON_BN_CLICKED(IDC_BUTTON2, OnOpenCom)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_WM_CLOSE()
	ON_WM_CANCELMODE()
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_EN_UPDATE(IDC_EDIT1, OnUpdateEdit1)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_WM_TIMER()
	ON_WM_CAPTURECHANGED()
	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRcar3_downloadDlg message handlers

BOOL CRcar3_downloadDlg::OnInitDialog()
{

	CDialog::OnInitDialog();
     pCommUtils= NULL;
	 mPort=-1;
     m_StartProcess = false;
      HANDLE h1 = NULL;                // 线程句柄
    DWORD pid = 0;                   // 保存cmd窗口的PID
	IsNeedSentCommand = false;
	usbDeviceFound=false;
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
    AddCom();
	m_CombolPort.SetCurSel(0);  
   
   

   // init_global_cmds(gobal_cmds);
    //init_xml();

	isUSBdownloadIng=false;
    isDebug=false;

	hThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc,this,0,&ThreadID);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRcar3_downloadDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRcar3_downloadDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRcar3_downloadDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}






void CRcar3_downloadDlg::OnOpenCom() 
{
	// TODO: Add your control notification handler code here
		// TODO: Add your control notification handler code here
     
  //      CommUtils mCommUtils ;//接下来通过CommUtils 的实例mCommUtils 来操作串口 
    //打开串口：
   //     unsigned char COMdata[101]; //接收的数据
   //     DWORD len = 1;
    //    bool ret;
//		int result=0;
 //       char ComPort[100];
 //       int Port=1;  //要操作的串口号
//		GetDlgItemText(IDC_EDIT1,ComPort,100);
//		sscanf(ComPort,"%d",&mPort);

	    
	   	int index=0;
	    CString comport;
	    index=m_CombolPort.GetCurSel();
		if(index <0)
		{
		  MessageBox(_T("请选择串口"));
		  return;
		}

	if(isUSBdownloadIng)
	{
	   MessageBox("USB正在烧录 \n");
	   return;
	}

	 if(m_StartProcess)
	 {
	    MessageBox("串口正在烧录 \n");
	   return;
	 
	 }

	    m_CombolPort.GetText(index,comport);
	    comport.Delete(0,3);
    
	    mPort=_ttoi(comport);

	    CString state;
	    m_Start.GetDlgItemText(IDC_BUTTON2,state);
		m_Start.GetWindowText(state);
         init_global_cmds(gobal_cmds);
		if(state == _T("开始"))
		{
	       m_Start.SetWindowText(_T("停止"));
		   m_StartProcess = true;
		}else
		{
		    m_Start.SetWindowText(_T("开始"));
			 m_StartProcess = false;
			 mPort = -1;
		}


}



BOOL CRcar3_downloadDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
	{
	   return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CRcar3_downloadDlg::OnDeviceChange(UINT nEventType,DWORD dwData)
{
 //DEV_BROADCAST_DEVICEINTERFACE* dbd = (DEV_BROADCAST_DEVICEINTERFACE*) dwData;
	// MessageBox("device ....");
   DEV_BROADCAST_HDR* dhr = (DEV_BROADCAST_HDR *)dwData;
 switch (nEventType)
 {
 case DBT_DEVICEREMOVECOMPLETE://移除设备
        if(dhr->dbch_devicetype == DBT_DEVTYP_PORT) 
		{ 
           PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)dhr; 
           int len = strlen(lpdbv->dbcp_name);
           CString name(lpdbv->dbcp_name);//COM8
          
         // RefreshCom();//刷新组合框的内容
		//	 CString action("remove device:");
         //   MessageBox(action+name);
		   RefreshCom(false,name);

		} 
	   break;
    
 case DBT_DEVICEARRIVAL://添加设备

      if(dhr->dbch_devicetype == DBT_DEVTYP_PORT) 
	  { 
         PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)dhr; 
         int len = strlen(lpdbv->dbcp_name);
         CString name(lpdbv->dbcp_name);//COM8
	//	 CString action("add device:");
     //    MessageBox(action+name);
   //RefreshCom();//刷新组合框的内容
		 RefreshCom(true,name);
	  } 
      
	 
       break;
 
 default:
  break;
 }
 
 return TRUE;
 
}







void CRcar3_downloadDlg::AddCom()
{
CArray<int,int> ports;
CArray<int,int> portse;
CArray<int,int>  portsu;
 ports.RemoveAll();
 portse.RemoveAll();
 portsu.RemoveAll();
 //因为至多有255个串口，所以依次检查各串口是否存在
 //如果能打开某一串口，或打开串口不成功，但返回的是 ERROR_ACCESS_DENIED错误信息，
 //都认为串口存在，只不过后者表明串口已经被占用
 //否则串口不存在
 for (int i=1; i<256; i++)
 {
  //Form the Raw device name
  CString sPort;
  sPort.Format(_T("\\\\.\\COM%d"), i);
  //Try to open the port
  BOOL bSuccess = FALSE;
  HANDLE hPort = ::CreateFile(sPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
  if (hPort == INVALID_HANDLE_VALUE)
  {
   DWORD dwError = GetLastError();
 
   if (dwError == ERROR_ACCESS_DENIED)
   {
    bSuccess = TRUE;
    portsu.Add(i);       //已占用的串口
   }
  }
  else
  {
   //The port was opened successfully
   bSuccess = TRUE;
   portse.Add(i);      ////可用的串口
   //Don't forget to close the port, since we are going to do nothing with it anyway
   CloseHandle(hPort);
  }
  //Add the port number to the array which will be returned
  if (bSuccess)
   ports.Add(i);   //所有存在的串口
 }
 unsigned short uicounter;  
 unsigned short uisetcom;
 CString str;  
 
 //获取可用的串口个数  
 uicounter = portse.GetSize();   
 //如果个数大于0  
 if(uicounter > 0)  
 {  
  //初始化串口列表框  
  for(int i=0; i<uicounter; i++)  
  {  
   uisetcom = portse.ElementAt(i);  
   str.Format(_T("COM%d "),uisetcom);  
   m_CombolPort.AddString(str);  
  }  
 }  
}

void CRcar3_downloadDlg::OnSelchangeList1() 
{
	// TODO: Add your control notification handler code here
	int index=0;
	CString comport;
	index=m_CombolPort.GetCurSel();
	m_CombolPort.GetText(index,comport);
	comport.Delete(0,3);

	mPort=_ttoi(comport);
	
}

void CRcar3_downloadDlg::RefreshCom( BOOL add,CString comport )
{

	CString temp;
	temp = comport;
	int temp_port;

    if(add)
	{
	 /*add a new port*/

      m_CombolPort.AddString(comport);


	}else
	{

		  temp.Delete(0,3);
    
	      temp_port=_ttoi(temp);
		 
		  /*stop the current process ,if the current port is removed*/
	    if(m_StartProcess&&(mPort == temp_port))
		{

		    m_Start.SetWindowText(_T("开始"));
			 m_StartProcess = false;
			 mPort = -1;
			
		}
		
	      m_CombolPort.DeleteString(m_CombolPort.FindString(0,comport ));
		  	m_CombolPort.SetCurSel(0);  
	      
   }
          
	 m_CombolPort.Invalidate();

}

void * CRcar3_downloadDlg::load_file(const char *file, unsigned int *sz)
{
        char *data;
        long size =0;
		long read_size=0;
		long temp_size=0;
		 char szport[200];
        //int fd;
        FILE *fd;
        fd = fopen(file, "rb");
        if (fd == NULL)
		{    
			  // printf("can not open %s  %s\n",file,strerror(errno));
			   
               sprintf(szport,"can not open %s  %s\n",file,strerror(errno));
               MessageBox(_T(szport));
			   
			   return 0;
		}
        size = filelength(fileno(fd));
        if (size <=0)
                goto fail;

        printf("%s lenth=%d  sizeof(char)=%d \n",file,size,sizeof(char));
        data =(char *)malloc(size);
        if (!data)
                goto fail;
		while(read_size < size)
        {
          temp_size = fread( (data+read_size), sizeof(char),1024,fd);
		   if(temp_size <=0)
		   {
		      printf("read %s fail,read_size =%d,   %s\n",file,read_size,strerror(errno));
		   }else
		   {
			 read_size = read_size+temp_size;
		     //printf("read_size =%d \n",read_size);
		   }

		}
        if (read_size != size) {
                free(data);
                goto fail;
        }

        fclose(fd);
        *sz = size;
        return (void*)data;

fail:
        printf("failed loading file\n");
        fclose(fd);
        return 0;
}

void * CRcar3_downloadDlg::load_data_file(char *file, unsigned int *size)
{
        void *data;
	    printf("start to load faile =%s \n",file);
        
        data = load_file(file, size);

        return data;
}

HWND GetWindowHandleByPID(DWORD dwProcessID)
{
    HWND h = GetTopWindow(0);
    while ( h )
    {
        DWORD pid = 0;
        DWORD dwTheardId = GetWindowThreadProcessId( h,&pid);
        if (dwTheardId != 0)	
        {
            if ( pid == dwProcessID/*your process id*/ )	
            {
                // here h is the handle to the window
 
				if (GetTopWindow(h))
				{
					return h;
				}
               // return h;	
            }	
        }	
        h = ::GetNextWindow( h , GW_HWNDNEXT);	
    }
    return NULL;
}

int CRcar3_downloadDlg::ProcessCmd( CString rec_cmd)
{
   // CString rec_cmd(_T(cmd));
	CString temp_string;
	void *data = NULL; 
	unsigned sz; 
	char file_name[100];
    int ret=0;
	

   

	//CString string_enter("\r\n");

	// rec_cmd.Replace("\n", "");
	// rec_cmd.Replace(" ","");
	// rec_cmd.Replace("\t","");
	// rec_cmd.Replace("\r","");

	for(int i=0;i<gobal_cmds.size;i++)
	{
	   temp_string =gobal_cmds.cmds[i].cmd;
    
	   if(gobal_cmds.cmds[i].used == true)
	   {
	     continue;//命令已经处理过了，处理下一条
	   }
       if(i>0)
	   {
	       if(gobal_cmds.cmds[i-1].used == false)
		   {
		     return 0;//上一条指令没有执行成功
		   }
	   
	   }

	   if(rec_cmd.Find(temp_string)!= -1)
	   {
	      if(gobal_cmds.cmds[i].type == 0  || gobal_cmds.cmds[i].type == 2) //字符回应
		  {

             if(gobal_cmds.cmds[i].type == 2)//改变波特率
			{
			   pCommUtils->CloseCom();
			   Sleep(1000);
			   if(pCommUtils->global_BaudRate == 115200)
			   {
                       pCommUtils->OpenCom(mPort,921600);
			   }
				else
				{
				         pCommUtils->OpenCom(mPort,115200);
				
				}

			   Sleep(2000);
			    // MessageBox(_T("波特率已经改变"));
			}



			 temp_string =gobal_cmds.cmds[i].response;
 
			 temp_string = temp_string + "\r\n";

              show_read_string = show_read_string +_T("输入：") + temp_string;
			 if(!pCommUtils->WriteCom((unsigned char*)temp_string.GetBuffer(temp_string.GetLength()), temp_string.GetLength()))
			{
		      MessageBox("write fail \n");
			 }
            gobal_cmds.cmds[i].used = true;
		  }	 

		 
		  if(gobal_cmds.cmds[i].type == 1)//文件回应
		  {
			   temp_string =gobal_cmds.cmds[i].response;
               memset(file_name,'\0',100);
			   strncpy(file_name,temp_string.GetBuffer(temp_string.GetLength()),temp_string.GetLength());
			   //   MessageBox(_T("加载文件：")+temp_string);

             
			   data=load_file(file_name,&sz);
			   
			   if(sz <0)
			   {
			      MessageBox(_T("加载失败：")+temp_string);
				  return 0;
			   }
             ret=do_send(pCommUtils,1024,data,sz,temp_string);
			 if(ret == ((int)sz))
			 {
			   //MessageBox(_T("发送成功：")+temp_string);
			 }else
			 {
			    MessageBox(_T("发送失败：")+temp_string);
               SetDlgItemText(IDC_EDIT2,_T("发送失败：")+temp_string);
		      
			  m_Start.SetWindowText(_T("开始"));
			  m_StartProcess = false;
				return 0;
			 }
		      gobal_cmds.cmds[i].used = true;
		  }

           if(gobal_cmds.cmds[i].type == 3)//烧录完整
		  {
			   pCommUtils->CloseCom();
		     
			  
			  SetDlgItemText(IDC_EDIT2,_T("串口烧录完成"));
			  show_read_string.Empty();
	    
		      SetDlgItemText(IDC_EDIT3,show_read_string);
		      gobal_cmds.cmds[i].used = true;
			  m_Start.SetWindowText(_T("开始"));
			  m_StartProcess = false;
			  mPort = -1;
               Sleep(1000);
			  while(!usbDeviceFound)
			  {
                     SetDlgItemText(IDC_EDIT2,_T("等待USB fastboot 端口"));
				   
			     	inputCmd=_T("fastboot devices");
 
	                 // 关闭CMD窗口
	                 ::SendMessage(GetWindowHandleByPID(pid), WM_CLOSE, NULL, NULL);
 
	                   // 销毁线程
	                   CloseHandle(h1);
 
	                   // 创建线程
	               h1 = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadUsbFunc, this, 0, &pid);
				    Sleep(2000);
			  }
                 

			   	if(!isUSBdownloadIng)
				{ 
					inputCmd=_T("download.bat");
 
	               // 关闭CMD窗口
	               ::SendMessage(GetWindowHandleByPID(pid), WM_CLOSE, NULL, NULL);
 
	                // 销毁线程
	                CloseHandle(h1);
 
	                // 创建线程
	                  h1 = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadUsbFunc, this, 0, &pid);

				}
		   }

          
		   return 0;
	   }
	}
	return 0;
}


int CRcar3_downloadDlg::ProcessFastbootCmd( CString rec_cmd)
{

    int left=0;
	int right=0;
    CString file_name;
	CString tem_rec = rec_cmd;
	CString temp;
	int precentage=0;
    m_ProgressBar.SetRange32(0,240);
    CString target=_T("====");


	if(rec_cmd.Find("start download")!=-1)  
	{
	
	   SetTimer(1,1000,NULL);
	   global_timer_count=0;	  
       isUSBdownloadIng=true;
	  // return 0;
	
	}

    


	if(rec_cmd.Find(target)!=-1)  
	{
	
	   //file_name=tem_rec.Delete(0,target.GetLength());
    //   rec_cmd.Replace("=", "");
      // file_name=tem_rec.Replace("=", "m");
		file_name=tem_rec;
		left=file_name.Find(_T(":"));
		file_name=file_name.Left(left);
		//file_name=file_name.Mid(5);
			SetDlgItemText(IDC_EDIT2,_T("正在烧录")+file_name);
       
        isUSBdownloadIng=true;
	//	 return 0;
	
	}

    if(rec_cmd.Find(_T("update successful"))!=-1)  
	{
	
	   SetDlgItemText(IDC_EDIT2,_T("烧录成功"));
	   KillTimer(1);
	   global_timer_count=0;
	   m_ProgressBar.SetPos(240);
	    show_read_string.Empty();
	    isUSBdownloadIng=false;
		SetDlgItemText(IDC_EDIT3,show_read_string);
	//	 return 0;
	}

	 //MessageBox(rec_cmd);
	 if(rec_cmd.Find(_T("0000	fastboot"))!=-1)  
	{
	   usbDeviceFound=true;
	  // MessageBox(_T("usbDeviceFound=true"));
	}

	if(rec_cmd.Find(_T("update fail"))!=-1)  
	{
	  
	   SetDlgItemText(IDC_EDIT2,_T("烧录失败"));
	   global_timer_count=0;
	   KillTimer(1);
	   m_ProgressBar.SetPos(0);
	   isUSBdownloadIng=false;
	  //  return 0;
	}



	return 0;

}


int CRcar3_downloadDlg::do_send(CommUtils *CommUtils, int xfer_size, void *data2, unsigned int sz2,CString FileName)
{
	   int bytes_sent;
        int expected_size;
        unsigned char *buf;
        unsigned short transaction = 0;
     //   struct dfu_status dst;
        bool ret;
        CString temp1;
		 CString temp2;
      

        buf = (unsigned char*)data2;
        expected_size =(int) sz2;
        bytes_sent = 0;
        
        m_ProgressBar.SetRange32(0,expected_size);
     
        while (bytes_sent < expected_size) {
                int bytes_left;
                int chunk_size;
                m_ProgressBar.SetPos(bytes_sent);
			
                bytes_left = expected_size - bytes_sent;
                if (bytes_left < xfer_size)
                        chunk_size = bytes_left;
                else
                        chunk_size = xfer_size;
				ret =CommUtils->WriteCom(buf, chunk_size);
                if (!ret) {
                        printf("Error during download");
                        goto out;
                }
                bytes_sent += chunk_size;
				    buf += chunk_size;
                temp1.Format(" %d",bytes_sent);
				temp2.Format("%d",expected_size);
               	SetDlgItemText(IDC_EDIT3,_T("正在发送")+FileName + temp1 + _T("--/--") + temp2);
				SetDlgItemText(IDC_EDIT2,_T("正在发送")+FileName);

           }

   

out:
	    m_ProgressBar.SetPos(0);
		SetDlgItemText(IDC_EDIT3,_T("发送")+ FileName +_T("完成"));
        return bytes_sent;

}

BOOL CRcar3_downloadDlg::G_StringToASII(CString &strSrc, BYTE *&pRefStoreBuff, int &nBuffLen, int &nLenResult)
{
/*
nLenResult = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, strSrc, -1, NULL, 0, NULL, NULL);
 if (nLenResult > nBuffLen)
 {
  delete[] pRefStoreBuff;
  pRefStoreBuff = new BYTE[nLenResult];
  if (!pRefStoreBuff)
   return FALSE;
  nBuffLen = nLenResult;
  */
	return true;
}

BOOL CRcar3_downloadDlg::init_global_cmds(serial_cmds &global_cmds)
{

 //::CoInitialize(NULL);
 MSXML2::IXMLDOMDocumentPtr XMLDOC; 
 MSXML2::IXMLDOMElementPtr XMLROOT;
 MSXML2::IXMLDOMElementPtr XMLELEMENT;
 MSXML2::IXMLDOMNodeListPtr XMLNODES; //某个节点的所以字节点
 MSXML2::IXMLDOMNamedNodeMapPtr XMLNODEATTS;//某个节点的所有属性;
 MSXML2::IXMLDOMNodePtr XMLNODE;
 HRESULT HR = XMLDOC.CreateInstance(_uuidof(MSXML2::DOMDocument30));
 if(!SUCCEEDED(HR))
 {
  MessageBox("faild!!");
  return true;
 }
 //  MessageBox("f!!");              
 HR =XMLDOC->load(".\\DOWNLOAD.XML");
 if(!SUCCEEDED(HR))
 {
  //MessageBox("load XML faile!!");
  //return true;
 }

 XMLROOT = XMLDOC->GetdocumentElement();//获得根节点;
 XMLROOT->get_childNodes(&XMLNODES);//获得根节点的所有子节点;
 long XMLNODESNUM,ATTSNUM;
 XMLNODES->get_length(&XMLNODESNUM);//获得所有子节点的个数;
 CString TMP;

 TMP.Format("%d",XMLNODESNUM);
 //MessageBox(TMP);
 global_cmds.size=(int)XMLNODESNUM ;
 for(int I=0;I<XMLNODESNUM;I++)
 {
  XMLNODES->get_item(I,&XMLNODE);//获得某个子节点;
  XMLNODE->get_attributes(&XMLNODEATTS);//获得某个节点的所有属性;
  XMLNODEATTS->get_length(&ATTSNUM);//获得所有属性的个数;
  global_cmds.cmds[I].used = false;
  for(int J=0;J<ATTSNUM;J++)
  {
   XMLNODEATTS->get_item(J,&XMLNODE);//获得某个属性;
   CString T1 = (char*)(_bstr_t)XMLNODE->nodeName;
   CString T2 = (char*)(_bstr_t)XMLNODE->text;
 
   if(T1 == _T("cmd"))
   {
      global_cmds.cmds[I].cmd = T2;
   }

   if(T1 == _T("response"))
   {
        global_cmds.cmds[I].response = T2;
   }
    
     if(T1 == _T("type"))
   {
         global_cmds.cmds[I].type = _ttoi(T2);
   }

  }

 }
    //Sleep(1000);

     XMLNODES.Release();
     XMLNODE.Release();
     XMLROOT.Release();
     XMLDOC.Release();
    // ::CoUninitialize();


   return true;
}


BOOL CRcar3_downloadDlg::init_xml()
{
#if 1
    str_cmd mycmds[]={{"please send !","RCAR3_IMAGE\\1cs_2x2G.mot",1,false},
		             { "Flash writer for R-Car H3/M3/M3N","em_e",0,false},
					 { "Select area(0-2)","0",0,false},
					 { "EM_E Complete","em_e",0,false},
					 { "Select area(0-2)","1",0,false},
					 { "EM_E Complete","em_e",0,false},
					 { "Select area(0-2)","2",0,false},
					 { "EM_E Complete","sup",0,false},
	                 { "Please change to 921.6Kbps baud rate setting of the terminal","em_w",2,false},
                 /*bootparam_sa0_hf.srec*/
					 { "Select area(0-2)","1",0,false},
					 { "Please Input Start Address in sector","000000",0,false},
					 { "Please Input Program Start Address","E6320000",0,false},
					  { "please send !","RCAR3_IMAGE\\bootparam_sa0_hf.srec",1,false},
					  { "EM_W Complete!","em_w",0,false},

                  /*bootparam_sa0_hf.srec*/
                     { "Select area(0-2)","1",0,false},
					 { "Please Input Start Address in sector","00001E",0,false},
					 { "Please Input Program Start Address","E6304000",0,false},
					  { "please send !","RCAR3_IMAGE\\bl2_hf.srec",1,false},
					  { "EM_W Complete!","em_w",0,false},


				    /*cert_header_sa6.srec*/
                     { "Select area(0-2)","1",0,false},
					 { "Please Input Start Address in sector","000180",0,false},
					 { "Please Input Program Start Address","E6320000",0,false},
					  { "please send !","RCAR3_IMAGE\\cert_header_sa6.srec",1,false},
					  { "EM_W Complete!","em_w",0,false},


					  /*bl31_hf.srec*/
                     { "Select area(0-2)","1",0,false},
					 { "Please Input Start Address in sector","000200",0,false},
					 { "Please Input Program Start Address","44000000",0,false},
					  { "please send !","RCAR3_IMAGE\\bl31_hf.srec",1,false},
					  { "EM_W Complete!","em_w",0,false},


					  	  /*tee_hf.srec*/
                     { "Select area(0-2)","1",0,false},
					 { "Please Input Start Address in sector","000400",0,false},
					 { "Please Input Program Start Address","44100000",0,false},
					  { "please send !","RCAR3_IMAGE\\tee_hf.srec",1,false},
					  { "EM_W Complete!","em_w",0,false},

					   	  /*u-boot-elf_hf.srec*/
                     { "Select area(0-2)","1",0,false},
					 { "Please Input Start Address in sector","001000",0,false},
					 { "Please Input Program Start Address","50000000",0,false},
					  { "please send !","RCAR3_IMAGE\\u-boot-elf_hf.srec",1,false},
					  { "EM_W Complete!","em_secsd",0,false},


					  { "Please Input EXT_CSD Index(H'00 - H'1FF)","b3",0,false},
					  { "Please Input Value(H'00 - H'FF)","8",0,false},
					  { "EXT_CSD[B3] = 0x08","em_secsd",0,false},
					  { "Please Input EXT_CSD Index(H'00 - H'1FF)","b1",0,false},
					  { "Please Input Value(H'00 - H'FF)","a",0,false},

                      { ">","请将车机拨到emmc启动模式并断电重启开发板",2,false},//将波特率改回115200
					  { "Hit any key to stop autoboot","T",0,false},
					  { "=>","fastboot",0,false},
					  { "R-CAR3-USBHS probed","串口烧录完成",3,false},
					  //{ "EM_E Complete","sup",0,false},
					  //{ "EM_E Complete","sup",0,false},
	};

  int	size= sizeof(mycmds)/sizeof(str_cmd);

  ::CoInitialize(NULL);
   MSXML2::IXMLDOMDocumentPtr XMLDOC;
   MSXML2::IXMLDOMElementPtr XMLROOT;
   HRESULT HR = XMLDOC.CreateInstance(_uuidof(MSXML2::DOMDocument30));
 if(!SUCCEEDED(HR))
 {
  MessageBox("faild!!");
  return false;
 }
 XMLROOT = XMLDOC->createElement("ROOT");
 XMLROOT->setAttribute("ID","12345");  //设置根标签的属性;
 XMLDOC->appendChild(XMLROOT);
 CString TMP;
  CString TMP_TYPE;
 MSXML2::IXMLDOMElementPtr XMLNODE;
 for(int I=0;I<size;I++)
 {
  TMP.Format("%d",I);
  XMLNODE = XMLDOC->createElement((_bstr_t)("cmd"));
  XMLNODE->put_text((_bstr_t)"NODETEXTS");//设置标签的文本内容;
  XMLNODE->setAttribute("ID",(_variant_t)TMP);//设置标签的属性及内容;
  XMLNODE->setAttribute("cmd",mycmds[I].cmd);
   XMLNODE->setAttribute("response",mycmds[I].response);
   TMP_TYPE.Format("%d",mycmds[I].type);
   XMLNODE->setAttribute("type",(_variant_t)TMP_TYPE);
  XMLROOT->appendChild(XMLNODE);
 }
 XMLDOC->save("DOWNLOAD.XML");
 XMLNODE.Release();
 XMLROOT.Release();
 XMLDOC.Release();
 ::CoUninitialize();
#endif
   return true;
}

static int match_fastboot_with_serial(usb_ifc_info* info, const char* local_serial) {
    // Require a matching vendor id if the user specified one with -i.
 /*
	if (vendor_id != 0 && info->dev_vendor != vendor_id) {
        return -1;
    }
*/
    if (info->ifc_class != 0xff || info->ifc_subclass != 0x42 || info->ifc_protocol != 0x03) {
        return -1;
    }

    // require matching serial number or device path if requested
    // at the command line with the -s option.
    if (local_serial && (strcmp(local_serial, info->serial_number) != 0 &&
                   strcmp(local_serial, info->device_path) != 0)) return -1;
    return 0;
}

static int printifc(usb_ifc_info *info,void *p)
{
  CString TMP;
CRcar3_downloadDlg *pDlg= (CRcar3_downloadDlg *)p ;

    TMP.Format("dev: csp=%02x/%02x/%02x v=%04x p=%04x  ifc: csp=%02x/%02x/%02x%s%s\n ",
           info->dev_class, info->dev_subclass, info->dev_protocol,
           info->dev_vendor, info->dev_product , info->ifc_class, info->ifc_subclass, info->ifc_protocol,
           info->has_bulk_in ? " in" : "",
           info->has_bulk_out ? " out" : "");

	pDlg->MessageBox(TMP);
   
    return -1;
}


static int list_devices_callback(usb_ifc_info* info,void *p) {
CString TMP;
CRcar3_downloadDlg *pDlg= (CRcar3_downloadDlg *)p ;
 // 

    if (match_fastboot_with_serial(info, NULL) == 0) {
        std::string serial = info->serial_number;
        

           TMP.Format("%-22s fastboot", serial.c_str());
           // printf("%-22s fastboot", serial.c_str());
		   printifc(info,p);
            if (strlen(info->device_path) > 0)
			{
				TMP.Format(" %s", info->device_path);
				printf(" %s", info->device_path);
				pDlg->MessageBox(TMP);

			}
        }
       
    

    return -1;
}

void CRcar3_downloadDlg::OnButton1() 
{

   usb_open(list_devices_callback,this);

#if 0

	// TODO: Add your control notification handler code here
    //	GetDlgItemText(IDC_EDIT1,inputCmd); //获取编辑框中输入的命令行

	if(m_StartProcess)
	{
	   MessageBox("串口正在烧录 \n");
	   return;
	
	}
	if(isUSBdownloadIng)
	{
	   MessageBox("USB正在烧录 \n");
	   return;
	}
	inputCmd=_T("download.bat");
 
	// 关闭CMD窗口
	::SendMessage(GetWindowHandleByPID(pid), WM_CLOSE, NULL, NULL);
 
	// 销毁线程
	CloseHandle(h1);
 
	// 创建线程
	h1 = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadUsbFunc, this, 0, &pid);
#endif
}

void CRcar3_downloadDlg::OnClose() 
{


	// TODO: Add your message handler code here and/or call default
			// 关闭CMD窗口
	::SendMessage(GetWindowHandleByPID(pid), WM_CLOSE, NULL, NULL);
 
	// 销毁线程
	CloseHandle(h1);
	// TODO: Add your message handler code here
    


	CDialog::OnClose();
}

void CRcar3_downloadDlg::OnCancelMode() 
{
	CDialog::OnCancelMode();

	
}

void CRcar3_downloadDlg::OnChangeEdit1() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
  
	
}

void CRcar3_downloadDlg::OnUpdateEdit1() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here

}

void CRcar3_downloadDlg::OnButton3() 
{
	// TODO: Add your control notification handler code here
	CString temp;
		  if(!pCommUtils->bOpenCom)
	{
	   MessageBox("串口未打开 \n");
	}
	else
	{
	   if(isDebug)
	   {
         IsNeedSentCommand=true;
	   }else
	   {
	      GetDlgItemText(IDC_EDIT1,temp);

	      if(temp==_T("debug"))
		  {
		     isDebug=true;
		  }
		  else
		  {
		     MessageBox("不是Debug 模式 \n");
		  }
	   }
	}
   

}

void CRcar3_downloadDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent == 1)
	{
	global_timer_count++;
//	m_ProgressBar.SetRange32(0,global_timer_count);
	m_ProgressBar.SetPos(global_timer_count);


	}
	CDialog::OnTimer(nIDEvent);
}

void CRcar3_downloadDlg::OnCaptureChanged(CWnd *pWnd) 
{
	// TODO: Add your message handler code here
	
	CDialog::OnCaptureChanged(pWnd);
}
