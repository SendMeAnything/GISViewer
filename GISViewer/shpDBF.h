// shpDBF.h: interface for the shpDBF class.
//
//////////////////////////////////////////////////////////////////////
#if !defined(AFX_SHPDBF_H__0F5DB6CD_FE5B_4E70_BBF2_4D67DF83C9D6__INCLUDED_)
#define AFX_SHPDBF_H__0F5DB6CD_FE5B_4E70_BBF2_4D67DF83C9D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>

#ifdef USE_DBMALLOC
#include <dbmalloc.h>
#endif

#define MAX_COLUMN_NUMBER 64
#define MAX_COLUMN_SIZE	265

#define TRIM_DBF_WHITESPACE

typedef enum {
  FTString,
  FTInteger,
  FTDouble,
  FTInvalid
} DBFFieldType;

class shpDBF
{
 private:
	FILE*	m_fp;
  
	int		m_nRecords;
	int		m_nRecordLength;
	int		m_nHeaderLength;
	int		m_nFields;
	int	*	m_panFieldOffset;
	int	*	m_panFieldSize;
	int	*	m_panFieldDecimals;
	char*	m_pachFieldType;
	char*	m_pszHeader;
  
	int		m_nCurrentRecord;
	int		m_bCurrentRecordModified;
	char*	m_pszCurrentRecord;
  
	int		m_bNoHeader;
	int		m_bUpdated;

 public:
	shpDBF();
	~shpDBF();

	int		Open( const char * pszDBFFile, const char * pszAccess );
	int		Create( const char * pszDBFFile );
	void	WriteHeader();
	void	FlushRecord();

	int		GetFieldCount();
	int		GetRecordCount();
	int		AddField(const char * pszFieldName, DBFFieldType eType, int nWidth, int nDecimals );
  	DBFFieldType GetFieldInfo(int iField, char * pszFieldName, int * pnWidth, int * pnDecimals );

	void*	ReadAttribute(int hEntity, int iField, char chReqType );  
	int		ReadIntegerAttribute(int iShape, int iField );
	double	ReadDoubleAttribute(int iShape, int iField );
	const char *ReadStringAttribute(int iShape, int iField );

	int WriteAttribute(int iShape, int iFeild, void* pValue);
	int WriteIntegerAttribute(int iShape, int iField, int nFieldValue );
	int WriteDoubleAttribute(int iShape, int iField, double dFieldValue );
	int WriteStringAttribute(int iShape, int iField, const char * pszFieldValue );

	void SetCurrentRecord(int iRecord);
	void SetCurrentRecordModify(int flag);

	void Close( );
};

#endif // !defined(AFX_SHPDBF_H__0F5DB6CD_FE5B_4E70_BBF2_4D67DF83C9D6__INCLUDED_)
