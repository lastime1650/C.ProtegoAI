#include "converter_string.h"

/*UNICODE_STRING --> ANSI_STRING*/
NTSTATUS UNICODE_to_ANSI(ANSI_STRING* OUTPUT_ansi, UNICODE_STRING* INPUT_unicode) {

	NTSTATUS status = STATUS_SUCCESS;

	status = RtlUnicodeStringToAnsiString(OUTPUT_ansi, (PCUNICODE_STRING)INPUT_unicode, TRUE);
	if (status != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "UNICODE_ ���� ANSI �� ��ȯ ����");

	}
	return status;

}


/*ANSI_STRING --> UNICODE_STRING*/
NTSTATUS ANSI_to_UNICODE(UNICODE_STRING* OUTPUT_unicode, ANSI_STRING INPUT_ansi) {

	NTSTATUS status = STATUS_SUCCESS;
	RtlInitEmptyUnicodeString(OUTPUT_unicode, NULL, 0);

	status = RtlAnsiStringToUnicodeString(OUTPUT_unicode, (PCANSI_STRING)&INPUT_ansi, TRUE);
	if (status != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ANSI ���� UNICODE �� ��ȯ ����");

	}

	status = RtlValidateUnicodeString(0, OUTPUT_unicode);
	if (!NT_SUCCESS(status)) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ANSI ���� UNICODE �� ��ȿ�� ����");
	}


	return status;

}