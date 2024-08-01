#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include <ntifs.h>
#include <ntddk.h>



// ���̿��� ���� ��������
#include "query_system_smbios_information.h"

// ioctl ����
#include "my_ioctl.h" 

// ���̼���/������Ʈ ����ü ����
#include "License_Agent_struct.h"
ID_information Driver_ID = { 0 };

// RUST�� ���� �α� ���� ���
#include "TCP_conn.h"

//default) ���������� ���� ���μ����� ���Ḯ��Ʈ�� UPDATE�ϴ� ���� ������ ���
#include "query_process.h"

//����͸�
#include "NotifyRoutine.h"

//���μ��� ����/���� ��ȣ��ġ
#include "ObRegisterCallback_.h"

// ����̽� �˻� for Volume
#include "Get_Volumes.h"

// ���͸� �˻�
//#include "Query_Files_in_Directories.h";

//�̴����� 
#include "minifilter_register.h"

/*
	LNK2019 ������ ( ���,�Լ����� C ���� )<- ������ �� ���� �� �ִ�. �ַ� ����� �����ϴ� C������ (����)�����ٰ� �ٽ� �����ؾ� �ذ�� ����
*/
NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry) {
	NTSTATUS status = STATUS_SUCCESS;

	if (Bring_API() != STATUS_SUCCESS) {
		return STATUS_UNSUCCESSFUL;
	}

	DeviceInitialize(driver_object);


	/*  ���� ���Ͻý��� �ε�� ����̺� ��ü Ȯ�� �� ���� ���Ḯ��Ʈ ���� */
	ListMountedDrives(NULL, FALSE); // [1/ ] ����̽� Volume ���Ḯ��Ʈ ����


	// SM_BIOS
	Query_SMBIOS_information();

	if( Make_ObRegisterCallback() != STATUS_SUCCESS) return STATUS_UNSUCCESSFUL;

	if (Initialize_IOCTL_communicate() == FALSE) {  // ��������� ��û ���.
		return STATUS_UNSUCCESSFUL;
	}

	


	//HANDLE thread_handle2 = 0;
	//status = PsCreateSystemThread(&thread_handle2, THREAD_ALL_ACCESS, NULL, NULL, NULL, Get_ALL_Process_List, NULL); // ���� ���� ���� üũ ������ [2/2]
	//ZwClose(thread_handle2);


	/* ������ ���� < TCP������ �����Ѵ� >*/
	HANDLE thread_handle = 0;
	status = PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, TCP_conn, NULL); // ���� ���� ���� üũ ������ [2/2]
	ZwClose(thread_handle);


	/*
		�˸���ƾ ��� ����!
	*/
	initialize_NotifyRoutine();

	/*
		������Ʈ ��ƾ ��� ����!
	*/
	/*
	status = Make_ObRegisterCallback();
	if (status != STATUS_SUCCESS) {//���� ���μ��� ����/���� ���
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Make_ObRegisterCallback ��Ͻ��� -> %p\n", status);
		return STATUS_UNSUCCESSFUL;
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Make_ObRegisterCallback ��ϼ��� \n");
	*/


	/*
		�̴����� ����

		����̽� ���� ���� -> �ñ״�ó ������ -> �̴����� ��� �� �ٷ� ��밡��. 

	*/

	

	// �ñ״�ó ���Ḯ��Ʈ�� TCP ����� �� �����Ǿ����

	// �̴����� ���
	//Initialize_Mini_Filter_Driver(driver_object);


	return status;
}