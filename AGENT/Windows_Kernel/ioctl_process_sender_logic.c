#include "ioctl_process_sender.h"

VOID IOCTL_process_sender_from_query(Pioctl_process_sender context) {
	/*
		유저모드 IOCTL에게 전달할 것.

		IOCTL - MUTEX 점유 -> 데이터 -> KEVENT 해제
	
	*/
	Delays(-1);
	KeWaitForSingleObject(&IOCTL_access_mutex, Executive, KernelMode, FALSE, NULL);

	while (1) {
		// IOCTL 유저모드에서 요청 올 때까지 무한 반복
		if (IOCTL_share_structure.is_usermode_request) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IOCTL_process_sender_from_query -> is_usermode_request TRUE\n");
			switch (context->KernelSendEnum) {
			case HOOKING_request:
				
				IOCTL_share_structure.ioctl_BUFFER->information = HOOKING_request;

				IOCTL_share_structure.ioctl_BUFFER->HOOK_DATA.PID = ((Pcomunication_ioctl_for_HOOKING)context->context)->PID;
				IOCTL_share_structure.ioctl_BUFFER->HOOK_DATA.Process_HANDLE = ((Pcomunication_ioctl_for_HOOKING)context->context)->Process_HANDLE;

				KeSetEvent(&IOCTL_share_structure.ioctl_event, 0, FALSE);
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " IOCTL_process_sender_from_query -- HOOKING_request\n");

			default:
				// 아무런 작업하지 않음
				break;
			}

			// if - TRUE이후 무한반복 탈출
			break;
		}
		
	}
	Delays(-1);
	KeSetEvent(&context->EVENT, 0, FALSE);
	KeReleaseMutex(&IOCTL_access_mutex, FALSE);
	
	
}