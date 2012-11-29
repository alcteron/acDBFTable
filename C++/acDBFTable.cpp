// acDBFTable_dll.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include "acDBFTable.h"
#include "acDBFTable_GetField.h"
#include "acDBFTable_SetField.h"
#include "acDBFTable_Index.h"

//============================================================//
//		acDBFTable class defines and implementations		  //
//============================================================//

//задает размер блоков данных, читаемых с диска 
//во время загрузки таблицы в память и/или сохранения её на диске 
const dword dwMaxReadBlockSize = 200*1024;

acDBFTable::acDBFTable( wchar_t *tname )
{
	MemBuf = NULL;
	pos = NULL;
	Field = NULL;
	
	Index = NULL;
	IndexBuf = NULL;
	dwIndexBufSize = 0;
	wIndexFCount = 0;
	waIndexFields = NULL;
	dwIndexCount = 0;

	if( !LoadTable(tname) ) return;
//===================================================================================//
	//сюда вставляются разные тестовые штуки и прочее
}

acDBFTable::~acDBFTable(void)
{
	if( MemBuf != NULL ) delete MemBuf;
	if( waOffset != NULL ) delete waOffset;
	if( Field != NULL ) delete Field;
	if( f != NULL ) fclose(f);
	if( Index != NULL )
	{
		ClearIndex();
		free( IndexBuf );
	}
	if( waIndexFields != NULL ) free(waIndexFields);
}

//============================================================//
//	acDBFTable::LoadTable(...); acDBFTable::CloseTable(...)	  //
//============================================================//
bool acDBFTable::LoadTable( wchar_t *tname )
{
	size_t i;
	dword j;
	char c;

	//Init file I/O
	if( _wfopen_s( &f, tname, L"r+") != 0 )	
	{
		bLastError = ERR_CANT_OPEN_TABLE;
		f = NULL;
		MemBuf = NULL;
		pos = NULL;
		wFieldsCount = 0;
		Record.RawData = NULL;
		Field = NULL;
		dwCurrentRecord = NULL;
		waOffset = NULL;
		return false;
	}
	if( fread(&Header, sizeof(Header), 1, f) == 0 ) goto err_cantopentable;
	
	//Read header info
	wFieldsCount = Header.wHeaderSize - sizeof(Header) - 1;
	wFieldsCount /= sizeof(*Field);

	//Read fields info
	Field = new asDBFField[wFieldsCount];
	if( fread(Field, sizeof(*Field)*wFieldsCount, 1, f) == 0 ) goto err_cantopentable2;

	//calculation offsets for evety field
	waOffset = new word[wFieldsCount];
	j = 0; ////remember deletion flag
	for( i = 0; i < wFieldsCount; i++ )
	{
		waOffset[i] = j;
		j += Field[i].bFieldSize;
	}

	if( fread(&c, 1, 1, f) == 0 ) goto err_cantopentable2;
	if( c != 13 )
	{
		bLastError = ERR_WRONG_TABLE_FORMAT;
		fclose(f);
		return false;
	}

	MemBuf = new byte[Header.wRecordSize*Header.dwRecordsCount];

	i = fread(MemBuf, Header.wRecordSize, Header.dwRecordsCount, f);
	//здесь можно добавить дополнительную проверку на нессответствие реально количества
	//реально прочитанных записей их количеству, указанному в заголовке таблицы

	if( Index != NULL ) ClearIndex();
	//сюда можно добавить поиск и загрузку индексного файла, но этого не требуется
	//для текущей задачи, так что можно оставить реализацию этой задачи на будующее.

	fclose(f);
	bLastError = ERR_NOERRORS;
	return true;
	
err_cantopentable2:
	delete Field;
	Field = NULL;
err_cantopentable:
	bLastError = ERR_CANT_READ_FROM_TABLE;
	fclose(f);
	return false;
}

bool acDBFTable::SaveTable(wchar_t *tname)
{
	FILE *a;
	byte s = 0x0D;
	size_t result;
	
	if( !Pack() ) return false;
		
	_wfopen_s(&a, tname, L"wb");
	
	result =  fwrite(&Header, sizeof(asDBFHeader), 1, a);
	if( result == 0 ) goto err_cantsavetable;

	result = fwrite(Field, sizeof(asDBFField), wFieldsCount, a);
	if( result == 0 ) goto err_cantsavetable;

	result = fwrite(&s, sizeof(byte), 1, a);
	if( result == 0 ) goto err_cantsavetable;

	result = fwrite(MemBuf, Header.wRecordSize, Header.dwRecordsCount, a);
	if( result == 0 ) goto err_cantsavetable;

	result = fclose(a);
	if( result != 0 ) goto err_cantsavetable;

	bLastError = ERR_NOERRORS;
	return true;

err_cantsavetable:
	bLastError = ERR_CANT_SAVE_TABLE;
	return false;
}

//============================================================//
//	acDBFTable::GoPrev(); acDBFTable::GoNext();				  //
//	acDBFTable::GoToRecord(...); acDBFTable::UpdateRecord()   //
//============================================================//
bool acDBFTable::GoNext()//working
{
	if( dwCurrentRecord < (Header.dwRecordsCount - 1) )
	{
		dwCurrentRecord++; 
		UpdateCurrentRecord(); 

		bLastError = ERR_NOERRORS;
		return true;
	}
	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::GoPrev()//working
{
	if( dwCurrentRecord > 0 )
	{ 
		dwCurrentRecord--; 
		UpdateCurrentRecord(); 

		bLastError = ERR_NOERRORS;
		return true;
	}
	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::GoToRecord(dword rec_no)
{
	if( rec_no < Header.dwRecordsCount ) 
	{
		dwCurrentRecord = rec_no; 
		UpdateCurrentRecord();

		bLastError = ERR_NOERRORS;
		return true;
	}
	bLastError = ERR_WRONG_INDEX;
	return false;	
}

void acDBFTable::UpdateCurrentRecord()//working, unsafe but private
{
	pos = MemBuf + dwCurrentRecord*Header.wRecordSize;
	Record.bDeletionFlag = *((char *)pos);
	Record.RawData = pos + 1;
}

//============================================================//
//		acDBFTable::IsDeleted(...)							  //
//============================================================//
bool acDBFTable::IsDeleted( dword r_num )
{
	if( GoToRecord(r_num) )
	{
		bLastError = ERR_NOERRORS;
		if( Record.bDeletionFlag == 0x20 ) return false;
		return true;
	}
	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::IsDeleted( void )
{
	bLastError = ERR_NOERRORS;
	if( Record.bDeletionFlag == 0x20 ) return false;
	return true;
}

//============================================================//
//		acDBFTable::acFieldToTrimStr(...)					  //
//============================================================// //work
	//left trim
char *acDBFTable::FieldToLTrimStr( dword r_num, word f_num, char *dest, size_t size )
{
	if( GoToRecord(r_num) ) return FieldToLTrimStr(f_num, dest, size);	
	return NULL;
}

char *acDBFTable::FieldToLTrimStr( word f_num, char *dest, size_t size )
{
	byte *fld_addr;
	size_t fld_size = Field[f_num].bFieldSize;
	fld_addr = Record.RawData + waOffset[f_num];

	while( (fld_size > 0 ) && (fld_addr[0] == ' ') ) 
	{
		fld_addr++;
		fld_size--;
	}
	if( size < fld_size ) fld_size = size;
	memcpy(dest, fld_addr, size);
	dest[fld_size] = '\0';
	return dest;
}
	//right trim
char *acDBFTable::FieldToRTrimStr( dword r_num, word f_num, char *dest, size_t size )
{
	if( GoToRecord(r_num) ) return FieldToRTrimStr(f_num, dest, size);	
	return NULL;
}

char *acDBFTable::FieldToRTrimStr( word f_num, char *dest, size_t size )
{
	byte *fld_addr, *e;
	size_t fld_size = Field[f_num].bFieldSize;
	fld_addr = Record.RawData + waOffset[f_num];
	
	e = fld_addr + fld_size-1;
	while( (fld_size > 0 ) && (*e == ' ') )
	{
		e--;
		fld_size--;
	}
	if( size < fld_size ) fld_size = size;
	memcpy(dest, fld_addr, fld_size);
	dest[fld_size] = '\0';
	return dest;
}
	//left and right trim
char *acDBFTable::FieldToATrimStr( dword r_num, word f_num, char *dest, size_t size )
{
	if( GoToRecord(r_num) ) return FieldToATrimStr(f_num, dest, size);	

	bLastError = ERR_WRONG_INDEX;
	return NULL;
}

char *acDBFTable::FieldToATrimStr( word f_num, char *dest, size_t size )
{
	byte *fld_addr, *e;
	size_t fld_size = Field[f_num].bFieldSize;
	fld_addr = Record.RawData + waOffset[f_num];
	e = fld_addr + fld_size - 1;
	
	while( (fld_size > 0) && (fld_addr[0] == ' ') ) 
	{
		fld_addr++;
		fld_size--;
	}
	while( (fld_size > 0) && (*e == ' ') )
	{
		e--;
		fld_size--;
	}
	if( size < fld_size ) fld_size = size;
	memcpy(dest, fld_addr, fld_size);
	dest[fld_size] = '\0';
	return dest;
}

//============================================================//
//		acDBFTable::acRecordToStr(...)						  //
//============================================================//  //work
char *acDBFTable::RecordToStr( dword r_num, char *dest, size_t size )
{
	if( GoToRecord(r_num) ) return RecordToStr(dest, size);

	bLastError = ERR_WRONG_INDEX;
	return NULL;
}

char *acDBFTable::RecordToStr( char *dest, size_t size )
{
	char *pos;
	word i;
	
	pos = dest;
	i = 0;
	while( i < wFieldsCount )
	{
		if( (size-1) > Field[i].bFieldSize )
		{
			GetField(i, pos);
			pos += Field[i].bFieldSize;
			*pos = ' ';	pos++;
			*pos = ' ';	pos++;
			size -= Field[i].bFieldSize + 3;

		} 
		else
		{
			pos++; 
			*pos = '\0'; 
			break;
		}
		i++;
	}
	*pos = '\0';
	return dest;
}

//============================================================//
//		acDBFTable::acTrimField(...)						  //
//============================================================//
bool acDBFTable::TrimField( dword r_num, word f_num )  //work
{
	if( GoToRecord( r_num ) ) return TrimField( f_num );
	
	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::TrimField( word f_num )  //work
{
	byte size;
	char *source, *dest, *e;

	if( f_num < wFieldsCount )
	{
		source = (char *)(Record.RawData + waOffset[f_num]);
		dest = source;

		size = Field[f_num].bFieldSize;
		e = source + size - 1;

		while( (size > 0) && (*source == ' ') )
		{
			source++;
			size--;
		}
		while( (size > 0) && (*source == ' ') )
		{
			e--;
			size--;
		}
		memcpy(dest, source, size);
		dest += size;
		memchr(dest, ' ', Field[f_num].bFieldSize - size);

		bLastError = ERR_NOERRORS;
		return true;
	}
	bLastError = ERR_WRONG_INDEX;
	return false;
}

//============================================================//
//		acDBFTable::GetFieldAddress(...)					  //
//============================================================//

//возвращает указатель типа byte на первый байт указанного
//поля указанной записи
//быстродействие O(1)
byte *acDBFTable::GetFieldAddress( dword r_num, word f_num )
{
	if( GoToRecord( 0 ) ) return GetFieldAddress( f_num );

	bLastError = ERR_WRONG_INDEX;
	return NULL;
}

//возвращает указатель типа byte на первый байт указанного 
//поля текущей записи
//быстродействие O(1)
byte *acDBFTable::GetFieldAddress( word f_num )
{
	if( f_num < wFieldsCount )
	{
		bLastError = ERR_NOERRORS;
		return (byte *)(Record.RawData + waOffset[f_num]);
	}
	bLastError = ERR_WRONG_INDEX;
	return NULL;
}

bool acDBFTable::Pack( void )
{	
//	char *temp_str;
	byte *rec_addr;
	byte *rec_addr_new;
//	byte del_flag; 
	byte del_flag_new; 
	for(dword i = 0; i<Header.dwRecordsCount;i++)
	{
		if (IsDeleted(i))
		{
			GoToRecord(Header.dwRecordsCount-1);
			rec_addr_new = Record.RawData;
			del_flag_new = Record.bDeletionFlag;
			GoToRecord(i);
			rec_addr = Record.RawData;
			*(char *)(rec_addr-1)=del_flag_new;
			for(int k=0; k<Header.wRecordSize-1;k++)
			{
				*(char *)rec_addr = *(char *)rec_addr_new;
				rec_addr++;
				rec_addr_new++;
			}
			Header.dwRecordsCount--;
			if (del_flag_new == '*') i--;
		}
	}
		return true;
}


/*/ Пример экспортированной переменной
ACDBFTABLE_DLL_API int nacDBFTable_dll=0;

// Пример экспортированной функции.
ACDBFTABLE_DLL_API int fnacDBFTable_dll(void)
{
	return 42;
}

// Конструктор для экспортированного класса.
// см. определение класса в acDBFTable_dll.h
/*CacDBFTable_dll::CacDBFTable_dll()
{
	return;
}*/
