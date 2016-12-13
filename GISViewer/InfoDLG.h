#pragma warning(disable : 4786)
#include <vector>
using namespace std;

#if !defined(AFX_INFODLG_H__92B90877_5915_4F71_8A9F_0D1F02536D8B__INCLUDED_)
#define AFX_INFODLG_H__92B90877_5915_4F71_8A9F_0D1F02536D8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoDLG.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInfoDLG dialog

class CInfoDLG : public CDialog
{
// Construction
public:
	CInfoDLG(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInfoDLG)
	enum {IDD = 134};
	CListCtrl	m_ctlInfo;
	//}}AFX_DATA
public:
	vector<CString>	m_strField;
	vector<CString>	m_strRecord;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoDLG)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	void		InitTextList();
	void        InsertItem(vector<pair<CString, CString>>& vstrPointINFO);

protected:

	// Generated message map functions
	//{{AFX_MSG(CInfoDLG)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFODLG_H__92B90877_5915_4F71_8A9F_0D1F02536D8B__INCLUDED_)
