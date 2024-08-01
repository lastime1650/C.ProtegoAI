#include "NotifyRoutine.h"

NTSTATUS initialize_NotifyRoutine() {
	NTSTATUS status;

	status = PsSetCreateProcessNotifyRoutineEx(PcreateProcessNotifyRoutineEx, FALSE);
	if (status != STATUS_SUCCESS) {//최초 프로세스 생성/제거 등록
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PsSetCreateProcessNotifyRoutineEx 등록실패 -> %p\n", status);
		return STATUS_UNSUCCESSFUL;
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PsSetCreateProcessNotifyRoutineEx 등록성공 \n");
	return STATUS_SUCCESS;
}