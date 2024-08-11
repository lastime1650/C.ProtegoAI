#ifndef ioctl_process_sender_H
#define ioctl_process_sender_H

#include <ntifs.h>
#include "my_ioctl.h"

#include "util_Delay.h"

// 비동기 스레드로 IOCTL 전달하는 스레드임 다양한 명령을 전달할 수 있음 

typedef struct ioctl_process_sender {
	COMMUNICATION_IOCTL_ENUM KernelSendEnum;
	PVOID context; // whatever

	KEVENT EVENT;

}ioctl_process_sender, *Pioctl_process_sender;

VOID IOCTL_process_sender_from_query(Pioctl_process_sender context);


#endif