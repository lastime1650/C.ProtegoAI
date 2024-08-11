#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "communication_with_RUST.h"
#include "util_Delay.h"

//#include "PLIST_Node_Manager.h"
#include "SEND_or_SAVE.h"

#include "Length_Based_Data_Parser.h" // RUST의 길이기반을 해석해야하므로 

VOID communication_server(PVOID context) { // 지속적으로 서버로부터 리시브 상태를 만듦
	UNREFERENCED_PARAMETER(context);

	NTSTATUS status = STATUS_SUCCESS;

	while (1)
	{
		if (is_connected_2_SERVER) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Always] 시작합니다...");
			PVOID Get_BUFFER = NULL;
			ULONG32 Get_BUFFER_len = 0;

			// SERVER_DATA_PROCESS <- enum값은 Mutex 해제를 절대 안하므로 겁나게 주의 해야함 
			status = RECEIVE_TCP_DATA__with_alloc(&Get_BUFFER, &Get_BUFFER_len, SERVER_DATA_PROCESS); // SERVER_DATA_PROCESS 는 뮤텍스 해제가 없음!

			/*
				STATUS
				만약 STATUS_INSUFFICIENT_RESOURCES 리턴인 경우 동적할당 실패를 의미
			*/
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Always]서버로부터 받은 데이터 사이즈 : %lu\n\n", Get_BUFFER_len);
			if (((status == STATUS_INSUFFICIENT_RESOURCES || status == STATUS_UNSUCCESSFUL))) {
				// 서버의 요청을 못 읽었을 때 (실패) /
				if (Get_BUFFER != NULL) {
					ExFreePoolWithTag(Get_BUFFER, 'RcDt');// 해제
				}
				// 서버의 요청을 못 읽었을 때
				continue;
			}
			else {
				// 서버의 요청을 읽었을 때 (성공) /





				SERVER_COMMAND server_cmd = 0;
				memcpy(&server_cmd, Get_BUFFER, 4);// 맨 앞 4byte를 읽어 enum값 취급
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " communication_server -> server_cmd 값 : %lu \n", server_cmd);

				
				PLength_Based_DATA_Node RAW_DATA_NODE_ADDR_from_length_based = NULL;

				switch (server_cmd) {



				case GET_file_bin:
					/*
						{4b (enum값)} + {문자열값 - Ansi}
					*/
					Get_BUFFER;
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\nFILE_bin_process \n  ");
					status = FILE_bin_process((PUCHAR)Get_BUFFER + 4, Get_BUFFER_len - 4);//서버의 "절대경로" 오프셋으로 옮겨 전달.
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "FILE_bin_process 성공 %p \n", status);
					break;


				case Are_you_ALIVE:
					/*
						{ (enum값) + ( AGENT_ID - Ansi ) }
					*/
					Get_BUFFER;
					status = Are_You_ALIVE_process((PUCHAR)Get_BUFFER + 4, Get_BUFFER_len - 4);
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Are_You_ALIVE_process 성공 %p ", status);
					break;
				case GET_collected_data:
					status = Send_or_Save(SEND_RAW_DATA, NULL, &RAW_DATA__external_var, &RAW_DATA_SIZE__external_var, SERVER_DATA_PROCESS);
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "GET_collected_data 결과 %p ", status);
					if (status != STATUS_SUCCESS) {
						/* 실패를 서버에게 전달 ;;*/
						ULONG32 return_enum = No;
						SEND_TCP_DATA(&return_enum, 4, SERVER_DATA_PROCESS);
					}
					break;
				case GET_action_process_creation: // [ Action ] 프로세스 생성에 관한 액션처리
					/*
						{enum값} + {4b}{Method} + {4b}{SHA256} + {4b}{TYPE} <<-- 이것은 길이기반 파싱을 해야함(반복문)
					*/
					status;
					//PUCHAR Start_Length_Based_ADDR = (PUCHAR)Get_BUFFER + 4;
					//ULONG32 ALL_SIZE = Get_BUFFER_len - 4;

					RAW_DATA_NODE_ADDR_from_length_based = Build_RAW_DATA((PUCHAR)Get_BUFFER, Get_BUFFER_len,TRUE);

					if (processing_action_with_server_Action_Process_Node(RAW_DATA_NODE_ADDR_from_length_based)) {
						ULONG32 response = Yes; //1030
						SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
					}
					else {
						ULONG32 response = No; //1030
						SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
					}

					// 길이기반 연결리스트 삭제
					RAW_DATA_node_FreePool(RAW_DATA_NODE_ADDR_from_length_based);

					break;

				case Signature_Management:
					status;
					/*
						1. Signature_Management 를 위한 command ( Header ) 

						+ 

						2. SIG_SIGNATURE 를 위한 command (단, RAW_DATA + RAW_DATA_SIZE 적용된 데이터 ) 
					*/
					RAW_DATA_NODE_ADDR_from_length_based = Build_RAW_DATA((PUCHAR)Get_BUFFER, Get_BUFFER_len, TRUE);
					if (Signature_append_or_remove_or_get(RAW_DATA_NODE_ADDR_from_length_based)) {
						ULONG32 response = Yes; //1030
						SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
					}
					else {
						ULONG32 response = No; //1030
						SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
					}

					break;
				default:
					break;
				}

				

				ExFreePoolWithTag(Get_BUFFER, 'RcDt');// 해제
				KeReleaseMutex(TCP_session, FALSE); ////////////////////////////////////////////// *************Release********************************
				Delays(-1);//지연
				continue;
			}

		}
	}
}
