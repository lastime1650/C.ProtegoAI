#ifndef Get_File_Index_INFORMATION_H
#define Get_File_Index_INFORMATION_H
#pragma warning(disable:4996)

#include <ntifs.h>


/*
	파일의 절대경로에 따라서 "파일 고유번호"를 획득한다.

	단, 조각모음기능을 유저모드에서등 사용하면 초기화 된다. 

*/

NTSTATUS Get_FILE_INDEX_INFORMATION(
	PUNICODE_STRING INPUT_absolute_FILE_PATH,
	ULONG64* OUTPUT_FILE_INDEX
);


#endif