#ifndef my_ioctl_H
#define my_ioctl_H

#include <ntifs.h>
#include <ntddk.h>

#include "converter_PID.h"
#include "Process_ObRegisterCallback_processing.h"

#define DEVICE_NAME L"\\Device\\My_AGENT_Device"
#define SYMLINK_NAME L"\\DosDevices\\My_AGENT_Device"
#define IOCTL_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)




typedef enum communication_ioctl_ENUM {

	CHECK = 1,

	SUCCESS_id = 100,
	FAIL_id = 101,

	REQUEST_all = 1029, // 라이선스 + 에이전트 ID 둘 다 있을 때,
	REQUESET_without_AGENT_ID = 1030, // 라이선스만 있을 때

	WAIT_FAILED_from_Center_Server = 2000, // RUST서버와 통신이 안될 때  ( 초기통신시 )
	WAIT_FAILED_from_Kernel = 2001, // 커널로부터 응답을 기다리지 못했을 때 ( 커널에게 문제가 발생할 경우 ) 
	WAIT_FAILED_from_User = 2002, // 유저로부터 응답을 기다리지 못했을 때


	HOOKING_request = 3000,

	HOOK_MON = 3001


}COMMUNICATION_IOCTL_ENUM;

// 주로 커널이 유저모드에게 전달하는 후킹 전달 요청 데이터
typedef struct comunication_ioctl_for_HOOKING {

	HANDLE PID;
	HANDLE Process_HANDLE;

}comunication_ioctl_for_HOOKING, * Pcomunication_ioctl_for_HOOKING;


// DLL 후크 걸린 프로세스에서 보내는 구조체임
typedef struct HOOK_IOCTL_DATA {
	HANDLE PID;//자신의 PID 
	UCHAR Hooked_API_NAME[128]; // 후크 걸린 API 이름
}HOOK_IOCTL_DATA, * PHOOK_IOCTL_DATA;



typedef struct communication_ioctl {

	COMMUNICATION_IOCTL_ENUM information;

	UCHAR license_ID[128];
	UCHAR Agent_ID[128];

	HANDLE Ioctl_User_Mode_ProcessId; // 추가됨 

	comunication_ioctl_for_HOOKING HOOK_DATA; // 추가됨

	HOOK_IOCTL_DATA API_HOOK_MON;

}COMMUNICATION_IOCTL, * PCOMMUNICATION_IOCTL;

typedef struct IOCTL_content {
	BOOLEAN is_init;

	KEVENT ioctl_event;
	BOOLEAN is_usermode_request;


	PCOMMUNICATION_IOCTL ioctl_BUFFER;

}IOCTL_content, * PIOCTL_content;





//전역변수
extern IOCTL_content IOCTL_share_structure;

//전역변수 KMUTEX ( 다양한 스레드에서 IOCTL를 하나씩 사용할 수 있도록 ) 
extern KMUTEX IOCTL_access_mutex;



PDEVICE_OBJECT DeviceInitialize(IN PDRIVER_OBJECT DriverObject);

NTSTATUS DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

NTSTATUS CreateClose(PDEVICE_OBJECT pDeviceObject, PIRP Irp);

NTSTATUS IoControl(PDEVICE_OBJECT pDeviceObject, PIRP Irp);


#endif