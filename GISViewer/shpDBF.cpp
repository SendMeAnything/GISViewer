// shpDBF.cpp: implementation of the shpDBF class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "shpDBF.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include <math.h>
#include <malloc.h>
#include <string.h>

typedef unsigned char uchar;

#ifndef FALSE
#  define FALSE		0
#  define TRUE		1
#endif

/************************************************************************/
/*                             SfRealloc()                              */
/*                                                                      */
/*      A realloc cover function that will access a NULL pointer as     */
/*      a valid input.                                                  */
/************************************************************************/
static void * SfRealloc( void * pMem, int nNewSize )
{
  if (pMem == NULL )
    return( (void *) malloc(nNewSize) );
  else
    return( (void *) realloc(pMem,nNewSize) );
}

shpDBF::~shpDBF()
{

}

shpDBF::shpDBF()
{
  m_fp = NULL;
  
  m_nRecords = 0;

  m_nRecordLength = 0;
  m_nHeaderLength = 0;
  m_nFields = 0;
  m_panFieldOffset = NULL;
  m_panFieldSize = NULL;
  m_panFieldDecimals = NULL;
  m_pachFieldType = NULL;

  m_pszHeader = NULL;
  
  m_nCurrentRecord = 0;
  m_bCurrentRecordModified = FALSE;
  m_pszCurrentRecord = NULL;
  
  m_bNoHeader = FALSE;
  m_bUpdated = FALSE;
}

/************************************************************************/
/*      This is called to write out the file header, and field          */
/*      descriptions before writing any actual data records.  This      */
/*      also computes all the DBFDataSet field offset/size/decimals     */
/*      and so forth values.                                            */
/************************************************************************/

void shpDBF::WriteHeader()
{
  uchar	abyHeader[32];
  int		i;

  if (!m_bNoHeader )
    return;

  m_bNoHeader = FALSE;
  
  /* -------------------------------------------------------------------- */
  /*	Initialize the file header information.				*/
  /* -------------------------------------------------------------------- */
  for( i = 0; i < 32; i++ )
    abyHeader[i] = 0;
  
  abyHeader[0] = 0x03;		/* memo field? - just copying 	*/
  
  /* date updated on close, record count preset at zero */
  
  abyHeader[8] = m_nHeaderLength % 256;
  abyHeader[9] = m_nHeaderLength / 256;
  
  abyHeader[10] = m_nRecordLength % 256;
  abyHeader[11] = m_nRecordLength / 256;
  
  /* -------------------------------------------------------------------- */
  /*      Write the initial 32 byte file header, and all the field        */
  /*      descriptions.                                     		*/
  /* -------------------------------------------------------------------- */
  fseek( m_fp, 0, 0 );
  fwrite( abyHeader, 32, 1, m_fp );
  fwrite( m_pszHeader, 32, m_nFields, m_fp );
  
  /* -------------------------------------------------------------------- */
  /*      Write out the newline character if there is room for it.        */
  /* -------------------------------------------------------------------- */
  if (m_nHeaderLength > 32*m_nFields + 32 ) {
    char	cNewline;
    
    cNewline = 0x0d;
    fwrite( &cNewline, 1, 1, m_fp );
  }
}

/************************************************************************/
/*                                                                      */
/*      Write out the current record if there is one.                   */
/************************************************************************/
void shpDBF::FlushRecord()
{
  int		nRecordOffset;
  
  if ( m_bCurrentRecordModified && m_nCurrentRecord > -1 ) 
	{
    m_bCurrentRecordModified = FALSE;
    
    nRecordOffset = m_nRecordLength * m_nCurrentRecord + m_nHeaderLength;

    fseek( m_fp, nRecordOffset, 0 );
    fwrite( m_pszCurrentRecord, m_nRecordLength, 1, m_fp );
  }
}

void shpDBF::SetCurrentRecord(int iRecord)
{
	m_nCurrentRecord = iRecord;
}

void shpDBF::SetCurrentRecordModify(int flag)
{
	m_bCurrentRecordModified = flag;
}

/************************************************************************/
/*      Open a .dbf file.                                               */
/************************************************************************/   
int shpDBF::Open( const char * pszFilename, const char * pszAccess )
{
  uchar		*pabyBuf;
  int		nFields, nRecords, nHeadLen, nRecLen, iField;
  char	        *pszDBFFilename;

  /* -------------------------------------------------------------------- */
  /*      We only allow the access strings "rb" and "r+".                  */
  /* -------------------------------------------------------------------- */
  if (strcmp(pszAccess,"r") != 0 && strcmp(pszAccess,"r+") != 0 
      && strcmp(pszAccess,"rb") != 0 && strcmp(pszAccess,"r+b") != 0 )
    return( FALSE );
    
  /* -------------------------------------------------------------------- */
  /*	Ensure the extension is converted to dbf or DBF if it is 	*/
  /*	currently .shp or .shx.						*/    
  /* -------------------------------------------------------------------- */
  pszDBFFilename = (char *) malloc(strlen(pszFilename)+1);
  strcpy_s(pszDBFFilename, strlen(pszFilename) + 1, pszFilename);
    
  if (strcmp(pszFilename+strlen(pszFilename)-4,".shp")
      || strcmp(pszFilename+strlen(pszFilename)-4,".shx") ) {
    strcpy_s( pszDBFFilename+strlen(pszDBFFilename)-4, 5,".dbf");
  }
  else if (strcmp(pszFilename+strlen(pszFilename)-4,".SHP")
	   || strcmp(pszFilename+strlen(pszFilename)-4,".SHX") ) {
    strcpy_s( pszDBFFilename+strlen(pszDBFFilename)-4, 5, ".DBF");
  }

  /* -------------------------------------------------------------------- */
  /*      Open the file.                                                  */
  /* -------------------------------------------------------------------- */
  fopen_s(&m_fp, pszDBFFilename, pszAccess);
  if (m_fp == NULL )
    return( FALSE );

  m_bNoHeader = FALSE;
  m_nCurrentRecord = -1;
  m_bCurrentRecordModified = FALSE;

  free( pszDBFFilename );

  /* -------------------------------------------------------------------- */
  /*  Read Table Header info                                              */
  /* -------------------------------------------------------------------- */
  pabyBuf = (uchar *) malloc(500);
  fread( pabyBuf, 32, 1, m_fp );

  m_nRecords = nRecords = 
    pabyBuf[4] + pabyBuf[5]*256 + pabyBuf[6]*256*256 + pabyBuf[7]*256*256*256;

  m_nHeaderLength = nHeadLen = pabyBuf[8] + pabyBuf[9]*256;
  m_nRecordLength = nRecLen = pabyBuf[10] + pabyBuf[11]*256;
    
  m_nFields = nFields = (nHeadLen - 32) / 32;
  
  m_pszCurrentRecord = (char *) malloc(nRecLen);

  /* -------------------------------------------------------------------- */
  /*  Read in Field Definitions                                           */
  /* -------------------------------------------------------------------- */
    
  pabyBuf = (uchar *) SfRealloc(pabyBuf,nHeadLen);
  m_pszHeader = (char *) pabyBuf;
  
  fseek( m_fp, 32, 0 );
  fread( pabyBuf, nHeadLen, 1, m_fp );

  m_panFieldOffset = (int *) malloc(sizeof(int) * nFields);
  m_panFieldSize = (int *) malloc(sizeof(int) * nFields);
  m_panFieldDecimals = (int *) malloc(sizeof(int) * nFields);
  m_pachFieldType = (char *) malloc(sizeof(char) * nFields);
  
  for( iField = 0; iField < nFields; iField++ ) {
    uchar	*pabyFInfo;
    
    pabyFInfo = pabyBuf+iField*32;

    if (pabyFInfo[11] == 'N' ) {
      m_panFieldSize[iField] = pabyFInfo[16];
      m_panFieldDecimals[iField] = pabyFInfo[17];
    }
    else {
      m_panFieldSize[iField] = pabyFInfo[16] + pabyFInfo[17]*256;
      m_panFieldDecimals[iField] = 0;
    }

    m_pachFieldType[iField] = (char) pabyFInfo[11];
    if (iField == 0 )
      m_panFieldOffset[iField] = 1;
    else
      m_panFieldOffset[iField] = 
	m_panFieldOffset[iField-1] + m_panFieldSize[iField-1];
    }

    return( TRUE );
}

/************************************************************************/
/*                              Close()                                 */
/************************************************************************/

void shpDBF::Close()
{
  /* -------------------------------------------------------------------- */
  /*      Write out header if not already written.                        */
  /* -------------------------------------------------------------------- */
  if (m_bNoHeader ) WriteHeader( );
  FlushRecord( );

  /* -------------------------------------------------------------------- */
  /*      Update last access date, and number of records if we have	*/
  /*	write access.                					*/
  /* -------------------------------------------------------------------- */
  if (m_bUpdated ) {
    uchar		abyFileHeader[32];
    
    fseek( m_fp, 0, 0 );
    fread( abyFileHeader, 32, 1, m_fp );
    
    abyFileHeader[1] = 95;			/* YY */
    abyFileHeader[2] = 7;			/* MM */
    abyFileHeader[3] = 26;			/* DD */
    
    abyFileHeader[4] = m_nRecords % 256;
    abyFileHeader[5] = (m_nRecords/256) % 256;
    abyFileHeader[6] = (m_nRecords/(256*256)) % 256;
    abyFileHeader[7] = (m_nRecords/(256*256*256)) % 256;
    
    fseek( m_fp, 0, 0 );
    fwrite( abyFileHeader, 32, 1, m_fp );
  }

  /* -------------------------------------------------------------------- */
  /*      Close, and free resources.                                      */
  /* -------------------------------------------------------------------- */
  fclose( m_fp );

  if (m_panFieldOffset != NULL ) {
    free( m_panFieldOffset );
    free( m_panFieldSize );
    free( m_panFieldDecimals );
    free( m_pachFieldType );
  }

  free( m_pszHeader );
  free( m_pszCurrentRecord );  
}

/************************************************************************/
/*                             DBFCreate()                              */
/*                                                                      */
/*      Create a new .dbf file.                                         */
/************************************************************************/
int shpDBF::Create( const char * pszFilename )
{
  FILE	*fp;

  /* -------------------------------------------------------------------- */
  /*      Create the file.                                                */
  /* -------------------------------------------------------------------- */
  fopen_s(&fp, pszFilename, "wb");
  if (fp == NULL )
    return( FALSE );

  fputc( 0, fp );
  fclose( fp );

  fopen_s(&fp, pszFilename, "rb+");
  if (fp == NULL )
    return( FALSE );

  /* -------------------------------------------------------------------- */
  /*	Create the info structure.										  */
  /* -------------------------------------------------------------------- */

  m_fp = fp;
  m_nRecords = 0;
  m_nFields = 0;
  m_nRecordLength = 1;
  m_nHeaderLength = 33;
  
  m_panFieldOffset = NULL;
  m_panFieldSize = NULL;
  m_panFieldDecimals = NULL;
  m_pachFieldType = NULL;
  m_pszHeader = NULL;
  
  m_nCurrentRecord = -1;
  m_bCurrentRecordModified = FALSE;
  m_pszCurrentRecord = NULL;
  
  m_bNoHeader = TRUE;
  
  return( TRUE );
}

/************************************************************************/
/*      Add a field to a newly created .dbf file before any records     */
/*      are written.                                                    */
/************************************************************************/
int shpDBF::AddField(const char * pszFieldName, 
		       DBFFieldType eType, int nWidth, int nDecimals )
{
  char	*pszFInfo;
  int		i;

  /* -------------------------------------------------------------------- */
  /*      Do some checking to ensure we can add records to this file.     */
  /* -------------------------------------------------------------------- */
  if (m_nRecords > 0 ) return( -1 );
  if (!m_bNoHeader ) return( -1 );
  if (eType != FTDouble && nDecimals != 0 ) return( -1 );

  /* -------------------------------------------------------------------- */
  /*      SfRealloc all the arrays larger to hold the additional field    */
  /*      information.                                                    */
  /* -------------------------------------------------------------------- */
  m_nFields++;
  
  m_panFieldOffset = (int *) 
    SfRealloc( m_panFieldOffset, sizeof(int) * m_nFields );

  m_panFieldSize = (int *) 
    SfRealloc( m_panFieldSize, sizeof(int) * m_nFields );
  
  m_panFieldDecimals = (int *) 
    SfRealloc( m_panFieldDecimals, sizeof(int) * m_nFields );
  
  m_pachFieldType = (char *) 
    SfRealloc( m_pachFieldType, sizeof(char) * m_nFields );

  /* -------------------------------------------------------------------- */
  /*      Assign the new field information fields.                        */
  /* -------------------------------------------------------------------- */
  m_panFieldOffset[m_nFields-1] = m_nRecordLength;
  m_nRecordLength += nWidth;
  m_panFieldSize[m_nFields-1] = nWidth;
  m_panFieldDecimals[m_nFields-1] = nDecimals;
  
  if (eType == FTString )
    m_pachFieldType[m_nFields-1] = 'C';
  else
    m_pachFieldType[m_nFields-1] = 'N';

  /* -------------------------------------------------------------------- */
  /*      Extend the required header information.                         */
  /* -------------------------------------------------------------------- */
  m_nHeaderLength += 32;
  m_bUpdated = FALSE;
  
  m_pszHeader = (char *) SfRealloc(m_pszHeader,m_nFields*32);
  
  pszFInfo = m_pszHeader + 32 * (m_nFields-1);
  
  for( i = 0; i < 32; i++ )
    pszFInfo[i] = '\0';
  
  if (strlen(pszFieldName) < 10 )
	  strncpy_s(pszFInfo, sizeof(pszFInfo), pszFieldName, strlen(pszFieldName));
  else
	  strncpy_s(pszFInfo, sizeof(pszFInfo), pszFieldName, 10);
  
  pszFInfo[11] = m_pachFieldType[m_nFields-1];
  
  if (eType == FTString ) {
    pszFInfo[16] = nWidth % 256;
    pszFInfo[17] = nWidth / 256;
  }
  else {
    pszFInfo[16] = nWidth;
    pszFInfo[17] = nDecimals;
  }
    
  /*--------------------------------------------------------------------*/
  /*     Make the current record buffer appropriately larger.           */
  /*--------------------------------------------------------------------*/
  m_pszCurrentRecord = (char *)SfRealloc(m_pszCurrentRecord, m_nRecordLength);
  
  return( m_nFields-1 );
}

/************************************************************************/
/*      Read one of the attribute fields of a record.                   */
/************************************************************************/
void* shpDBF::ReadAttribute(int hEntity, int iField, char chReqType)
{
  int	nRecordOffset;
  uchar	*pabyRec;
  void	*pReturnField = NULL;
  
  static double dDoubleField;
  static char * pszStringField = NULL;
  static int	nStringFieldLen = 0;

  /* -------------------------------------------------------------------- */
  /*	Have we read the record?					*/
  /* -------------------------------------------------------------------- */
  if (hEntity < 0 || hEntity >= m_nRecords )
    return( FALSE );

  if (m_nCurrentRecord != hEntity ) {
    FlushRecord( );

    nRecordOffset = m_nRecordLength * hEntity + m_nHeaderLength;
    
    fseek( m_fp, nRecordOffset, 0 );
    fread( m_pszCurrentRecord, m_nRecordLength, 1, m_fp );
    
    m_nCurrentRecord = hEntity;
  }

  pabyRec = (uchar *) m_pszCurrentRecord;
  
  /* -------------------------------------------------------------------- */
  /*	Ensure our field buffer is large enough to hold this buffer.	*/
  /* -------------------------------------------------------------------- */
  if (m_panFieldSize[iField]+1 > nStringFieldLen ) {
    nStringFieldLen = m_panFieldSize[iField]*2 + 10;
    pszStringField = (char *) SfRealloc(pszStringField,nStringFieldLen);
  }

  /* -------------------------------------------------------------------- */
  /*	Extract the requested field.					*/
  /* -------------------------------------------------------------------- */
  strncpy_s(pszStringField, nStringFieldLen, (const char*)pabyRec + m_panFieldOffset[iField],
	   m_panFieldSize[iField] );
  pszStringField[m_panFieldSize[iField]] = '\0';

  pReturnField = pszStringField;

  /* -------------------------------------------------------------------- */
  /*      Decode the field.                                               */
  /* -------------------------------------------------------------------- */
  if (chReqType == 'N' ) {
    sscanf_s( pszStringField, "%lf", &dDoubleField );

    pReturnField = &dDoubleField;
  }

  /* -------------------------------------------------------------------- */
  /*      Should we trim white space off the string attribute value?      */
  /* -------------------------------------------------------------------- */
#ifdef TRIM_DBF_WHITESPACE
  else {
    char	*pchSrc, *pchDst;
    
    pchDst = pchSrc = pszStringField;
    while( *pchSrc == ' ' )
      pchSrc++;
    
    while( *pchSrc != '\0' )
      *(pchDst++) = *(pchSrc++);
    *pchDst = '\0';
    
    while( *(--pchDst) == ' ' && pchDst != pszStringField )
      *pchDst = '\0';  
  }
#endif
  
  return( pReturnField );
}

/************************************************************************/
/*      Read an integer attribute.                                      */
/************************************************************************/
int shpDBF::ReadIntegerAttribute( int iRecord, int iField )

{
  double	*pdValue;

  pdValue = (double *)ReadAttribute(iRecord, iField, 'N' );

  return( (int) *pdValue );
}

/************************************************************************/
/*      Read a double attribute.                                        */
/************************************************************************/

double	shpDBF::ReadDoubleAttribute( int iRecord, int iField )
{
    double	*pdValue;

    pdValue = (double *) ReadAttribute( iRecord, iField, 'N' );

    return( *pdValue );
}

/************************************************************************/
/*      Read a string attribute.                                        */
/************************************************************************/

const char * shpDBF::ReadStringAttribute( int iRecord, int iField )
{
  return( (const char *) ReadAttribute( iRecord, iField, 'C' ) );
}

/************************************************************************/
/*      Return the number of fields in this table.                      */
/************************************************************************/

int shpDBF::GetFieldCount( )
{
  return( m_nFields );
}

/************************************************************************/
/*                         DBFGetRecordCount()                          */
/*                                                                      */
/*      Return the number of records in this table.                     */
/************************************************************************/

int shpDBF::GetRecordCount( )
{
  return( m_nRecords );
}

/************************************************************************/
/*      Return any requested information about the field.               */
/************************************************************************/

DBFFieldType shpDBF::GetFieldInfo(int iField, 
																		char * pszFieldName,
																		int * pnWidth, 
																		int * pnDecimals)
{
  if (iField < 0 || iField >= m_nFields )
    return( FTInvalid );
  
  if (pnWidth != NULL )
    *pnWidth = m_panFieldSize[iField];
  
  if (pnDecimals != NULL )
    *pnDecimals = m_panFieldDecimals[iField];
  
  if (pszFieldName != NULL ) {
    int	i;
	int n = strlen(pszFieldName);
	strncpy_s(pszFieldName, 11, (char *)m_pszHeader + iField * 32, 11);
    pszFieldName[11] = '\0';
    for( i = 10; i > 0 && pszFieldName[i] == ' '; i-- )
      pszFieldName[i] = '\0';
  }
  
  if (m_pachFieldType[iField] == 'N' 
      || m_pachFieldType[iField] == 'D' ) {
    if (m_panFieldDecimals[iField] > 0 )
      return( FTDouble );
    else
      return( FTInteger );
  }
  else {
    return( FTString );
  }
}

/************************************************************************/
/*	Write an attribute record to the file.				*/
/************************************************************************/

int shpDBF::WriteAttribute(int hEntity, int iField, void * pValue )
{
  int	nRecordOffset, i, j;
  uchar	*pabyRec;
  char	szSField[40], szFormat[12];

  /* -------------------------------------------------------------------- */
  /*	Is this a valid record?						*/
  /* -------------------------------------------------------------------- */
  if (hEntity < 0 || hEntity > m_nRecords )
    return( FALSE );
  
  if (m_bNoHeader ) WriteHeader();

  /* -------------------------------------------------------------------- */
  /*      Is this a brand new record?                                     */
  /* -------------------------------------------------------------------- */
  if (hEntity == m_nRecords ) {
    FlushRecord( );

    m_nRecords++;
    for( i = 0; i < m_nRecordLength; i++ ) m_pszCurrentRecord[i] = ' ';
    
    m_nCurrentRecord = hEntity;
  }

  /* -------------------------------------------------------------------- */
  /*      Is this an existing record, but different than the last one     */
  /*      we accessed?                                                    */
  /* -------------------------------------------------------------------- */
  if (m_nCurrentRecord != hEntity ) {
    FlushRecord();

    nRecordOffset = m_nRecordLength * hEntity + m_nHeaderLength;
    
    fseek( m_fp, nRecordOffset, 0 );
    fread( m_pszCurrentRecord, m_nRecordLength, 1, m_fp );
    
    m_nCurrentRecord = hEntity;
  }
  
  pabyRec = (uchar *) m_pszCurrentRecord;
  
  /* -------------------------------------------------------------------- */
  /*      Assign all the record fields.                                   */
  /* -------------------------------------------------------------------- */
  switch( m_pachFieldType[iField] ) {
  case 'D':
  case 'N':
    if (m_panFieldDecimals[iField] == 0 ) {
	  sprintf_s(szFormat, sizeof(szFormat), "%%%dd", m_panFieldSize[iField]);
	  sprintf_s(szSField, sizeof(szSField), szFormat, (int)*((double *)pValue));
      if ((int)strlen(szSField) > m_panFieldSize[iField] )
				szSField[m_panFieldSize[iField]] = '\0';
	  strncpy_s((char *)(pabyRec + m_panFieldOffset[iField]), sizeof(pabyRec), szSField, strlen(szSField));
    }
    else {
		sprintf_s(szFormat, sizeof(szFormat), "%%%d.%df",
	       m_panFieldSize[iField],
	       m_panFieldDecimals[iField] );
		sprintf_s(szSField, sizeof(szSField), szFormat, *((double *)pValue));
      if ((int)strlen(szSField) > m_panFieldSize[iField] )
				szSField[m_panFieldSize[iField]] = '\0';
	  strncpy_s((char *)(pabyRec + m_panFieldOffset[iField]), sizeof(pabyRec), szSField, strlen(szSField));
    }
    break;

  default:
    if ((int)strlen((char *) pValue) > m_panFieldSize[iField] )
      j = m_panFieldSize[iField];
    else
      j = strlen((char *) pValue);
    
	strncpy_s((char *)(pabyRec + m_panFieldOffset[iField]), sizeof(pabyRec), (char *) pValue, j);
    break;
  }

  m_bCurrentRecordModified = TRUE;
  m_bUpdated = TRUE;
  
  return( TRUE );
}

/************************************************************************/
/*      Write a double attribute.                                       */
/************************************************************************/

int shpDBF::WriteDoubleAttribute(int iRecord, int iField, double dValue )
{
  return(WriteAttribute( iRecord, iField, (void *) &dValue ) );
}

/************************************************************************/
/*      Write a integer attribute.                                      */
/************************************************************************/

int shpDBF::WriteIntegerAttribute( int iRecord, int iField, int nValue )
{
  double	dValue = nValue;
  return( WriteAttribute( iRecord, iField, (void *) &dValue ) );
}

/************************************************************************/
/*                      DBFWriteStringAttribute()                       */
/*                                                                      */
/*      Write a string attribute.                                       */
/************************************************************************/

int shpDBF::WriteStringAttribute( int iRecord, int iField, const char * pszValue )
{
  return( WriteAttribute( iRecord, iField, (void *) pszValue ) );
}

