#ifndef RUST_DLP_Get_File_SIgnatures_H
#define RUST_DLP_Get_File_SIgnatures_H

#include <ntifs.h>

#include "Length_Based_Data_Parser.h" // 길이-기반
#include "policy_signature_list_manager.h"

#include "converter_string.h"

/*  RUST 서버로부터 얻은 CHAR형 아스키문자열로 확장자 문자열을 "길이-기반"으로 받아 전용 연결리스트에 등록 */

typedef enum SIG_STATUS {
	is_register = 1,
	is_remove,
	is_get
}SIG_STATUS;


// 등록인지 삭제인지 또는 현재 등록된 정보를 요청하는 상태인지? 필수.
BOOLEAN Signature_append_or_remove_or_get(PLength_Based_DATA_Node INPUT);

#endif