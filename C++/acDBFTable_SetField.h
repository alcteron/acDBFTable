//============================================================//
//		INTEGER												  //
//============================================================//
	//short int
bool acDBFTable::SetField( dword r_num, word f_num, short num )
{
	if( GoToRecord(r_num) ) return SetField(f_num, num);

	bLastError = ERR_WRONG_INDEX;
	return false;
}


bool acDBFTable::SetField( word f_num, short num )
{
	byte *fld_addr;

	if( f_num >= wFieldsCount )
	{
		bLastError = ERR_WRONG_INDEX;
		return false;
	}

	fld_addr = Record.RawData + waOffset[f_num];
	if( Field[f_num].bFieldType == '2' )
	{
		*(short *)fld_addr = num;
		bLastError = ERR_NOERRORS;
		return true;
	}
	bLastError = ERR_INCOMPATIBLE_FIELD_TYPE;
	return false;
}

	//int
bool acDBFTable::SetField( dword r_num, word f_num, int num )
{
	if( GoToRecord(r_num) ) return SetField(f_num, num);

	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::SetField( word f_num, int num )
{
	byte *fld_addr;

	if( f_num >= wFieldsCount )
	{
		bLastError = ERR_WRONG_INDEX;
		return false;
	}

	fld_addr = Record.RawData + waOffset[f_num];
	if( Field[f_num].bFieldType == '4' )
	{
		*(int *)fld_addr = num;
		bLastError = ERR_NOERRORS;
		return true;
	}
	bLastError = ERR_INCOMPATIBLE_FIELD_TYPE;
	return false;
}

	//long int
bool acDBFTable::SetField( dword r_num, word f_num, __int64 num )
{
	if( GoToRecord(r_num) ) return SetField(f_num, num);

	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::SetField( word f_num, __int64 num )
{
	byte *fld_addr;

	if( f_num >= wFieldsCount )
	{
		bLastError = ERR_WRONG_INDEX;
		return false;
	}

	fld_addr = Record.RawData + waOffset[f_num];
	if( Field[f_num].bFieldType == '8' )
	{
		*(__int64 *)fld_addr = num;
		bLastError = ERR_NOERRORS;
		return true;
	}
	bLastError = ERR_INCOMPATIBLE_FIELD_TYPE;
	return false;
}


//============================================================//
//		FLOAT												  //
//============================================================//
bool acDBFTable::SetField( dword r_num, word f_num, float num )
{
	if( GoToRecord(r_num) ) return SetField(f_num, num);

	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::SetField( word f_num, float num )
{
	byte *fld_addr;
	word fld_size;
	char *temp_str;
	if( f_num >= wFieldsCount )
	{
		bLastError = ERR_WRONG_INDEX;
		return false;
	}
	
	fld_addr = Record.RawData + waOffset[f_num];
	fld_size = Field[f_num].bFieldSize;
	if( (Field[f_num].bFieldType == 'f') || (Field[f_num].bFieldType == 'F' ) )
	{
		temp_str =  (char *)malloc(fld_size + 1);
		
		memchr(temp_str, ' ', fld_size);
		temp_str[fld_size] = '\0';

		_snprintf(temp_str, fld_size, "%f", num);
		memcpy(fld_addr, temp_str, fld_size);

		free(temp_str);

		bLastError = ERR_NOERRORS;
		return true;
	}
	bLastError = ERR_INCOMPATIBLE_FIELD_TYPE;
	return false;
}