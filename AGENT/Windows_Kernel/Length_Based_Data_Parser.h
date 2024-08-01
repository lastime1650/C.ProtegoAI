#ifndef Length_Based_Data_Parser_H
#define Length_Based_Data_Parser_H

#include <ntifs.h>
#include <ntddk.h>

/*
	RUST �������� ����, ���̱�� �����͸� �ؼ��ϴ� �뵵.
*/

// ���Ḯ��Ʈ ���
typedef struct Length_Based_DATA_Node {

	PUCHAR Previous_Address;

	PUCHAR RAW_DATA;
	ULONG32 RAW_DATA_Size;

	PUCHAR Next_Address;

}Length_Based_DATA_Node, * PLength_Based_DATA_Node;

//��� ����
PLength_Based_DATA_Node Create_Length_Based_DATA_Node(
	PLength_Based_DATA_Node Previous_Address,

	PUCHAR RAW_DATA,
	ULONG32 RAW_DATA_Size
);

//��� �߰�
PLength_Based_DATA_Node APPEND_Length_Based_DATA_Node(
	PLength_Based_DATA_Node Current_Node,

	PUCHAR RAW_DATA,
	ULONG32 RAW_DATA_Size
);

/* �ݺ������� �о�, ����-��� �����͸� �ؼ� */
BOOLEAN Get_RAW_DATA_one_time_from_Length_Based_DATA(

	PUCHAR Input_DATA,

	PUCHAR* Output_DATA,
	ULONG32* Output_DATA_Size

);

/* ���̱���� �ؼ��ϰ�, ���Ḯ��Ʈ�� �����ϴ� �Լ� */
PLength_Based_DATA_Node Build_RAW_DATA(
	PUCHAR Input_Start_RAW_DATA,
	ULONG32 Input_RAW_DATA_ALL_SIZE
);

/* ���Ḯ��Ʈ ��ü �Ҵ����� */
VOID RAW_DATA_node_FreePool(
	PLength_Based_DATA_Node Start_Node
);




#endif