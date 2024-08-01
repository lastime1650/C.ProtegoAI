#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "query_process_information.h"


/*
	Handle 값을 파라미터로 전달하여 특정된 프로세스의 정보를 반환한다.
*/
NTSTATUS Query_Process_info(
	HANDLE* real_process_handle, PROCESSINFOCLASS input_Process_class,
	PUNICODE_STRING output_unicode

) {
	if (*real_process_handle == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Query_Process_info -> real_process_handle 핸들 값이 NULL입니다.\n");
		return STATUS_UNSUCCESSFUL;
	}



	NTSTATUS status = STATUS_SUCCESS;

	if (input_Process_class == ProcessBasicInformation) { // 프로세스 정보
		PROCESS_BASIC_INFORMATION process_basic_info = { 0, };
		ULONG32 process_basic_info_len = 0;
		status = ZwQueryInformationProcess(*real_process_handle, ProcessBasicInformation, &process_basic_info, sizeof(process_basic_info), (PULONG)&process_basic_info_len);

		return status;
	}
	else if (input_Process_class == ProcessImageFileName) { // 프로세스 절대 경로
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
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Query_Process_info -> ZwQueryInformationProcess 실패, 상태: 0x%X\n", status);
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

		// output_unicode의 Buffer가 NULL인 경우 메모리 할당
		if (output_unicode->Buffer == NULL) {
			output_unicode->MaximumLength = process_image_file_name->Length + sizeof(WCHAR); // Null terminator를 위한 공간 포함
			output_unicode->Buffer = (PWCHAR)ExAllocatePoolWithTag(NonPagedPool, output_unicode->MaximumLength, 'UncB');
			if (output_unicode->Buffer == NULL) {
				ExFreePoolWithTag(get_buffer, 'PrNm');
				return STATUS_INSUFFICIENT_RESOURCES;
			}
		}

		// output_unicode의 버퍼가 충분히 큰지 확인해야 합니다.
		if (output_unicode->MaximumLength < process_image_file_name->Length + sizeof(WCHAR)) {
			ExFreePoolWithTag(get_buffer, 'PrNm');
			return STATUS_BUFFER_TOO_SMALL;
		}

		// UNICODE_STRING 복사
		RtlCopyUnicodeString(output_unicode, process_image_file_name);

		// Null terminator 추가
		output_unicode->Buffer[output_unicode->Length / sizeof(WCHAR)] = L'\0';


		ExFreePoolWithTag(get_buffer, 'PrNm');

		return status;


	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Query_Process_info -> 현재 처리할 수 없는 프로세스 CLASS 입니다.\n");
		return STATUS_UNSUCCESSFUL;
	}





}