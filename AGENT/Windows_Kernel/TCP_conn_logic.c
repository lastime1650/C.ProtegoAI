#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "TCP_conn.h"

#include "Build_Socket.h"

#include "util_Delay.h"// ������


#include "License_Agent_struct.h" // ���̼���,������Ʈ ID


#include "communication_with_RUST.h" // ���� ���� �� ����Ǵ� ��


BOOLEAN is_connected_2_SERVER = FALSE;
KEVENT is_connected_event = { 0 };

VOID TCP_conn(PVOID context) {
	UNREFERENCED_PARAMETER(context);
	//KeSetEvent
	KeInitializeEvent(&is_connected_event, SynchronizationEvent, FALSE);

	while (1) {
		/*�̹� ����Ǿ� �ִ°�? */
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 KeReadStateEvent �׽�Ʈ -> %d\n", KeReadStateEvent(&is_connected_event));
		if (!is_connected_2_SERVER) {
			/* TCP ���� ���� �õ� */
			/* TCP ���� �Ǿ��°�? */
			if (Make_TCP_Connection((PCHAR)"59.3.230.120", (USHORT)1029, &COMMAND_NewSocket) != STATUS_SUCCESS) { // [1/2]
				if (COMMAND_NewSocket == NULL) {// [2/2]
					Delays(-1);
					continue;
				}
			}
			else {
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TCP_conn => Make_TCP_Connection() ����!\n");

				/* Agent_ID�� ���� �����Ѵ�.
					HDD�� ����Ǹ�,
					������ �䱸�ϴ� "������"�� Agent_ID�� HDD�� ������ ������ SHA512(128B) ���� �޾Ƽ� HDD�� �����Ѵ�.
				*/
				if (GET_AGENT_ID() != STATUS_SUCCESS) {
					return;
				}

			}
		}
		is_connected_2_SERVER = TRUE;
		Delays(-1);

		HANDLE thread_handle = 0;
		PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, communication_server, NULL); // ���� ���� ���ù� ������

		Delays(-1);
		ZwClose(thread_handle);
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TCP_conn => is_connected_event �̺�Ʈ ��ݽõ�!!\n");
		KeWaitForSingleObject(&is_connected_event, Executive, KernelMode, FALSE, NULL); // ������ ����� �����尡 �ٽ� �帥��.


	}
}