#include "SEND_or_SAVE.h"

PUCHAR RAW_DATA__external_var = NULL; // PLIST 관리자 헤더에 선언됨
ULONG32 RAW_DATA_SIZE__external_var = 0; // PLIST 관리자 헤더에 선언됨

Util_Mutex_with_Lock MUTEX_for_External_var_Read_Write = { NULL,0 }; //여기 로직 내부적으로 씀


NTSTATUS Send_or_Save(SEND_or_SAVE_enum select, ULONG32* TYPE, PUCHAR* RAW_DATA, ULONG32* RAW_DATA_SIZE, SOCKET_INFORMATION receive_types) {

	if( Initilize_or_Locking_PKmutex(&MUTEX_for_External_var_Read_Write, TRUE)==FALSE) return STATUS_UNSUCCESSFUL; // 자원 독점 ( RAW_DATA__external_var 에 대한 상호배제 적용 )

	NTSTATUS status = STATUS_SUCCESS;

	switch (select) {
	case SEND_RAW_DATA:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_RAW_DATA\n");

		/*
			아래를 호출해서
				RUST에게 성공적으로 전달했을 때만..
					=> "RAW_DATA__external_var" , "RAW_DATA_SIZE__external_var" 은 NULL초기화 된다.
		*/

		status = RAWDATA_SEND__when_request_from_server(
			&(*RAW_DATA), /*축적된 데이터 ( 중복된 노드 정보가 담김 ) */
			&(*RAW_DATA_SIZE), /*축적된 데이터의 모든 데이터의 길이*/

			receive_types // TCP Server에게 전달 시, Mutex 점유할지 말지 선택
		);

		break;



	case SAVE_RAW_DATA:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SAVE_RAW_DATA\n");
		/*
		
			연결리스트 구조체(PLIST) -> 1차원 길이-기반 변환 -> 전역변수에 복붙 하는 과정임
		
		*/


		//RAW_DATA__external_var; RAW_DATA_SIZE__external_var;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TYPE -> %lu \n", *TYPE);
		if (SAVE_DATA_with_Length_based_RAW_DATA( // 이 함수 호출 시 성공적이지 않으면 BSOD 발생
			*TYPE,
			(PLIST)*RAW_DATA/*연결리스트의 노드 마지막 addr*/,
			*RAW_DATA_SIZE/*모든 연결리스트의 노드 길이 ( 절대적임 ULONG32 + PUCHAR 가 1set ) */,

			/*
				아래 2개 변수는 모두 전역변수인데, 관리를 위해 파라미터 넘김
			*/
			&RAW_DATA__external_var,  /*전역변수 주소*/
			&RAW_DATA_SIZE__external_var//, /*전역변수 Ulong32*/


		) != STATUS_SUCCESS) {

			status = STATUS_UNSUCCESSFUL;
			break;
		}



	default:
		status = STATUS_UNSUCCESSFUL;
	}

	Release_PKmutex(&MUTEX_for_External_var_Read_Write); // 자원 독점 해제
	return status;
}