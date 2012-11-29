// Приведенный ниже блок ifdef - это стандартный метод создания макросов, упрощающий процедуру 
// экспорта из библиотек DLL. Все файлы данной DLL скомпилированы с использованием символа ACDBFTABLE_DLL_EXPORTS,
// заданного в командной строке. Данный символ не должен быть определен ни в одном проекте,
// использующем данную DLL. Благодаря этому любой другой проект, чьи исходные файлы включают данный файл, видит 
// функции ACDBFTABLE_DLL_API как импортированные из DLL, тогда как данная DLL видит символы,
// определяемые данным макросом, как экспортированные.
#ifdef ACDBFTABLE_DLL_EXPORTS
#define ACDBFTABLE_DLL_API __declspec(dllexport)
#else
#define ACDBFTABLE_DLL_API __declspec(dllimport)
#endif

#include "BasicTypes.h"
#include "acDBFErrors.h"

//if you want to use additional index handler, then write it
//and then uncomment line below
#define AC_DBF_TABLE_USE_INDEX_HANDLER
//the index handler should be declared as
//void IndexHandler( char * )
//where argument determines key value
#pragma pack(1)
struct asDBFHeader
{
	byte bFlags;            //01 byte
	byte bYear, bMonth, bDay; //03 bytes
	dword dwRecordsCount;   //04 bytes
	word wHeaderSize;       //02 bytes
	word wRecordSize;       //02 bytes
	word wReserved;         //02 bytes
	byte bTransactionFlags; //01 byte
	byte bEncryption;       //01 byte
	byte baUseUserEnvironment[12]; //12 bytes
	byte bUseIndex;         //01 byte
	byte bCodePage;         //01 byte
	word wReserved2;        //02 bytes
};

#pragma pack(1)
struct asDBFField
{
	char strName[11];		//11 bytes
	byte bFieldType;		//01 byte
	dword dwAddress;		//04 bytes
	byte bFieldSize;		//01 byte
	byte bFractionalSize;	//01 byte
	word wReserved;			//02 bytes
	byte bWorkAreaID;		//01 byte
	word wMutiUser;			//02 bytes
	byte bSetFields;        //01 byte
	byte baReserved[7];		//07 bytes
	byte MDXIDIncluded;		//01 byte
};

struct asDBFRecord
{
	byte bDeletionFlag;     //01 byte
	//custom data
	byte *RawData;          //04 bytes
};

#define KeyLength 192
struct ACDBFTABLE_DLL_API asDBFIndex
{
	dword dwRecNum;
	char *Key;
};


struct ACDBFTABLE_DLL_API asDBFTableOptions
{
	bool isKeepDeletedRecords;
};

class ACDBFTABLE_DLL_API acDBFTable
{
private:
	friend inline bool IsLesser( asDBFIndex *a, dword i1, dword i2 );
//public:
	FILE *f;
	byte *MemBuf;
	byte *pos;
	byte *IndexBuf;

	word *waIndexFields;
	word wIndexFCount;

	dword dwIndexBufSize;
public:
	dword dwCurrentRecord; //current record
	dword dwCurrentIndex;  //current index

private:
	void UpdateCurrentRecord( void ); //used in GoTo(...), GoNext() and GoPrev()
public:
	acDBFTable( wchar_t * tname );
	~acDBFTable(void);

	asDBFHeader Header; //represent common information from table header
	asDBFField *Field;  //array of records, which describes table structure
	asDBFRecord Record; //structure, which represents current record in table
	asDBFIndex *Index;  //structure, which represents table index

	byte bLastError;    //last error code
	word wFieldsCount;  //number of records in table
	dword dwIndexCount; //number of indexed items
	word *waOffset;     //array of relative offsets for each field in record

	//safe
	bool LoadTable( wchar_t *tname ); //load table into program memory
	bool SaveTable( wchar_t *tname ); //save table to file

	bool Pack( void );

	bool GoToRecord(dword rec_no);    //change current record to rec_no
	bool GoNext( void );              //go to next record if possible
	bool GoPrev( void );			  //go to prev record if possible

	//returns value of the f_num field of the r_num or current record as string
	bool GetField( word f_num, char *dest );
	bool GetField( dword r_num, word f_num, char *dest );
	bool GetField( word f_num, word &year, word &month, word &day );
	bool GetField( dword r_num, word f_num, word &year, word &month, word &day );
	bool GetField( word f_num, float &num );
	bool GetField( dword r_num, word f_num, float &num );
	bool GetField( word f_num, __int64 &num );
	bool GetField( dword r_num, word f_num, __int64 &num );
	//safe

	//unsafe
	char *FieldToLTrimStr( dword r_num, word f_num, char *dest, size_t size ); //return field value as trimmed string
	char *FieldToLTrimStr( word f_num, char *dest, size_t size );				//left trim
	
	char *FieldToRTrimStr( dword r_num, word f_num, char *dest, size_t size );
	char *FieldToRTrimStr( word f_num, char *dest, size_t size );				//right trim

	char *FieldToATrimStr( dword r_num, word f_num, char *dest, size_t size );
	char *FieldToATrimStr( word f_num, char *dest, size_t size );				//left and right trim
	//unsafe

	//safe
	char *RecordToStr( dword r_num, char *dest, size_t size ); //converts values of the whole record to single string
	char *RecordToStr( char *dest, size_t size );

	bool SetField( dword r_num, word f_num, short num );		//sets
	bool SetField( dword r_num, word f_num, int num );
	bool SetField( dword r_num, word f_num, __int64 num );

	bool SetField( word f_num, short num );
	bool SetField( word f_num, int num );
	bool SetField( word f_num, __int64 num );

	bool SetField( dword r_num, word f_num, float num );
	bool SetField( word f_num, float num );

	bool TrimField( dword r_num, word f_num );
	bool TrimField( word f_num );

	//delete r_num or current record
	bool IsDeleted( dword r_num );
	bool IsDeleted( void );

	byte *GetFieldAddress( dword r_num, word f_num );
	byte *GetFieldAddress( word f_num );

	//index performing finctions
	bool MakeIndex( word argc, word *argv );
	void ClearIndex( void );
	bool ReIndex( word argc, word *argv );

	bool GoToIndex( dword i_num );
	bool GoNextIndex( void );
	bool GoPrevIndex( void );
	//safe

	friend void IndexHandler( char * );
};
// Этот класс экспортирован из acDBFTable_dll.dll
/*class ACDBFTABLE_DLL_API acDBFTable {
public:
	CacDBFTable_dll(void);
	// TODO. Добавьте здесь свои методы.
};

extern ACDBFTABLE_DLL_API int nacDBFTable_dll;

ACDBFTABLE_DLL_API int fnacDBFTable_dll(void);*/

void IndexHandler( char *strKey )
{
	OemToCharA(strKey, strKey);
	_strlwr(strKey);
	for( unsigned int i = 0; i < strlen(strKey); i++ )
	{
		if( strKey[i] == 'ё' ) strKey[i] = 'е';
		if( strKey[i] == 'ъ' ) strKey[i] = 'ь';
		if( strKey[i] == 'й' ) strKey[i] = 'и';
	}
}