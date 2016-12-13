
// GISViewerView.h : CGISViewerView 클래스의 인터페이스
//

#pragma once
#include <vector>
using namespace std;

#include "shape.h"

class CGISViewerView : public CView
{
protected: // serialization에서만 만들어집니다.
	CGISViewerView();
	DECLARE_DYNCREATE(CGISViewerView)

// 특성입니다.
public:
	CGISViewerDoc* GetDocument() const;

	CShape		m_SHP;
	MBR			m_MBR;
	bool		m_bOpenSHP;

	GeoPoint	m_geoCenterPoint;	// 지도 중심 좌표
	CPoint		m_scrCenterPoint;	// 화면 중심 좌표
	double		m_lfZoomFactor;		// ZoomIn/out을 위한 변수
	double		m_lfRatio;			// 스크린 -> Geo 좌표..
	CPoint		m_ptZoomWindowLT, m_ptZoomWindowBR; //좌상단 점과 우하단 점..
	CRect		m_rctScreen;

	// 화면 제어 관련..
	enum ManipulMode { None = 0, ZoomIn, ZoomOut, ZoomWindow, Panning, CenterMove, Infotool } m_eManipulationMode;

// 작업입니다.
public:
	void ReMBR(const MBR& _mbr);
	CStatusBar*		GetStatusBar();
	GeoPoint		DeviceToWorld(const CPoint& _scrPoint);
	CPoint			WorldToDevice(const GeoPoint& _geoPoint);

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CGISViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	afx_msg void OnZoomIn();
	afx_msg void OnUpdateZoomIn(CCmdUI* pCmdUI);
	afx_msg void OnZoomOut();
	afx_msg void OnUpdateZoomOut(CCmdUI* pCmdUI);
	afx_msg void OnZoomAll();
	afx_msg void OnCenterMove();
	afx_msg void OnUpdateCenterMove(CCmdUI* pCmdUI);
	afx_msg void OnPanning();
	afx_msg void OnUpdatePanning(CCmdUI* pCmdUI);
	afx_msg void OnZoomWindow();
	afx_msg void OnUpdateZoomWindow(CCmdUI* pCmdUI);
	afx_msg void OnShapeInfo();
	afx_msg void OnUpdateShapeInfo(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnFileOpen();
	afx_msg void OnFileClose();

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // GISViewerView.cpp의 디버그 버전
inline CGISViewerDoc* CGISViewerView::GetDocument() const
   { return reinterpret_cast<CGISViewerDoc*>(m_pDocument); }
#endif

