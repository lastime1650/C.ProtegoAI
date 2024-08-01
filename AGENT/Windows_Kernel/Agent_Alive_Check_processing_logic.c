#pragma warning(disable:4100)
//#pragma warning(disable:4996)
#include "Agent_Alive_Check_processing.h"

NTSTATUS Are_You_ALIVE_process(PUCHAR BUFFER, ULONG32 BUFFER_Size) {



	if (!(memcmp(BUFFER, Driver_ID.AGENT_ID, 128) == 0)) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[areyoualive] AGENT_ID°¡ ´Ù¸¨´Ï´Ù.");
		ULONG32 response = No;
		SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
		return STATUS_UNSUCCESSFUL;
	}

	ULONG32 response = Yes;
	SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
	return STATUS_SUCCESS;

}