#ifndef PLIST_Node_Manager_H
#define PLIST_Node_Manager_H
/*
	RUST에게 전달하기 위한 데이터(연결리스트-전역변수)를 관리하는 헤더
*/

#include <ntifs.h>
#include <ntddk.h>

#include "License_Agent_struct.h" // 에이전트 ID 가져오려고


typedef struct LIST {

	ULONG32 RAW_DATA_len;
	PUCHAR RAW_DATA; // 여기서 새로 동적할당하지 않고, 바로 전역변수에 축적된다. (결론= 전역변수에 저장 시, 비동기 처리 절대 불가능 ) 

	PUCHAR previous_addr;
	PUCHAR next_addr;
}LIST, * PLIST;

// 노드생성
PLIST CREATE_tag_LIST(
	ULONG32 RAW_DATA_len,
	PUCHAR RAW_DATA,
	PUCHAR Previous_addr,
	ULONG32* inout_SIZE
);

// 노드 추가
PLIST APPENDLIST(
	ULONG32 RAW_DATA_len,
	PUCHAR RAW_DATA,
	PLIST Node,
	ULONG32* inout_SIZE
);


// 모든 노드 삭제
VOID REMOVE__ALL_LIST(PLIST Lastest_node);



/////[ 연결리스트 모든 노드를 한 전역변수에.. ]//////
extern PUCHAR RAW_DATA__external_var;
extern ULONG32 RAW_DATA_SIZE__external_var;



// "한 노드"를 연결리스트에 APPEND하고 한 전역변수에 저장하는 함수( 안에서 Link_node__2__Mem_Alloc() 를 호출함 )
NTSTATUS SAVE_DATA_with_Length_based_RAW_DATA(
	ULONG32 TYPE,

	PLIST input_RAW_DATA__LIST_tail_node, // 원본 노드 마지막 주소를 받는다
	ULONG32 input_RAW_DATA__LIST_tail_node_SIZE, // 원본 노드 사이즈 

	 PUCHAR* inoutput_OLD_RAW_DATA_alloc_addr, /*공용자원 읽기쓰기 -> 최초의 경우 NULL임*/
	 ULONG32* inoutput_OLD_RAW_DATA_alloc_addr_SIZE /*공용자원 읽기쓰기*/
);



// PLIST 한 노드를 "길이-기반"으로 구축함
NTSTATUS Link_node__2__Mem_Alloc(
	ULONG32 input_TYPE,

	PLIST input_LIST_Tail_node_addr,
	ULONG32 input_LIST_Tail_node_addr_SIZE,

	PUCHAR* output_result_allocated_addr,
	ULONG32* output_result_allocated_addr__SIZE

);


// 동적변수에 "길이-기반" 데이터를 갖다 COPY해버리도록 함 ( 아래 함수를 호출한 후, RUST에게 보낼 수 있는 완성형체가 됨 )
NTSTATUS APPEND_Length_Based_DATA(
	PUCHAR* OLD_input_data, ULONG32* OLD_input_data_size,

	PUCHAR* CURRENT_input_data, ULONG32* CURRENT_input_data_size,

	PUCHAR* output_data, ULONG32* output_data_size);

/////[ 송신처리 ]//////

#include "TCP_send_or_Receiver.h"

// 연결리스트를 한 전역변수로 만든 변수를 RUST서버에 전달하는 함수 
NTSTATUS RAWDATA_SEND__when_request_from_server(
	PUCHAR* RAW_DATA,
	ULONG32* RAW_DATA_SIZE,
	SOCKET_INFORMATION receive_types
);


#endif