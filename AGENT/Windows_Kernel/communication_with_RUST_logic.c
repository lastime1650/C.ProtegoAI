#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "communication_with_RUST.h"
#include "util_Delay.h"

//#include "PLIST_Node_Manager.h"
#include "SEND_or_SAVE.h"

VOID communication_server(PVOID context) { // ���������� �����κ��� ���ú� ���¸� ����
	UNREFERENCED_PARAMETER(context);

	NTSTATUS status = STATUS_SUCCESS;

	while (1)
	{
		if (is_connected_2_SERVER) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Always] �����մϴ�...");
			PVOID Get_BUFFER = NULL;
			ULONG32 Get_BUFFER_len = 0;

			// SERVER_DATA_PROCESS <- enum���� Mutex ������ ���� ���ϹǷ� �̳��� ���� �ؾ��� 
			status = RECEIVE_TCP_DATA__with_alloc(&Get_BUFFER, &Get_BUFFER_len, SERVER_DATA_PROCESS); // SERVER_DATA_PROCESS �� ���ؽ� ������ ����!

			/*
				STATUS
				���� STATUS_INSUFFICIENT_RESOURCES ������ ��� �����Ҵ� ���и� �ǹ�
			*/
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Always]�����κ��� ���� ������ ������ : %lu\n\n", Get_BUFFER_len);
			if (((status == STATUS_INSUFFICIENT_RESOURCES || status == STATUS_UNSUCCESSFUL))) {
				// ������ ��û�� �� �о��� �� (����) /
				if (Get_BUFFER != NULL) {
					ExFreePoolWithTag(Get_BUFFER, 'RcDt');// ����
				}
				// ������ ��û�� �� �о��� ��
				continue;
			}
			else {
				// ������ ��û�� �о��� �� (����) /





				SERVER_COMMAND server_cmd = 0;
				memcpy(&server_cmd, Get_BUFFER, 4);// �� �� 4byte�� �о� enum�� ���
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " communication_server -> server_cmd �� : %lu \n", server_cmd);
				switch (server_cmd) {



				case GET_file_bin:
					/*
						{4b (enum��)} + {���ڿ��� - Ansi}
					*/
					Get_BUFFER;
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\nFILE_bin_process \n  ");
					status = FILE_bin_process((PUCHAR)Get_BUFFER + 4, Get_BUFFER_len - 4);//������ "������" ���������� �Ű� ����.
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FILE_bin_process ���� %p \n", status);
					break;


				case Are_you_ALIVE:
					/*
						{ (enum��) + ( AGENT_ID - Ansi ) }
					*/
					Get_BUFFER;
					status = Are_You_ALIVE_process((PUCHAR)Get_BUFFER + 4, Get_BUFFER_len - 4);
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Are_You_ALIVE_process ���� %p ", status);
					break;
				case GET_collected_data:
					status = Send_or_Save(SEND_RAW_DATA, NULL, &RAW_DATA__external_var, &RAW_DATA_SIZE__external_var, SERVER_DATA_PROCESS);
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "GET_collected_data ��� %p ", status);
					if (status != STATUS_SUCCESS) {
						/* ���и� �������� ���� ;;*/
						ULONG32 return_enum = No;
						SEND_TCP_DATA(&return_enum, sizeof(return_enum), SERVER_DATA_PROCESS);
					}
					break;
				case GET_action_process_creation: // [ Action ] ���μ��� ������ ���� �׼�ó��
					/*
						{enum��} + {4b}{Method} + {4b}{SHA256} + {4b}{TYPE} <<-- �̰��� ���̱�� �Ľ��� �ؾ���(�ݺ���)
					*/
					Initilize_or_Locking_PKmutex(&Action_for_proces_routine_node_KMUTEX, TRUE);
					Action_for_process_creation((PUCHAR)Get_BUFFER + 4, Get_BUFFER_len - 4);
					Release_PKmutex(&Action_for_proces_routine_node_KMUTEX);






				default:
					break;
				}



				ExFreePoolWithTag(Get_BUFFER, 'RcDt');// ����
				KeReleaseMutex(TCP_session, FALSE); ////////////////////////////////////////////// *************Release********************************
				Delays(-1);
				continue;
			}

		}
	}
}
