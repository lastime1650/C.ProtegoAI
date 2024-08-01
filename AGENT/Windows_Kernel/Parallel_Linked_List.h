#ifndef PARALLEL_NODE_MANAGER_H
#define PARALLEL_NODE_MANAGER_H

#include <ntifs.h>
#pragma warning(disable:4996)

#include "Mutex_with_Lock.h"

typedef struct Dynamic_NODE {

	PUCHAR Previous_Node;

	ULONG64 NODE_SECTION_INDEX;// 노드 덩어리 인식용 (1차원+2차원)
	ULONG64 NODE_RELATION_INDEX;// 노드 상대 인덱스 (오로지 1차원)

	PUCHAR DATA;
	ULONG32 DATA_SIZE;

	PUCHAR NODE_SECTION_START_NODE_ADDRESS; // 이 섹션 노드의 시작주소 ( APPEND될 때마다 같게해야함 )

	ULONG32 Node_Search_VALUE; // [추가된] 노드 식별용 (2차원용)
	BOOLEAN is_end_node; // [추가된] 노드 섹션에서 마지막 인가? (1차원)

	PUCHAR Next_Node;

}Dynamic_NODE, * PDynamic_NODE;

extern PDynamic_NODE external_start_node;
extern PDynamic_NODE external_current_node;

PDynamic_NODE Create_Node(PUCHAR SECTION_START_NODE_ADDRESS, PUCHAR Previous_Node, ULONG64 NODE_SECTION_INDEX, PUCHAR DATA, ULONG32 DATA_SIZE, ULONG32 Node_Search_VALUE);

PDynamic_NODE Append_Node(PUCHAR SECTION_START_NODE_ADDRESS, PDynamic_NODE NODE, ULONG64 NODE_SECTION_INDEX, PUCHAR DATA, ULONG32 DATA_SIZE, ULONG32 Node_Search_VALUE);

// 시작주소부터의 노드 모두 삭제 (INDEX수치 값의 기준인, 1차원방식)
BOOLEAN Remove_Node(PDynamic_NODE NODE_SECTION_Start_Address);

// 같은 SECTION_INDEX 내에서만 특정 연결리스트를 DATA로 비교하여 삭제함. 
BOOLEAN Remove_1Dim_Node_with_Search_Value(PDynamic_NODE NODE_SECTION_Start_Address, PUCHAR compare_DATA, ULONG32 compare_DATA_Size);

// 시작주소부터의 노드 모두 삭제 (Search_Value수치 값의 기준인, 2차원 방식)
BOOLEAN Remove_Node_with_Search_Value(PDynamic_NODE NODE_SECTION_Specified_Start_Address, ULONG32 Dir_Search_Value);


/*
	[1/2] [노드 가져오기] - INDEX 기반 검색임 (1차원취급)

		< 1차원 적 > ( 시작주소에 포함된 " SECTION_INDEX "값이 동일한 노드만 탐색함.
*/
// 노드 가져오기 방법들 
PDynamic_NODE Get_Node_1Dim(
	PDynamic_NODE NODE_SECTION_Start_Address,
	ULONG32 node_count_for_field
);// 노드 가져오기 ( count ) 0부터 하여 "하나씩" 가져올 수 있음. 

PDynamic_NODE Get_Node_memcmp_1Dim(
	PDynamic_NODE NODE_SECTION_Start_Address,
	PUCHAR DATA,
	ULONG32 DATA_SIZE
);// 노드 가져오기 memcmp() 사용


/*
	[2/2] [노드 가져오기] - Point! 섹션단위 검색임 (2차원취급)

		< 2차원 적 > ( 중복된 여러 섹션들 모두 탐색함 )
*/
PDynamic_NODE Get_Node_2Dim(
	ULONG32 Node_Search_VALUE,
	PUCHAR DATA,
	ULONG32 DATA_SIZE,

	PDynamic_NODE* OUTPUT_SECTION_START_ADDRESS
);

PDynamic_NODE Get_Node_2Dim_with_Relation_Index(
	ULONG32 Node_Search_VALUE,
	PUCHAR DATA,
	ULONG32 DATA_SIZE,

	PDynamic_NODE* OUTPUT_SECTION_START_ADDRESS,
	ULONG64 Relation_index
);




BOOLEAN is_valid_address(PDynamic_NODE NODE_SECTION_Start_Address); // 주소 유효성 검사 

VOID print_node();

// 노드 총 관리 함수 
BOOLEAN Build_up_Node(
	PUCHAR DATA,
	ULONG32 DATA_SIZE,

	BOOLEAN is_init,
	PDynamic_NODE* output_Start_Node_of_NODE_SECTION, // index 섹션 시작 점의 노드 주소를 반환 ( index갱신될 때 대비용 )  
	ULONG32 Node_Search_VALUE // 노드 타입을 의미함 (태그) 

);



#endif