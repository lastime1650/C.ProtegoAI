#pragma warning(disable:4100)
#pragma warning(disable:4996)


#include "ProcessNotifyRoutine.h"


#include "TCP_conn.h"
#include "SHA256.h"

#include "SEND_or_SAVE.h"
#include "PLIST_Node_Manager.h"

VOID THREAD_for_send_to_process_data(PPROCESS_ROUTINE_struct context) {
	/*
		������ �̰�
	*/
	PROCESS_ROUTINE_struct DATA = { 0 };
	DATA.is_process_creating = context->is_process_creating;
	DATA.eprocess_addr = context->eprocess_addr;
	DATA.process_pid = context->process_pid;
	DATA.EXE_bin = (PUCHAR)ExAllocatePoolWithTag(PagedPool, context->EXE_bin_size, 'FILE');
	memcpy(DATA.EXE_bin, context->EXE_bin, context->EXE_bin_size);
	DATA.EXE_bin_size = context->EXE_bin_size;


	DATA.sha256 = (PUCHAR)ExAllocatePoolWithTag(PagedPool, SHA256_String_Byte_Length - 1, 'SHA');
	if (DATA.sha256 == NULL) {
		ExFreePoolWithTag(DATA.EXE_bin, 'FILE');
		ExFreePoolWithTag(context->sha256, 'SHA');
		KeSetEvent(&context->event_for_saving, 0, FALSE);
		return;
	}

	memcpy(DATA.sha256, context->sha256, SHA256_String_Byte_Length - 1);  //��罺ũ�� �߻� ( PID_to_EXE �����ؼ� �װ� �����͹�����) 

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " PcreateProcessNotifyRoutineEx sha256_2 -> %s\n", DATA.sha256);
	ExFreePoolWithTag(context->sha256, 'SHA');
	

	/* �̺�Ʈ ���� ���� */
	
	KeSetEvent(&context->event_for_saving, 0, FALSE); // ��ƾ�Լ��� ���� ���μ����� ���������� ������ �� �ְ� �ȴ�. ( ��, �����ʹ� ���� ��ȿX ) 




	/*�ڵ� ���� �ð��� �ɸ� �� �����Ƿ�, ���� ������ ȯ�濡�� ���� */
	HANDLE real_handle = 0;
	Get_real_HANDLE_from_pid(DATA.process_pid, &real_handle, KernelMode);
	DATA.process_handle = real_handle;
	/*
		���̱�� �����͸� �������, ������ �����͸� ����Ʈ�� ���� �ؾ���.
	*/
	/*
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		[��忡 ���� �ִ� ���!]
			�������� ���� ���� �����Ͽ����Ƿ�, Į�� �� �������� ���� ���� �ʵ��� �� ���������� ��带 �����Ѵ�.
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	*/
	ULONG32 NODE_SIZE = 0;

	PLIST A_node = CREATE_tag_LIST(sizeof(DATA.is_process_creating), (PUCHAR)&DATA.is_process_creating, NULL, &NODE_SIZE); // 1

	A_node = APPENDLIST(sizeof(DATA.process_pid), (PUCHAR)&DATA.process_pid, A_node, &NODE_SIZE); // PID �� 2

	/* ANSI�� FULL-PATH �ֱ� */
	ANSI_STRING output_for_fullpath = { 0, }; UNICODE_STRING unicode_fullpath = { 0, };
	//Query_Process_info(&DATA.process_handle, ProcessImageFileName, &unicode_fullpath); // optional �����ڵ尡 �ִ� ���, �߰����� ó���� �ʿ��� �� ���.
	PID_to_ANSI_FULL_PATH(DATA.process_pid, &unicode_fullpath, KernelMode);

	UNICODE_to_ANSI(&output_for_fullpath, &unicode_fullpath);

	A_node = APPENDLIST(output_for_fullpath.MaximumLength, (PUCHAR)output_for_fullpath.Buffer, A_node, &NODE_SIZE); // ANSI ���� Ŀ�α��-������ 3


	A_node = APPENDLIST(4, (PUCHAR)&DATA.EXE_bin_size, A_node, &NODE_SIZE);
	A_node = APPENDLIST(SHA256_String_Byte_Length - 1, (PUCHAR)DATA.sha256, A_node, &NODE_SIZE);

	/*
		[1] �������� (1b)
		[2] pid (8b)
		[3] full-path ������ ( ���� )
		[4] exe_size ( 4b )
		[5] SHA256 ( 64b )

		���� �����͸� ���� �� ���� ��, 5����Ʈ -> "_FAIL" �� �����Ѵ�.
	*/

	ULONG32 TYPE = (ULONG32)process_routine; // Ÿ�Ը��
	Send_or_Save(SAVE_RAW_DATA, &TYPE, &(PUCHAR)A_node, &NODE_SIZE, NONE); // �̹� ��ȣ���� ( ResourcesLite ) �� ����Ǿ� ���� -> ����-�������� ���
	ExFreePoolWithTag(unicode_fullpath.Buffer, 'UncB');
	ExFreePoolWithTag(DATA.EXE_bin, 'FILE');
	ExFreePoolWithTag(DATA.sha256, 'SHA');
	/*
		������ʹ� �Ҵ�� ��� ���������� �Ҵ����� �Ǿ�߸���
	*/
	return;
}





void PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo) {

	///////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////

	if (!is_connected_2_SERVER) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PcreateProcessNotifyRoutineEx -> is_connected_2_SERVER ��: FALSE \n");
		return;

	}



	/*
		���μ��� ������
		�ڵ� ��
		PID ��
		�����ð� ?
	*/
	PROCESS_ROUTINE_struct TMP_data_pack = { 0 }; // �������� ���Ǵ� �༮
	KeInitializeEvent(&TMP_data_pack.event_for_saving, SynchronizationEvent, FALSE);// �̺�Ʈ �ʱ�ȭ [1/?]


	TMP_data_pack.eprocess_addr = Process;


	TMP_data_pack.process_pid = ProcessId;

	if (CreateInfo) {
		TMP_data_pack.is_process_creating = TRUE; // TRUE -> ���μ��� ���� ��

	}
	else {
		/* ���μ��� ���� ��*/
		TMP_data_pack.is_process_creating = FALSE; // FALSE -> ���μ��� ���� ��
	}


	PUCHAR EXE_bin = NULL;
	ULONG EXE_size = 0;
	PUCHAR sha256 = ExAllocatePoolWithTag(NonPagedPool, SHA256_String_Byte_Length, 'SHA');
	if (sha256 == NULL) return;

	if (PID_to_EXE(ProcessId, &EXE_bin, &EXE_size, (PCHAR)sha256, KernelMode) != STATUS_SUCCESS) {
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PcreateProcessNotifyRoutineEx -> PID_to_EXE ���� \n");
		ExFreePoolWithTag(sha256, 'SHA');
		return;
	}

	if (EXE_bin != NULL && sha256 != NULL) {
		TMP_data_pack.EXE_bin = EXE_bin;
		TMP_data_pack.EXE_bin_size = EXE_size;
		TMP_data_pack.sha256 = sha256;


		HANDLE thread_handle;
		PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, THREAD_for_send_to_process_data, &TMP_data_pack); // ���μ��� ������ �����忡 ����
		ZwClose(thread_handle);
		/*
			ȣ���� ������	<	THREAD_for_send_to_process_data	>	���� �������� ������ ������ ���
		*/

		KeWaitForSingleObject(&TMP_data_pack.event_for_saving, Executive, KernelMode, FALSE, NULL); // �̺�Ʈ ����


	}
	else {
		ExFreePoolWithTag(sha256, 'SHA');
		return;
	}



}




