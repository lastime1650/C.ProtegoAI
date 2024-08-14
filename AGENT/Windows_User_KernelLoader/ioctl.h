#ifndef IOCTL_H
#define IOCTL_H

#include <stdio.h>
#include <Windows.h>

/*
	IOCTL �۾� �����
*/
#include "Hooking.h"

#define IOCTL_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef enum communication_ioctl_ENUM {

	CHECK = 1,

	SUCCESS = 100,
	FAIL = 101,
	REQUEST_all = 1029,
	REQUESET_without_AGENT_ID = 1030,
	WAIT_FAILED_from_Center_Server = 2000,
	WAIT_FAILED_from_Kernel = 2001,
	WAIT_FAILED_from_User = 2002,


	HOOKING_request = 3000,

	HOOK_MON = 3001// API ��ũ�� �Լ����� ��û�ϴ� ����

} COMMUNICATION_IOCTL_ENUM;


// �ַ� Ŀ���� ������忡�� �����ϴ� ��ŷ ���� ��û ������
typedef struct comunication_ioctl_for_HOOKING {

	HANDLE PID;
	HANDLE Process_HANDLE;

}comunication_ioctl_for_HOOKING, *Pcomunication_ioctl_for_HOOKING;

// DLL ���� ����ϴ� ����ü
typedef struct HOOK_IOCTL_DATA {
	BOOLEAN is_admin_running; // �� ���μ����� ���� ������ ��������
	HANDLE PID;//�ڽ��� PID 
	UCHAR Hooked_API_NAME[128]; // ��ũ �ɸ� API �̸�
}HOOK_IOCTL_DATA, * PHOOK_IOCTL_DATA;

typedef struct communication_ioctl {
	COMMUNICATION_IOCTL_ENUM information;
	UCHAR license_ID[128];
	UCHAR Agent_ID[128];

	HANDLE Ioctl_User_Mode_ProcessId; // �߰��� 

	comunication_ioctl_for_HOOKING HOOK_DATA; // �߰���

	HOOK_IOCTL_DATA API_HOOK_MON; // DLL ���� ����ϴ� ����ü

} COMMUNICATION_IOCTL, * PCOMMUNICATION_IOCTL;



BOOLEAN Initialize_communicate(PCOMMUNICATION_IOCTL BUFFER, HANDLE* hDevice);

BOOLEAN Keeping_communicate(PCOMMUNICATION_IOCTL BUFFER, HANDLE hDevice);



#endif // IOCTL.h