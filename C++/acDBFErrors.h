#pragma once
#ifndef AC_DBF_NO_ERRORS
  #define AC_DBF_NO_ERRORS

	//действие выполнено без ошибок
    #define ERR_NOERRORS 0
	//Ошибка при открытии таблицы (скорее всего файл не найден)
	#define ERR_CANT_OPEN_TABLE 1
	//Ошибка чтения файла файла
	#define ERR_CANT_READ_FROM_TABLE 2
	//Ошибка записи в файл
	#define ERR_CANT_WRITE_TO_TABLE 3
	//Заголовок файла не соответствует формату
	#define ERR_WRONG_TABLE_FORMAT 4
	//Несовместимый формат поля
	#define ERR_INCOMPATIBLE_FIELD_TYPE 5
	//Некорректное значение индекса (Например в GoToRecord( dword ))
	#define ERR_WRONG_INDEX 6
	//Недопустимое значение поля
	#define ERR_WRONG_VALUE 7
	//Заголовок таблицы содержит некорректные данные
	#define ERR_INCORRECT_HEADER_DATA 8
	//Ошибка удаления индекса
	#define ERR_INDEX_CLEARING_ERROR 9
	//Попытка обратиться к несуществующему индексу (Например в GoToIndex (dword))
	#define ERR_INDEX_NOT_EXIST 10
	//Ошибка выделения памяти
	#define ERR_CANT_ALLOC_MEMORY 11
	//Ошибка при сохранении таблицы в файл
	#define ERR_CANT_SAVE_TABLE 12

#endif