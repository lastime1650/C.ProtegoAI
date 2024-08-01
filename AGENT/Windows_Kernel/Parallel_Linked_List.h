#ifndef PARALLEL_NODE_MANAGER_H
#define PARALLEL_NODE_MANAGER_H

#include <ntifs.h>
#pragma warning(disable:4996)

#include "Mutex_with_Lock.h"

typedef struct Dynamic_NODE {

	PUCHAR Previous_Node;

	ULONG64 NODE_SECTION_INDEX;// ��� ��� �νĿ� (1����+2����)
	ULONG64 NODE_RELATION_INDEX;// ��� ��� �ε��� (������ 1����)

	PUCHAR DATA;
	ULONG32 DATA_SIZE;

	PUCHAR NODE_SECTION_START_NODE_ADDRESS; // �� ���� ����� �����ּ� ( APPEND�� ������ �����ؾ��� )

	ULONG32 Node_Search_VALUE; // [�߰���] ��� �ĺ��� (2������)
	BOOLEAN is_end_node; // [�߰���] ��� ���ǿ��� ������ �ΰ�? (1����)

	PUCHAR Next_Node;

}Dynamic_NODE, * PDynamic_NODE;

extern PDynamic_NODE external_start_node;
extern PDynamic_NODE external_current_node;

PDynamic_NODE Create_Node(PUCHAR SECTION_START_NODE_ADDRESS, PUCHAR Previous_Node, ULONG64 NODE_SECTION_INDEX, PUCHAR DATA, ULONG32 DATA_SIZE, ULONG32 Node_Search_VALUE);

PDynamic_NODE Append_Node(PUCHAR SECTION_START_NODE_ADDRESS, PDynamic_NODE NODE, ULONG64 NODE_SECTION_INDEX, PUCHAR DATA, ULONG32 DATA_SIZE, ULONG32 Node_Search_VALUE);

// �����ּҺ����� ��� ��� ���� (INDEX��ġ ���� ������, 1�������)
BOOLEAN Remove_Node(PDynamic_NODE NODE_SECTION_Start_Address);

// ���� SECTION_INDEX �������� Ư�� ���Ḯ��Ʈ�� DATA�� ���Ͽ� ������. 
BOOLEAN Remove_1Dim_Node_with_Search_Value(PDynamic_NODE NODE_SECTION_Start_Address, PUCHAR compare_DATA, ULONG32 compare_DATA_Size);

// �����ּҺ����� ��� ��� ���� (Search_Value��ġ ���� ������, 2���� ���)
BOOLEAN Remove_Node_with_Search_Value(PDynamic_NODE NODE_SECTION_Specified_Start_Address, ULONG32 Dir_Search_Value);


/*
	[1/2] [��� ��������] - INDEX ��� �˻��� (1�������)

		< 1���� �� > ( �����ּҿ� ���Ե� " SECTION_INDEX "���� ������ ��常 Ž����.
*/
// ��� �������� ����� 
PDynamic_NODE Get_Node_1Dim(
	PDynamic_NODE NODE_SECTION_Start_Address,
	ULONG32 node_count_for_field
);// ��� �������� ( count ) 0���� �Ͽ� "�ϳ���" ������ �� ����. 

PDynamic_NODE Get_Node_memcmp_1Dim(
	PDynamic_NODE NODE_SECTION_Start_Address,
	PUCHAR DATA,
	ULONG32 DATA_SIZE
);// ��� �������� memcmp() ���


/*
	[2/2] [��� ��������] - Point! ���Ǵ��� �˻��� (2�������)

		< 2���� �� > ( �ߺ��� ���� ���ǵ� ��� Ž���� )
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




BOOLEAN is_valid_address(PDynamic_NODE NODE_SECTION_Start_Address); // �ּ� ��ȿ�� �˻� 

VOID print_node();

// ��� �� ���� �Լ� 
BOOLEAN Build_up_Node(
	PUCHAR DATA,
	ULONG32 DATA_SIZE,

	BOOLEAN is_init,
	PDynamic_NODE* output_Start_Node_of_NODE_SECTION, // index ���� ���� ���� ��� �ּҸ� ��ȯ ( index���ŵ� �� ���� )  
	ULONG32 Node_Search_VALUE // ��� Ÿ���� �ǹ��� (�±�) 

);



#endif