#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "TCP_send_or_Receiver.h"
#include "util_Delay.h"

PKMUTEX TCP_session = NULL;

BOOLEAN Initilize_PKmutex() {
	if (TCP_session == NULL) {
		TCP_session = (PKMUTEX)ExAllocatePoolWithTag(NonPagedPool, sizeof(KMUTEX), 'TCPm');// TCP mutex
		memset(TCP_session, 0, sizeof(KMUTEX));
		if (TCP_session == NULL) return FALSE;

		KeInitializeMutex(TCP_session, 0);

	}

	return TRUE;
}

NTSTATUS RECEIVE_TCP_DATA__with_alloc(PVOID* output_BUFFER, ULONG32* output_BUFFER_SIZE, SOCKET_INFORMATION receive_types) { // ������ ��

	if (Initilize_PKmutex() == FALSE) return STATUS_UNSUCCESSFUL;

	/*
		�� 2�� �������� Receive��
		[1/2]
			4����Ʈ�� SERVER�κ��� �о ������ ��ȯ�Ͽ� ����
		[2/2]
			([1/2]) ���� �͸�ŭ�� ũ��� �����Ҵ���
	*/
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc -> TCP ���ؽ� �����õ� \n");
	KeWaitForSingleObject(TCP_session, Executive, KernelMode, FALSE, NULL); // Lock
	NTSTATUS status = STATUS_SUCCESS;

	*output_BUFFER = NULL;
	ULONG32 allocate_size = 0;
	if (Send_TO_TCP_Server(&allocate_size, sizeof(allocate_size), TCP_DATA_RECEIVE, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[1/2]
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 2 (����)  \n");
		status = STATUS_UNSUCCESSFUL;
	}
	else {
		if (allocate_size == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 2-1 (����) allocate_size���̰� 0 ��  \n");
			status = STATUS_UNSUCCESSFUL;
			KeReleaseMutex(TCP_session, FALSE); // Release
			return status;
		}

		/* �ϴ� �׽�Ʈ������ �ְ�ޱ� ����*/
		//ULONG32 test = 1029;
		//if (Send_TO_TCP_Server(&test, sizeof(test), TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) {
			//return STATUS_UNSUCCESSFUL;
		//}

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 3 (1�� ����) �����κ��� ������ ���� -> %lu \n", allocate_size);
		*output_BUFFER_SIZE = allocate_size; // ����ũ��
		*output_BUFFER = (PVOID)ExAllocatePoolWithTag(PagedPool, *output_BUFFER_SIZE, 'RcDt'); // �����Ҵ�
		if (*output_BUFFER == NULL || allocate_size == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 4 \n");
			status = STATUS_INSUFFICIENT_RESOURCES;
		}
		else {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 5 ( 2���� ����ϰ� �ֽ��ϴ�.. )  \n");
			//Delays(-1);
			if (Send_TO_TCP_Server(*output_BUFFER, *output_BUFFER_SIZE, TCP_DATA_RECEIVE, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[2/2]
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 6 \n");
				status = STATUS_UNSUCCESSFUL;
			}
			else {
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 7 \n");
				status = STATUS_SUCCESS;
			}
		}
	}

	if (receive_types == SERVER_DATA_PROCESS) {
		//KeReleaseMutex(&TCP_session, FALSE); <- ���⼱ ���� ���ؽ��� ���������ʰ�, ȣ���� �������� �����Ѵٴ� �ǹ̷� �ۼ���.
		return status;
	}
	else {
		KeReleaseMutex(TCP_session, FALSE); // Release
		return status;
	}


}


NTSTATUS SEND_TCP_DATA(PVOID input_BUFFER, ULONG32 input_BUFFER_SIZE, SOCKET_INFORMATION receive_types) { // �۽��� ��
	NTSTATUS status = STATUS_SUCCESS;

	if (Initilize_PKmutex() == FALSE) return STATUS_UNSUCCESSFUL;

	if (receive_types != SERVER_DATA_PROCESS) {
		KeWaitForSingleObject(TCP_session, Executive, KernelMode, FALSE, NULL); // Lock
	}


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA ũ�� -> %d\n", input_BUFFER_SIZE);
	if (Send_TO_TCP_Server(&input_BUFFER_SIZE, sizeof(input_BUFFER_SIZE), TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[1/2] ���� ����
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA 1 - fail");
		status = STATUS_UNSUCCESSFUL;
	}
	else {
		if (Send_TO_TCP_Server(input_BUFFER, input_BUFFER_SIZE, TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[2/2] ������ ����
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA 2 - fail");
			status = STATUS_UNSUCCESSFUL;
		}
		else {
			status = STATUS_SUCCESS;
		}

	}
	Delays(-1);
	if (receive_types != SERVER_DATA_PROCESS) {
		KeReleaseMutex(TCP_session, FALSE); // Release
	}


	return status;
}

VOID RECEIVE_or_SEND_TCP_DATA__with_alloc_Free(PVOID* input_BUFFER_for_freepool) {
	ExFreePoolWithTag(*input_BUFFER_for_freepool, 'RcDt');
	return;
}


