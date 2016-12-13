// Shape.cpp: implementation of the CShape class.
//
// Coded by Scene, Sanghee(shshin@gaia3d.com)
// 2001-11-14
// Ver 0.5
// Copyright(c) 2001 Gaia3D Inc & Scene, Sanghee
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// 위의 원작자가 만든 CShape 클래스를 기반으로 
// 1) 그래프의 노드와 링크 데이터 연결관계, 형상을 파악할 수 있게 읽는 기능
// 2) 읽은 데이터를 화면에 출력하는 기능
// 1), 2)번 기능을 만족할 수 있도록 CShape 클래스를 개선
//  
// # 개선사항 :
// - 노트와 링크 데이터를 둘다 읽어와서 같은 View에 출력 하도록 수정
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Shape.h"

#include "time.h"

CShape::CShape()
{
	m_bOpen = false;
	
	m_MBR.xmax = m_MBR.ymax = m_MBR.xmin = m_MBR.ymin = 0.;
	
	m_nShapeType = 0;

	m_pSHPPoints = nullptr;
	m_pSHPPolyObjects = nullptr;

	m_PointColor = RGB(0, 0, 0);	//=============검은색
	m_PolyColor = RGB(255, 255, 0); //=============노란색

	m_lineWidth = 0;
	m_lineType = 0;

	m_nDetectPixelSize = 4;
}

CShape::~CShape()
{
	Close();
}

void CShape::SwapWord(int _length, void* _pWord)   
{
	int		i;
    unsigned char	temp;

    for( i=0; i< _length/2; i++ )
    {
		temp = ((unsigned char *) _pWord)[i];
		((unsigned char *) _pWord)[i] = ((unsigned char *) _pWord)[_length-i-1];
		((unsigned char *) _pWord)[_length-i-1] = temp;
    }
}

bool CShape::Open(CString _filename)
{
	m_bOpen = false;
	m_vSelectPoint.clear();

	FILE *fpSHP, *fpSHX;
	int tmp;
	
	m_strSHPPath = _filename;	//============= SHP file path

	tmp = _filename.GetLength()-4;
	m_strSHXPath = _filename.Mid(0, tmp) + _T(".shx"); //============= SHX file path
	m_strDBFPath = _filename.Mid(0, tmp) + _T(".dbf");	   //============= dbf file path

	//============= open SHP/SHX file
	char * SHPbuffer;
	char * SHXbuffer;
	char * DBFbuffer;

	int SHPLen = WideCharToMultiByte(CP_ACP, 0, m_strSHPPath, -1, NULL, 0, NULL, NULL);
	SHPbuffer = new char[SHPLen + 1];
	int SHXLen = WideCharToMultiByte(CP_ACP, 0, m_strSHXPath, -1, NULL, 0, NULL, NULL);
	SHXbuffer = new char[SHXLen + 1];
	int DBFLen = WideCharToMultiByte(CP_ACP, 0, m_strSHXPath, -1, NULL, 0, NULL, NULL);
	DBFbuffer = new char[DBFLen + 1];

	WideCharToMultiByte(CP_ACP, 0, m_strSHPPath, -1, SHPbuffer, SHPLen, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_strSHXPath, -1, SHXbuffer, SHXLen, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, m_strDBFPath, -1, DBFbuffer, DBFLen, NULL, NULL);

	errno_t err;
	err = fopen_s(&fpSHP, SHPbuffer, "rb");
	err = fopen_s(&fpSHX, SHXbuffer, "rb");
	delete[] SHPbuffer;
	delete[] SHXbuffer;

	if (err != 0)
	{
		AfxMessageBox(_T("SHP File Error!!"), IDOK | MB_ICONSTOP);
		return false;
	}

	/* Now read a Header of SHP/SHX */
	//=============  Get File size & Number of records
	fseek(fpSHX, 24L, SEEK_SET);
	fread(&tmp, 4, 1, fpSHX);
	SwapWord(4, &tmp);	//=============  Byte Order Changing	
	int m_nFileSize = tmp * 2;	//=============  get file size
	int nRecords = (m_nFileSize - 100) / 8;	//============= get a number of records
	//=============  get shp type
	int nOldShapeType = m_nShapeType;
	fseek(fpSHP, 32L, SEEK_SET);
	fread(&m_nShapeType, 4, 1, fpSHP);

	int nOldPointRecords = 0; 
	int nOldPolyRecords  = 0;
	if (m_nShapeType == SHPT_POINT)
	{
		nOldPointRecords = m_nPointRecords;
		m_nPointRecords = nRecords;
	}
	else
	{
		nOldPolyRecords = m_nPolyRecords;
		m_nPolyRecords  = nRecords;
	}
	
	//=============  Check a invalid shp type
	if(!(m_nShapeType == 1 || m_nShapeType == 3 || m_nShapeType == 5 || m_nShapeType == 8)) 
	{
		AfxMessageBox(_T("Invalid SHP Type.\nThis program only support Point, Arc, Polygon & MultiPoint SHP Type."), IDOK | MB_ICONSTOP);
		return false;
	}

	//=============  get a MBR
	fread(&m_MBR, sizeof(MBR), 1, fpSHP);
	//=============  real routine of feature reading(point, arc, polygon, multipoint)

	switch(m_nShapeType)
	{
	//=============  포인트를 읽어들인다.
	case SHPT_POINT:
		if (m_pSHPPoints == nullptr) 
			m_pSHPPoints = new GeoPoint[m_nPointRecords];
		else
		{
			delete[] m_pSHPPoints;
			m_pSHPPoints = new GeoPoint[m_nPointRecords];
		}
			
		for (int i = 0; i < m_nPointRecords; ++i)
		{
			fseek(fpSHX, 100 + i * 8, SEEK_SET);
			fread(&tmp, 4, 1, fpSHX);
			SwapWord(4, &tmp);
			tmp=tmp * 2;

			fseek(fpSHP, tmp + 12, SEEK_SET);
			fread((GeoPoint*)&m_pSHPPoints[i], 16, 1, fpSHP);
		}

		// Get Field number..
		m_DBFPoint.Open(DBFbuffer, "rb");
		delete[] DBFbuffer;

		if (m_nPointRecords == m_DBFPoint.GetRecordCount())
		{
			m_nPointField = m_DBFPoint.GetFieldCount();
		}
		else
		{
			AfxMessageBox(_T(".dbf file is difference from .ship file"), IDOK | MB_ICONSTOP);
			Close();
		}

		break;

	// 폴리라인과 폴리곤을 읽어들인다.
	case SHPT_ARC :
	case SHPT_POLYGON:
		if (m_pSHPPolyObjects == nullptr)
			m_pSHPPolyObjects = new PolyObject[m_nPolyRecords];
		else
		{
			for (int i = 0; i < nOldPolyRecords; ++i)
			{
				delete[] m_pSHPPolyObjects[i].m_pParts;
				delete[] m_pSHPPolyObjects[i].m_pPoints;
			}
			delete[] m_pSHPPolyObjects;
			m_pSHPPolyObjects = new PolyObject[m_nPolyRecords];
		}

		m_DBFPoly.Open(DBFbuffer, "rb");
		delete[] DBFbuffer;
		if (m_nPolyRecords == m_DBFPoly.GetRecordCount())
		{
			for (int i = 0; i<m_nPolyRecords; ++i)
			{
				fseek(fpSHX, 100 + i * 8, SEEK_SET);
				fread(&tmp, 4, 1, fpSHX);
				SwapWord(4, &tmp);
				tmp = tmp * 2;

				fseek(fpSHP, tmp + 12, SEEK_SET);
				fread((MBR*)&m_pSHPPolyObjects[i].m_MBR, 32, 1, fpSHP);	//============= get MBR
				fread(&m_pSHPPolyObjects[i].m_nNumParts, 4, 1, fpSHP);	//============= get parts number
				fread(&m_pSHPPolyObjects[i].m_nNumPoints, 4, 1, fpSHP);  //============= get points number

				m_pSHPPolyObjects[i].m_pParts = new int[m_pSHPPolyObjects[i].m_nNumParts];
				m_pSHPPolyObjects[i].m_pPoints = new GeoPoint[m_pSHPPolyObjects[i].m_nNumPoints];

				for (int j = 0; j<m_pSHPPolyObjects[i].m_nNumParts; ++j)
					fread(&m_pSHPPolyObjects[i].m_pParts[j], 4, 1, fpSHP);

				for (int j = 0; j < m_pSHPPolyObjects[i].m_nNumPoints; ++j)
				{
					fread(&m_pSHPPolyObjects[i].m_pPoints[j], 16, 1, fpSHP);
				}
			}
		}
		else
		{
			AfxMessageBox(_T(".dbf file is difference from .ship file"), IDOK | MB_ICONSTOP);
			Close();
		}
		break;
	
		//============= 다중점을 읽어들인다.
	case SHPT_MULTIPOINT:
		if (m_pSHPPolyObjects == nullptr)
			m_pSHPPolyObjects = new PolyObject[m_nPolyRecords];
		else 
		{
			for (int i = 0; i < nOldPolyRecords; ++i)
				delete[] m_pSHPPolyObjects[i].m_pPoints;

			delete[] m_pSHPPolyObjects;
			m_pSHPPolyObjects = new PolyObject[m_nPolyRecords];
		}
		for(int i=0;i<m_nPolyRecords;++i)
		{
			fseek(fpSHX, 100+i*8, SEEK_SET);
			fread(&tmp, 4, 1, fpSHX);
			SwapWord(4, &tmp);
			tmp=tmp*2;

			fseek(fpSHP, tmp+12, SEEK_SET);
			fread((MBR*)&m_pSHPPolyObjects[i].m_MBR, 32, 1, fpSHP);	//============= get MBR
			fread(&m_pSHPPolyObjects[i].m_nNumPoints, 4, 1, fpSHP);	//============= get points number

			m_pSHPPolyObjects[i].m_pPoints = new GeoPoint[m_pSHPPolyObjects[i].m_nNumPoints];

			for(int j=0;j<m_pSHPPolyObjects[i].m_nNumPoints;++j)
				fread(&m_pSHPPolyObjects[i].m_pPoints[j], 16, 1, fpSHP);
		}
		break;
	}
	//============= SHP Node = 검은색, Link = 노란색
	m_PointColor = RGB(0, 0, 0); 
	m_PolyColor  = RGB(255, 255, 0);

	fclose(fpSHX);
	fclose(fpSHP);

	m_bOpen = true;
	return true;
}

void CShape::Draw(CDC* _pDC, double _ZoomFactor, GeoPoint _ptMapCenter, CRect _rctScreen, double _lfRatio)
{
	m_lfZoomFactor = _ZoomFactor;
	m_geoCenterPoint = _ptMapCenter;
	m_scrCenterPoint.x = int(_rctScreen.Width()/2);
	m_scrCenterPoint.y = int(_rctScreen.Height()/2);
	m_lfMainRatio = _lfRatio;

	switch(m_nShapeType)
	{
	case SHPT_POINT:		//=============point drawing
	case SHPT_ARC:			//=============arc drawing
		DrawArc(_pDC);
		DrawPoint(_pDC);
		DrawSelected(_pDC);
		break;
	case SHPT_POLYGON:		//=============polygon drawing
		DrawPolygon(_pDC);
		break;

	case SHPT_MULTIPOINT:	//=============multipoint drawing
		DrawMultiPoint(_pDC);
		break;
	}
}

CPoint CShape::WorldToDevice(const GeoPoint& _geoPoint, double _lfRatio)
{
	CPoint	tmpPoint;

	tmpPoint.x = long(_lfRatio * (_geoPoint.x - m_geoCenterPoint.x) * m_lfZoomFactor + m_scrCenterPoint.x);
	tmpPoint.y = long(_lfRatio * (m_geoCenterPoint.y - _geoPoint.y) * m_lfZoomFactor + m_scrCenterPoint.y);

	return tmpPoint;
}

GeoPoint CShape::DeviceToWorld(const CPoint& _point, double _lfRatio)
{
	GeoPoint	tmpGeoPoint;

	tmpGeoPoint.x = m_geoCenterPoint.x + (_point.x - m_scrCenterPoint.x) / (m_lfZoomFactor * _lfRatio);
	tmpGeoPoint.y = m_geoCenterPoint.y - (_point.y - m_scrCenterPoint.y) / (m_lfZoomFactor * _lfRatio);

	return tmpGeoPoint;
}

//============= 포인트를 그린다.
void CShape::DrawPoint(CDC* _pDC)
{
	if (m_pSHPPoints == nullptr)
		return;

	CPen pen;
	CBrush	brush;

	CPoint	tmpPoint;

	int savedDC = _pDC->SaveDC();

	pen.CreatePen(m_lineType, m_lineWidth, m_PointColor);
	brush.CreateSolidBrush(m_PointColor);

	_pDC->SelectObject(&pen);
	_pDC->SelectObject(&brush);

	//============= 포인트의 개수만큼 돌면서 화면에 그린다.
	for(int i = 0; i < m_nPointRecords; ++i)
	{
		tmpPoint = WorldToDevice(m_pSHPPoints[i], m_lfMainRatio);
		_pDC->Ellipse(tmpPoint.x-2, tmpPoint.y-2, tmpPoint.x+2, tmpPoint.y+2);
	}
	_pDC->RestoreDC(savedDC);
}

//============= 폴리라인 혹은 ARC라는 놈을 그린다.
void CShape::DrawArc(CDC* _pDC)
{
	if (m_pSHPPolyObjects == nullptr)
		return;

	CPen	pen;
	int savedDC = _pDC->SaveDC();

	pen.CreatePen(m_lineType, m_lineWidth, m_PolyColor);
	_pDC->SelectObject(&pen);

	//============= 폴리라인의 개수만큼 돌면서 그린다.
	for(int i = 0; i < m_nPolyRecords; ++i)
	{
		//============= 화면에 그릴 포인트를 할당하고..
		CPoint*	pScrPoints = new CPoint[m_pSHPPolyObjects[i].m_nNumPoints];
		for(int j = 0; j < m_pSHPPolyObjects[i].m_nNumPoints; ++j)
			pScrPoints[j] = WorldToDevice(m_pSHPPolyObjects[i].m_pPoints[j], m_lfMainRatio);

		//============= 파트에 관한 메모리를 할당하고..
		int* pParts = new int[m_pSHPPolyObjects[i].m_nNumParts];
		for(int j = 0; j < m_pSHPPolyObjects[i].m_nNumParts; ++j)
		{
			if(j==m_pSHPPolyObjects[i].m_nNumParts-1)
				pParts[j] = m_pSHPPolyObjects[i].m_nNumPoints - m_pSHPPolyObjects[i].m_pParts[j];
			else
				pParts[j] = m_pSHPPolyObjects[i].m_pParts[j+1] - m_pSHPPolyObjects[i].m_pParts[j];
		}
		//============= 그린다..
		_pDC->PolyPolyline(pScrPoints, (DWORD*)pParts, m_pSHPPolyObjects[i].m_nNumParts);

		delete [] pScrPoints;
		delete [] pParts;
	}
	_pDC->RestoreDC(savedDC);
}

//============= 폴리곤을 그린다.
void CShape::DrawPolygon(CDC* _pDC)
{
	if (m_pSHPPolyObjects == nullptr)
		return;

	CPen	pen;
	CBrush	brush;
	
	int savedDC = _pDC->SaveDC();
	pen.CreatePen(m_lineType, m_lineWidth, RGB(0,0,0));
	brush.CreateSolidBrush(m_PolyColor);
	
	_pDC->SelectObject(&brush);
	_pDC->SelectObject(&pen);

	for(int i=0;i<m_nPolyRecords;++i)
	{
		//============= 스크린 포인트를 할당하고..
		CPoint*	pScrPoints = new CPoint[m_pSHPPolyObjects[i].m_nNumPoints];
		for(int j=0;j<m_pSHPPolyObjects[i].m_nNumPoints;++j)
			pScrPoints[j] = WorldToDevice(m_pSHPPolyObjects[i].m_pPoints[j], m_lfMainRatio);

		//============= 파트 정보를 정리하고..
		int* pParts = new int[m_pSHPPolyObjects[i].m_nNumParts];
		for(int j=0;j<m_pSHPPolyObjects[i].m_nNumParts;++j)
		{
			if(j==m_pSHPPolyObjects[i].m_nNumParts-1)
				pParts[j] = m_pSHPPolyObjects[i].m_nNumPoints - m_pSHPPolyObjects[i].m_pParts[j];
			else
				pParts[j] = m_pSHPPolyObjects[i].m_pParts[j+1] - m_pSHPPolyObjects[i].m_pParts[j];
		}
		//============= 그린다..
		_pDC->PolyPolygon(pScrPoints, pParts, m_pSHPPolyObjects[i].m_nNumParts);

		delete [] pScrPoints;
		delete [] pParts;
	}
	_pDC->RestoreDC(savedDC);
}

//============= 다중점을 그린다.
void CShape::DrawMultiPoint(CDC* _pDC)
{
	if (m_pSHPPolyObjects == nullptr)
		return;

	CPen	pen;
	
	int savedDC = _pDC->SaveDC();

	pen.CreatePen(m_lineType, m_lineWidth, m_PolyColor);
	_pDC->SelectObject(&pen);

	for(int i = 0; i < m_nPolyRecords; ++i)
	{
		//============= 스크린용 점을 할당한다.
		CPoint*	pScrPoints = new CPoint[m_pSHPPolyObjects[i].m_nNumPoints];
		for(int j = 0; j < m_pSHPPolyObjects[i].m_nNumPoints; ++j)
		{
			pScrPoints[j] = WorldToDevice(m_pSHPPolyObjects[i].m_pPoints[j], m_lfMainRatio);
			_pDC->Ellipse(pScrPoints[j].x - 2, pScrPoints[j].y - 2, pScrPoints[j].x + 2, pScrPoints[j].y + 2);
		}
		delete [] pScrPoints;
	}
	_pDC->RestoreDC(savedDC);
}
//============= 선택한 노드 Draw
void CShape::DrawSelected(CDC* _pDC)
{
	CBrush	brush;
	CPen	pen;
	CPoint	tmpPoint;

	int savedDC = _pDC->SaveDC();

	pen.CreatePen(PS_SOLID, 0, RGB(0, 0, 0));
	brush.CreateSolidBrush(RGB(255, 0, 0));

	_pDC->SelectObject(&pen);
	_pDC->SelectObject(&brush);

	int nOffset = 3;

	for (unsigned int i = 0; i < m_vSelectPoint.size(); ++i)
	{
		tmpPoint = WorldToDevice(m_vSelectPoint[i].Pos, m_lfMainRatio);
		_pDC->Rectangle(tmpPoint.x - nOffset, tmpPoint.y - nOffset, tmpPoint.x + nOffset, tmpPoint.y + nOffset);
	}

	_pDC->RestoreDC(savedDC);
}
//============= 화면 클릭시 노드 모양 범위에 들어오는지 아닌지 구분하기 위해서
bool CShape::PtInRect(const GeoPoint& _geoInputPoint, const GeoPoint& _geoRectCenter)
{
	if (_geoInputPoint.x < (_geoRectCenter.x - (double)m_nDetectPixelSize / m_lfZoomFactor / m_lfMainRatio))
		return false;
	else if (_geoInputPoint.x >(_geoRectCenter.x + (double)m_nDetectPixelSize / m_lfZoomFactor / m_lfMainRatio))
		return false;
	else if (_geoInputPoint.y < (_geoRectCenter.y - (double)m_nDetectPixelSize / m_lfZoomFactor / m_lfMainRatio))
		return false;
	else if (_geoInputPoint.y >(_geoRectCenter.y + (double)m_nDetectPixelSize / m_lfZoomFactor / m_lfMainRatio))
		return false;
	else
		return true;
}
//============= 정보 조회 마우스 모드로 노드 선택하면 선택한 노드 정보를 표시해 준다.
bool CShape::InfoShow(CWnd* pWnd, GeoPoint point, vector<pair<CString, CString>>& vstrPointINFO)
{
	SelectedPoint stSelectPoint;
	stSelectPoint.Init();

	int nIDX = -1;
	for (int i = 0; i < m_nPointRecords; ++i)
	{
		if (PtInRect(m_pSHPPoints[i], point))
		{
			nIDX = i;
			stSelectPoint.nIDX = nIDX;
			stSelectPoint.Pos.x = m_pSHPPoints[i].x;
			stSelectPoint.Pos.y = m_pSHPPoints[i].y;
			break;
		}
	}

	if (nIDX == -1)
		return false;

	for (int j = 0; j < m_nPointField; ++j)
	{
		char			field[80];
		int				width, decimal;

		CString	strAttribute=_T("");
		strAttribute = m_DBFPoint.ReadStringAttribute(nIDX, j);
		m_DBFPoint.GetFieldInfo(j, field, &width, &decimal);
		CString strField(field);

		if (strField == _T("NODE_ID") && j == 0)
		{
			stSelectPoint.strNodeID = strAttribute;
		}
		else if (strField == _T("NODE_NAME") && j == 2)
		{
			stSelectPoint.strNodeName = strAttribute;
		}
		vstrPointINFO.push_back(make_pair(strField, strAttribute));
	}

	if (m_vSelectPoint.size() == 2) //============= 출발지와 도착지를 선택 할 수 있도록..
		m_vSelectPoint.clear();

	m_vSelectPoint.push_back(stSelectPoint);
	
	return true;
}

//============= 할당된 메모리를 제거한다.
bool CShape::Close()
{
	if (m_pSHPPoints != nullptr)
	{
		delete[]m_pSHPPoints;
		m_pSHPPoints = nullptr;
	}
		

	if (m_pSHPPolyObjects != nullptr)
	{
		for (int i = 0; i < m_nPolyRecords; ++i)
		{
			delete[] m_pSHPPolyObjects[i].m_pParts;
			delete[] m_pSHPPolyObjects[i].m_pPoints;
		}
		delete[] m_pSHPPolyObjects;
		m_pSHPPolyObjects = nullptr;
	}
		
	m_vSelectPoint.clear();
	return true;
}