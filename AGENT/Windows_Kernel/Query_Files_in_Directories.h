#ifndef Query_Files_in_Directories_H
#define Query_Files_in_Directories_H

#include <ntifs.h>
#include "Get_Volumes.h" // 볼륨이 존재할 때 작동가능함.

/*
	유틸리티 형식으로 변환하였으므로, 
	리턴 전용 연결리스트를 구현해야함.

	1. 파일 절대경로 (UNICODE_STRING.Buffer) 

	-> Search_Value = 'Dirs'
*/
#include "Parallel_Linked_List.h"
#define Node_Search_Value = 'Dirs';


/*
	디렉터리 순회하여 파일 및 디렉터리 찾을 수 있음

	DirectoryPath 인수값의 마지막은 \\ 을 추가하지 않아도 됨

*/

// 3번째 인자값은 최초 호출 시엔 꼭 유효한 주소를 넣어야함
// 호출이 모두 리턴되면, 3번째 인자값을 통하여 노드에 접근할 수 있음
// 단, 외부에서 할당해제해야함

/*
	전체 검색은 미친 짓임.PWCH로 필터링가능
	-> 필터링방식은?
	> 대소문자 상관없이 포함되면 TRUE 임
	
	만약 인수값이 NULL이면 전체 스캔을 의미한다. 

	
*/
#include "test_Unicode_word_include.h" // ContainsStringInsensitive()
ULONG32 ListDirectories(
	
	PUNICODE_STRING DirectoryPath, BOOLEAN is_init, 
	
	PUNICODE_STRING INPUT_Hint_Data,
	
	PDynamic_NODE* Output_Node);

BOOLEAN ListDirectories_PoolFree(PDynamic_NODE Node_for_PoolFREE, ULONG32 Dir_Search_Value);

#endif // !Query_Files_in_Directories_H
