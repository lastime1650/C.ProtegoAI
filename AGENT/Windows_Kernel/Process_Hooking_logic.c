#include "Process_Hooking.h"

BOOLEAN Hooking_Start(HANDLE PID, PVOID LoadLibrary_Address, PCHAR DLL_ANSi) {
	// ��ŷ�� �����Ѵ�. 

	// PID to HANDLE
	HANDLE PID_HANDLE = 0;
	Get_real_HANDLE_from_pid(PID, &PID_HANDLE, KernelMode);
	if (PID_HANDLE == 0) return FALSE;

	// ���� ���μ����� DLL ���ڿ� �����Ҵ� �� ����
	PVOID Base_addr = NULL;
	ZwAllocateVirtualMemory(PID_HANDLE, &Base_addr, 0, 256, MEM_COMMIT, PAGE_READWRITE);
	if (Base_addr == NULL) return FALSE;

	// PID to EPROCESS
	PEPROCESS Eprocess = NULL;
	PsLookupProcessByProcessId(PID,&Eprocess);


	KAPC_STATE ApcState;
	KeStackAttachProcess(Eprocess,&ApcState);
	// ������� ���ؽ�Ʈ ���� 

	KeUnstackDetachProcess(&ApcState);
}