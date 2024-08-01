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

	SUCCESS_id = 100,
	FAIL_id = 101,

	REQUEST_all = 1029, // ���̼��� + ������Ʈ ID �� �� ���� ��,
	REQUESET_without_AGENT_ID = 1030, // ���̼����� ���� ��

	WAIT_FAILED_from_Center_Server = 2000, // RUST������ ����� �ȵ� ��  ( �ʱ���Ž� )
	WAIT_FAILED_from_Kernel = 2001, // Ŀ�ηκ��� ������ ��ٸ��� ������ �� ( Ŀ�ο��� ������ �߻��� ��� ) 
	WAIT_FAILED_from_User = 2002 // �����κ��� ������ ��ٸ��� ������ ��


}COMMUNICATION_IOCTL_ENUM;

typedef struct communication_ioctl {

	COMMUNICATION_IOCTL_ENUM information;

	UCHAR license_ID[128];
	UCHAR Agent_ID[128];

	HANDLE Ioctl_User_Mode_ProcessId; // �߰��� 

}COMMUNICATION_IOCTL, * PCOMMUNICATION_IOCTL;

typedef struct IOCTL_content {
	BOOLEAN is_init;

	KEVENT ioctl_event;
	BOOLEAN is_usermode_request;
	PCOMMUNICATION_IOCTL ioctl_BUFFER;
}IOCTL_content, * PIOCTL_content;





//��������
extern IOCTL_content IOCTL_share_structure;




PDEVICE_OBJECT DeviceInitialize(IN PDRIVER_OBJECT DriverObject);

NTSTATUS DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

NTSTATUS CreateClose(PDEVICE_OBJECT pDeviceObject, PIRP Irp);

NTSTATUS IoControl(PDEVICE_OBJECT pDeviceObject, PIRP Irp);


#endif