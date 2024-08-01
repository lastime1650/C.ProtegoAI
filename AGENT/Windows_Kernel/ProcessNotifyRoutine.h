#ifndef ProcessNotifyRoutine_H
#define ProcessNotifyRoutine_H

#include <ntifs.h>
#include <ntddk.h>

//#include "PLIST_Node_Manager.h"


#include "converter_string.h"
#include "converter_PID.h"


typedef struct PROCESS_ROUTINE_struct {
	KEVENT event_for_saving;// 다른 스레드로 정보 이전용..  잠시 알림루틴함수 스레드의 흐름을 얼리기 위함

	PEPROCESS eprocess_addr;

	BOOLEAN is_process_creating; // TRUE면 프로세스 생성임을 인지
	HANDLE process_handle;//핸들 값
	HANDLE process_pid;//PID 값

	PUCHAR EXE_bin;
	ULONG EXE_bin_size;
	PUCHAR sha256;

	UNICODE_STRING process_ImageName;
}PROCESS_ROUTINE_struct, * PPROCESS_ROUTINE_struct;





// RUST를 위한 연결리스트의 " 한 노드 " 를 추가/생성하는 함수 ( 비동기 Thread ) 
VOID THREAD_for_send_to_process_data(PPROCESS_ROUTINE_struct context);

// 프로세스 알림 루틴
void PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);



#endif