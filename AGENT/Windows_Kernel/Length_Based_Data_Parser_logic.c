#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "Length_Based_Data_Parser.h"



PLength_Based_DATA_Node Create_Length_Based_DATA_Node(
	PLength_Based_DATA_Node Previous_Address,

	PUCHAR RAW_DATA,
	ULONG32 RAW_DATA_Size
) {
	PLength_Based_DATA_Node new_node = ExAllocatePoolWithTag(NonPagedPool, sizeof(Length_Based_DATA_Node), 'NoDe');
	if (Previous_Address == NULL) {
		new_node->Previous_Address = NULL;
	}
	else {
		new_node->Previous_Address = (PUCHAR)Previous_Address;
	}

	new_node->RAW_DATA = RAW_DATA;
	new_node->RAW_DATA_Size = RAW_DATA_Size;

	new_node->Next_Address = NULL;

	return new_node;
}

PLength_Based_DATA_Node APPEND_Length_Based_DATA_Node(
	PLength_Based_DATA_Node Current_Node,

	PUCHAR RAW_DATA,
	ULONG32 RAW_DATA_Size
) {

	PLength_Based_DATA_Node new_node = Create_Length_Based_DATA_Node(
		Current_Node,
		RAW_DATA,
		RAW_DATA_Size
	);

	Current_Node->Next_Address = (PUCHAR)new_node;

	return new_node;

}


/* 길이기반 등 처리가 필요한 함수 */
BOOLEAN Get_RAW_DATA_one_time_from_Length_Based_DATA(

	PUCHAR Input_DATA,

	PUCHAR* Output_DATA,
	ULONG32* Output_DATA_Size

) {
	// 4바이트 끊어서 읽어, RAW_DATA의 길이를 추출
	ULONG32 RAW_DATA_Size = 0;
	memcpy(&RAW_DATA_Size, Input_DATA, 4);


	// "_END"인가? -> NULL반환
	if (memcmp(&RAW_DATA_Size, "_END", 4) == 0) return FALSE;

	// RAW_DATA추출
	*Output_DATA_Size = RAW_DATA_Size;
	*Output_DATA = Input_DATA + 4;

	return TRUE;

}

PLength_Based_DATA_Node Build_RAW_DATA(
	PUCHAR Input_Start_RAW_DATA,
	ULONG32 Input_RAW_DATA_ALL_SIZE
) {

	PLength_Based_DATA_Node Output_Address = NULL;
	PLength_Based_DATA_Node tmp_node = NULL;


	PUCHAR finish_RAW_DATA_address = Input_Start_RAW_DATA + Input_RAW_DATA_ALL_SIZE;

	PUCHAR current_RAW_DATA = Input_Start_RAW_DATA;

	int index = 0;

	while (current_RAW_DATA <= finish_RAW_DATA_address) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " index-> %d \n", index);

		ULONG32 current_RAW_DATA_SIZE = 0;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [전] current_RAW_DATA %p / 길이: %lu \n", current_RAW_DATA, current_RAW_DATA_SIZE);
		// 길이 와 데이터 추출
		if (Get_RAW_DATA_one_time_from_Length_Based_DATA(
			current_RAW_DATA,
			&current_RAW_DATA,
			&current_RAW_DATA_SIZE
		) == FALSE) break;

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [후] current_RAW_DATA %p / 길이: %lu \n", current_RAW_DATA, current_RAW_DATA_SIZE);
		// 노드에 저장
		// 노드의 RAW_DATA는 절대 동적할당하지 않고, 포인터만 가르킨다. ( 이미 데이터는 Receive 때 동적할당되어 있으니, ) 
		if (Output_Address == NULL) {
			Output_Address = Create_Length_Based_DATA_Node(
				NULL,
				current_RAW_DATA,
				current_RAW_DATA_SIZE
			);
			tmp_node = Output_Address;
		}
		else {
			tmp_node = APPEND_Length_Based_DATA_Node(
				tmp_node,
				current_RAW_DATA,
				current_RAW_DATA_SIZE
			);
		}


		current_RAW_DATA = current_RAW_DATA + current_RAW_DATA_SIZE; // 다음 RAW_DATA의 4바이트 길이 쪽으로 이동
		index += 1;
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " index-> %d \n", index);
	return Output_Address;
}

VOID RAW_DATA_node_FreePool(
	PLength_Based_DATA_Node Start_Node
) {
	PLength_Based_DATA_Node current_node = Start_Node;
	do {

		PLength_Based_DATA_Node tmp_current_node = (PLength_Based_DATA_Node)current_node->Next_Address;

		ExFreePoolWithTag(current_node, 'NoDe');

		current_node = tmp_current_node;
	} while (current_node != NULL);
}