#pragma warning(disable:4100)
#pragma warning(disable:4996)


#include "ProcessNotifyRoutine.h"


#include "TCP_conn.h"
#include "SHA256.h"

#include "SEND_or_SAVE.h"
#include "PLIST_Node_Manager.h"

VOID THREAD_for_send_to_process_data(PPROCESS_ROUTINE_struct context) {
	/*
		데이터 이관
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

	memcpy(DATA.sha256, context->sha256, SHA256_String_Byte_Length - 1);  //블루스크린 발생 ( PID_to_EXE 실패해서 그걸 가져와버린듯) 

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " PcreateProcessNotifyRoutineEx sha256_2 -> %s\n", DATA.sha256);
	ExFreePoolWithTag(context->sha256, 'SHA');
	

	/* 이벤트 점유 해제 */
	
	KeSetEvent(&context->event_for_saving, 0, FALSE); // 루틴함수는 이제 프로세스를 정상적으로 생성할 수 있게 된다. ( 단, 포인터는 이제 유효X ) 




	/*핸들 얻기는 시간이 걸릴 수 있으므로, 별도 스레드 환경에서 ㄱㄱ */
	HANDLE real_handle = 0;
	Get_real_HANDLE_from_pid(DATA.process_pid, &real_handle, KernelMode);
	DATA.process_handle = real_handle;
	/*
		길이기반 데이터를 만드려면, 현재의 데이터를 리스트를 만들어서 해야함.
	*/
	/*
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		[노드에 값을 넣는 방법!]
			역순으로 값이 들어가게 설계하였으므로, 칼럼 맨 마지막에 들어가는 것은 필드의 맨 마지막부터 노드를 생성한다.
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	*/
	ULONG32 NODE_SIZE = 0;

	PLIST A_node = CREATE_tag_LIST(sizeof(DATA.is_process_creating), (PUCHAR)&DATA.is_process_creating, NULL, &NODE_SIZE); // 1

	A_node = APPENDLIST(sizeof(DATA.process_pid), (PUCHAR)&DATA.process_pid, A_node, &NODE_SIZE); // PID 값 2

	/* ANSI의 FULL-PATH 넣기 */
	ANSI_STRING output_for_fullpath = { 0, }; UNICODE_STRING unicode_fullpath = { 0, };
	//Query_Process_info(&DATA.process_handle, ProcessImageFileName, &unicode_fullpath); // optional 유니코드가 있는 경우, 추가적인 처리가 필요한 것 취급.
	PID_to_ANSI_FULL_PATH(DATA.process_pid, &unicode_fullpath, KernelMode);

	UNICODE_to_ANSI(&output_for_fullpath, &unicode_fullpath);

	A_node = APPENDLIST(output_for_fullpath.MaximumLength, (PUCHAR)output_for_fullpath.Buffer, A_node, &NODE_SIZE); // ANSI 형의 커널기반-절대경로 3


	A_node = APPENDLIST(4, (PUCHAR)&DATA.EXE_bin_size, A_node, &NODE_SIZE);
	A_node = APPENDLIST(SHA256_String_Byte_Length - 1, (PUCHAR)DATA.sha256, A_node, &NODE_SIZE);

	/*
		[1] 생성여부 (1b)
		[2] pid (8b)
		[3] full-path 절대경로 ( 가변 )
		[4] exe_size ( 4b )
		[5] SHA256 ( 64b )

		만약 데이터를 얻을 때 실패 시, 5바이트 -> "_FAIL" 를 삽입한다.
	*/

	ULONG32 TYPE = (ULONG32)process_routine; // 타입명시
	Send_or_Save(SAVE_RAW_DATA, &TYPE, &(PUCHAR)A_node, &NODE_SIZE, NONE); // 이미 상호배제 ( ResourcesLite ) 가 적용되어 있음 -> 동적-전역변수 대상
	ExFreePoolWithTag(unicode_fullpath.Buffer, 'UncB');
	ExFreePoolWithTag(DATA.EXE_bin, 'FILE');
	ExFreePoolWithTag(DATA.sha256, 'SHA');
	/*
		여기부터는 할당된 모든 동적변수가 할당해제 되어야만함
	*/
	return;
}





void PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo) {

	///////////////////////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////

	if (!is_connected_2_SERVER) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PcreateProcessNotifyRoutineEx -> is_connected_2_SERVER 값: FALSE \n");
		return;

	}



	/*
		프로세스 절대경로
		핸들 값
		PID 값
		생성시간 ?
	*/
	PROCESS_ROUTINE_struct TMP_data_pack = { 0 }; // 공통으로 사용되는 녀석
	KeInitializeEvent(&TMP_data_pack.event_for_saving, SynchronizationEvent, FALSE);// 이벤트 초기화 [1/?]


	TMP_data_pack.eprocess_addr = Process;


	TMP_data_pack.process_pid = ProcessId;

	if (CreateInfo) {
		TMP_data_pack.is_process_creating = TRUE; // TRUE -> 프로세스 생성 중

	}
	else {
		/* 프로세스 제거 됨*/
		TMP_data_pack.is_process_creating = FALSE; // FALSE -> 프로세스 제거 중
	}


	PUCHAR EXE_bin = NULL;
	ULONG EXE_size = 0;
	PUCHAR sha256 = ExAllocatePoolWithTag(NonPagedPool, SHA256_String_Byte_Length, 'SHA');
	if (sha256 == NULL) return;

	if (PID_to_EXE(ProcessId, &EXE_bin, &EXE_size, (PCHAR)sha256, KernelMode) != STATUS_SUCCESS) {
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PcreateProcessNotifyRoutineEx -> PID_to_EXE 실패 \n");
		ExFreePoolWithTag(sha256, 'SHA');
		return;
	}

	if (EXE_bin != NULL && sha256 != NULL) {
		TMP_data_pack.EXE_bin = EXE_bin;
		TMP_data_pack.EXE_bin_size = EXE_size;
		TMP_data_pack.sha256 = sha256;


		HANDLE thread_handle;
		PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, THREAD_for_send_to_process_data, &TMP_data_pack); // 프로세스 정보를 스레드에 전달
		ZwClose(thread_handle);
		/*
			호출한 스레드	<	THREAD_for_send_to_process_data	>	에서 해제해줄 때까지 무한정 대기
		*/

		KeWaitForSingleObject(&TMP_data_pack.event_for_saving, Executive, KernelMode, FALSE, NULL); // 이벤트 점유


	}
	else {
		ExFreePoolWithTag(sha256, 'SHA');
		return;
	}



}




