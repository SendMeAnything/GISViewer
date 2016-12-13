
/* -------------------------------------------------------------------- */
/*      Shape types (nSHPType)                                          */
/* -------------------------------------------------------------------- */
#define SHPT_NULL	0
#define SHPT_POINT	1
#define SHPT_ARC	3
#define SHPT_POLYGON	5
#define SHPT_MULTIPOINT	8
#define SHPT_POINTZ	11
#define SHPT_ARCZ	13
#define SHPT_POLYGONZ	15
#define SHPT_MULTIPOINTZ 18
#define SHPT_POINTM	21
#define SHPT_ARCM	23
#define SHPT_POLYGONM	25
#define SHPT_MULTIPOINTM 28
#define SHPT_MULTIPATCH 31

/* -------------------------------------------------------------------- */
/*      Minimum Bounding Rectangle                                      */
/* -------------------------------------------------------------------- */
typedef struct
{
	double	xmin;
	double	ymin;
	double	xmax;
	double	ymax;
}MBR;

/* -------------------------------------------------------------------- */
/*      Point Struct                                                    */
/* -------------------------------------------------------------------- */
typedef struct{
	double x;
	double y;
}GeoPoint;

/* -------------------------------------------------------------------- */
/*      Selected Point Struct                                           */
/* -------------------------------------------------------------------- */
typedef struct{
	int nIDX;
	CString strNodeID;
	CString strNodeName;
	GeoPoint Pos;

	void Init()
	{
		nIDX = 0;
		strNodeID = _T("");
		strNodeName = _T("");
		Pos.x = 0.f;
		Pos.y = 0.f;
	}
}SelectedPoint;

/* -------------------------------------------------------------------- */
/*      PoliLine & Polygon                                              */
/* -------------------------------------------------------------------- */
typedef struct{
	MBR			m_MBR;
	int			m_nNumParts;
	int			m_nNumPoints;
	int*		m_pParts;
	GeoPoint*	m_pPoints;
}PolyObject;