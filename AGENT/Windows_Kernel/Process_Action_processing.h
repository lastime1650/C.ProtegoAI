#ifndef Process_Action_processing_H
#define Process_Action_processing_H

#include <ntifs.h>
#include <ntddk.h>

#include "Mutex_with_Lock.h"
#include "converter_PID.h" // PID�� ���� SHA256�� ���������� ���ϱ� ���Ͽ�
#include "Length_Based_Data_Parser.h" // RUST�� ���̱���� �ؼ��ؾ��ϹǷ� 
#include "TCP_conn.h"

/*
	���μ��� �ݹ��Լ� Action ó�� �Լ���
*/
typedef struct Action_for_process_creation_NODE {

	PUCHAR Previous_Address;

	BOOLEAN is_Block; // Block�� ���ΰ�? 
	UCHAR SHA256[SHA256_String_Byte_Length]; // 64+1(\x00) ����Ʈ (�����޸�)
	Type_Feature feature; // Create ? Remove ? ... 

	PUCHAR Next_Address;

}Action_for_process_creation_NODE, * PAction_for_process_creation_NODE;

//�׼� ��� ����
PAction_for_process_creation_NODE Create_process_routine_NODE(
	PAction_for_process_creation_NODE Previous_Node,
	BOOLEAN is_Block,
	PUCHAR SHA256,
	Type_Feature feature
);

//�׼� ��� �߰�
PAction_for_process_creation_NODE APPEND_process_routine_NODE(
	PAction_for_process_creation_NODE current_node,
	BOOLEAN is_Block,
	PUCHAR SHA256,
	Type_Feature feature
);

//�׼� ��� �ϳ� ���� ( SHA256 ���� )
PAction_for_process_creation_NODE Search_process_routine_NODE(
	PAction_for_process_creation_NODE Start_Node,
	PUCHAR target_SHA256
);

//�׼� ��� �ϳ� ����(Ȯ���) ( SHA256 ���� + Extended )
PAction_for_process_creation_NODE Ex_Search_process_routine_NODE(
	PAction_for_process_creation_NODE Start_Node,
	BOOLEAN is_Block,
	PUCHAR target_SHA256,
	Type_Feature feature
);

// ��� ���� ���ִ� �Լ�
VOID DELETE_Action_One_Node(PAction_for_process_creation_NODE One_Node);

//�׼� ���Ḯ��Ʈ ����Ʈ��
VOID Print_process_routine_NODE(
	PAction_for_process_creation_NODE Start_Node
);



PAction_for_process_creation_NODE Get_Action_Node_With_SHA256(HANDLE Input_PID);

//ObRegisterCallback ���� ���� ��������
extern PAction_for_process_creation_NODE Action_for_process_routine_node_Start_Address; // ��� ���� �ּ�
extern PAction_for_process_creation_NODE Action_for_process_routine_node_Current_Address; // �̰� �Լ����� ����ϴ� ó������

extern Util_Mutex_with_Lock Action_for_proces_routine_node_KMUTEX; // ��ȣ����

BOOLEAN Action_for_process_creation(PUCHAR GET_BUFFER, ULONG32 GET_BUFFER_len);
//////////////////////////////////////////////////////////////////////////////// ���μ��� ���� ���� ��ġ ó��

#endif