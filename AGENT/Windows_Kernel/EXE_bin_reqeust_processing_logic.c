#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "EXE_bin_request_processing.h"

/*
	서버로부터...

		특정 파일의 "절대경로"를 Ansi(Ascii)로 받아,
		"Readfile" 후 그 데이터/사이즈 + 전역정보 를 서버에게 최종 전달한다.

*/

NTSTATUS FILE_bin_process(PUCHAR Absolute_KERNEL_FILE_FULL_PATH__Ansi, ULONG32 FILE_FULL_PATH_STRING_SIZE) {

	NTSTATUS status = STATUS_SUCCESS;
	/*

		ANSI -> UNICODE

	*/
	ANSI_STRING FILE_FULL_PATH_ansi = { 0, };
	RtlInitAnsiString(&FILE_FULL_PATH_ansi, (LPCSTR)Absolute_KERNEL_FILE_FULL_PATH__Ansi); // 문자열 해결 다음부턴 이렇게 하도록! 

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " \n %Z \n\n", FILE_FULL_PATH_ansi);




	UNICODE_STRING FILE_FULL_PATH_unicode = { 0, };
	ANSI_to_UNICODE(&FILE_FULL_PATH_unicode, FILE_FULL_PATH_ansi);



	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " \n  %d @@ %wZ \n\n", FILE_FULL_PATH_unicode.MaximumLength, FILE_FULL_PATH_unicode);
	/*exe의 바이너리를 읽고 해시 구하기*/
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



		/* 서버 전송 */
		if (SEND_TCP_DATA(EXE_binary, EXE_binary_size, TCP_DATA_SEND) != STATUS_SUCCESS) {
			status = STATUS_UNSUCCESSFUL;
		}
		ExFreePoolWithTag(EXE_binary, 'FILE');
	}
	else {
		/* 서버 전송 (실패) */
		ULONG32 FAIL = No;
		if (SEND_TCP_DATA(&FAIL, 4, TCP_DATA_SEND) != STATUS_SUCCESS) {
			status = STATUS_UNSUCCESSFUL;
		}
	}

	return STATUS_SUCCESS;

}