#ifndef Query_Files_in_Directories_with_NT_Path_H
#define Query_Files_in_Directories_with_NT_Path_H



/*
	미니필터 시그니처 전용 디렉터리 파일 탐색 API
*/
#include <ntifs.h>
#include "Get_Volumes.h" // PALL_DEVICE_DRIVES
#include "policy_signature_list_manager.h" // Ppolicy_signature_struct

#include "Query_Files_in_Directories.h" // 전체 디렉터리 탐색 유틸리티
/*
	아래 함수를 이용하여 각 드라이브 문자가 "유효한" DRIVE마다 디렉터리 모두 순회(쿼리)
*/
BOOLEAN ListDirectories_with_extension_signature(
	Ppolicy_signature_struct Extensions_Start_Node,
	PALL_DEVICE_DRIVES DRIVE_Start_Node

);


#endif