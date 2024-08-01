#ifndef License_Agent_struct_H
#define License_Agent_struct_H

#include <ntifs.h>
#include <ntddk.h>
#include "my_ioctl.h"

// COMMUNICATION_IOCTL_ENUM <- my_ioctl

typedef struct ID_info {// [NEW]
	COMMUNICATION_IOCTL_ENUM information; // ID ���� Ȯ�ο�
	UCHAR AGENT_ID[128]; // ANsi 
	UCHAR LICENSE_ID[128]; // ANsi 
	
	/*[�ϵ���� ������-�ʵ�]*/
	UCHAR SMBIOS_TYPE_1[256]; // �ý��� ����
	UCHAR SMBIOS_TYPE_2[256]; // �������� ����

}ID_information, * PID_information;


//��������
extern ID_information Driver_ID;


// ���� ������忡�� ���̼��� + ������Ʈ ID�� �������� ������ ��ٷȴٰ�(�̺�Ʈ-����) Driver_ID ���������� ���� ����. 
BOOLEAN Initialize_IOCTL_communicate();


// AGENT_ID, LICENSE_ID�� RUST�� ����Ͽ� ������.
NTSTATUS GET_AGENT_ID();

#endif