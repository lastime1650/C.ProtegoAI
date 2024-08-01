#include "Query_Files_in_Directories_with_NT_Path.h"

/*
	미니필터 시그니처 전용 디렉터리 파일 탐색 API
*/

BOOLEAN ListDirectories_with_extension_signature(
	Ppolicy_signature_struct Extensions_Start_Node,
	PALL_DEVICE_DRIVES DRIVE_Start_Node

) {
	if (Extensions_Start_Node == NULL || DRIVE_Start_Node == NULL) return FALSE;

	PALL_DEVICE_DRIVES current_volume_drives_node = DRIVE_Start_Node;

	// 볼륨들
	do {

		if (

			current_volume_drives_node->DRIVE_ALPHABET.Length == 4 && current_volume_drives_node->DRIVE_ALPHABET.Buffer[1] == L':' &&
			((current_volume_drives_node->DRIVE_ALPHABET.Buffer[0] >= L'A' && current_volume_drives_node->DRIVE_ALPHABET.Buffer[0] <= L'Z') ||
				(current_volume_drives_node->DRIVE_ALPHABET.Buffer[0] >= L'a' && current_volume_drives_node->DRIVE_ALPHABET.Buffer[0] <= L'z'))

		) {
			
			Ppolicy_signature_struct current_Extensions_Signature_Node = Extensions_Start_Node;
			// 시그니처들 
			do {
				/*
					받아온 output 포인터 값을 참조하고, 연결리스트 할당해제할 수 있도록 한다. 
				*/
				PDynamic_NODE output = NULL;

				ULONG32 Dynamic_NODE_Search_Value = ListDirectories(
					&current_volume_drives_node->DRIVE_ALPHABET,
					TRUE,
					&current_Extensions_Signature_Node->Extension,
					&output // 걍 무조건 넣어야함 ( 호출 후 이 값이 NULL아니면, 1개이상의 노드를 받은 것. ) 
				);

				/*
					output 처리( 시그니처 연결리스트가 이미 등록되어 있어야만 작동된다. ) 
				*/
				if (output != NULL && Dynamic_NODE_Search_Value == 'Dirs') {

					PDynamic_NODE current_Directory_files_node = output;
					// 가져온 비동기 연결리스트
					do {
						
						if (current_Directory_files_node->Node_Search_VALUE == Dynamic_NODE_Search_Value) {
							
							UNICODE_STRING fullPath = { 0, };
							RtlInitUnicodeString(&fullPath, (PWCH)current_Directory_files_node->DATA);
							
							//current_Directory_files_node->;

							Policy_Signature_Compare((PUCHAR)Policy_Signature_Start_Node, &fullPath, NULL, NULL, signature_SAVE_Mode, NULL, NULL);

						}

						
					} while (current_Directory_files_node !=NULL);

					Remove_Node_with_Search_Value(output, Dynamic_NODE_Search_Value);
				}

				current_Extensions_Signature_Node = (Ppolicy_signature_struct)current_Extensions_Signature_Node->Next_Node;

			} while (current_Extensions_Signature_Node != NULL);

			
		}

		current_volume_drives_node = (PALL_DEVICE_DRIVES)current_volume_drives_node->Next_Node;
	} while (current_volume_drives_node != NULL);

	return TRUE;
}