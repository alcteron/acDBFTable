//»спользуетс€ дл€ создани€ нового индекса DBF таблицы
//argc - количество аргументов
//argv - массив индексов полей, из который и будет формироватьс€
//сводный ключ дл€ индексации. 
//argv копируетс€ в класс, так что требуетс€ освобождение в главной программе
//приблизительное быстродействие - O(n) дл€ генерации ключа и O(log(n)) дл€ сортировки
// based on merge sort method
// go to http://ru.wikipedia.org/wiki/—ортировка_сли€нием
//      or
//       http://en.wikipedia.org/wiki/Merge_sort
//		and
//		 http://ru.wikibooks.org/wiki/ѕример_реализации_сортировки_сли€нием
//      or
//		 http://en.wikibooks.org/wiki/Algorithm_Implementation/Sorting/Merge_sort
// for more theory, samples and other information
// work for O(log(n))
bool acDBFTable::MakeIndex(word argc, word *argv)
{
	dword i, j, k;//, t;
	dword blocksize = 2;
	dword lsize, rsize;
	char *cpos;

	char fs[KeyLength];

	asDBFIndex *pa, *pb, *cb, *tmp_index = NULL;

//validation
	//проверка аргументов
	if( argc == 0 )
	{
		bLastError = ERR_WRONG_VALUE;
		return false;
	}

	for( i = 0; i < argc; i++ ) if( argv[i] >= wFieldsCount ) 
	{
		bLastError = ERR_WRONG_INDEX;
		return false;
	}

	//очистка пам€ти
	ClearIndex();

	//сохранение полей индекса ключа
	wIndexFCount = argc;
	waIndexFields = (word *)realloc(waIndexFields, sizeof(word)*argc);
	memcpy(waIndexFields, argv, sizeof(word)*argc);

	//проверка и подготовка пам€ти дл€ хранени€ индекса
	if( dwIndexBufSize != sizeof(asDBFIndex)*Header.dwRecordsCount )
	{
		dwIndexBufSize = (sizeof(asDBFIndex) + KeyLength)*Header.dwRecordsCount;
		IndexBuf = (byte *)realloc(IndexBuf, dwIndexBufSize);
		if( IndexBuf == NULL )
		{
			bLastError = ERR_CANT_ALLOC_MEMORY;
			return false;
		}
	}
	
	Index = (asDBFIndex *)IndexBuf;
	cpos = (char *)(IndexBuf + sizeof(asDBFIndex)*Header.dwRecordsCount);

//создание ключей
	GoToRecord(0);
	for( i = 0; i < Header.dwRecordsCount; i++ ) 
		if( !IsDeleted() ) //Record.bDeletionFlag == ' '
		{	
			Index[i].dwRecNum = i;
			Index[i].Key = cpos;
			for( j = 0; j < wIndexFCount; j++ )
			{
				FieldToATrimStr(waIndexFields[j], fs, KeyLength);
				memcpy(cpos, fs, strlen(fs));
				cpos += strlen(fs);
			}
			*(cpos++) = '\0';
#if defined AC_DBF_TABLE_USE_INDEX_HANDLER
			IndexHandler(Index[i].Key); //дополнительна€ обработка сформированного ключа
#endif
			dwIndexCount++;
			GoNext();
		}

//сортировка ключей
	//выделение пам€ти
	tmp_index = (asDBFIndex *)malloc(sizeof(asDBFIndex)*dwIndexCount);
	pa = tmp_index;
    pb = Index;
	//первична€ сортировка
	for( i = 0; i < dwIndexCount-1; i+=2 )//первична€ сортировка
		if( strcmp(Index[i].Key, Index[i+1].Key) < 0 ) { tmp_index[i] = Index[i]; tmp_index[i+1] = Index[i+1]; }
												  else { tmp_index[i] = Index[i+1]; tmp_index[i+1] = Index[i]; }
											
	
	if( i < dwIndexCount ) tmp_index[i] = Index[i];

    //инициализаци€
	pa = tmp_index;
    pb = Index;
	blocksize = 2;

	//основна€ сортировка
	while( blocksize < dwIndexCount ) //perform while block size are less then array size
	{
		i = 0; //setting base indexes for left block
		k = 0; // and result array
		while( i < dwIndexCount ) //while entire array aren't scanned
		{
			if( (i + blocksize) >= dwIndexCount ) //if only one block remain don't touch him,
			{
				memcpy(&pb[k], &pa[i], sizeof(asDBFIndex)*(dwIndexCount - i) ); //just copy this block to output array
				k += dwIndexCount - i;
				break; 
			}

			lsize = blocksize; //set length of left block
			if( (i + blocksize + blocksize) < dwIndexCount ) rsize = blocksize; 
														else rsize = dwIndexCount - i - blocksize; //set length of right block

			j = i + lsize; //set base index for right block

			while( lsize && rsize ) //starting merging of blocks
			{
				if( strcmp(pa[i].Key, pa[j].Key) <= 0 )
				{
					pb[k] = pa[i];
					i++;
					lsize--;
				}
				else
				{
					pb[k] = pa[j];
					j++;
					rsize--;
				}
				k++;
			}
			if( rsize == 0 ){ memcpy(&pb[k], &pa[i], lsize*sizeof(*pa)); i += lsize; k += lsize;}
			if( lsize == 0 ){ memcpy(&pb[k], &pa[j], rsize*sizeof(*pa)); j += rsize; k += rsize;}
			i += blocksize;
		}
		blocksize *= 2; //double size of block
		cb = pb; pb = pa; pa = cb; //switch input and output arrays
	}
	
//free memory
	if( Index != pa ) memcpy(Index, tmp_index, dwIndexCount * sizeof(asDBFIndex));
	free(tmp_index);
	pb = NULL;
	pa = NULL;
	tmp_index = NULL;

	bLastError = ERR_NOERRORS;
	return true;//ReIndex();
}

// bool ReIndex( void )
bool acDBFTable::ReIndex( word argc, word *argv )
{
	ClearIndex();
	return MakeIndex(argc, argv);
}

//ќчищает существующий индекс.
//≈сли индекс не существует, не делает ничего
//Ѕыстродействие O(n)
void acDBFTable::ClearIndex( void )
{
	//dword i;
	//delete Index;
	if( Index != NULL ) Index = NULL;
	
	wIndexFCount = 0;
	dwIndexCount = 0;
}

//сравнивает два ключа в индексе а, и если первый ключ меньше,
//возвращает true
//быстродействие O(1)

bool acDBFTable::GoToIndex( dword i_num )
{
	if( i_num < dwIndexCount )
	{
		dwCurrentIndex = i_num;
		dwCurrentRecord = Index[i_num].dwRecNum;
		UpdateCurrentRecord();
		bLastError = ERR_NOERRORS;
		return true;
	}
	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::GoNextIndex( void )
{
	if( dwCurrentIndex < (dwIndexCount - 1) )
	{
		dwCurrentIndex++;
		dwCurrentRecord = Index[dwCurrentIndex].dwRecNum;
		UpdateCurrentRecord();
		bLastError = ERR_NOERRORS;
	}
	bLastError = ERR_WRONG_INDEX;
	return false;
}

bool acDBFTable::GoPrevIndex( void )
{
	if( dwCurrentIndex > 0 )
	{
		dwCurrentIndex--;
		dwCurrentRecord = Index[dwCurrentIndex].dwRecNum;
		UpdateCurrentRecord();
		bLastError = ERR_NOERRORS;
	}
	bLastError = ERR_WRONG_INDEX;
	return false;
}