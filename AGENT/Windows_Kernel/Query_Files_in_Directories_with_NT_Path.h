#ifndef Query_Files_in_Directories_with_NT_Path_H
#define Query_Files_in_Directories_with_NT_Path_H



/*
	�̴����� �ñ״�ó ���� ���͸� ���� Ž�� API
*/
#include <ntifs.h>
#include "Get_Volumes.h" // PALL_DEVICE_DRIVES
#include "policy_signature_list_manager.h" // Ppolicy_signature_struct

#include "Query_Files_in_Directories.h" // ��ü ���͸� Ž�� ��ƿ��Ƽ
/*
	�Ʒ� �Լ��� �̿��Ͽ� �� ����̺� ���ڰ� "��ȿ��" DRIVE���� ���͸� ��� ��ȸ(����)
*/
BOOLEAN ListDirectories_with_extension_signature(
	Ppolicy_signature_struct Extensions_Start_Node,
	PALL_DEVICE_DRIVES DRIVE_Start_Node

);


#endif