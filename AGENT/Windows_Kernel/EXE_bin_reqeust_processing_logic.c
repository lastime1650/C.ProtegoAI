#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "EXE_bin_request_processing.h"

/*
	�����κ���...

		Ư�� ������ "������"�� Ansi(Ascii)�� �޾�,
		"Readfile" �� �� ������/������ + �������� �� �������� ���� �����Ѵ�.

*/

NTSTATUS FILE_bin_process(PUCHAR Absolute_KERNEL_FILE_FULL_PATH__Ansi, ULONG32 FILE_FULL_PATH_STRING_SIZE) {

	NTSTATUS status = STATUS_SUCCESS;
	/*

		ANSI -> UNICODE

	*/
	ANSI_STRING FILE_FULL_PATH_ansi = { 0, };
	RtlInitAnsiString(&FILE_FULL_PATH_ansi, (LPCSTR)Absolute_KERNEL_FILE_FULL_PATH__Ansi); // ���ڿ� �ذ� �������� �̷��� �ϵ���! 

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " \n %Z \n\n", FILE_FULL_PATH_ansi);




	UNICODE_STRING FILE_FULL_PATH_unicode = { 0, };
	ANSI_to_UNICODE(&FILE_FULL_PATH_unicode, FILE_FULL_PATH_ansi);



	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " \n  %d @@ %wZ \n\n", FILE_FULL_PATH_unicode.MaximumLength, FILE_FULL_PATH_unicode);
	/*exe�� ���̳ʸ��� �а� �ؽ� ���ϱ�*/
	PVOID EXE_binary = NULL;
	ULONG32 EXE_binary_size = 0;

	if (ALL_in_ONE_FILE_IO(&EXE_binary, (PULONG)&EXE_binary_size, FILE_FULL_PATH_unicode, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) {
		status = STATUS_UNSUCCESSFUL;
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "EXE_binary_size->%d\n", EXE_binary_size);
	if (EXE_binary != NULL) {
		//PUCHAR NEW = ExAllocatePoolWithTag(PagedPool, EXE_binary_size + 4, 'test');
		//memcpy(NEW, EXE_binary, EXE_binary_size);
		//memcpy(NEW + EXE_binary_size, "_END", sizeof("_END") - 1);



		/* ���� ���� */
		if (SEND_TCP_DATA(EXE_binary, EXE_binary_size, TCP_DATA_SEND) != STATUS_SUCCESS) {
			status = STATUS_UNSUCCESSFUL;
		}
		ExFreePoolWithTag(EXE_binary, 'FILE');
	}
	else {
		/* ���� ���� (����) */
		ULONG32 FAIL = No;
		if (SEND_TCP_DATA(&FAIL, 4, TCP_DATA_SEND) != STATUS_SUCCESS) {
			status = STATUS_UNSUCCESSFUL;
		}
	}

	return STATUS_SUCCESS;

}