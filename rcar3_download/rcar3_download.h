// rcar3_download.h : main header file for the RCAR3_DOWNLOAD application
//

#if !defined(AFX_RCAR3_DOWNLOAD_H__9012296D_D388_40A4_9553_5C6DA1E6EB9F__INCLUDED_)
#define AFX_RCAR3_DOWNLOAD_H__9012296D_D388_40A4_9553_5C6DA1E6EB9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CRcar3_downloadApp:
// See rcar3_download.cpp for the implementation of this class
//

class CRcar3_downloadApp : public CWinApp
{
public:
	CRcar3_downloadApp();
    //int Port_num =0;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRcar3_downloadApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRcar3_downloadApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RCAR3_DOWNLOAD_H__9012296D_D388_40A4_9553_5C6DA1E6EB9F__INCLUDED_)
