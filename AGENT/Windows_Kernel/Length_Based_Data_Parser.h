#ifndef Length_Based_Data_Parser_H
#define Length_Based_Data_Parser_H

#include <ntifs.h>
#include <ntddk.h>

/*
	RUST 서버에서 보낸, 길이기반 데이터를 해석하는 용도.
*/

// 연결리스트 노드
typedef struct Length_Based_DATA_Node {

	PUCHAR Previous_Address;

	PUCHAR RAW_DATA;
	ULONG32 RAW_DATA_Size;

	PUCHAR Next_Address;

}Length_Based_DATA_Node, * PLength_Based_DATA_Node;

//노드 구축
PLength_Based_DATA_Node Create_Length_Based_DATA_Node(
	PLength_Based_DATA_Node Previous_Address,

	PUCHAR RAW_DATA,
	ULONG32 RAW_DATA_Size
);

//노드 추가
PLength_Based_DATA_Node APPEND_Length_Based_DATA_Node(
	PLength_Based_DATA_Node Current_Node,

	PUCHAR RAW_DATA,
	ULONG32 RAW_DATA_Size
);

/* 반복적으로 읽어, 길이-기반 데이터를 해석 */
BOOLEAN Get_RAW_DATA_one_time_from_Length_Based_DATA(

	PUCHAR Input_DATA,

	PUCHAR* Output_DATA,
	ULONG32* Output_DATA_Size

);

/* 길이기반을 해석하고, 연결리스트를 구축하는 함수 */
PLength_Based_DATA_Node Build_RAW_DATA(
	PUCHAR Input_Start_RAW_DATA,
	ULONG32 Input_RAW_DATA_ALL_SIZE
);

/* 연결리스트 전체 할당해제 */
VOID RAW_DATA_node_FreePool(
	PLength_Based_DATA_Node Start_Node
);




#endif