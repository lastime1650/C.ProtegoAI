#include "NotifyRoutine.h"

NTSTATUS initialize_NotifyRoutine() {
	NTSTATUS status;

	status = PsSetCreateProcessNotifyRoutineEx(PcreateProcessNotifyRoutineEx, FALSE);
	if (status != STATUS_SUCCESS) {//���� ���μ��� ����/���� ���
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PsSetCreateProcessNotifyRoutineEx ��Ͻ��� -> %p\n", status);
		return STATUS_UNSUCCESSFUL;
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PsSetCreateProcessNotifyRoutineEx ��ϼ��� \n");
	return STATUS_SUCCESS;
}