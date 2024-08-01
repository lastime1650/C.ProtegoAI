#ifndef Process_Action_processing_H
#define Process_Action_processing_H

#include <ntifs.h>
#include <ntddk.h>

#include "Mutex_with_Lock.h"
#include "converter_PID.h" // PID를 통해 SHA256을 최종적으로 구하기 위하여
#include "Length_Based_Data_Parser.h" // RUST의 길이기반을 해석해야하므로 
#include "TCP_conn.h"

/*
	프로세스 콜백함수 Action 처리 함수들
*/
typedef struct Action_for_process_creation_NODE {

	PUCHAR Previous_Address;

	BOOLEAN is_Block; // Block할 것인가? 
	UCHAR SHA256[SHA256_String_Byte_Length]; // 64+1(\x00) 바이트 (동적메모리)
	Type_Feature feature; // Create ? Remove ? ... 

	PUCHAR Next_Address;

}Action_for_process_creation_NODE, * PAction_for_process_creation_NODE;

//액션 노드 생성
PAction_for_process_creation_NODE Create_process_routine_NODE(
	PAction_for_process_creation_NODE Previous_Node,
	BOOLEAN is_Block,
	PUCHAR SHA256,
	Type_Feature feature
);

//액션 노드 추가
PAction_for_process_creation_NODE APPEND_process_routine_NODE(
	PAction_for_process_creation_NODE current_node,
	BOOLEAN is_Block,
	PUCHAR SHA256,
	Type_Feature feature
);

//액션 노드 하나 추출 ( SHA256 기준 )
PAction_for_process_creation_NODE Search_process_routine_NODE(
	PAction_for_process_creation_NODE Start_Node,
	PUCHAR target_SHA256
);

//액션 노드 하나 추출(확장된) ( SHA256 기준 + Extended )
PAction_for_process_creation_NODE Ex_Search_process_routine_NODE(
	PAction_for_process_creation_NODE Start_Node,
	BOOLEAN is_Block,
	PUCHAR target_SHA256,
	Type_Feature feature
);

// 노드 삭제 해주는 함수
VOID DELETE_Action_One_Node(PAction_for_process_creation_NODE One_Node);

//액션 연결리스트 프린트용
VOID Print_process_routine_NODE(
	PAction_for_process_creation_NODE Start_Node
);



PAction_for_process_creation_NODE Get_Action_Node_With_SHA256(HANDLE Input_PID);

//ObRegisterCallback 에서 사용될 전역변수
extern PAction_for_process_creation_NODE Action_for_process_routine_node_Start_Address; // 노드 시작 주소
extern PAction_for_process_creation_NODE Action_for_process_routine_node_Current_Address; // 이건 함수에서 사용하는 처리용임

extern Util_Mutex_with_Lock Action_for_proces_routine_node_KMUTEX; // 상호배제

BOOLEAN Action_for_process_creation(PUCHAR GET_BUFFER, ULONG32 GET_BUFFER_len);
//////////////////////////////////////////////////////////////////////////////// 프로세스 생성 방지 조치 처리

#endif