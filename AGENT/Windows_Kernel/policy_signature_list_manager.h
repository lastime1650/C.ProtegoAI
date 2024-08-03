#ifndef policy_signature_list_manager_H
#define policy_signature_list_manager_H
#pragma warning(disable:4996)
#include <ntifs.h>
#include "File_io.h"
#include "SHA256.h"
#include "Get_File_Index_INFORMATION.h"



extern KMUTEX MUTEX_signature;

BOOLEAN DoesEndWith(PUNICODE_STRING A, PUNICODE_STRING B);

typedef enum policy_signature_struct_ENUM {
	signature_SAVE_Mode, // ���� ��� 
	signature_NORMAL_Mode, // �̸� ����� �ñ״�ó SHA256 ���� ��� ������ ��
	signature_COMPARE_Mode, // �ܼ� Ȯ���� �˻�

	signature_REMOVE_file_node_with_FULL_PATH, // [�������] ���ڿ�

	signature_get_file_node_with__SHA256___but_idc_about_signature, // �ñ״�ó ������� ������ SHA256�� �������� ���� ��
	signature_get_file_node_with__FULL_PATH___but_idc_about_signature, // �ñ״�ó ������� ������ �����ΰ� �������� ���� ��
	signature_get_FILE_UNIQUE_INDEX, // ������ ���� �ε����� ������ ��� �����ϴ� ��. 

	signature_set_file_node_with__SHA256__but_idc_about_signature,// ���� SHA256 ����
	signature_set_file_node_with__FULL_PATH__but_idc_about_signature, // ���� �� ����
	signature_set_file_node_with__FULL_PATH__and__SHA256_but_idc_about_signature, // ( SHA256 + ���� �� )

	signature_reEDIT_about_file_extension // IRP_MJ_SET_INFORMATIO���� ����ϴ� ENUM�� (  SHA256 �� ����� ���ϸ� 2������ Ȯ���� ������ Ž���Ѵ�. )

}policy_signature_struct_ENUM;


/*
	�����κ��� �ʱ⿡ ������ �系���� ��� �ñ״�ó�� �������� ���Ḯ��Ʈ�� ����� �����

	ANSI_STRING���� ".txt" �̷� ���·� ���´�.

	������ ���� �Լ��� �����ȴ�.

	[ 0. ���� ���Ḯ��Ʈ ���� ���渮��Ʈ -> policy_signature_files_struct ]

		0.1. ��� ����, �߰�, ���� ����Ʈ, 1 ��� ��������


	[ 1. ���Ḯ��Ʈ �Ŵ��� ]
		1-1. ������
		1-2. ����߰�
		1-3. ��� ��ü ����
		1-4. ��� �ĺ� ����
		1-5. ��� �ϳ� �������� 

	[ 2. �ñ״�ó �Ŵ��� ]
		2-1. �ñ״�ó ��� �� ���� �Ŵ���
		2-2. �ñ״�ó compare(��ġ�� ��) �Լ�

*/

typedef struct File_Dir_METADATA {
	LONGLONG CreationTime;
	LONGLONG LastAccessTime;
	LONGLONG LastWriteTime;
	LONGLONG ChangeTime;
}File_Dir_METADATA, *PFile_Dir_METADATA;

// ���� �� Ȯ���ڿ� ���� ���� �����ε�
typedef struct policy_signature_files_struct {

	PUCHAR Previous_Node;

	UNICODE_STRING FULL_PATH;
	UCHAR SHA256[SHA256_String_Byte_Length];
	ULONG64 FILE_unique_index;

	File_Dir_METADATA FILE_METADATA; // ���� ��Ÿ������

	PUCHAR Next_Node;

}policy_signature_files_struct, *Ppolicy_signature_files_struct;

Ppolicy_signature_files_struct Create_Policy_Signature_files_Node(PUCHAR Previcous_Node, PUNICODE_STRING FULL_PATH, PUCHAR SHA256_p, ULONG64* FILE_UNIQUE_INDEX, PFile_Dir_METADATA FILE_METADATA); // 0

Ppolicy_signature_files_struct Append_Policy_Signature_files_Node(Ppolicy_signature_files_struct Node, PUNICODE_STRING FULL_PATH, PUCHAR SHA256_p, ULONG64* FILE_UNIQUE_INDEX, PFile_Dir_METADATA FILE_METADATA); // 0

BOOLEAN Remove_All_Policy_Signature_files_Node(Ppolicy_signature_files_struct Start_NODE);// 0

BOOLEAN Remove_Specified_Policy_Signature_files_Node(Ppolicy_signature_files_struct* Start_Node, Ppolicy_signature_files_struct* Current_Node, Ppolicy_signature_files_struct Specified_Node);

VOID print_All_Policy_Signature_files_Node(Ppolicy_signature_files_struct Start_NODE);// 0

/*
	2��°�� �߿��� �Լ�!
	2���� ���Ḯ��Ʈ�� ���Ͽ� 2���� ���Ḯ��Ʈ�� ��� �ϳ��� GET�ϰų�, SET�� �� ����
*/
Ppolicy_signature_files_struct Get_or_Set_policy_signature_files_Specified_Node(
	
	Ppolicy_signature_files_struct Start_NODE, 
	PUNICODE_STRING Option_INPUT_FULL_PATH, 
	PUCHAR Option_INPUT_SHA256, 
	policy_signature_struct_ENUM Mode,
	ULONG64* File_unique_index
);










// Ȯ���ڸ���Ʈ
typedef struct policy_signature_struct {
	
	PUCHAR Previous_Node;

	UNICODE_STRING Extension; // �����ڵ�� �����ȴ�.

	Ppolicy_signature_files_struct FILEs_of_Extension_list_start_node; // �� ����� Ȯ���ڸ� ���� ���ϵ��� "���� ����Ʈ ���� �ּ�"

	Ppolicy_signature_files_struct FILEs_of_Extension_list_current_node; // "���� ����Ʈ ���� �ּ�"

	PUCHAR Next_Node;

}policy_signature_struct, *Ppolicy_signature_struct;

// ��������
extern Ppolicy_signature_struct Policy_Signature_Start_Node;
extern Ppolicy_signature_struct Policy_Signature_Current_Node;

//[1. ���Ḯ��Ʈ �Ŵ���]
Ppolicy_signature_struct Create_Policy_Signature_Node(PUCHAR Previcous_Node, PUNICODE_STRING Extension); //1-1

Ppolicy_signature_struct Append_Policy_Signature_Node(Ppolicy_signature_struct Node, PUNICODE_STRING Extension); // 1-2

BOOLEAN Remove_All_Policy_Signature_Node();// 1-3


BOOLEAN Remove_Specified_Policy_Signature_Node(Ppolicy_signature_struct StartNode, PUNICODE_STRING Extension);
BOOLEAN Remove_Specified_Policy_Signature_Node_Processing(Ppolicy_signature_struct* Start_Node, Ppolicy_signature_struct* Current_Node, Ppolicy_signature_struct Specified_Node);



VOID print_All_Policy_Signature_Node();

//BOOLEAN Specified_Node_Remover(PUCHAR Start_Node, PUNICODE_STRING Extension); // 1-4


 // 1-5

// [ 2. �ñ״�ó �Ŵ��� ]
//BOOLEAN Policy_Signature_Register_Remove(PUCHAR data, BOOLEAN is_register); // 2-1

/* "
	�߿� �Լ�
*/
BOOLEAN Policy_Signature_Compare(
	
	PUCHAR Start_Node, 

	PUNICODE_STRING Option_INPUT_Extension, 
	PUCHAR Option_INPUT_SHA256, 

	Ppolicy_signature_struct* Option_OUTPUT_important_node, 
	
	policy_signature_struct_ENUM Mode, 
	Ppolicy_signature_files_struct* Option_OUTPUT_file_node,

	PFile_Dir_METADATA FILE_DIR_METADATA
); // 2-2





#endif 