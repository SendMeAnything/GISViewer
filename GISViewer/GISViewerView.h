
// GISViewerView.h : CGISViewerView Ŭ������ �������̽�
//

#pragma once
#include <vector>
using namespace std;

#include "shape.h"

class CGISViewerView : public CView
{
protected: // serialization������ ��������ϴ�.
	CGISViewerView();
	DECLARE_DYNCREATE(CGISViewerView)

// Ư���Դϴ�.
public:
	CGISViewerDoc* GetDocument() const;

	CShape		m_SHP;
	MBR			m_MBR;
	bool		m_bOpenSHP;

	GeoPoint	m_geoCenterPoint;	// ���� �߽� ��ǥ
	CPoint		m_scrCenterPoint;	// ȭ�� �߽� ��ǥ
	double		m_lfZoomFactor;		// ZoomIn/out�� ���� ����
	double		m_lfRatio;			// ��ũ�� -> Geo ��ǥ..
	CPoint		m_ptZoomWindowLT, m_ptZoomWindowBR; //�»�� ���� ���ϴ� ��..
	CRect		m_rctScreen;

	// ȭ�� ���� ����..
	enum ManipulMode { None = 0, ZoomIn, ZoomOut, ZoomWindow, Panning, CenterMove, Infotool } m_eManipulationMode;

// �۾��Դϴ�.
public:
	void ReMBR(const MBR& _mbr);
	CStatusBar*		GetStatusBar();
	GeoPoint		DeviceToWorld(const CPoint& _scrPoint);
	CPoint			WorldToDevice(const GeoPoint& _geoPoint);

// �������Դϴ�.
public:
	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// �����Դϴ�.
public:
	virtual ~CGISViewerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
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

#ifndef _DEBUG  // GISViewerView.cpp�� ����� ����
inline CGISViewerDoc* CGISViewerView::GetDocument() const
   { return reinterpret_cast<CGISViewerDoc*>(m_pDocument); }
#endif

