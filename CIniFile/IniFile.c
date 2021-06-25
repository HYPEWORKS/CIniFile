/**
 * IniFile.c - Implementation of IniFile.h
 * Created by Josh Kennedy on 13 July 2017
 *
 * CIniFile: C implementation of reading ini configuration files.
 * An open source project of HYPEWORKS.
 *
 * Copyright (C) 2017-2021 HYPEWORKS Ltd Co.
 */

/*
 * This software library is open source and licensed under the MIT License.
 *
 * Read LICENSE for the full license text.
 */

#include "IniFile.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>

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

char* strndup_optimized(const char* s, size_t n, size_t len)
{
	char* result;

	if (n < len)
		len = n;

	result = (char*)malloc(len + 1);
	if (!result)
		return 0;

	result[len] = '\0';
	return (char*)memcpy(result, s, len);
}

char* substring(const char* str, size_t begin, size_t len)
{
	size_t origLen = strlen(str);

	if (str == 0 || origLen == 0 || origLen < begin ||
		origLen < (begin + len))
		return 0;

	return strndup_optimized(str + begin, len, origLen);
}

char* substring_optimized(const char* str, size_t begin, size_t len,
	size_t origLen)
{
	if (str == 0 || origLen == 0 || origLen < begin ||
		origLen < (begin + len))
		return 0;

	return strndup_optimized(str + begin, len, origLen);
}

#ifndef __GNUC__
/* https://github.com/ChristopherWilks/megadepth/blob/master/getline.c */
/* The original code is public domain -- Will Hartung 4/9/09 */
/* Modifications, public domain as well, by Antti Haapala, 11/10/17
   - Switched to getc on 5/23/19 */

/* if typedef doesn't exist (msvc, blah) */
typedef intptr_t ssize_t;

ssize_t getline(char** lineptr, size_t* n, FILE* stream)
{
	size_t pos;
	int c;

	if (lineptr == NULL || stream == NULL || n == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	c = getc(stream);
	if (c == EOF)
	{
		return -1;
	}

	if (*lineptr == NULL)
	{
		*lineptr = malloc(128);
		if (*lineptr == NULL)
		{
			return -1;
		}
		*n = 128;
	}

	pos = 0;
	while (c != EOF)
	{
		if (pos + 1 >= *n)
		{
			size_t new_size = *n + (*n >> 2);
			if (new_size < 128)
			{
				new_size = 128;
			}
			char* new_ptr = realloc(*lineptr, new_size);
			if (new_ptr == NULL)
			{
				return -1;
			}
			*n = new_size;
			*lineptr = new_ptr;
		}

		((unsigned char*)(*lineptr))[pos++] = c;
		if (c == '\n')
		{
			break;
		}
		c = getc(stream);
	}

	(*lineptr)[pos] = '\0';
	return pos;
}
#endif

bool startsWith(const char* str, const char* search)
{
	return (strncmp(str, search, strlen(search)) == 0);
}

bool startsWith_optimized(const char* str, const char* search, size_t len)
{
	return (strncmp(str, search, len) == 0);
}

/* Error handling code */

void __IniFile_SetErrorHint(const char* message, int code)
{
	if (!__IniFile_ErrorHint)
	{
		/* TODO: What do if we get here from a malloc error ?! */
		__IniFile_ErrorHint = (IniErrorHint*)malloc(sizeof(IniErrorHint));
	}

	if (__IniFile_ErrorHint)
	{
		__IniFile_ErrorHint->errorText = message;
		__IniFile_ErrorHint->errorCode = code;
	}
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
	char* buffer = NULL;
	size_t lineLength = 0;

	__IniFile_ClearErrorHint();

	buffer = malloc(sizeof(char) * DM_INI_MAX_LINE_BUFFER);

	if (!buffer)
	{
		__IniFile_SetErrorHint(DM_INI_ERROR_MESSAGE_MALLOC_FAIL, 7);
	}

	fp = fopen(filename, "r");

	if (!fp)
	{
		__IniFile_SetErrorHint(DM_INI_ERROR_MESSAGE_FOPEN_FAIL, errno);

		free(buffer);

		return NULL;
	}

	while (getline(&buffer, &lineLength, fp) != -1)
	{
		/*printf(buffer);*/
	}

	free(buffer);

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

bool __IniFile_ReadLine(const char* line, IniItem* item, IniItem* section)
{
	return false;
}

bool __IniFile_IsLineCommented(const char* line)
{
	if (!line)
		return false;

	/* For simplicity's sake, we consider a blank line a comment. */
	if (line[0] == '\n')
		return true;

	if (line[0] == DM_INI_COMMENT_1 || line[0] == DM_INI_COMMENT_2)
		return true;

	if (line[0] == DM_INI_COMMENT_3 && line[1] == DM_INI_COMMENT_3)
		return true;

	if (line[0] == DM_INI_COMMENT_3 && line[1] == DM_INI_COMMENT_4)
		return true;

	return false;
}

bool __IniFile_IsBeginBlockComment(const char* line)
{
	return (line && line[0] == DM_INI_COMMENT_3 &&
		line[1] == DM_INI_COMMENT_4);
}

bool __IniFile_IsEndBlockComment(const char* line)
{
	size_t len = 0;

	if (!line)
		return false;

	len = strlen(line) - 1;

	return (line[len - 1] == DM_INI_COMMENT_4 && line[len] == DM_INI_COMMENT_3);
}

bool __IniFile_IsSectionDeclaration(const char* line)
{
	size_t len = 0;

	if (!line)
		return false;

	len = strlen(line) - 1;

	return (line[0] == DM_LEFT_BRACKET && line[len] == DM_RIGHT_BRACKET);
}

char* __IniFile_GetSectionName(const char* line)
{
	size_t len = 0;

	if (!line)
		return false;

	len = strlen(line) - 1;

	if (!(line[0] == DM_LEFT_BRACKET && line[len] == DM_RIGHT_BRACKET))
		return NULL;

	return substring(line, 1, len - 1);
}
