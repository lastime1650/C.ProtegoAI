#include "ioctl_process_sender.h"

VOID IOCTL_process_sender_from_query(Pioctl_process_sender context) {
	/*
		������� IOCTL���� ������ ��.

		IOCTL - MUTEX ���� -> ������ -> KEVENT ����
	
	*/
	Delays(-1);
	KeWaitForSingleObject(&IOCTL_access_mutex, Executive, KernelMode, FALSE, NULL);

	while (1) {
		// IOCTL ������忡�� ��û �� ������ ���� �ݺ�
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
				// �ƹ��� �۾����� ����
				break;
			}

			// if - TRUE���� ���ѹݺ� Ż��
			break;
		}
		
	}
	Delays(-1);
	KeSetEvent(&context->EVENT, 0, FALSE);
	KeReleaseMutex(&IOCTL_access_mutex, FALSE);
	
	
}