#pragma once
#ifndef AC_DBF_NO_ERRORS
  #define AC_DBF_NO_ERRORS

	//�������� ��������� ��� ������
    #define ERR_NOERRORS 0
	//������ ��� �������� ������� (������ ����� ���� �� ������)
	#define ERR_CANT_OPEN_TABLE 1
	//������ ������ ����� �����
	#define ERR_CANT_READ_FROM_TABLE 2
	//������ ������ � ����
	#define ERR_CANT_WRITE_TO_TABLE 3
	//��������� ����� �� ������������� �������
	#define ERR_WRONG_TABLE_FORMAT 4
	//������������� ������ ����
	#define ERR_INCOMPATIBLE_FIELD_TYPE 5
	//������������ �������� ������� (�������� � GoToRecord( dword ))
	#define ERR_WRONG_INDEX 6
	//������������ �������� ����
	#define ERR_WRONG_VALUE 7
	//��������� ������� �������� ������������ ������
	#define ERR_INCORRECT_HEADER_DATA 8
	//������ �������� �������
	#define ERR_INDEX_CLEARING_ERROR 9
	//������� ���������� � ��������������� ������� (�������� � GoToIndex (dword))
	#define ERR_INDEX_NOT_EXIST 10
	//������ ��������� ������
	#define ERR_CANT_ALLOC_MEMORY 11
	//������ ��� ���������� ������� � ����
	#define ERR_CANT_SAVE_TABLE 12

#endif