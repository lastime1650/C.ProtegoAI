#ifndef Get_File_Index_INFORMATION_H
#define Get_File_Index_INFORMATION_H
#pragma warning(disable:4996)

#include <ntifs.h>


/*
	������ �����ο� ���� "���� ������ȣ"�� ȹ���Ѵ�.

	��, ������������� ������忡���� ����ϸ� �ʱ�ȭ �ȴ�. 

*/

NTSTATUS Get_FILE_INDEX_INFORMATION(
	PUNICODE_STRING INPUT_absolute_FILE_PATH,
	ULONG64* OUTPUT_FILE_INDEX
);


#endif