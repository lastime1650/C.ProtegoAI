#ifndef minifilter_handlers_H
#define minifilter_handlers_H
#pragma warning(disable:4996)
#include <fltKernel.h> // 필터 드라이버 헤더 ( 링커 삽입하쇼 ) 
//#include "minifilter_util.h"
#include "File_io.h"
#include "SHA256.h"
#include "policy_signature_list_manager.h" // 시그니처로 얻은 파일 리스트
#include "Get_Volumes.h"
#include "test_Unicode_word_include.h"

// 컨텍스트 구조체 선언
/*
    PreCreate 핸들러
    파일 및 디렉터리: 열기, 생성, 삭제
*/
typedef struct Pre_CONTEXT {
    BOOLEAN is_changed_FULL_PATH;
    WCHAR FULL_PATH[312];
    ULONG32 FULL_PATH_LENGTH;

    ULONG32 ACCESS_MASK;
    ULONG32 IRP_MajorFunction_append_value_for_sum;

    BOOLEAN is_changed_SHA256; // SHA256값이 중간에 바꼈는지 확인용
    UCHAR SHA256[SHA256_String_Byte_Length];// SHA256


}Pre_CONTEXT, *PPre_CONTEXT;

typedef struct pre_filter_struct {
    BOOLEAN is_IRP_monitoring_active;

    BOOLEAN is_get_something; // 다른 IRP에서 받았는지 확인용


    BOOLEAN is_get_SHA256; //SHA256 값 얻었는 지 
    
    BOOLEAN is_Success_open_that_FILE_when_IRP_MJ_CREATE; //IRP_MJ_CREATE시 파일 열기 시도할 때, 성공했는가?  

    BOOLEAN keep_DENIED; // 계속 다음 호출에 대하여 차단상태로 둘 것인지

    Pre_CONTEXT PreCreate_Context;
}pre_filter_struct, *Ppre_filter_struct;


//전역변수 선언

extern pre_filter_struct Pre_struct_Share_variable;





// 핸들러 선언

// Pre 원형
/*
FLT_PREOP_CALLBACK_STATUS PreOperationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);
*/

// 콜백 함수 선언
FLT_PREOP_CALLBACK_STATUS
Pre_filter_Handler(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
);


extern const FLT_OPERATION_REGISTRATION Callbacks[];


/*
    미니필터 부가 기능 
*/

// 파일 열기 시도하여 유효성 검사
HANDLE is_valid_FILE( PUNICODE_STRING FILE_ABSOULTE_PATH);


// 파일 단순 이동 감지 ( rename 시 미리 변경될 경로가 달라지는 것을 미리알 수 있음. ) 
BOOLEAN is_move(PUNICODE_STRING original_PATH, PUNICODE_STRING renaming_PATH);

// 파일 복사 이동 감지 ( 필터 민감도 높음 ) 
BOOLEAN is_copy_move(ULONG FILE_ACCESS_MASK, BOOLEAN is_Success_open_that_FILE_when_IRP_MJ_CREATE);

// 절대경로가 외장하드 소속인가? 
BOOLEAN is_External_NT_PATH(PUNICODE_STRING FILE_ABSOULTE_PATH);


// 파일명 변경 감지 in IRP_MJ_SET_INFORMATION > FileRenameInformation CLASS


#endif 