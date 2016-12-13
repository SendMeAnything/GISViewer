// InfoDLG.cpp : implementation file
//

#include "stdafx.h"
#include "GISViewer.h"
#include "InfoDLG.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInfoDLG dialog


CInfoDLG::CInfoDLG(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoDLG::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInfoDLG)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInfoDLG::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInfoDLG)
	DDX_Control(pDX, IDC_TEXTLIST, m_ctlInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInfoDLG, CDialog)
	//{{AFX_MSG_MAP(CInfoDLG)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInfoDLG message handlers

BOOL CInfoDLG::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	InitTextList();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInfoDLG::InitTextList()
{
	LV_COLUMN	lvcol;

	//============= 컬럼 헤더의 설정
	lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcol.fmt = LVCFMT_CENTER;
	lvcol.cx = 85;
	lvcol.pszText = _T("필드");
	lvcol.iSubItem = 0;
	m_ctlInfo.InsertColumn(0, &lvcol);

	//============= 두번째 컬럼 설정..
	lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvcol.fmt = LVCFMT_CENTER;
	lvcol.cx = 100;
	lvcol.pszText = _T("정보");
	lvcol.iSubItem = 1;
	m_ctlInfo.InsertColumn(1, &lvcol);
}

void CInfoDLG::OnClose() 
{
	CDialog::OnClose();
}

void CInfoDLG::PostNcDestroy() 
{
	((CMainFrame*)AfxGetMainWnd())->m_pInfoDLG = NULL;
	delete this;
	
	CDialog::PostNcDestroy();
}

void CInfoDLG::InsertItem(vector<pair<CString, CString>>& vstrPointINFO)
{
	//============= 데이타를 리스트 컨트롤에 등록한다.
	m_ctlInfo.DeleteAllItems();

	LV_ITEM	lvitem;
	for (unsigned int idx = 0; idx < vstrPointINFO.size(); idx++)
	{
		lvitem.mask = LVIF_TEXT;
		lvitem.iItem = idx;
		lvitem.iSubItem = 0;
		lvitem.pszText = (LPTSTR)(LPCTSTR)vstrPointINFO[idx].first;
		lvitem.cchTextMax = vstrPointINFO[idx].first.GetLength();
		
		m_ctlInfo.InsertItem(&lvitem);
	}

	for (unsigned int idx = 0; idx < vstrPointINFO.size(); idx++)
	{
		lvitem.mask = LVIF_TEXT;
		lvitem.iItem = idx;
		lvitem.iSubItem = 1;
		lvitem.pszText = (LPTSTR)(LPCTSTR)vstrPointINFO[idx].second;
		lvitem.cchTextMax = vstrPointINFO[idx].second.GetLength();
		
		m_ctlInfo.SetItem(&lvitem);
	}
}
