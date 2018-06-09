/**
 * IniFile.c - Implementation of IniFile.h
 * Created by Josh Kennedy on 13 July 2017
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

#include "IniFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <stdbool.h>

#define HASH_SIZE 8675309

IniErrorHint* __IniFile_ErrorHint = NULL;

/* Utility methods */

#ifdef _WIN32
char* strndup(const char* s, size_t n)
{
	char* result;
	size_t len = strlen(s);

	if (n < len)
		len = n;

	result = (char*)malloc(len + 1);
	if (!result)
		return 0;

	result[len] = '\0';
	return (char*)memcpy(result, s, len);
}
#endif

char* substring(const char* str, size_t begin, size_t len)
{
	if (str == 0 || strlen(str) == 0 || strlen(str) < begin || 
		strlen(str) < (begin + len))
		return 0;

	return strndup(str + begin, len);
}

bool startsWith(const char* str, const char* search)
{
	return (strncmp(str, search, strlen(search)) == 0);
}

/* Error handling code */

void __IniFile_SetErrorHint(const char* message, int code)
{
	if (!__IniFile_ErrorHint)
	{
		__IniFile_ErrorHint = (IniErrorHint*)malloc(sizeof(IniErrorHint));
	}

	__IniFile_ErrorHint->errorText = message;
	__IniFile_ErrorHint->errorCode = code;
}

void __IniFile_ClearErrorHint()
{
	if (!__IniFile_ErrorHint) return;

	free(__IniFile_ErrorHint);

	__IniFile_ErrorHint = NULL;
}

IniErrorHint* IniFile_GetErrorHint()
{
	return __IniFile_ErrorHint;
}

/* The real meaty parts */

long __IniFile_Hash(const char* str)
{
	long val = 1;
	const char* s = str;

	for (val = 1; *s != '\0'; ++s)
	{
		val = *s + 179 * val;
	}

	return val % HASH_SIZE;
}

IniItem* IniItem_Initialize()
{
	IniItem* item = NULL;
	__IniFile_ClearErrorHint();

	item = malloc(sizeof(IniItem));

	if (!item)
	{
		__IniFile_SetErrorHint(DM_INI_ERROR_MESSAGE_MALLOC_FAIL, 5);
	}

	return item;
}

void IniItem_Free(IniItem* item)
{
	if (!item) return;

	free(item);
}

IniSection* IniSection_Initialize()
{
	IniSection* section = NULL;

	__IniFile_ClearErrorHint();

	section = malloc(sizeof(IniSection));

	if (!section)
	{
		__IniFile_SetErrorHint(DM_INI_ERROR_MESSAGE_MALLOC_FAIL, 6);
	}
	
	return section;
}

void IniSection_Free(IniSection* section)
{
	IniItem* item = NULL;

	if (!section) return;

	item = section->itemList;

	while (item)
	{
		IniItem_Free(item);

		item++;
	}

	free(section);
}

IniFile* IniFile_ReadFile(const char* filename)
{
	FILE* fp = NULL;

	__IniFile_ClearErrorHint();

	fp = fopen(filename, "r");

	if (!fp)
	{
		__IniFile_SetErrorHint(DM_INI_ERROR_MESSAGE_FOPEN_FAIL, errno);

		return NULL;
	}

	fclose(fp);

	return NULL;
}

void IniFile_Free(IniFile* file)
{
	IniItem* item = NULL;
	IniSection* section = NULL;

	if (!file) return;

	item = file->globalList;

	while (item)
	{
		IniItem_Free(item);

		item++;
	}

	section = file->sectionList;

	while (section)
	{
		IniSection_Free(section);

		section++;
	}

	free(section);
}

int __IniFile_ReadLine(const char* line, IniItem* item, IniItem* section)
{
	return 0;
}

int __IniFile_IsLineCommented(const char* line)
{
	if (!line) return 0;

	if (line[0] == DM_INI_COMMENT_1 || line[0] == DM_INI_COMMENT_2) return 1;

	if (line[0] == DM_INI_COMMENT_3 && line[1] == DM_INI_COMMENT_3) return 1;

	return 0;
}
