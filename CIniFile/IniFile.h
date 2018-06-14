/**
 * IniFile.h - Declaration of data structures and functions handling ini files.
 * Created by Josh Kennedy on 12 July 2017
 *
 * CIniFile: C implementation of reading ini configuration files.
 * An open source project of The DigitalMagic Company.
 *
 * Copyright (C) 2017 DigitalMagic LLC.
 */

/*
 * This software library is licensed under the University of Illinois/NCSA Open
 * Source License.
 *
 * Read LICENSE.TXT for the full license text.
 */

#include <stdbool.h>

#ifndef DM_INI_FILE_H_
#define DM_INI_FILE_H_

// Max buffer size for each line. Can be modified for the needs of the application.
#define DM_INI_MAX_LINE_BUFFER 2048

// hashtag swag y'all
#define DM_INI_COMMENT_1 '#'

// you always have to keep track of these buggers.
#define DM_INI_COMMENT_2 ';'

// One-half of "//" or beginning or ending half of a block.
#define DM_INI_COMMENT_3 '/'

// Segment of a block comment.
#define DM_INI_COMMENT_4 '*'


#define DM_INI_ERROR_MESSAGE_FOPEN_FAIL "Can't open file for reading. Please check errno"
#define DM_INI_ERROR_MESSAGE_MALLOC_FAIL "Couldn't allocate memory! This is bad."

/**
 * @brief A helping hand if/when you get errors.
 *
 * Structure that contains an error message to help out the developer.
 */
typedef struct
{
	/* The text of the error message. */
	const char* errorText;

	/* The error code that caused the message. */
	int errorCode;
} IniErrorHint;

static IniErrorHint* __IniFile_ErrorHint;

void __IniFile_SetErrorHint(const char* message, int code);
void __IniFile_ClearErrorHint();

long __IniFile_Hash(const char* str);

/**
 * @brief Grabs the latest error from this library.
 */
IniErrorHint* IniFile_GetErrorHint();

/**
 * @brief The basic data structure of an ini file.
 *
 * This is the individual item that may belong either to a section or globally.
 */
typedef struct
{
	/* The lookup value for the item. */
	const char* key;

	/* The value that the item contains. */
	const char* value;
} IniItem;

/**
 * @brief Initializes the IniItem structure.
 *
 * This function will initialize an IniItem pointer with empty values.
 *
 * @return Returns an initialized pointer to an IniItem.
 * @note Any errors will be written to IniFile_GetErrorHint().
 */
IniItem* IniItem_Initialize();

/**
 * @brief Frees the memory associated with this item.
 *
 * Uninitializes the structure and frees the memory allocated to IniItem.
 *
 * @param A non-null pointer to the structure that will be deallocated.
 */
void IniItem_Free(IniItem* item);

/**
 * @brief A named collection of IniItem's.
 *
 * Section is an optional collection of items that can be used to organize
 * items into named groups.
 */
typedef struct 
{
	/* Name of the section. */
	const char* name;

	/* Start of the list of items. */
	IniItem* itemList;
} IniSection;

/**
 * @brief Initialized a IniSection.
 *
 * This function will initialize an IniSection pointer with empty values.
 *
 * @return Returns an initialized pointer to an IniSection.
 * @note Any errors will be written to IniFile_GetErrorHint().
 */
IniSection* IniSection_Initialize();

/**
 * @brief Frees the memory associated with this section.
 *
 * Uninitializes the structure and frees the memory allocated to IniSection.
 * Which will also deallocate memory for each item in the section.
 *
 * @param A non-null pointer to the structure that will be deallocated.
 */
void IniSection_Free(IniSection* section);

/**
 * @brief This is a basic representation of an Ini file.
 *
 * The file has a global item list and a list of sections that contain items.
 *
 * @note Sections are an optional structure to help group item lists by name.
 */
typedef struct
{
	IniItem* globalList;
	IniSection* sectionList;
} IniFile;

IniFile* IniFile_ReadFile(const char* filename);

void IniFile_Free(IniFile* file);

bool __IniFile_ReadLine(const char* line, IniItem* item, IniItem* section);

bool __IniFile_IsLineCommented(const char* line);

bool __IniFile_IsBeginBlockComment(const char* line);

bool __IniFile_IsEndBlockComment(const char* line);

// TODO: Functions for section and implement the file structure and functions.

#endif
