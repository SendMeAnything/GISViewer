
// GISViewerView.cpp : CGISViewerView 클래스의 구현
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "GISViewer.h"
#endif

#include "GISViewerDoc.h"
#include "GISViewerView.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGISViewerView

IMPLEMENT_DYNCREATE(CGISViewerView, CView)

BEGIN_MESSAGE_MAP(CGISViewerView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)

	ON_COMMAND(IDM_ZOOM_IN, OnZoomIn)
	ON_UPDATE_COMMAND_UI(IDM_ZOOM_IN, OnUpdateZoomIn)
	ON_COMMAND(IDM_ZOOM_OUT, OnZoomOut)
	ON_UPDATE_COMMAND_UI(IDM_ZOOM_OUT, OnUpdateZoomOut)
	ON_COMMAND(IDM_ZOOM_ALL, OnZoomAll)
	ON_COMMAND(IDM_CENTER_MOVE, OnCenterMove)
	ON_UPDATE_COMMAND_UI(IDM_CENTER_MOVE, OnUpdateCenterMove)
	ON_COMMAND(IDM_PANNING, OnPanning)
	ON_UPDATE_COMMAND_UI(IDM_PANNING, OnUpdatePanning)
	ON_COMMAND(IDM_ZOOM_WINDOW, OnZoomWindow)
	ON_UPDATE_COMMAND_UI(IDM_ZOOM_WINDOW, OnUpdateZoomWindow)
	ON_COMMAND(IDM_SHAPE_INFO, OnShapeInfo)
	ON_UPDATE_COMMAND_UI(IDM_SHAPE_INFO, OnUpdateShapeInfo)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(IDM_FILE_CLOSE, OnFileClose)

END_MESSAGE_MAP()

// CGISViewerView 생성/소멸

CGISViewerView::CGISViewerView()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	m_bOpenSHP = false;
	m_lfZoomFactor = 1.0;
	m_geoCenterPoint.x = m_geoCenterPoint.y = 0.0;
	m_scrCenterPoint.x = m_scrCenterPoint.y = 0;
}

CGISViewerView::~CGISViewerView()
{
}

BOOL CGISViewerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CGISViewerView 그리기

void CGISViewerView::OnDraw(CDC* pDC)
{
	CGISViewerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.

	//============= 화면 크기 대비 MBR 크기의 종횡비 계산
	CRect rect;
	GetClientRect(&rect);

	double	lfRatio_X = fabs((rect.Width()) / (m_MBR.xmax - m_MBR.xmin));
	double	lfRatio_Y = fabs((rect.Height()) / (m_MBR.ymax - m_MBR.ymin));

	if (lfRatio_X < lfRatio_Y)
		m_lfRatio = lfRatio_X;
	else
		m_lfRatio = lfRatio_Y;

	m_scrCenterPoint.x = (rect.right - rect.left) / 2;
	m_scrCenterPoint.y = (rect.bottom - rect.top) / 2;

	//============= 더블 버퍼링
	CDC mem_dc;
	CDC* pDrawDC = pDC;
	CBitmap bitmap;
	CBitmap* pOldBitmap=nullptr;

	//============= 메모리 DC 호환성 맞춰주기
	if (mem_dc.CreateCompatibleDC(pDC))
	{
		if (bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height()))
		{
			pDrawDC = &mem_dc;
			pOldBitmap = mem_dc.SelectObject(&bitmap);
		}
	}
	pDrawDC->FillSolidRect(&rect, RGB(255, 255, 255));

	//============= SHP 파일 읽은 정보들을 점, 아크..등 타입에 맞춰서 그려준다.
	if (m_bOpenSHP)
	{
		m_SHP.Draw(pDrawDC, m_lfZoomFactor, m_geoCenterPoint, rect, m_lfRatio);
		m_bOpenSHP = false;
	}
		
	//============= 메모리 DC가 준비 완료되면 메인DC에 그려준다.
	if (pDrawDC != pDC)
	{
		pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(),
			&mem_dc, 0, 0, SRCCOPY);
		mem_dc.SelectObject(pOldBitmap);
	}
}


// CGISViewerView 인쇄

BOOL CGISViewerView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CGISViewerView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CGISViewerView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}

void CGISViewerView::OnFileOpen()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY,
					_T("SHP Files (*.shp)|*.shp|All Files (*.*)|*.*|"), NULL);

	if (dlg.DoModal() == IDOK)
	{
		CString strFileExt = _T("");
		strFileExt = dlg.GetFileExt();
		strFileExt.MakeLower();

		//============= 실제 확장자를 확인함.
		if (strFileExt == "shp")
		{
			//if (m_bOpenSHP)
			//	m_SHP.Close();

			if (!m_SHP.Open(dlg.GetPathName()))
			{
				AfxMessageBox(_T("An Error Occured!!"), IDOK | MB_ICONSTOP);
				return;
			}
			ReMBR(m_SHP.m_MBR);
			m_bOpenSHP = true;
			Invalidate();
		}	
	}
}
void CGISViewerView::OnFileClose()
{
	m_SHP.Close();
	m_bOpenSHP = false;
	Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CSHPViewerView message handlers
void CGISViewerView::OnZoomIn()
{
	(m_eManipulationMode == ZoomIn) ? m_eManipulationMode = None : m_eManipulationMode = ZoomIn;
}

void CGISViewerView::OnUpdateZoomIn(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_eManipulationMode == ZoomIn));
}

void CGISViewerView::OnZoomOut()
{
	(m_eManipulationMode == ZoomOut) ? m_eManipulationMode = None : m_eManipulationMode = ZoomOut;
}

void CGISViewerView::OnUpdateZoomOut(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_eManipulationMode == ZoomOut));
}

void CGISViewerView::OnCenterMove()
{
	(m_eManipulationMode == CenterMove) ? m_eManipulationMode = None : m_eManipulationMode = CenterMove;
}

void CGISViewerView::OnUpdateCenterMove(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_eManipulationMode == CenterMove));
}

void CGISViewerView::OnPanning()
{
	(m_eManipulationMode == Panning) ? m_eManipulationMode = None : m_eManipulationMode = Panning;
}

void CGISViewerView::OnUpdatePanning(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_eManipulationMode == Panning));
}

void CGISViewerView::OnZoomWindow()
{
	(m_eManipulationMode == ZoomWindow) ? m_eManipulationMode = None : m_eManipulationMode = ZoomWindow;
}

void CGISViewerView::OnUpdateZoomWindow(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_eManipulationMode == ZoomWindow));
}

void CGISViewerView::OnShapeInfo()
{
	(m_eManipulationMode == Infotool) ? m_eManipulationMode = None : m_eManipulationMode = Infotool;
}

void CGISViewerView::OnUpdateShapeInfo(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_eManipulationMode == Infotool));
}

void CGISViewerView::OnZoomAll()
{
	//지도중심좌표 구하기
	m_geoCenterPoint.x = (m_MBR.xmin + m_MBR.xmax) / 2;
	m_geoCenterPoint.y = (m_MBR.ymin + m_MBR.ymax) / 2;

	m_lfZoomFactor = 1.0;
	m_bOpenSHP = true;
	Invalidate();
}

void CGISViewerView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	m_scrCenterPoint.x = cx / 2;
	m_scrCenterPoint.y = cy / 2;
}

void CGISViewerView::OnLButtonDown(UINT nFlags, CPoint point)
{
	switch (m_eManipulationMode)
	{
	case ZoomIn:
		m_lfZoomFactor *= 1.6;
		m_bOpenSHP = true;
		Invalidate();
		break;

	case ZoomOut:
		m_lfZoomFactor *= 0.6;
		m_bOpenSHP = true;
		Invalidate();
		break;

	case CenterMove:
		m_geoCenterPoint = DeviceToWorld(point);
		m_bOpenSHP = true;
		Invalidate();
		break;

	case ZoomWindow:
	case Panning:
		m_ptZoomWindowLT = m_ptZoomWindowBR = point;
		SetCapture();
		break;

	case Infotool:
		vector <pair<CString, CString>> vstrPointINFO;
		m_SHP.InfoShow(this, DeviceToWorld(point), OUT vstrPointINFO);

		CInfoDLG*& pInfoDLG = ((CMainFrame*)AfxGetMainWnd())->m_pInfoDLG;
		if (pInfoDLG->GetSafeHwnd() != NULL)
		{
			pInfoDLG->InsertItem(IN vstrPointINFO);
			pInfoDLG->ShowWindow(SW_SHOW);
		}
		else
		{
			pInfoDLG = new CInfoDLG;
			pInfoDLG->Create(IDD_INFO_DLG);
			pInfoDLG->InsertItem(IN vstrPointINFO);
			pInfoDLG->ShowWindow(SW_SHOW);
		}
		m_bOpenSHP = true;
		Invalidate();
		break;
	}
	CView::OnLButtonDown(nFlags, point);
}

void CGISViewerView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect	rect;			// 화면크기를 받아오는 변수

	switch (m_eManipulationMode)
	{
	case ZoomWindow:  // 영역확대인 경우
		GeoPoint	tmpPoint1, tmpPoint2;
		double ZoomWidth, ZoomHeight;
		double ZoomRatio;

		ReleaseCapture();  // 마우스 버튼이 놓이는 순간 마우스 이벤트 관할권을 Windows에 돌려준다. 
		// 쌍을 이루는 함수는 SetCapture()임.. OnLButtonDown에서 찾을 수 있음
		m_ptZoomWindowBR = point;
		tmpPoint1 = DeviceToWorld(m_ptZoomWindowLT);
		tmpPoint2 = DeviceToWorld(m_ptZoomWindowBR);

		// 실좌표계에서의 확대창의 크기를 계산한다.
		ZoomWidth = fabs(tmpPoint1.x - tmpPoint2.x);
		ZoomHeight = fabs(tmpPoint1.y - tmpPoint2.y);

		GetClientRect(&rect);  // 화면의 크기를 받아온다.

		if (ZoomWidth >= ZoomHeight)
			ZoomRatio = rect.Width() / ZoomWidth;	//Zoom의 크기를 계산한다.
		else
			ZoomRatio = rect.Height() / ZoomHeight;

		m_lfZoomFactor = ZoomRatio / m_lfRatio;  // ZoomIn, ZoomOut을 위해 m_fZoomRatio로 계산해 처리한다. 

		m_geoCenterPoint.x = (tmpPoint1.x + tmpPoint2.x) / 2;  //지도 중심 좌표를 계산해 준다.
		m_geoCenterPoint.y = (tmpPoint1.y + tmpPoint2.y) / 2;  //지도 중심 좌표를 계산한다.
		m_bOpenSHP = true;
		Invalidate();

		break;

	case Panning:  // 패닝 명령인 경우...
		ReleaseCapture();  // 마우스 이벤트 관할을 윈도우로 돌려주고..
		break;
	}
	CView::OnLButtonUp(nFlags, point);
}

void CGISViewerView::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect	tmpRect;
	GetClientRect(&tmpRect);

	m_scrCenterPoint.x = tmpRect.Width() / 2;
	m_scrCenterPoint.y = tmpRect.Height() / 2;

	GeoPoint	tmp;

	CStatusBar *pStatusBar = GetStatusBar(); // 상태바 포인터를 얻고..
	if (!pStatusBar) return;


	// Draw a Geo-Coordinate of mouse point
	tmp = DeviceToWorld(point);
	//i = sprintf_s(buffer, "X=%.2lf   ", tmp.x);
	//i += sprintf_s(buffer + i, "Y=%.2lf", tmp.y);

	if (::IsWindow(m_hWnd) && IsWindowVisible())
	{
		//pStatusBar->SetWindowTextW((LPCTSTR)buffer);
		pStatusBar->UpdateWindow();
	}

	// Mouse manipulation mode
	switch (m_eManipulationMode)
	{
	case ZoomWindow:
		if (nFlags & MK_LBUTTON)
		{
			CClientDC	dc(this);
			CPen	cPen, *oldPen;

			cPen.CreatePen(PS_DOT, 1, RGB(0, 0, 0));
			oldPen = (CPen*)dc.SelectObject(&cPen);
			dc.SetROP2(R2_XORPEN);

			//기존의 Window을 지운다.
			dc.MoveTo(m_ptZoomWindowLT);
			dc.LineTo(m_ptZoomWindowLT.x, m_ptZoomWindowBR.y);
			dc.LineTo(m_ptZoomWindowBR);
			dc.LineTo(m_ptZoomWindowBR.x, m_ptZoomWindowLT.y);
			dc.LineTo(m_ptZoomWindowLT);
			// 새로운 window를 그린다.
			dc.MoveTo(m_ptZoomWindowLT);
			dc.LineTo(m_ptZoomWindowLT.x, point.y);
			dc.LineTo(point);
			dc.LineTo(point.x, m_ptZoomWindowLT.y);
			dc.LineTo(m_ptZoomWindowLT);

			m_ptZoomWindowBR = point;
			dc.SelectObject(oldPen);
		}
		break;

	case Panning:
		if (nFlags & MK_LBUTTON)
		{
			double dX = (m_ptZoomWindowBR.x - point.x);
			double dY = (m_ptZoomWindowBR.y - point.y);

			m_geoCenterPoint.x += dX / (m_lfRatio * m_lfZoomFactor);
			m_geoCenterPoint.y -= dY / (m_lfRatio * m_lfZoomFactor);

			m_ptZoomWindowBR = point;
			m_bOpenSHP = true;
			InvalidateRect(NULL);
		}
	}
	CView::OnMouseMove(nFlags, point);
}
CPoint CGISViewerView::WorldToDevice(const GeoPoint& _geoPoint)
{
	CPoint	tmpPoint;

	tmpPoint.x = long(m_lfRatio * (_geoPoint.x - m_geoCenterPoint.x) * m_lfZoomFactor + m_scrCenterPoint.x);
	tmpPoint.y = long(m_lfRatio * (m_geoCenterPoint.y - _geoPoint.y) * m_lfZoomFactor + m_scrCenterPoint.y);

	return tmpPoint;
}

GeoPoint CGISViewerView::DeviceToWorld(const CPoint& _scrPoint)
{
	GeoPoint	tmpGeoPoint;

	tmpGeoPoint.x = m_geoCenterPoint.x + (_scrPoint.x - m_scrCenterPoint.x) / (m_lfZoomFactor * m_lfRatio);
	tmpGeoPoint.y = m_geoCenterPoint.y - (_scrPoint.y - m_scrCenterPoint.y) / (m_lfZoomFactor * m_lfRatio);

	return tmpGeoPoint;
}

BOOL CGISViewerView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	HCURSOR	hCursor;

	switch (m_eManipulationMode)
	{
	case ZoomIn:		// ZoomIn, ZoomWindow는 같은 커서 사용
	case ZoomWindow:
		hCursor = AfxGetApp()->LoadCursor(IDC_ZOOM_IN);
		SetCursor(hCursor);
		return true;

	case ZoomOut:		// ZoomOut 커서..
		hCursor = AfxGetApp()->LoadCursor(IDC_ZOOM_OUT);
		SetCursor(hCursor);
		return true;

	case Panning:		// 패닝 커서..
		hCursor = AfxGetApp()->LoadCursor(IDC_PANNING);
		SetCursor(hCursor);
		return true;

	case CenterMove:	// 중심이동 커서..
		hCursor = AfxGetApp()->LoadCursor(IDC_ZOOM_CENTER);
		SetCursor(hCursor);
		return true;

	case Infotool:
		hCursor = AfxGetApp()->LoadCursor(IDC_SELECTOR);
		SetCursor(hCursor);
		return true;
	}

	return CView::OnSetCursor(pWnd, nHitTest, message);
}

BOOL CGISViewerView::OnEraseBkgnd(CDC* pDC)
{
	m_bOpenSHP = true;
	return false;
}

CStatusBar* CGISViewerView::GetStatusBar()
{
	CFrameWnd *pFrame = (CFrameWnd*)AfxGetMainWnd();
	if (!pFrame || !pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)))
		return NULL;

	CStatusBar* pBar = (CStatusBar*)pFrame->GetMessageBar();
	if (!pBar || !pBar->IsKindOf(RUNTIME_CLASS(CStatusBar)))
		return NULL;

	return pBar;
}

void CGISViewerView::ReMBR(const MBR& _mbr)
{
	m_MBR = _mbr;

	m_geoCenterPoint.x = (m_MBR.xmax + m_MBR.xmin) / 2;
	m_geoCenterPoint.y = (m_MBR.ymax + m_MBR.ymin) / 2;
}
afx_msg void OnFileOpen();
// CGISViewerView 진단

#ifdef _DEBUG
void CGISViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CGISViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGISViewerDoc* CGISViewerView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGISViewerDoc)));
	return (CGISViewerDoc*)m_pDocument;
}
#endif //_DEBUG


// CGISViewerView 메시지 처리기
