#ifndef ProcessNotifyRoutine_H
#define ProcessNotifyRoutine_H

#include <ntifs.h>
#include <ntddk.h>

//#include "PLIST_Node_Manager.h"


#include "converter_string.h"
#include "converter_PID.h"


typedef struct PROCESS_ROUTINE_struct {
	KEVENT event_for_saving;// �ٸ� ������� ���� ������..  ��� �˸���ƾ�Լ� �������� �帧�� �󸮱� ����

	PEPROCESS eprocess_addr;

	BOOLEAN is_process_creating; // TRUE�� ���μ��� �������� ����
	HANDLE process_handle;//�ڵ� ��
	HANDLE process_pid;//PID ��

	PUCHAR EXE_bin;
	ULONG EXE_bin_size;
	PUCHAR sha256;

	UNICODE_STRING process_ImageName;
}PROCESS_ROUTINE_struct, * PPROCESS_ROUTINE_struct;





// RUST�� ���� ���Ḯ��Ʈ�� " �� ��� " �� �߰�/�����ϴ� �Լ� ( �񵿱� Thread ) 
VOID THREAD_for_send_to_process_data(PPROCESS_ROUTINE_struct context);

// ���μ��� �˸� ��ƾ
void PcreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);



#endif