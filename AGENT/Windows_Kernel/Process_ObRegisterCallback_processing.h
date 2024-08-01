#ifndef Process_ObRegisterCallback_processing_H
#define Process_ObRegisterCallback_processing_H

#include <ntifs.h>

#include "Mutex_with_Lock.h"
#include "TCP_conn.h"
#include "Length_Based_Data_Parser.h"
#include "converter_PID.h"

#include "Parallel_Linked_List.h" // 비동기 연결리스트


//프로세스 콜백함수 Action 처리 함수들
/*
typedef struct Action_for_process_creation_NODE {

	PUCHAR Previous_Address;

	BOOLEAN is_Block; // Block할 것인가? 
	UCHAR SHA256[SHA256_String_Byte_Length]; // 64+1(\x00) 바이트 (동적메모리)
	Type_Feature feature; // Create ? Remove ? ... 

	PUCHAR Next_Address;

}Action_for_process_creation_NODE, * PAction_for_process_creation_NODE;
*/

typedef struct ActionProcessNode {
	BOOLEAN is_Block; // Block할 것인가? 
	UCHAR SHA256[SHA256_String_Byte_Length]; // 65-1(\x00) 바이트 (동적메모리)
	Type_Feature feature;
}ActionProcessNode, *PActionProcessNode;


BOOLEAN Create_or_Append_Action_Process_Node(PActionProcessNode input);

/*

is_Block(1b) = Relation_index == 0
SHA256(65b) = Relation_index == 1
feature(4b) = Relation_index == 2

*/
extern PDynamic_NODE Dynamic_Node_Action_Process_Node;

// SHA256이 Action연결리스트에 존재하는가? 
BOOLEAN is_exist_program_Action_Process_Node(PActionProcessNode input);

BOOLEAN compare_Action_Node_with_PID(HANDLE PID, MODE Kernel_or_User);

// 연결리스트 노드 하나 삭제
BOOLEAN Remove_one_node_Action_Process_Node(PActionProcessNode input);

#endif