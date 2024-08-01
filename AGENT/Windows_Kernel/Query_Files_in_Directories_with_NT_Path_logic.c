#include "Query_Files_in_Directories_with_NT_Path.h"

/*
	�̴����� �ñ״�ó ���� ���͸� ���� Ž�� API
*/

BOOLEAN ListDirectories_with_extension_signature(
	Ppolicy_signature_struct Extensions_Start_Node,
	PALL_DEVICE_DRIVES DRIVE_Start_Node

) {
	if (Extensions_Start_Node == NULL || DRIVE_Start_Node == NULL) return FALSE;

	PALL_DEVICE_DRIVES current_volume_drives_node = DRIVE_Start_Node;

	// ������
	do {

		if (

			current_volume_drives_node->DRIVE_ALPHABET.Length == 4 && current_volume_drives_node->DRIVE_ALPHABET.Buffer[1] == L':' &&
			((current_volume_drives_node->DRIVE_ALPHABET.Buffer[0] >= L'A' && current_volume_drives_node->DRIVE_ALPHABET.Buffer[0] <= L'Z') ||
				(current_volume_drives_node->DRIVE_ALPHABET.Buffer[0] >= L'a' && current_volume_drives_node->DRIVE_ALPHABET.Buffer[0] <= L'z'))

		) {
			
			Ppolicy_signature_struct current_Extensions_Signature_Node = Extensions_Start_Node;
			// �ñ״�ó�� 
			do {
				/*
					�޾ƿ� output ������ ���� �����ϰ�, ���Ḯ��Ʈ �Ҵ������� �� �ֵ��� �Ѵ�. 
				*/
				PDynamic_NODE output = NULL;

				ULONG32 Dynamic_NODE_Search_Value = ListDirectories(
					&current_volume_drives_node->DRIVE_ALPHABET,
					TRUE,
					&current_Extensions_Signature_Node->Extension,
					&output // �� ������ �־���� ( ȣ�� �� �� ���� NULL�ƴϸ�, 1���̻��� ��带 ���� ��. ) 
				);

				/*
					output ó��( �ñ״�ó ���Ḯ��Ʈ�� �̹� ��ϵǾ� �־�߸� �۵��ȴ�. ) 
				*/
				if (output != NULL && Dynamic_NODE_Search_Value == 'Dirs') {

					PDynamic_NODE current_Directory_files_node = output;
					// ������ �񵿱� ���Ḯ��Ʈ
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