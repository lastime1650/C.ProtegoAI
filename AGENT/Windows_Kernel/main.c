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


// test
#include "policy_signature_list_manager.h"
#include "Length_Based_Maker.h"
#include "RUST_DLP_Get_File_SIgnatures.h"
#include "Length_Based_Data_Parser.h"
#include "Query_Files_in_Directories_with_NT_Path.h"
#include "Parallel_Linked_List.h"


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

	//if( Make_ObRegisterCallback() != STATUS_SUCCESS) return STATUS_UNSUCCESSFUL;

	/*
		TEST
		-> ���� �����κ��� �ñ״�ó�� ���� ��,( �ʱ� ��� X ) ���Ḯ��Ʈ�� �߰��ϱ� ( �ñ״�ó ���Ḯ��Ʈ + ���� �ñ״�ó ���� ���Ḯ��Ʈ ) 
	*/
	/*
	CHAR test[] = "hwpx";

	PUCHAR APPENDING = NULL;
	ULONG32 APPENDING_SIZE = 0;

	SIG_STATUS t = is_register;
	
	Length_Based_MAKER((SIG_STATUS)0,(PUCHAR) &t,sizeof(t),&APPENDING ,&APPENDING_SIZE);
	Length_Based_MAKER((SIG_STATUS)0, (PUCHAR)&test, sizeof(test) - 1, &APPENDING, &APPENDING_SIZE);
	PLength_Based_DATA_Node SHOOT =  Build_RAW_DATA((PUCHAR)APPENDING, APPENDING_SIZE, TRUE);
	if (SHOOT) {
		Signature_append_or_remove_or_get(SHOOT);

		ListDirectories_with_extension_signature(Policy_Signature_Start_Node, ALL_DEVICE_DRIVES_Start_Node);

	}
	*/
	
	

	if (Initialize_IOCTL_communicate() == FALSE) {  // ��������� ��û ���.
		return STATUS_UNSUCCESSFUL;
	}
	//�׽�Ʈ
	KeSetEvent(&IOCTL_share_structure.ioctl_event, 0, FALSE);

	//Initialize_Mini_Filter_Driver(driver_object);

	HANDLE thread_handle2 = 0;
	status = PsCreateSystemThread(&thread_handle2, THREAD_ALL_ACCESS, NULL, NULL, NULL, Get_ALL_Process_List, NULL); // ���� ���� ���� üũ ������ [2/2]
	//ZwClose(thread_handle2);


	/* ������ ���� < TCP������ �����Ѵ� >*/
	HANDLE thread_handle = 0;
	status = PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, TCP_conn, NULL); // ���� ���� ���� üũ ������ [2/2]
	ZwClose(thread_handle);


	/*
		�˸���ƾ ��� ����!
	*/
	//initialize_NotifyRoutine();

	/*
		������Ʈ ��ƾ ��� ����!
	*/
	
	//status = Make_ObRegisterCallback();
	//if (status != STATUS_SUCCESS) {//���� ���μ��� ����/���� ���
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Make_ObRegisterCallback ��Ͻ��� -> %p\n", status);
	//	return STATUS_UNSUCCESSFUL;
	//}
	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Make_ObRegisterCallback ��ϼ��� \n");
	


	/*
		�̴����� ����

		����̽� ���� ���� -> �ñ״�ó ������ -> �̴����� ��� �� �ٷ� ��밡��. 

	*/

	

	// �ñ״�ó ���Ḯ��Ʈ�� TCP ����� �� �����Ǿ����

	// �̴����� ���
	//Initialize_Mini_Filter_Driver(driver_object);


	return status;
}