#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "query_process_information.h"


/*
	Handle ���� �Ķ���ͷ� �����Ͽ� Ư���� ���μ����� ������ ��ȯ�Ѵ�.
*/
NTSTATUS Query_Process_info(
	HANDLE* real_process_handle, PROCESSINFOCLASS input_Process_class,
	PUNICODE_STRING output_unicode

) {
	if (*real_process_handle == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Query_Process_info -> real_process_handle �ڵ� ���� NULL�Դϴ�.\n");
		return STATUS_UNSUCCESSFUL;
	}



	NTSTATUS status = STATUS_SUCCESS;

	if (input_Process_class == ProcessBasicInformation) { // ���μ��� ����
		PROCESS_BASIC_INFORMATION process_basic_info = { 0, };
		ULONG32 process_basic_info_len = 0;
		status = ZwQueryInformationProcess(*real_process_handle, ProcessBasicInformation, &process_basic_info, sizeof(process_basic_info), (PULONG)&process_basic_info_len);

		return status;
	}
	else if (input_Process_class == ProcessImageFileName) { // ���μ��� ���� ���
		if (output_unicode == NULL) return STATUS_UNSUCCESSFUL;

		PVOID get_buffer = NULL;
		ULONG process_FULL_NAME_info_len = 0;

		while (ZwQueryInformationProcess(*real_process_handle, ProcessImageFileName, get_buffer, process_FULL_NAME_info_len, &process_FULL_NAME_info_len) == STATUS_INFO_LENGTH_MISMATCH) {
			if (get_buffer != NULL) {
				break;
			}
			get_buffer = ExAllocatePoolWithTag(NonPagedPool, process_FULL_NAME_info_len, 'PrNm');
		}
		if (!NT_SUCCESS(status)) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Query_Process_info -> ZwQueryInformationProcess ����, ����: 0x%X\n", status);
			ExFreePoolWithTag(get_buffer, 'PrNm');
			return status;
		}

		if (get_buffer == NULL) {
			return STATUS_UNSUCCESSFUL;
		}


		PUNICODE_STRING process_image_file_name = (PUNICODE_STRING)get_buffer;
		if (process_image_file_name->Buffer == NULL) {
			ExFreePoolWithTag(get_buffer, 'PrNm');
			return STATUS_UNSUCCESSFUL;
		}

		// output_unicode�� Buffer�� NULL�� ��� �޸� �Ҵ�
		if (output_unicode->Buffer == NULL) {
			output_unicode->MaximumLength = process_image_file_name->Length + sizeof(WCHAR); // Null terminator�� ���� ���� ����
			output_unicode->Buffer = (PWCHAR)ExAllocatePoolWithTag(NonPagedPool, output_unicode->MaximumLength, 'UncB');
			if (output_unicode->Buffer == NULL) {
				ExFreePoolWithTag(get_buffer, 'PrNm');
				return STATUS_INSUFFICIENT_RESOURCES;
			}
		}

		// output_unicode�� ���۰� ����� ū�� Ȯ���ؾ� �մϴ�.
		if (output_unicode->MaximumLength < process_image_file_name->Length + sizeof(WCHAR)) {
			ExFreePoolWithTag(get_buffer, 'PrNm');
			return STATUS_BUFFER_TOO_SMALL;
		}

		// UNICODE_STRING ����
		RtlCopyUnicodeString(output_unicode, process_image_file_name);

		// Null terminator �߰�
		output_unicode->Buffer[output_unicode->Length / sizeof(WCHAR)] = L'\0';


		ExFreePoolWithTag(get_buffer, 'PrNm');

		return status;


	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Query_Process_info -> ���� ó���� �� ���� ���μ��� CLASS �Դϴ�.\n");
		return STATUS_UNSUCCESSFUL;
	}





}