#ifndef minifilter_handlers_H
#define minifilter_handlers_H
#pragma warning(disable:4996)
#include <fltKernel.h> // ���� ����̹� ��� ( ��Ŀ �����ϼ� ) 
//#include "minifilter_util.h"
#include "File_io.h"
#include "SHA256.h"
#include "policy_signature_list_manager.h" // �ñ״�ó�� ���� ���� ����Ʈ
#include "Get_Volumes.h"
#include "test_Unicode_word_include.h"

// ���ؽ�Ʈ ����ü ����
/*
    PreCreate �ڵ鷯
    ���� �� ���͸�: ����, ����, ����
*/
typedef struct Pre_CONTEXT {
    BOOLEAN is_changed_FULL_PATH;
    WCHAR FULL_PATH[312];
    ULONG32 FULL_PATH_LENGTH;

    ULONG32 ACCESS_MASK;
    ULONG32 IRP_MajorFunction_append_value_for_sum;

    BOOLEAN is_changed_SHA256; // SHA256���� �߰��� �ٲ����� Ȯ�ο�
    UCHAR SHA256[SHA256_String_Byte_Length];// SHA256


}Pre_CONTEXT, *PPre_CONTEXT;

typedef struct pre_filter_struct {
    BOOLEAN is_IRP_monitoring_active;

    BOOLEAN is_get_something; // �ٸ� IRP���� �޾Ҵ��� Ȯ�ο�


    BOOLEAN is_get_SHA256; //SHA256 �� ����� �� 
    
    BOOLEAN is_Success_open_that_FILE_when_IRP_MJ_CREATE; //IRP_MJ_CREATE�� ���� ���� �õ��� ��, �����ߴ°�?  

    BOOLEAN keep_DENIED; // ��� ���� ȣ�⿡ ���Ͽ� ���ܻ��·� �� ������

    Pre_CONTEXT PreCreate_Context;
}pre_filter_struct, *Ppre_filter_struct;


//�������� ����

extern pre_filter_struct Pre_struct_Share_variable;





// �ڵ鷯 ����

// Pre ����
/*
FLT_PREOP_CALLBACK_STATUS PreOperationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);
*/

// �ݹ� �Լ� ����
FLT_PREOP_CALLBACK_STATUS
Pre_filter_Handler(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
);


extern const FLT_OPERATION_REGISTRATION Callbacks[];


/*
    �̴����� �ΰ� ��� 
*/

// ���� ���� �õ��Ͽ� ��ȿ�� �˻�
HANDLE is_valid_FILE( PUNICODE_STRING FILE_ABSOULTE_PATH);


// ���� �ܼ� �̵� ���� ( rename �� �̸� ����� ��ΰ� �޶����� ���� �̸��� �� ����. ) 
BOOLEAN is_move(PUNICODE_STRING original_PATH, PUNICODE_STRING renaming_PATH);

// ���� ���� �̵� ���� ( ���� �ΰ��� ���� ) 
BOOLEAN is_copy_move(ULONG FILE_ACCESS_MASK, BOOLEAN is_Success_open_that_FILE_when_IRP_MJ_CREATE);

// �����ΰ� �����ϵ� �Ҽ��ΰ�? 
BOOLEAN is_External_NT_PATH(PUNICODE_STRING FILE_ABSOULTE_PATH);


// ���ϸ� ���� ���� in IRP_MJ_SET_INFORMATION > FileRenameInformation CLASS


#endif 