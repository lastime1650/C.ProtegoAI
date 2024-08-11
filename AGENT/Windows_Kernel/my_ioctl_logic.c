#pragma warning(disable:4100)
#include "my_ioctl.h"
#include "Initialize_API.h"


IOCTL_content IOCTL_share_structure = { 0, };
KMUTEX IOCTL_access_mutex = { 0, };

PDEVICE_OBJECT DeviceInitialize(IN PDRIVER_OBJECT DriverObject) {


	if (!DriverObject) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DriverObject가 비었습니다.\n");
		return NULL;
	}

	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING deviceName, symlinkName;

	RtlInitUnicodeString(&deviceName, DEVICE_NAME);
	RtlInitUnicodeString(&symlinkName, SYMLINK_NAME);

	// 디바이스 객체 생성 
	status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(status)) {
		return NULL;
	}
	// 심볼릭 링크 생성 - 유저모드에서 커널에 접근하기 위한.. 
	status = IoCreateSymbolicLink(&symlinkName, &deviceName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(pDeviceObject);
		return NULL;
	}



	KeInitializeEvent(&IOCTL_share_structure.ioctl_event, SynchronizationEvent, FALSE); // 커널 ioctl에서 사용되는 이벤트.
	
	KeInitializeMutex(&IOCTL_access_mutex, 0);
	IOCTL_share_structure.is_usermode_request = FALSE; // ioctl 유저모드에서 요청안온걸로 초기화
	IOCTL_share_structure.ioctl_BUFFER = NULL;
	IOCTL_share_structure.is_init = TRUE; // 초기화 동작임을 확인.





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
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "드라이버 언로드 됨!\n");

	return STATUS_SUCCESS;

}


// 디바이스 접근시 처리하는 함수 ( 사용하지 않지만, IOCTL 시 기본으로 필요 ) 
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

		// 유저모드에서 요청옴!

		IOCTL_share_structure.ioctl_BUFFER = (PCOMMUNICATION_IOCTL)Irp->AssociatedIrp.SystemBuffer;// 유저모드 데이터 얻기


		/*
			추가된,

			DLL 후크 걸린 프로세스에서 IOCTL 요청이 온경우, 자동처리해야함

		*/

		if (IOCTL_share_structure.ioctl_BUFFER->information == HOOK_MON) {

			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " DLL에서 요청옴 -> PID %lu , API_NAME %s \n", IOCTL_share_structure.ioctl_BUFFER->API_HOOK_MON.PID, IOCTL_share_structure.ioctl_BUFFER->API_HOOK_MON.Hooked_API_NAME);
			Irp->IoStatus.Information = sizeof(COMMUNICATION_IOCTL);// 전송할 크기 지정( 필수 ) 
			Irp->IoStatus.Status = status;
			IoCompleteRequest(Irp, IO_NO_INCREMENT);// 전송
			return status;
		}

		/*
			===============================================================
		*/

		IOCTL_share_structure.is_usermode_request = TRUE; // 유저모드에서 요청을 받고 대기하고 있어요!!!!!

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "유저모드에서 요청이 왔고 대기 중입니다!  \n");


		
		if (IOCTL_share_structure.is_init) {

			// 특정 프로세스 핸들의 권한을 모두 접근가능하도록 조정
			
			PEPROCESS process_obj = NULL;
			PsLookupProcessByProcessId((HANDLE)IOCTL_share_structure.ioctl_BUFFER->Ioctl_User_Mode_ProcessId, &process_obj);

			HANDLE handle = 0;
			ObOpenObjectByPointer(process_obj, OBJ_KERNEL_HANDLE, NULL, PROCESS_ALL_ACCESS, *PsProcessType, KernelMode, &handle);

			if (handle) {
				ZwClose(handle);
			}

			/*  Ioctl 유저모드 프로세스 보호를 위하여, ObRegisterCallbacks 연결리스트 생성*/
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
				IoCompleteRequest(Irp, IO_NO_INCREMENT);// 전송
				return status;
			}

			ExFreePoolWithTag(EXE, 'FILE');
			EXE_Size = 0;

			// 연결리스트에 등록하기전에, 이미 존재하는 지 확인하고, (혹시 모를 것을 대비하기 위하여 존재하는경우, UPDATE하도록 기존의 것을 지운다. 
			if (!is_exist_program_Action_Process_Node(&Action_Data)) {
				// 이미 존재하지 않을 때,
				//Remove_one_node_Action_Process_Node(&Action_Data);

				
				// process 닫을 때를 거부하도록 연결리스트에 등록한다 
				if (Create_or_Append_Action_Process_Node(&Action_Data) == FALSE) {
					status = STATUS_INVALID_DEVICE_REQUEST;
					Irp->IoStatus.Status = status;
					IoCompleteRequest(Irp, IO_NO_INCREMENT);// 전송
					return status;
				}
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "is_exist_program_Action_Process_Node 존재하지 않음 -> %s \n", Action_Data.SHA256);

			}


			
		}

		KeWaitForSingleObject(&IOCTL_share_structure.ioctl_event, Executive, KernelMode, FALSE, NULL); // 실행 멈춤 ( 다른 스레드에서 풀어야함 )

		IOCTL_share_structure.is_usermode_request = FALSE;

		if (IOCTL_share_structure.is_init) {
			
			IOCTL_share_structure.is_init = FALSE; // 초기아님
		}

		

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "유저모드에서 요청 처리 완료!\n");
		Irp->IoStatus.Information = sizeof(COMMUNICATION_IOCTL);// 전송할 크기 지정( 필수 ) 
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);// 전송


	return status;
}