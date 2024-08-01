#include "converter_PID.h"

#include "query_process_information.h"

//[1/3]
NTSTATUS Get_real_HANDLE_from_pid(HANDLE INPUT_pid, HANDLE* OUTPUT_real_handle, MODE Kernel_or_User) {
	NTSTATUS status = STATUS_SUCCESS;
	/*
		EPROCESS�� ������� ��, ���� �ڵ� ���
	*/
	/*
	
	*/
	if (Kernel_or_User == KernelMode) {
		PEPROCESS eprocess = NULL;
		status = PsLookupProcessByProcessId(INPUT_pid, &eprocess);
		if (eprocess == NULL) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Get_real_HANDLE_from_pid ->  PsLookupProcessByProcessId ����! %p", status);
			return status;
		}

		status = ObOpenObjectByPointer(eprocess, 0, NULL, 0, 0, Kernel_or_User, OUTPUT_real_handle); // ���� Ƚ�� ����
		if (*OUTPUT_real_handle == 0 || status != STATUS_SUCCESS) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Get_real_HANDLE_from_pid ->  ObOpenObjectByPointer ����! %p", status);
			*OUTPUT_real_handle = 0;
			ObDereferenceObject(eprocess);
			return status;
		}

		ObDereferenceObject(eprocess);
	}
	else {
		OBJECT_ATTRIBUTES objAttributes;
		CLIENT_ID clientId;

		InitializeObjectAttributes(&objAttributes, NULL, 0, NULL, NULL);
		clientId.UniqueProcess = (HANDLE)INPUT_pid;
		clientId.UniqueThread = NULL;

		status = ZwOpenProcess(OUTPUT_real_handle, PROCESS_ALL_ACCESS, &objAttributes, &clientId);
	}

	
	return status;
}


//[2/3]
#include "File_io.h"
NTSTATUS PID_to_EXE(
	HANDLE Input_PID,

	PUCHAR* EXE_binary,
	ULONG* EXE_binary_Size,

	CHAR* SHA256_hashed,
	MODE Kernel_or_User
) {
	NTSTATUS status;

	// �ڵ���
	HANDLE Real_HANDLE = 0;
	Get_real_HANDLE_from_pid(Input_PID, &Real_HANDLE, Kernel_or_User);
	if (Real_HANDLE == 0) {
		return STATUS_UNSUCCESSFUL;
	}


	// ������ ���
	UNICODE_STRING unicode_fullpath = { 0, }; unicode_fullpath.Buffer = NULL;
	//if (Query_Process_info(&Real_HANDLE, ProcessImageFileName, &unicode_fullpath) != STATUS_SUCCESS) {
		//return STATUS_UNSUCCESSFUL;
	//}
	if (PID_to_ANSI_FULL_PATH(Input_PID, &unicode_fullpath, Kernel_or_User) != STATUS_SUCCESS) {
		if (unicode_fullpath.Buffer != NULL) {
			ExFreePoolWithTag(unicode_fullpath.Buffer, 'UncB');
		}
		return STATUS_UNSUCCESSFUL;
	}
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n1--> %wZ\n", unicode_fullpath);

	//���� ���


	if (ALL_in_ONE_FILE_IO(EXE_binary, EXE_binary_Size, unicode_fullpath, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " ALL_in_ONE_FILE_IO EXE�������� ����  \n");
		status = STATUS_UNSUCCESSFUL;
	}
	else {


		//CHAR SHA256_hashed[SHA256_Length] = { 0, };
		if (GET_SHA256_HASHING(*EXE_binary, *EXE_binary_Size, SHA256_hashed) != STATUS_SUCCESS) {
			status = STATUS_UNSUCCESSFUL;
			ExFreePoolWithTag(unicode_fullpath.Buffer, 'UncB');
			return status;
		}

		//ExFreePoolWithTag(EXE_binary, 'FILE');
		status = STATUS_SUCCESS;
	}
	
	ExFreePoolWithTag(unicode_fullpath.Buffer, 'UncB');
	return status;
}



//[3/3]
NTSTATUS PID_to_ANSI_FULL_PATH(HANDLE PID, PUNICODE_STRING output_fullpath, MODE Kernel_or_User) {
	// �ڵ���
	HANDLE Real_HANDLE = 0;
	if (Get_real_HANDLE_from_pid(PID, &Real_HANDLE, Kernel_or_User) != STATUS_SUCCESS) {
		return STATUS_UNSUCCESSFUL;
	}



	// ������ ���
	if (Query_Process_info(&Real_HANDLE, ProcessImageFileName, output_fullpath) != STATUS_SUCCESS) {
		return STATUS_UNSUCCESSFUL;
	}

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [PID_to_ANSI_FULL_PATH]-------===>> %wZ\n", *((PUNICODE_STRING)*output_fullpath));

	return STATUS_SUCCESS;
}