#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "PLIST_Node_Manager.h"
/*
	여기는 " 한 PLIST "를 가지고 "길이-기반" 형태로 변환하는 작업을 한다. 
*/



// PLIST 한 노드를 "길이-기반"으로 구축함
NTSTATUS Link_node__2__Mem_Alloc(
	ULONG32 input_TYPE,

	PLIST input_LIST_Tail_node_addr,
	ULONG32 input_LIST_Tail_node_addr_SIZE,

	PUCHAR* output_result_allocated_addr,
	ULONG32* output_result_allocated_addr__SIZE

){
	ULONG32 Length_Based_RAW_DATA_len = 0;
	ULONG32 i = 0;

	do {
		/*
			데이터의 길이를 측정하여 동적 할당하기 위한 총량을 구한다
		*/
		i++; // 반복카운트

		Length_Based_RAW_DATA_len += input_LIST_Tail_node_addr->RAW_DATA_len;//현재 주소에서 RAW_DATA 길이 중복저장 [1/4]
		input_LIST_Tail_node_addr = (PLIST)input_LIST_Tail_node_addr->previous_addr; // 주소 이전으로 갱신하여 이동


		if (input_LIST_Tail_node_addr->previous_addr == NULL) {
			i++; //마지막 추가 카운트 
			/* 이 영역에 도달하면, 맨 처음 노드까지 온 것이다. */
			Length_Based_RAW_DATA_len += input_LIST_Tail_node_addr->RAW_DATA_len;
			break;
		}


	} while (input_LIST_Tail_node_addr->previous_addr != NULL);
	PLIST RAW_DATA__LIST__BACKUP = input_LIST_Tail_node_addr; // 2차 인덱싱 용 주소 -> 이때는 노드의 ( 맨 처음 )시작주소로 백업한다.






	Length_Based_RAW_DATA_len += sizeof(input_TYPE); // TYPE 4바이트 중복저장 [2/4]
	Length_Based_RAW_DATA_len += (sizeof(ULONG32) * i); // RAW_DATA를 가르키는 길이 (4바이트) 각각 중복저장 [3/4]
	/*
		[_END]
		Ascii -> 5F 45 4E 44
	*/
	Length_Based_RAW_DATA_len += 4;// END 4바이트 삽입 [4/4]

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[길이기반] 동적할당할 총 길이 -> %llu \n", Length_Based_RAW_DATA_len);
	/*

		동적할당 시도

	*/
	PUCHAR all_of_data = (PUCHAR)ExAllocatePoolWithTag(PagedPool, Length_Based_RAW_DATA_len, 'ALL'); // tag-> Length Based Raw Data
	if (all_of_data == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}


	PUCHAR Header_addr = all_of_data;//시작 주소 백업
	PUCHAR Tail_addr = (PUCHAR)all_of_data + ((SIZE_T)Length_Based_RAW_DATA_len - 4); // 끝 주소 구하기 ( 단! _END 의 _ 주소는 제외. 

	/* TYPE을 넣자 [1/2]*/
	memcpy(all_of_data, &input_TYPE, sizeof(input_TYPE)); // 4바이트 주소 복사 ++ 
	all_of_data = all_of_data + 4; // 주소 이동

	/* RAW_DATA__LIST__BACKUP == RAW_DATA__LIST */

	do {
		/*
			끝에 도달하면 탈출 [2/2]
		*/
		if (all_of_data == Tail_addr) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "끝 주소에 도달하였으므로 탈출합니다. \n");
			break;
		}

		memcpy(all_of_data, &RAW_DATA__LIST__BACKUP->RAW_DATA_len, sizeof(ULONG32)); // 4바이트 주소 복사 ++
		all_of_data = all_of_data + 4; // 주소 4 증가

		memcpy(all_of_data, RAW_DATA__LIST__BACKUP->RAW_DATA, RAW_DATA__LIST__BACKUP->RAW_DATA_len);  // 가변길이 RAW_DATA 복사 ++
		all_of_data = all_of_data + RAW_DATA__LIST__BACKUP->RAW_DATA_len;//주소 n 증가



		RAW_DATA__LIST__BACKUP = (PLIST)RAW_DATA__LIST__BACKUP->next_addr; // 주소 이전으로 갱신하여 이동

		if (RAW_DATA__LIST__BACKUP->next_addr == NULL) {
			/* 이 영역에 도달하면, 맨 마지막 노드까지 온 것이다. */
			memcpy(all_of_data, &RAW_DATA__LIST__BACKUP->RAW_DATA_len, sizeof(ULONG32)); // 4바이트 주소 복사 ++
			all_of_data = all_of_data + 4; // 주소 4 증가

			memcpy(all_of_data, RAW_DATA__LIST__BACKUP->RAW_DATA, RAW_DATA__LIST__BACKUP->RAW_DATA_len);  // 가변길이 RAW_DATA 복사 ++
			all_of_data = all_of_data + RAW_DATA__LIST__BACKUP->RAW_DATA_len;//주소 n 증가

			break;
		}


	} while (RAW_DATA__LIST__BACKUP->next_addr != NULL);

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "_END  MEMCPY 하기! -> Tail_add	r 의 주소 -> %p \n", Tail_addr);
	/*[+] _END */memcpy(Tail_addr, "_END", (sizeof("_END") - 1)); // 마지막 \x00은 넣지 않고 4바이트로 명시해서 넣는다.

	*output_result_allocated_addr = Header_addr; // output 전달
	*output_result_allocated_addr__SIZE = Length_Based_RAW_DATA_len; // output 전달

	/*
		이제 반환되면, 해당 리스트는 모두 할당해제되어야만한다.
	*/

	return STATUS_SUCCESS;
}