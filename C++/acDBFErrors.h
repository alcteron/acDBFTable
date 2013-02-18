#pragma once
#ifndef AC_DBF_NO_ERRORS
  #define AC_DBF_NO_ERRORS

	// Operation successful
    #define ERR_NOERRORS 0
	// Table opening error (file not found)
	#define ERR_CANT_OPEN_TABLE 1
	// Error reading data from file
	#define ERR_CANT_READ_FROM_TABLE 2
	// Error writing data to file
	#define ERR_CANT_WRITE_TO_TABLE 3
	// Incorrect file header
	#define ERR_WRONG_TABLE_FORMAT 4
	// Unsupported field type
	#define ERR_INCOMPATIBLE_FIELD_TYPE 5
	// Incorrect index value (For example in GoToRecord( dword ))
	#define ERR_WRONG_INDEX 6
	// Invalid field value
	#define ERR_WRONG_VALUE 7
	// Incorrect data in table header
	#define ERR_INCORRECT_HEADER_DATA 8
	// Index deteion error
	#define ERR_INDEX_CLEARING_ERROR 9
	// Attempt to access to the non exist index (For example in GoToIndex (dword))
	#define ERR_INDEX_NOT_EXIST 10
	// Memory allocation error
	#define ERR_CANT_ALLOC_MEMORY 11
	// Table saving error
	#define ERR_CANT_SAVE_TABLE 12

#endif
