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

NTSTATUS RECEIVE_TCP_DATA__with_alloc(PVOID* output_BUFFER, ULONG32* output_BUFFER_SIZE, SOCKET_INFORMATION receive_types) { // 수신할 때

	if (Initilize_PKmutex() == FALSE) return STATUS_UNSUCCESSFUL;

	/*
		총 2차 과정으로 Receive함
		[1/2]
			4바이트를 SERVER로부터 읽어서 정수로 변환하여 저장
		[2/2]
			([1/2]) 읽은 것만큼의 크기로 동적할당함
	*/
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc -> TCP 뮤텍스 점유시도 \n");
	KeWaitForSingleObject(TCP_session, Executive, KernelMode, FALSE, NULL); // Lock
	NTSTATUS status = STATUS_SUCCESS;

	*output_BUFFER = NULL;
	ULONG32 allocate_size = 0;
	if (Send_TO_TCP_Server(&allocate_size, sizeof(allocate_size), TCP_DATA_RECEIVE, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[1/2]
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 2 (실패)  \n");
		status = STATUS_UNSUCCESSFUL;
	}
	else {
		if (allocate_size == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 2-1 (실패) allocate_size길이가 0 임  \n");
			status = STATUS_UNSUCCESSFUL;
			KeReleaseMutex(TCP_session, FALSE); // Release
			return status;
		}

		/* 일단 테스트용으로 주고받기 전달*/
		//ULONG32 test = 1029;
		//if (Send_TO_TCP_Server(&test, sizeof(test), TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) {
			//return STATUS_UNSUCCESSFUL;
		//}

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 3 (1차 성공) 서버로부터 가져온 길이 -> %lu \n", allocate_size);
		*output_BUFFER_SIZE = allocate_size; // 동적크기
		*output_BUFFER = (PVOID)ExAllocatePoolWithTag(PagedPool, *output_BUFFER_SIZE, 'RcDt'); // 동적할당
		if (*output_BUFFER == NULL || allocate_size == 0) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 4 \n");
			status = STATUS_INSUFFICIENT_RESOURCES;
		}
		else {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RECEIVE_TCP_DATA__with_alloc 5 ( 2차를 대기하고 있습니다.. )  \n");
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
		//KeReleaseMutex(&TCP_session, FALSE); <- 여기선 절대 뮤텍스를 해제하지않고, 호출한 영역에서 해제한다는 의미로 작성됨.
		return status;
	}
	else {
		KeReleaseMutex(TCP_session, FALSE); // Release
		return status;
	}


}


NTSTATUS SEND_TCP_DATA(PVOID input_BUFFER, ULONG32 input_BUFFER_SIZE, SOCKET_INFORMATION receive_types) { // 송신할 때
	NTSTATUS status = STATUS_SUCCESS;

	if (Initilize_PKmutex() == FALSE) return STATUS_UNSUCCESSFUL;

	if (receive_types != SERVER_DATA_PROCESS) {
		KeWaitForSingleObject(TCP_session, Executive, KernelMode, FALSE, NULL); // Lock
	}


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA 크기 -> %d\n", input_BUFFER_SIZE);
	if (Send_TO_TCP_Server(&input_BUFFER_SIZE, sizeof(input_BUFFER_SIZE), TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[1/2] 길이 전달
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_TCP_DATA 1 - fail");
		status = STATUS_UNSUCCESSFUL;
	}
	else {
		if (Send_TO_TCP_Server(input_BUFFER, input_BUFFER_SIZE, TCP_DATA_SEND, &COMMAND_NewSocket) != STATUS_SUCCESS) { //[2/2] 데이터 전달
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


