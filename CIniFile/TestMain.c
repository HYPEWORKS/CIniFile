/**
 * TestMain.c - Test suite and example code.
 * Created by Josh Kennedy on 13 July 2017
 *
 * CIniFile: C implementation of reading ini configuration files.
 * An open source project of The DigitalMagic Company.
 *
 * Copyright (C) 2017-2019 DigitalMagic LLC.
 */

/*
 * This software library is open source and licensed under the MIT License.
 *
 * Read LICENSE for the full license text.
 */

#define TEST_CODE

#include "IniFile.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#if _DEBUG
#if _MSC_VER
#include <intrin.h>
#define BREAKPOINT() __debugbreak()
#else
#include <signal.h>
#define BREAKPOINT() raise(SIGINT);
#endif
#else
#define BREAKPOINT() (void)0
#endif

#ifndef __FUNCSIG__
#define __FUNCSIG__ __PRETTY_FUNCTION__
#endif

#define TEST_SUCCESS 0
#define TEST_FAIL 1

#define ASSERT() printf("Assertion failure in method %s on line %d\n", \
	__FUNCSIG__, __LINE__); BREAKPOINT(); return TEST_FAIL;

#define ASSERT_TRUE(x) if (x != true) { ASSERT() }
#define ASSERT_FALSE(x) if (x != false) { ASSERT() }

#define ASSERT_NULL(x) if (x) { ASSERT() }
#define ASSERT_NOT_NULL(x) if (!x) { ASSERT() }

#define ASSERT_EQUALS(x, y) if (x != y) { ASSERT() }
#define ASSERT_NOT_EQUALS(x, y) if (x == y) { ASSERT() }

#define ASSERT_STR_EQUALS(x, y) if (strcmp(x, y) != 0) { ASSERT() }
#define ASSERT_STR_NOT_EQUALS(x, y) if (strcmp(x, y) == 0) { ASSERT() }

typedef int(*TestFunction)();

int TestErrorHint()
{
	IniErrorHint* hint = IniFile_GetErrorHint();

	ASSERT_NULL(hint);

	__IniFile_SetErrorHint("test", -1);
	hint = IniFile_GetErrorHint();

	ASSERT_NOT_NULL(hint);

	ASSERT_EQUALS(hint->errorCode, -1);
	ASSERT_STR_EQUALS("test", hint->errorText);

	__IniFile_ClearErrorHint();
	hint = IniFile_GetErrorHint();

	ASSERT_NULL(hint);

	return TEST_SUCCESS;
}

int TestFileRead()
{
	// A bit of a sanity check.
	FILE* testFile = NULL;
	testFile = fopen("test.ini", "r");

	ASSERT_NOT_NULL(testFile);

	fclose(testFile);

	IniFile* fileData = IniFile_ReadFile("test.ini");

	ASSERT_NULL(fileData);

	IniFile_Free(fileData);

	return TEST_SUCCESS;
}

int TestHashing()
{
	const char* test1 = "hello world";
	long hashed1 = __IniFile_Hash(test1);

	const char* test2 = "Josh is very cool and this is just some long string!";
	long hashed2 = __IniFile_Hash(test2);

	const char* test3 = "a";
	long hashed3 = __IniFile_Hash(test3);

	const char* test4 = "ab";
	long hashed4 = __IniFile_Hash(test4);

	ASSERT_NOT_EQUALS(hashed1, 0);

	ASSERT_NOT_EQUALS(hashed2, 0);

	ASSERT_EQUALS(hashed3, 276);

	ASSERT_EQUALS(hashed4, 49502);

	return TEST_SUCCESS;
}

int TestComments()
{
	const char* comment1 = "// wow";
	const char* comment2 = "# such";
	const char* comment3 = "; test";
	const char* comment4 = "/*  so";
	const char* comment5 = "much */";
	const char* comment6 = "/* test test test wow */";

	ASSERT_TRUE(__IniFile_IsLineCommented(comment1));
	ASSERT_TRUE(__IniFile_IsLineCommented(comment2));
	ASSERT_TRUE(__IniFile_IsLineCommented(comment3));
	ASSERT_TRUE(__IniFile_IsLineCommented(comment4));
	ASSERT_FALSE(__IniFile_IsLineCommented(comment5));
	ASSERT_TRUE(__IniFile_IsLineCommented(comment6));

	ASSERT_TRUE(__IniFile_IsBeginBlockComment(comment4));
	ASSERT_TRUE(__IniFile_IsEndBlockComment(comment5));

	ASSERT_TRUE(__IniFile_IsBeginBlockComment(comment6));
	ASSERT_TRUE(__IniFile_IsEndBlockComment(comment6));

	return TEST_SUCCESS;
}

int TestSection()
{
	const char* section1 = "[section1]";

	ASSERT_TRUE(__IniFile_IsSectionDeclaration(section1));

	char* section1_eval = __IniFile_GetSectionName(section1);

	ASSERT_NOT_NULL(section1_eval);
	ASSERT_STR_EQUALS(section1_eval, "section1");

	return TEST_SUCCESS;
}

void RegisterTest(TestFunction tf, const char* title)
{
	int value = tf();

	printf("%s: ", title);

	if (value == TEST_SUCCESS)
	{
		printf("OK");
	}
	else
	{
		printf("FAIL");
	}

	printf("\n");
}

/* Our entry point. */
int main(int argc, char* argv[])
{
	printf("CIniFile Test Suite!\n\n");

	RegisterTest(TestErrorHint, "Error Hint Functionality");
	RegisterTest(TestFileRead, "File Reading Functionality");
	RegisterTest(TestHashing, "String Hashing Functionality");
	RegisterTest(TestComments, "Comment Parsing Functionality");
	RegisterTest(TestSection, "Section Parsing Functionality");

	return 0;
}
