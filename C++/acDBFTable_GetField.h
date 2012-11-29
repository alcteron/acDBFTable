//============================================================//
//		STRING												  //
//============================================================//
bool acDBFTable::GetField( word f_num, char *dest )//working on 'C', 'D', 'N', 'F' modes
{
	byte *fld_addr;
	char *temp_str;
	word fld_size;
	
	if( f_num >= wFieldsCount ) 
	{
		bLastError = ERR_WRONG_INDEX;
		return false;
	}
	
	fld_addr = Record.RawData + waOffset[f_num];
	fld_size = Field[f_num].bFieldSize;
	switch( Field[f_num].bFieldType)
	{
	case 'c':	//string
	case 'C':	//strcpy_s(dest, Field[f_num].bFieldSize, (char *)rec_addr);
				memcpy(dest, fld_addr, fld_size);
				dest += fld_size;
				*dest = '\0';
				break;
	case 'd':	//date ИСПРАВИТЬ!!!!!!!!!!!!!!!!!!!!!!
	case 'D':	*dest = *fld_addr; dest++;
				*dest = *(fld_addr + 1); dest++; //year
				*dest = *(fld_addr + 2); dest++;
				*dest = *(fld_addr + 3); dest++;
				*dest = '/'; dest++;
				*dest = *(fld_addr + 4); dest++; //month
				*dest = *(fld_addr + 5); dest++;
				*dest = '/'; dest++;
				*dest = *(fld_addr + 6); dest++; //day
				*dest = *(fld_addr + 7); dest++;
				memcpy(dest, fld_addr, 10);
				dest[4] = '/';
				dest[7] = '/';
				dest[10] = '\0'; //used for YYYY/MM/DD format
				break;
	case 'n':	//fixed point
	case 'N':	
	case 'f':	//float point
	case 'F':	temp_str = (char *)malloc(fld_size + 1);
				temp_str[fld_size] = '\0';
				memcpy(temp_str, fld_addr, fld_size);
				if( _atoflt( (_CRT_FLOAT *)dest, temp_str ) != 0 )
				{
					bLastError = ERR_WRONG_VALUE;
					return false;
				}
				free(temp_str);
				break;
				//binary 2-byte integer
	case '2':	*(short int *)dest = *(short int *)fld_addr; //rewrite
				break;
				//binary 4-byte integer
	case '4':	*(int *)dest = *(int *)fld_addr; //rewrite
				break;
				//binary 8-byte integer
	case '8':	*(__int64 *)dest = *(__int64 *)fld_addr; //rewrite
	}

	bLastError = ERR_NOERRORS;
	return true;
}

bool acDBFTable::/*GetField*/GetField( dword r_num, word f_num, char *dest ) //working
{
	if( GoToRecord(r_num) ) return GetField(f_num, dest);

	bLastError = ERR_WRONG_INDEX;
	return false;
}


//============================================================//
//		DATE												  //
//============================================================//
bool acDBFTable::GetField(dword r_num, word f_num, word &day, word &month, word &year)//working
{
	if( GoToRecord(r_num) ) return GetField(f_num, day, month, year);

	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::GetField( word f_num, word &day, word &month, word &year )//working
{
	byte *fld_addr;
	char temp_str[5];
	if( f_num >= wFieldsCount )
	{
		bLastError = ERR_WRONG_INDEX;
		return false;
	}
	
	if( (Field[f_num].bFieldType == 'd') || (Field[f_num].bFieldType == 'D') )
	{
		fld_addr = Record.RawData + waOffset[f_num];

		temp_str[0] = *fld_addr;
		temp_str[1] = *(fld_addr + 1);
		temp_str[2] = *(fld_addr + 2);
		temp_str[3] = *(fld_addr + 3);
		temp_str[4] = '\0';
		year = atoi(temp_str);  

		temp_str[0] = *(fld_addr + 4);
		temp_str[1] = *(fld_addr + 5);
		temp_str[2] = '\0';		
		month = atoi(temp_str);

		temp_str[0] = *(fld_addr + 6);
		temp_str[1] = *(fld_addr + 7);
		day = atoi(temp_str);

		bLastError = ERR_NOERRORS;
		return true;
	}

	bLastError = ERR_INCOMPATIBLE_FIELD_TYPE;
	return false;
}


//============================================================//
//		FLOAT												  //
//============================================================//
bool acDBFTable::GetField( dword r_num, word f_num, float &num )
{
	if( GoToRecord(r_num) ) return GetField(f_num, num);
	
	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::GetField( word f_num, float &num )
{
	byte *fld_addr;
	word fld_size;
	char *temp_str;
	if( f_num >= wFieldsCount )
	{
		bLastError = ERR_WRONG_INDEX;
		return false;
	}

	if( (Field[f_num].bFieldType == 'f') || (Field[f_num].bFieldType == 'F') || (Field[f_num].bFieldType == 'n') || Field[f_num].bFieldType == 'N' )
	{
		fld_addr = Record.RawData + waOffset[f_num];
		fld_size = Field[f_num].bFieldSize;

		temp_str = new char[fld_size + 1];

		temp_str[fld_size] = '\0';
		memcpy(temp_str, (void *)fld_addr, fld_size);
		//Есть вариант сделать в начале .cpp файла #define fld_size Field[f_num].bFieldSize
		//и в конце #undef fld_size соответственно, однако не факт, что это сильно ускорит программу

		if( _atoflt( (_CRT_FLOAT *)&num, temp_str) == 0 )
		{
			bLastError = ERR_NOERRORS;
			return true;
		}
		delete temp_str;
		bLastError = ERR_WRONG_VALUE;
		return false;
	}

	bLastError = ERR_INCOMPATIBLE_FIELD_TYPE;
	return false;
}

//============================================================//
//		INTEGER												  //
//============================================================//
bool acDBFTable::GetField( dword r_num, word f_num, __int64 &num)
{
	if( GoToRecord(r_num) ) return GetField(f_num, num);

	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::GetField( word f_num, __int64 &num )
{
	byte *fld_addr;
	if( f_num >= wFieldsCount )
	{
		bLastError = ERR_WRONG_INDEX;
		return false;
	}

	fld_addr = Record.RawData + waOffset[f_num];
	switch(Field[f_num].bFieldType)
	{
	case '2':	num = *(short *)fld_addr;
				break;
	case '4':	num = *(int *)fld_addr;
				break;
	case '8':	num = *(__int64 *)fld_addr;
				break;
	default:	bLastError = ERR_INCOMPATIBLE_FIELD_TYPE;
				return false; 
	}
	bLastError = ERR_NOERRORS;
	return true;
}