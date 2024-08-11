#pragma warning(disable:4100)
#include "my_ioctl.h"
#include "Initialize_API.h"


IOCTL_content IOCTL_share_structure = { 0, };
KMUTEX IOCTL_access_mutex = { 0, };

PDEVICE_OBJECT DeviceInitialize(IN PDRIVER_OBJECT DriverObject) {


	if (!DriverObject) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DriverObject�� ������ϴ�.\n");
		return NULL;
	}

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING deviceName, symlinkName;

	RtlInitUnicodeString(&deviceName, DEVICE_NAME);
	RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);

	// ����̽� ��ü ���� 
	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(status)) {
		return NULL;
	}
	// �ɺ��� ��ũ ���� - ������忡�� Ŀ�ο� �����ϱ� ����.. 
	status = IoCreateSymbolicLink(&symlinkName, &deviceName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(pDeviceObject);
		return NULL;
	}



	KeInitializeEvent(&IOCTL_share_structure.ioctl_event, SynchronizationEvent, FALSE); // Ŀ�� ioctl���� ���Ǵ� �̺�Ʈ.
	
	KeInitializeMutex(&IOCTL_access_mutex, 0);
	IOCTL_share_structure.is_usermode_request = FALSE; // ioctl ������忡�� ��û�ȿ°ɷ� �ʱ�ȭ
	IOCTL_share_structure.ioctl_BUFFER = NULL;
	IOCTL_share_structure.is_init = TRUE; // �ʱ�ȭ �������� Ȯ��.





	DriverObject->DriverUnload = DriverUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;

	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return pDeviceObject;
}


NTSTATUS DriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symlinkName;

	RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);
	IoDeleteSymbolicLink(&symlinkName);

	IoDeleteDevice(DriverObject->DeviceObject);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "����̹� ��ε� ��!\n");

	return STATUS_SUCCESS;

}


// ����̽� ���ٽ� ó���ϴ� �Լ� ( ������� ������, IOCTL �� �⺻���� �ʿ� ) 
NTSTATUS CreateClose(PDEVICE_OBJECT pDeviceObject, PIRP Irp) {
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS IoControl(PDEVICE_OBJECT pDeviceObject, PIRP Irp) {
	PIO_STACK_LOCATION pIoStackIrp = NULL;
	NTSTATUS status = STATUS_SUCCESS;

	pIoStackIrp = IoGetCurrentIrpStackLocation(Irp);

	switch (pIoStackIrp->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_TEST:

		// ������忡�� ��û��!

		IOCTL_share_structure.ioctl_BUFFER = (PCOMMUNICATION_IOCTL)Irp->AssociatedIrp.SystemBuffer;// ������� ������ ���


		/*
			�߰���,

			DLL ��ũ �ɸ� ���μ������� IOCTL ��û�� �°��, �ڵ�ó���ؾ���

		*/

		if (IOCTL_share_structure.ioctl_BUFFER->information == HOOK_MON) {

			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " DLL���� ��û�� -> PID %lu , API_NAME %s \n", IOCTL_share_structure.ioctl_BUFFER->API_HOOK_MON.PID, IOCTL_share_structure.ioctl_BUFFER->API_HOOK_MON.Hooked_API_NAME);
			Irp->IoStatus.Information = sizeof(COMMUNICATION_IOCTL);// ������ ũ�� ����( �ʼ� ) 
			Irp->IoStatus.Status = status;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);// ����
			return status;
		}

		/*
			===============================================================
		*/

		IOCTL_share_structure.is_usermode_request = TRUE; // ������忡�� ��û�� �ް� ����ϰ� �־��!!!!!

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "������忡�� ��û�� �԰� ��� ���Դϴ�!  \n");


		
		if (IOCTL_share_structure.is_init) {

			// Ư�� ���μ��� �ڵ��� ������ ��� ���ٰ����ϵ��� ����
			
			PEPROCESS process_obj = NULL;
			PsLookupProcessByProcessId((HANDLE)IOCTL_share_structure.ioctl_BUFFER->Ioctl_User_Mode_ProcessId, &process_obj);

			HANDLE handle = 0;
			ObOpenObjectByPointer(process_obj, OBJ_KERNEL_HANDLE, NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &handle);

			if (handle) {
				ZwClose(handle);
			}

			/*  Ioctl ������� ���μ��� ��ȣ�� ���Ͽ�, ObRegisterCallbacks ���Ḯ��Ʈ ����*/
			PUCHAR EXE = NULL;
			ULONG EXE_Size = 0;

			ActionProcessNode Action_Data = { 0, };
			memset(&Action_Data, 0, sizeof(ActionProcessNode));
			Action_Data.feature = Remove;
			Action_Data.is_Block = TRUE;
			Action_Data.SHA256;

			//if (PID_to_EXE(IOCTL_share_structure.ioctl_BUFFER->Ioctl_User_Mode_ProcessId, &EXE, &EXE_Size, (PCHAR)&Action_Data.SHA256, KernelMode) != STATUS_SUCCESS) {
			if (PID_to_EXE((HANDLE)IOCTL_share_structure.ioctl_BUFFER->Ioctl_User_Mode_ProcessId, &EXE, &EXE_Size, (PCHAR)&Action_Data.SHA256, KernelMode) != STATUS_SUCCESS) {
				status = STATUS_INVALID_DEVICE_REQUEST;
				Irp->IoStatus.Status = status;
				IoCompleteRequest(Irp, IO_NO_INCREMENT);// ����
				return status;
			}

			ExFreePoolWithTag(EXE, 'FILE');
			EXE_Size = 0;

			// ���Ḯ��Ʈ�� ����ϱ�����, �̹� �����ϴ� �� Ȯ���ϰ�, (Ȥ�� �� ���� ����ϱ� ���Ͽ� �����ϴ°��, UPDATE�ϵ��� ������ ���� �����. 
			if (!is_exist_program_Action_Process_Node(&Action_Data)) {
				// �̹� �������� ���� ��,
				//Remove_one_node_Action_Process_Node(&Action_Data);

				
				// process ���� ���� �ź��ϵ��� ���Ḯ��Ʈ�� ����Ѵ� 
				if (Create_or_Append_Action_Process_Node(&Action_Data) == FALSE) {
					status = STATUS_INVALID_DEVICE_REQUEST;
					Irp->IoStatus.Status = status;
					IoCompleteRequest(Irp, IO_NO_INCREMENT);// ����
					return status;
				}
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "is_exist_program_Action_Process_Node �������� ���� -> %s \n", Action_Data.SHA256);

			}


			
		}

		KeWaitForSingleObject(&IOCTL_share_structure.ioctl_event, Executive, KernelMode, FALSE, NULL); // ���� ���� ( �ٸ� �����忡�� Ǯ����� )

		IOCTL_share_structure.is_usermode_request = FALSE;

		if (IOCTL_share_structure.is_init) {
			
			IOCTL_share_structure.is_init = FALSE; // �ʱ�ƴ�
		}

		

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "������忡�� ��û ó�� �Ϸ�!\n");
		Irp->IoStatus.Information = sizeof(COMMUNICATION_IOCTL);// ������ ũ�� ����( �ʼ� ) 
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);// ����


	return status;
}