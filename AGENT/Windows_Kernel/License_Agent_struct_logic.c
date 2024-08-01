#include "License_Agent_struct.h"
#include "TCP_send_or_Receiver.h"
#include "util_Delay.h"

#include "my_ioctl.h"
BOOLEAN Initialize_IOCTL_communicate() {


	//ULONG32 limit_delay_count = 1000;

	if (IOCTL_share_structure.is_init) { // 초기인가? 
		while (1) {

			if (IOCTL_share_structure.is_usermode_request && (IOCTL_share_structure.ioctl_BUFFER != NULL)) {
				// 여기서는 유저모드에서 요청이 왔을 때 실행되는 영역

				/*
					[NEW] Ioctl의 User_Mode 프로그램의 PID를 항상 받아야함. (전송마다)
					해당 PID는 커널모드에서 외부삭제요청으로부터 "보호"받도록 해야함. -> ObRegisterCallbacks 전역연결리스트에 추가 
				*/


				switch (IOCTL_share_structure.ioctl_BUFFER->information) {
				case REQUEST_all:
					// 라이선스 + 에이전트 ID 둘 다 있을 때,
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IOCTL 요청 -> REQUEST_all\n");

					Driver_ID.information = IOCTL_share_structure.ioctl_BUFFER->information;

					memcpy(Driver_ID.LICENSE_ID, IOCTL_share_structure.ioctl_BUFFER->license_ID, 128); // 이식

					memcpy(Driver_ID.AGENT_ID, IOCTL_share_structure.ioctl_BUFFER->Agent_ID, 128); // 이식

					return TRUE;

				case REQUESET_without_AGENT_ID:
					// 라이선스만 있을 때  (새로 AGENT_ID 발급이 필요해요!)

					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IOCTL 요청 -> REQUESET_without_AGENT_ID\n");

					Driver_ID.information = IOCTL_share_structure.ioctl_BUFFER->information;

					memcpy(Driver_ID.LICENSE_ID, IOCTL_share_structure.ioctl_BUFFER->license_ID, 128); // 이식

					return TRUE;

				default:
					return FALSE; // 이상한 값을 받았거나, 실패된 경우
				}

			}
			else {
				// 제한시간을 하고, limit 횟수만큼 제한시간을 반복하고, 이래도 else 면, 실패 리턴.
				//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IOCTL 요청 대기 중입니다.\n");
				Delays(-1); // Default 3초 대기.

				//limit_delay_count -= 1;
				//if (limit_delay_count == 0) return FALSE;

				continue;
			}


		}
	}
	else {
		// 초기 이후의 요청임 
		return FALSE; 
	}



	return FALSE;
}

//


NTSTATUS GET_AGENT_ID() {

	/*
		최초 TCP 서버와 연결 후, Agent ID 를 확인한다. ( SMBIOS 값은 Query_SMBIOS_information() 함수에서 이미 받아왔다 ) 
		[ (SHA512) + (FULL-PATH)   ]
	*/

	PVOID Generated_AGENT_ID_by_RUST = NULL;
	ULONG32 Generated_AGENT_ID_by_RUST_len = 0;
	RECEIVE_TCP_DATA__with_alloc(&Generated_AGENT_ID_by_RUST, &Generated_AGENT_ID_by_RUST_len, TCP_DATA_RECEIVE);

	if (Generated_AGENT_ID_by_RUST_len < 128 || Generated_AGENT_ID_by_RUST == NULL) { // SHA512 길이가 아니면 실패!
		return STATUS_UNSUCCESSFUL;
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[AGENT_ID()] Generated_AGENT_ID_by_RUST_len 결과 -> %lu", Generated_AGENT_ID_by_RUST_len);




	// {AGENT_ID} + {LICENSE_ID} 통합 저장 정적 변수( Stack ) 
	UCHAR SEND_RESULT[( sizeof(Driver_ID)-4 )] = {0,}; // 4를 빼는 이유는 ENUM 구조체 제외(총512바이트)를 위하여


	if (IOCTL_share_structure.is_init) {

		switch (IOCTL_share_structure.ioctl_BUFFER->information) {
		case REQUEST_all:
			// 라이선스 + 에이전트 ID 둘 다 있을 때, ( 기존 Initialize_ID 함수를 통해서 등록된 전역변수 Driver_ID의 ID 2개를 RUST에게 전달. )

			Driver_ID.information = IOCTL_share_structure.ioctl_BUFFER->information;

			memcpy(Driver_ID.LICENSE_ID, IOCTL_share_structure.ioctl_BUFFER->license_ID, 128); // 이식

			memcpy(Driver_ID.AGENT_ID, IOCTL_share_structure.ioctl_BUFFER->Agent_ID, 128); // 이식

			break;

		case REQUESET_without_AGENT_ID:
			// 라이선스만 있을 때  (새로 AGENT_ID 발급이 필요해요!)

			Driver_ID.information = IOCTL_share_structure.ioctl_BUFFER->information;

			memcpy(Driver_ID.LICENSE_ID, IOCTL_share_structure.ioctl_BUFFER->license_ID, 128); // 이식

			memcpy(Driver_ID.AGENT_ID, Generated_AGENT_ID_by_RUST, 128); // **이식 ( RUST에서 기본 Generate된 것을 반영한다. )

			break;

		default:
			return STATUS_UNSUCCESSFUL; // 이상한 값을 받았거나, 실패된 경우 ( 이때는 드라이버를 종료하도록 하거나 해야할 듯
		}

	}
	else {
		// 이 경우는, 이미 초기화는 이전에 했엇고, 중간에 연결이 끊겼다가 다시 연결하는 경우임. 

	}



	/*소켓 전달 용 copy */
	memcpy(SEND_RESULT, Driver_ID.AGENT_ID, sizeof(Driver_ID.AGENT_ID));
	memcpy(SEND_RESULT + sizeof(Driver_ID.AGENT_ID), Driver_ID.LICENSE_ID, sizeof(Driver_ID.LICENSE_ID));
	memcpy(SEND_RESULT + sizeof(Driver_ID.AGENT_ID) + sizeof(Driver_ID.LICENSE_ID) , Driver_ID.SMBIOS_TYPE_1, sizeof(Driver_ID.SMBIOS_TYPE_1));
	memcpy(SEND_RESULT + sizeof(Driver_ID.AGENT_ID) + sizeof(Driver_ID.LICENSE_ID) + sizeof(Driver_ID.SMBIOS_TYPE_1), Driver_ID.SMBIOS_TYPE_2, sizeof(Driver_ID.SMBIOS_TYPE_2));



	/* 서버에게 데이터 송수신 준비되었다고 전달한다. */
	if (SEND_TCP_DATA(SEND_RESULT, sizeof(SEND_RESULT), TCP_DATA_SEND) != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[AGENT - FAIL] SEND_TCP_DATA 실패!");
		return STATUS_UNSUCCESSFUL;
	}


	/* 서버의 결과를 받아야한다.-> IOCTL이 대기중이면 결과를 알려줘야함 */
	
	PVOID result_from_server_addr = NULL;
	ULONG32 result_from_server_addr_size = 0;
	if (RECEIVE_TCP_DATA__with_alloc((PVOID*) &result_from_server_addr, (ULONG32*)&result_from_server_addr_size, TCP_DATA_RECEIVE) == STATUS_UNSUCCESSFUL ) {
		// ioctl에게 최종결과 전달
		if (IOCTL_share_structure.is_init) {
			IOCTL_share_structure.ioctl_BUFFER->information = FAIL_id;

			KeSetEvent(&IOCTL_share_structure.ioctl_event, 0, FALSE); //ioctl의 실행을 재개
		}


		ExFreePoolWithTag(Generated_AGENT_ID_by_RUST, 'RcDt');
		return STATUS_UNSUCCESSFUL;
	}


	// ioctl에게 최종결과 전달하기
	COMMUNICATION_IOCTL_ENUM result =  (COMMUNICATION_IOCTL_ENUM)(*(ULONG32*)result_from_server_addr);
	if (result == SUCCESS_id) {
		if (IOCTL_share_structure.is_init) {

			// Driver_ID에서 AGENT_ID가 최초 등록성공된 경우 Driver_ID 전역변수에 memcpy해야한다.(시스템에 저장해서 사용)
			if (IOCTL_share_structure.ioctl_BUFFER->information == REQUESET_without_AGENT_ID) {
				memcpy(Driver_ID.AGENT_ID, Generated_AGENT_ID_by_RUST, 128);
			}

			// 최종결과 전달.
			IOCTL_share_structure.ioctl_BUFFER->information = SUCCESS_id;

			KeSetEvent(&IOCTL_share_structure.ioctl_event, 0, FALSE); //ioctl의 실행을 재개
		}
	}
	else {
		// 서버에서 실패되었을 경우;; 
		if (IOCTL_share_structure.is_init) {
		

			// 최종결과 전달.
			IOCTL_share_structure.ioctl_BUFFER->information = FAIL_id;

			KeSetEvent(&IOCTL_share_structure.ioctl_event, 0, FALSE); //ioctl의 실행을 재개
		}
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[AGENT - FAILED]");
		ExFreePoolWithTag(Generated_AGENT_ID_by_RUST, 'RcDt');
		return STATUS_UNSUCCESSFUL;
	}
	


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n[AGENT_ID결과 SUCCESSED]-> AGENT_ID: %128s \n LICENSE_ID: %128s\n", Driver_ID.AGENT_ID, Driver_ID.LICENSE_ID);


	ExFreePoolWithTag(Generated_AGENT_ID_by_RUST, 'RcDt');

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[AGENT - SUCCESS]");
	return STATUS_SUCCESS;

}