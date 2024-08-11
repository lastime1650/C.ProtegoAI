#ifndef ioctl_process_sender_H
#define ioctl_process_sender_H

#include <ntifs.h>
#include "my_ioctl.h"

#include "util_Delay.h"

// �񵿱� ������� IOCTL �����ϴ� �������� �پ��� ����� ������ �� ���� 

typedef struct ioctl_process_sender {
	COMMUNICATION_IOCTL_ENUM KernelSendEnum;
	PVOID context; // whatever

	KEVENT EVENT;

}ioctl_process_sender, *Pioctl_process_sender;

VOID IOCTL_process_sender_from_query(Pioctl_process_sender context);


#endif