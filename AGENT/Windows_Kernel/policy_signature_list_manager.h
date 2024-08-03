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
	signature_SAVE_Mode, // 저장 기능 
	signature_NORMAL_Mode, // 미리 저장된 시그니처 SHA256 포함 노드 가져올 때
	signature_COMPARE_Mode, // 단순 확장자 검사

	signature_REMOVE_file_node_with_FULL_PATH, // [삭제모드] 문자열

	signature_get_file_node_with__SHA256___but_idc_about_signature, // 시그니처 상관없이 파일의 SHA256가 동일한지 보는 것
	signature_get_file_node_with__FULL_PATH___but_idc_about_signature, // 시그니처 상관없이 파일의 절대경로가 동일한지 보는 것
	signature_get_FILE_UNIQUE_INDEX, // 오로지 파일 인덱스만 가지고 노드 추출하는 것. 

	signature_set_file_node_with__SHA256__but_idc_about_signature,// 파일 SHA256 수정
	signature_set_file_node_with__FULL_PATH__but_idc_about_signature, // 파일 명 수정
	signature_set_file_node_with__FULL_PATH__and__SHA256_but_idc_about_signature, // ( SHA256 + 파일 명 )

	signature_reEDIT_about_file_extension // IRP_MJ_SET_INFORMATIO에서 사용하는 ENUM값 (  SHA256 과 변경될 파일명 2가지로 확장자 변경을 탐지한다. )

}policy_signature_struct_ENUM;


/*
	서버로부터 초기에 가져온 사내문서 취급 시그니처를 동적으로 연결리스트를 만들어 등록함

	ANSI_STRING으로 ".txt" 이런 형태로 얻어온다.

	다음과 같은 함수가 구현된다.

	[ 0. 메인 연결리스트 안의 연길리스트 -> policy_signature_files_struct ]

		0.1. 노드 생성, 추가, 삭제 프린트, 1 노드 가져오기


	[ 1. 연결리스트 매니저 ]
		1-1. 노드생성
		1-2. 노드추가
		1-3. 노드 전체 삭제
		1-4. 노드 식별 삭제
		1-5. 노드 하나 가져오기 

	[ 2. 시그니처 매니저 ]
		2-1. 시그니처 등록 및 삭제 매니저
		2-2. 시그니처 compare(일치한 지) 함수

*/

typedef struct File_Dir_METADATA {
	LONGLONG CreationTime;
	LONGLONG LastAccessTime;
	LONGLONG LastWriteTime;
	LONGLONG ChangeTime;
}File_Dir_METADATA, *PFile_Dir_METADATA;

// 로컬 내 확장자에 묶인 파일 절대경로들
typedef struct policy_signature_files_struct {

	PUCHAR Previous_Node;

	UNICODE_STRING FULL_PATH;
	UCHAR SHA256[SHA256_String_Byte_Length];
	ULONG64 FILE_unique_index;

	File_Dir_METADATA FILE_METADATA; // 파일 메타데이터

	PUCHAR Next_Node;

}policy_signature_files_struct, *Ppolicy_signature_files_struct;

Ppolicy_signature_files_struct Create_Policy_Signature_files_Node(PUCHAR Previcous_Node, PUNICODE_STRING FULL_PATH, PUCHAR SHA256_p, ULONG64* FILE_UNIQUE_INDEX, PFile_Dir_METADATA FILE_METADATA); // 0

Ppolicy_signature_files_struct Append_Policy_Signature_files_Node(Ppolicy_signature_files_struct Node, PUNICODE_STRING FULL_PATH, PUCHAR SHA256_p, ULONG64* FILE_UNIQUE_INDEX, PFile_Dir_METADATA FILE_METADATA); // 0

BOOLEAN Remove_All_Policy_Signature_files_Node(Ppolicy_signature_files_struct Start_NODE);// 0

BOOLEAN Remove_Specified_Policy_Signature_files_Node(Ppolicy_signature_files_struct* Start_Node, Ppolicy_signature_files_struct* Current_Node, Ppolicy_signature_files_struct Specified_Node);

VOID print_All_Policy_Signature_files_Node(Ppolicy_signature_files_struct Start_NODE);// 0

/*
	2번째로 중요한 함수!
	2차원 연결리스트에 대하여 2차원 연결리스트의 노드 하나를 GET하거나, SET할 수 있음
*/
Ppolicy_signature_files_struct Get_or_Set_policy_signature_files_Specified_Node(
	
	Ppolicy_signature_files_struct Start_NODE, 
	PUNICODE_STRING Option_INPUT_FULL_PATH, 
	PUCHAR Option_INPUT_SHA256, 
	policy_signature_struct_ENUM Mode,
	ULONG64* File_unique_index
);










// 확장자리스트
typedef struct policy_signature_struct {
	
	PUCHAR Previous_Node;

	UNICODE_STRING Extension; // 유니코드로 관리된다.

	Ppolicy_signature_files_struct FILEs_of_Extension_list_start_node; // 이 노드의 확장자를 가진 파일들의 "연결 리스트 시작 주소"

	Ppolicy_signature_files_struct FILEs_of_Extension_list_current_node; // "연결 리스트 현재 주소"

	PUCHAR Next_Node;

}policy_signature_struct, *Ppolicy_signature_struct;

// 전역변수
extern Ppolicy_signature_struct Policy_Signature_Start_Node;
extern Ppolicy_signature_struct Policy_Signature_Current_Node;

//[1. 연결리스트 매니저]
Ppolicy_signature_struct Create_Policy_Signature_Node(PUCHAR Previcous_Node, PUNICODE_STRING Extension); //1-1

Ppolicy_signature_struct Append_Policy_Signature_Node(Ppolicy_signature_struct Node, PUNICODE_STRING Extension); // 1-2

BOOLEAN Remove_All_Policy_Signature_Node();// 1-3


BOOLEAN Remove_Specified_Policy_Signature_Node(Ppolicy_signature_struct StartNode, PUNICODE_STRING Extension);
BOOLEAN Remove_Specified_Policy_Signature_Node_Processing(Ppolicy_signature_struct* Start_Node, Ppolicy_signature_struct* Current_Node, Ppolicy_signature_struct Specified_Node);



VOID print_All_Policy_Signature_Node();

//BOOLEAN Specified_Node_Remover(PUCHAR Start_Node, PUNICODE_STRING Extension); // 1-4


 // 1-5

// [ 2. 시그니처 매니저 ]
//BOOLEAN Policy_Signature_Register_Remove(PUCHAR data, BOOLEAN is_register); // 2-1

/* "
	중요 함수
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