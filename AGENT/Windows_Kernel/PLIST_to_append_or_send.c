#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "PLIST_Node_Manager.h"
/*
	이 로직은 
	
		1."길이-기반"데이터를 RUST에 보낼 전역변수에 APPEND,

		2.소켓으로 RUST에게 전달하는 것이 구현

	되어있다.
*/



//
NTSTATUS APPEND_Length_Based_DATA(
	PUCHAR* OLD_input_data, ULONG32* OLD_input_data_size,

	PUCHAR* CURRENT_input_data, ULONG32* CURRENT_input_data_size,

	PUCHAR* output_data, ULONG32* output_data_size) {
	/*
		[주의사항]
			OLD - input_data를 src DATA로 memcpy 복사할 때, 기존 SHA512(128) 값을 제외하고 넣어야함;;
	*/

	if (OLD_input_data == NULL && OLD_input_data_size == NULL) { // 전역공용변수에 최초로 값을 넣을 때 
		PUCHAR NEW_addr = NULL;
		ULONG32 NEW_addr_size = *CURRENT_input_data_size + 128;

		NEW_addr = ExAllocatePoolWithTag(PagedPool, NEW_addr_size, 'ALL');
		if (NEW_addr == NULL) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		memcpy((PVOID)NEW_addr, (PVOID)Driver_ID.AGENT_ID, 128);

		memcpy((PUCHAR)NEW_addr + 128, *CURRENT_input_data, *CURRENT_input_data_size);

		ExFreePoolWithTag(*CURRENT_input_data, 'ALL'); // Link_node__2__Mem_Alloc 할당 해제 
		*CURRENT_input_data_size = 0;

		*output_data = NEW_addr; // 전역공용변수 할당 주소 이동. 
		*output_data_size = NEW_addr_size;

		return STATUS_SUCCESS;
	}
	else { // 전역공용변수는 이전에도 할당되어 있었으며, 추가로 값을 넣을 때 
		PUCHAR NEW_addr = NULL;
		ULONG32 NEW_addr_size = *OLD_input_data_size + *CURRENT_input_data_size;// +128;

		NEW_addr = ExAllocatePoolWithTag(PagedPool, NEW_addr_size, 'ALL');
		if (NEW_addr == NULL) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		//memcpy((PVOID)NEW_addr, (PVOID)AGENT_ID, 128);

		memcpy((PUCHAR)NEW_addr, *OLD_input_data, *OLD_input_data_size); // SHA512 + 이전정보가 이미 OLD에 있음

		memcpy((PUCHAR)((PUCHAR)NEW_addr + *OLD_input_data_size), *CURRENT_input_data, *CURRENT_input_data_size);

		ExFreePoolWithTag(*CURRENT_input_data, 'ALL'); // Link_node__2__Mem_Alloc 할당 해제 
		*CURRENT_input_data_size = 0;
		ExFreePoolWithTag(*OLD_input_data, 'ALL'); // 전역공용변수 할당해제
		*CURRENT_input_data_size = 0;

		*output_data = NEW_addr; // 전역공용변수 할당 주소 이동. 
		*output_data_size = NEW_addr_size;

		return STATUS_SUCCESS;
	}



}



//
NTSTATUS SAVE_DATA_with_Length_based_RAW_DATA(
	ULONG32 TYPE,

	PLIST input_RAW_DATA__LIST_tail_node, // 원본 노드 마지막 주소를 받는다
	ULONG32 input_RAW_DATA__LIST_tail_node_SIZE, // 원본 노드 사이즈 

	PUCHAR* inoutput_OLD_RAW_DATA_alloc_addr, /*공용자원 읽기쓰기 -> 최초의 경우 NULL임*/
	ULONG32* inoutput_OLD_RAW_DATA_alloc_addr_SIZE /*공용자원 읽기쓰기*/
) {

	PUCHAR Parsed__all_of_data = NULL; // <===== 연결 리스트 노드들을 모두 다 가져와 하나의 데이터에 저장. ( 길이-기반 구조 형식으로.,, ) 
	ULONG32 Length_Based_RAW_DATA_len = 0;

	PLIST Node_Lastest_addr_BACKUP = input_RAW_DATA__LIST_tail_node;
	// [1/2] 파싱 후 동적메모리 할당
	if (Link_node__2__Mem_Alloc(TYPE, input_RAW_DATA__LIST_tail_node, input_RAW_DATA__LIST_tail_node_SIZE, &Parsed__all_of_data, &Length_Based_RAW_DATA_len) != STATUS_SUCCESS) {
		return STATUS_UNSUCCESSFUL;
	}


	/*
		Old가 null이 아니라면, 재 할당하여 모든 것을 합하여  재 동적할당 한다. ( 축적 )
		아래 if else 문에서 else가 됨
	*/
	if (*inoutput_OLD_RAW_DATA_alloc_addr != NULL && *inoutput_OLD_RAW_DATA_alloc_addr_SIZE > 0) { // [2/2] 


		APPEND_Length_Based_DATA(inoutput_OLD_RAW_DATA_alloc_addr, inoutput_OLD_RAW_DATA_alloc_addr_SIZE, &Parsed__all_of_data, &Length_Based_RAW_DATA_len, inoutput_OLD_RAW_DATA_alloc_addr, inoutput_OLD_RAW_DATA_alloc_addr_SIZE);


	}
	else {
		/*
			NEW 최초로 실행되는 경우의 영역
		*/

		APPEND_Length_Based_DATA(NULL, NULL, &Parsed__all_of_data, &Length_Based_RAW_DATA_len, inoutput_OLD_RAW_DATA_alloc_addr, inoutput_OLD_RAW_DATA_alloc_addr_SIZE);
	}

	REMOVE__ALL_LIST(Node_Lastest_addr_BACKUP/* 마지막 노드 주소 */); // 연결리스트 노드 싹다 해제

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[길이기반] output_NEW_RAW_DATA_alloc_addr_SIZE 총 길이 -> %llu \n", *inoutput_OLD_RAW_DATA_alloc_addr_SIZE);

	return STATUS_SUCCESS;
}






/////[send 부분]////
/*
	서버로부터 모든 데이터를 전달하라는 명령을 받으면 호출되는 함수
	데이터를 서버에게 전달한다
*/
NTSTATUS RAWDATA_SEND__when_request_from_server(_Inout_ PUCHAR* RAW_DATA, _Inout_ ULONG32* RAW_DATA_SIZE, SOCKET_INFORMATION receive_types) {

	if (*RAW_DATA == NULL || *RAW_DATA_SIZE == 0) {

		return STATUS_UNSUCCESSFUL;
	}





	PUCHAR remember_RAW_DATA_addr = *RAW_DATA;
	ULONG32 remember_RAW_DATA_size = *RAW_DATA_SIZE;

	NTSTATUS status = STATUS_SUCCESS;

	/* 서버에 전송 */
	if (SEND_TCP_DATA(*RAW_DATA, *RAW_DATA_SIZE, receive_types) != STATUS_SUCCESS) {
		/* 실패적으로 전송했을 때,,,,*/
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> Length-based 데이터 전송 최종 실패!.. \n");

		/* 주소,데이터 값 무결성 검사 */
		if (remember_RAW_DATA_addr != *RAW_DATA && remember_RAW_DATA_size != *RAW_DATA_SIZE) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> 무결성 무너짐 (1) \n");
		}

		status = STATUS_UNSUCCESSFUL;
	}
	else {
		/* 성공적으로 전송했을 때,,,,*/
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> Length-based 데이터 전송 최종 성공!!!!.. \n");

		/* 주소,데이터 값 무결성 검사 */
		if (remember_RAW_DATA_addr != *RAW_DATA && remember_RAW_DATA_size != *RAW_DATA_SIZE) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> 무결성 무너짐 (2) \n");
			status = STATUS_UNSUCCESSFUL;
		}
		else {
			/* 리스트 전부 할당해제 */
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RAWDATA_SEND__when_request_from_server -> 호출 전 FREE_pool____Dynamic_Allocation_mem ==>> RAW_DATA_addr: %p , RAW_DATA_size: %p , %d\n", RAW_DATA__external_var, &RAW_DATA_SIZE__external_var, RAW_DATA_SIZE__external_var);
			ExFreePoolWithTag((PVOID)*RAW_DATA, (ULONG)'ALL'); //성공적으로 전송하였으니, 아예 싹다 초기화
			*RAW_DATA = NULL;
			*RAW_DATA_SIZE = 0;
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> 할당해제\n");
			status = STATUS_SUCCESS;
		}


	}

	return status;
}