#include "policy_signature_list_manager.h"

Ppolicy_signature_struct Policy_Signature_Start_Node = NULL;
Ppolicy_signature_struct Policy_Signature_Current_Node = NULL;

KMUTEX MUTEX_signature = { 0, };

// �����ڵ� ���ڿ� �� ���ڸ� �������� ��ġ���� 
BOOLEAN DoesEndWith(PUNICODE_STRING A, PUNICODE_STRING B) {
	if (A->Length < B->Length) {
		return FALSE; // A�� ���̰� B���� ª���� ��ġ�� �� ����
	}

	UNICODE_STRING subString;
	subString.Buffer = A->Buffer + ( (A->Length - B->Length) / sizeof(WCHAR) );
	subString.Length = B->Length;
	subString.MaximumLength = B->Length;

	return RtlEqualUnicodeString(&subString, B, TRUE);
}

// [ 2. �ñ״�ó �Ŵ��� ]
/*
BOOLEAN Policy_Signature_Register_Remove(PWCH data, BOOLEAN is_register) { // 2-1
	

	if (is_register) {
		if (Policy_Signature_Start_Node == NULL) {


			Policy_Signature_Start_Node = Create_Policy_Signature_Node(NULL, data, (ULONG32)(wcslen(data) * sizeof(WCHAR)));
			Policy_Signature_Current_Node = Policy_Signature_Start_Node;

		}
		else {
			/ �ߺ� �˻��ϰ� �־���� 
			Policy_Signature_Current_Node = Append_Policy_Signature_Node((PUCHAR)Policy_Signature_Current_Node, data, (ULONG32)(wcslen(data) * sizeof(WCHAR)));
		}
	}
	else {
		if (Policy_Signature_Start_Node != NULL) {
		}
		else {
			return STATUS_UNSUCCESSFUL;
		}
	}
	
	return STATUS_SUCCESS;

}
*/

/*





	0.





*/
Ppolicy_signature_files_struct Create_Policy_Signature_files_Node(PUCHAR Previcous_Node, PUNICODE_STRING FULL_PATH, PUCHAR SHA256_p, ULONG64* FILE_UNIQUE_INDEX, PFile_Dir_METADATA FILE_METADATA) { // 0

	Ppolicy_signature_files_struct New_Node = ExAllocatePoolWithTag(NonPagedPool, sizeof(policy_signature_files_struct), 'PoSg');
	if (New_Node == NULL) return NULL;

	if (Previcous_Node == NULL) {
		New_Node->Previous_Node = NULL;
	}
	else {
		New_Node->Previous_Node = Previcous_Node;
	}



	New_Node->FULL_PATH.Length = (USHORT)wcslen(FULL_PATH->Buffer) * sizeof(WCHAR);
	New_Node->FULL_PATH.MaximumLength = (USHORT)(wcslen(FULL_PATH->Buffer) + 1) * sizeof(WCHAR);
	New_Node->FULL_PATH.Buffer = ExAllocatePoolWithTag(NonPagedPool, New_Node->FULL_PATH.MaximumLength, 'PoSg');
	if (New_Node->FULL_PATH.Buffer == NULL) {
		ExFreePoolWithTag(New_Node, 'PoSg');
		return NULL;
	}

	RtlCopyMemory(New_Node->FULL_PATH.Buffer, FULL_PATH->Buffer, New_Node->FULL_PATH.Length);
	New_Node->FULL_PATH.Buffer[FULL_PATH->Length / sizeof(WCHAR)] = L'\0'; // NULL ���� ���� �߰�

	RtlCopyMemory(New_Node->SHA256, SHA256_p, SHA256_String_Byte_Length); // SHA256 ����

	// ���� ��Ÿ������ �߰�
	if (FILE_METADATA != NULL) {
		New_Node->FILE_METADATA.CreationTime = FILE_METADATA->CreationTime;
		New_Node->FILE_METADATA.LastAccessTime = FILE_METADATA->LastAccessTime;
		New_Node->FILE_METADATA.LastWriteTime = FILE_METADATA->LastWriteTime;
		New_Node->FILE_METADATA.ChangeTime = FILE_METADATA->ChangeTime;
	}
	



	New_Node->FILE_unique_index = *FILE_UNIQUE_INDEX; // ���� �ý��ۿ����� ���� ���� ��ȣ

	New_Node->Next_Node = NULL;
	return New_Node;
}

Ppolicy_signature_files_struct Append_Policy_Signature_files_Node(Ppolicy_signature_files_struct Node, PUNICODE_STRING FULL_PATH, PUCHAR SHA256_p, ULONG64* FILE_UNIQUE_INDEX, PFile_Dir_METADATA FILE_METADATA) { // 0
	Ppolicy_signature_files_struct New_Node = Create_Policy_Signature_files_Node((PUCHAR)Node, FULL_PATH, SHA256_p, FILE_UNIQUE_INDEX, FILE_METADATA);
	if (New_Node == NULL) return NULL;

	Node->Next_Node = (PUCHAR)New_Node;

	New_Node->Previous_Node = (PUCHAR)Node;

	return New_Node;
}

BOOLEAN Remove_All_Policy_Signature_files_Node(Ppolicy_signature_files_struct Start_NODE) {// 0

	if (Start_NODE == NULL) return FALSE;

	Ppolicy_signature_files_struct current_node = Start_NODE;

	do {
		ExFreePoolWithTag((PVOID)current_node->FULL_PATH.Buffer, 'PoSg');

		Ppolicy_signature_files_struct tmp_next_node = NULL;

		tmp_next_node = (Ppolicy_signature_files_struct)current_node->Next_Node;
		ExFreePoolWithTag((PVOID)current_node, 'PoSg');

		current_node = tmp_next_node;
	} while (current_node != NULL);

	return TRUE;
}


// hint�� �� Ư�� 1�� ��带 ��ȯ
Ppolicy_signature_files_struct Get_or_Set_policy_signature_files_Specified_Node(Ppolicy_signature_files_struct Start_NODE , PUNICODE_STRING Option_INPUT_FULL_PATH, PUCHAR Option_INPUT_SHA256, policy_signature_struct_ENUM Mode, ULONG64* File_unique_index) {
	
	
	if (Start_NODE == NULL) return NULL;

	Ppolicy_signature_files_struct current_node = Start_NODE;

	do {

		// Ư�� ���͸�
		if (Mode == signature_reEDIT_about_file_extension) {
			/*
				IRP_MJ_SET_INFORMATION���� << Ȯ���� ���� >> �� Ž�������� Ư�� ������.

				SHA256���� ��带 ��ȯ�Ѵ�. 

			*/
			if (Option_INPUT_SHA256 == NULL) return NULL;
			if (memcmp(Option_INPUT_SHA256, (PUCHAR)&current_node->SHA256, SHA256_String_Byte_Length) == 0) {
				return current_node;
			}
			else {
				return NULL;
			}

		}

		

		/*
			���� ID INDEX���� ���ƾ��� 
		*/
		if (current_node->FILE_unique_index == *File_unique_index) {

			if (Option_INPUT_FULL_PATH != NULL && Mode != signature_set_file_node_with__SHA256__but_idc_about_signature) {
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RtlEqualUnicodeString ( %wZ vs %wZ ) %d \n", *Option_INPUT_FULL_PATH, current_node->FULL_PATH, Mode);


				if (Mode == signature_set_file_node_with__FULL_PATH__but_idc_about_signature || Mode == signature_set_file_node_with__FULL_PATH__and__SHA256_but_idc_about_signature) {

					// SET
					/*
						FULL_PATH�� ������ ����
					*/

					USHORT Length = (USHORT)wcslen(Option_INPUT_FULL_PATH->Buffer) * sizeof(WCHAR);;
					USHORT MaximumLength = (USHORT)(wcslen(Option_INPUT_FULL_PATH->Buffer) + 1) * sizeof(WCHAR);

					PWCH tmp_remember = current_node->FULL_PATH.Buffer;
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [SET] --- \n ");
					current_node->FULL_PATH.Buffer = ExAllocatePoolWithTag(NonPagedPool, MaximumLength, 'PoSg');
					if (current_node->FULL_PATH.Buffer == NULL) {
						return NULL;
					}
					current_node->FULL_PATH.Length = Length;
					current_node->FULL_PATH.MaximumLength = MaximumLength;


					RtlCopyMemory(current_node->FULL_PATH.Buffer, Option_INPUT_FULL_PATH->Buffer, current_node->FULL_PATH.Length);
					current_node->FULL_PATH.Buffer[Option_INPUT_FULL_PATH->Length / sizeof(WCHAR)] = L'\0'; // NULL ���� ���� �߰�

					ExFreePoolWithTag(tmp_remember, 'PoSg');
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [SET] ���ϸ� ����Ϸ� -> %wZ\n ", current_node->FULL_PATH);
				}
				else {


					// GET
					if (RtlEqualUnicodeString(Option_INPUT_FULL_PATH, &current_node->FULL_PATH, TRUE)) {
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\nGet_policy_signature_files_Specified_Node -> %wZ ��ġ��!! \n", current_node->FULL_PATH);
						return current_node;
					}
					else {
						return NULL;
					}
					

				}

			}




			if (Option_INPUT_SHA256 != NULL) {
				if (Mode == signature_set_file_node_with__SHA256__but_idc_about_signature || Mode == signature_set_file_node_with__FULL_PATH__and__SHA256_but_idc_about_signature) {
					// SET
					/*
						SHA256���� ������ ����
					*/
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [ SET ] SHA256 ���� \n");
					memcpy((PUCHAR)&current_node->SHA256, Option_INPUT_SHA256, SHA256_String_Byte_Length);
					
				}
				else {
					// GET
					if (memcmp(Option_INPUT_SHA256, (PUCHAR)&current_node->SHA256, SHA256_String_Byte_Length) == 0) {
						return current_node;
					}
					else {
						return NULL;
					}
				}
			}

			return current_node; // ������ �� ������ ���� ID �� �������Ƿ� ������ ( �� ������ ���� ���ܰ� ���� )
		}
		
		
		

		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\nlist-sub_files -> %wZ \n", current_node->FULL_PATH);
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\nlist-sub_files[ SHA256 ] -> %s \n", current_node->SHA256);


		current_node = (Ppolicy_signature_files_struct)current_node->Next_Node;
	} while (current_node != NULL);

	return NULL;

}



VOID print_All_Policy_Signature_files_Node(Ppolicy_signature_files_struct Start_NODE) {// 0
	if (Start_NODE == NULL) return;

	Ppolicy_signature_files_struct current_node = Start_NODE;

	do {

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\nlist-sub_files -> %wZ \n", current_node->FULL_PATH);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\nlist-sub_files[ SHA256 ] -> %s \n", current_node->SHA256);


		current_node = (Ppolicy_signature_files_struct)current_node->Next_Node;
	} while (current_node != NULL);

	return;
}

/*





	1. 





*/
/*
	���� �߿��� �Լ��� 

	�Ķ���� Mode ���ڿ� ���� ���� ���� + SHA256 ��带 �߰��� �� �ְ�,

	��ġ�Ͽ��� ���� ��带 ���������� �� �� ����

*/
BOOLEAN Policy_Signature_Compare(PUCHAR Start_Node, PUNICODE_STRING INPUT_Extension, PUCHAR Option_INPUT_SHA256, Ppolicy_signature_struct* Option_OUTPUT_important_node, policy_signature_struct_ENUM Mode, Ppolicy_signature_files_struct* Option_OUTPUT_file_node, PFile_Dir_METADATA FILE_DIR_METADATA) { // 2-2
	
	// Start_Node == Policy_Signature_Start_Node ! 

	if (Policy_Signature_Start_Node == NULL ) return FALSE;

	ULONG64 FILE_UNIQUE_INDEX = 0;


	

	Ppolicy_signature_struct current_node = (Ppolicy_signature_struct)Start_Node;

	do {

		
		if (Mode == signature_get_file_node_with__SHA256___but_idc_about_signature) {
			/*
				ȣ���ڰ� SHA256�����͸� ������ SHA256 ��� ������ ������ ��
			*/
			/* ���� ���� ID �������� */
			if (Get_FILE_INDEX_INFORMATION(INPUT_Extension, &FILE_UNIQUE_INDEX) != STATUS_SUCCESS || FILE_UNIQUE_INDEX == 0) {
				return FALSE;
			}


			if (Option_INPUT_SHA256 == NULL || Option_OUTPUT_file_node == NULL) return FALSE; // �� Mode�� �ʿ��� ���ڰ��� NULL�̸� FALSE 

			if (current_node->FILEs_of_Extension_list_start_node == NULL) return FALSE; // �ñ״�ó Ž���� ���� ���Ḯ��Ʈ ���� �ּҰ� NULL�̸� FALSE


			*Option_OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node( // ���⼭ ������ ���� �������� SHA256 ��ġ�� ���� !!!!

				current_node->FILEs_of_Extension_list_start_node,
				NULL,
				Option_INPUT_SHA256,
				Mode,
				&FILE_UNIQUE_INDEX
			);

			if (Option_OUTPUT_important_node != NULL) *Option_OUTPUT_important_node = current_node; // Option ó�� 

			if (*Option_OUTPUT_file_node != NULL) { // SHA256 ���� ��� �ּ� �������� ����ó�� 
				return TRUE;
			}




		}
		else if (Mode == signature_get_file_node_with__FULL_PATH___but_idc_about_signature) {
			/*
				ȣ���ڰ� PUNICODE_STRIN input �����͸� ������ ����  ��� ������ ������ ��
			*/
			/* ���� ���� ID �������� */
			if (Get_FILE_INDEX_INFORMATION(INPUT_Extension, &FILE_UNIQUE_INDEX) != STATUS_SUCCESS || FILE_UNIQUE_INDEX == 0) {
				return FALSE;
			}

			if (INPUT_Extension == NULL || Option_OUTPUT_file_node == NULL) return FALSE; // �� Mode�� �ʿ��� ���ڰ��� NULL�̸� FALSE 

			if (current_node->FILEs_of_Extension_list_start_node == NULL) return FALSE; // �ñ״�ó Ž���� ���� ���Ḯ��Ʈ ���� �ּҰ� NULL�̸� FALSE


			*Option_OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node( // ���⼭ ������ ���� �������� SHA256 ��ġ�� ���� !!!!

				current_node->FILEs_of_Extension_list_start_node,
				INPUT_Extension,
				NULL,
				Mode,
				&FILE_UNIQUE_INDEX

			);

			if (Option_OUTPUT_important_node != NULL) *Option_OUTPUT_important_node = current_node; // Option ó�� 

			if (*Option_OUTPUT_file_node != NULL) { // SHA256 ���� ��� �ּ� �������� ����ó�� 
				return TRUE;
			}
		}
		else if (Mode == signature_set_file_node_with__SHA256__but_idc_about_signature || Mode == signature_set_file_node_with__FULL_PATH__but_idc_about_signature || Mode == signature_set_file_node_with__FULL_PATH__and__SHA256_but_idc_about_signature) {
			/*
				ȣ���ڰ� Ư�� �ñ״�ó ��尡 ���� 2���� ���Ḯ��Ʈ�� ���Ͽ� ������ �ϴ� �� 
				
				�Ķ���Ͱ� NULL�̸� �ڵ����� ��������. �������� �Ķ������ ��쿡�� ó����

			*/
			/* ���� ���� ID �������� */
			if (Get_FILE_INDEX_INFORMATION(INPUT_Extension, &FILE_UNIQUE_INDEX) != STATUS_SUCCESS || FILE_UNIQUE_INDEX == 0) {
				return FALSE;
			}


			if (current_node->FILEs_of_Extension_list_start_node == NULL) return FALSE; // �ñ״�ó Ž���� ���� ���Ḯ��Ʈ ���� �ּҰ� NULL�̸� FALSE
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [ SET ] ���� \n ");
			
			if (Mode == signature_set_file_node_with__FULL_PATH__but_idc_about_signature) {
				Get_or_Set_policy_signature_files_Specified_Node( // ���⼭ ������ ���� �������� SHA256 ��ġ�� ���� !!!!

					current_node->FILEs_of_Extension_list_start_node,
					INPUT_Extension,
					NULL,
					Mode,
					&FILE_UNIQUE_INDEX

				);
			}
			else if (Mode == signature_set_file_node_with__SHA256__but_idc_about_signature) {
				Get_or_Set_policy_signature_files_Specified_Node( // ���⼭ ������ ���� �������� SHA256 ��ġ�� ���� !!!!

					current_node->FILEs_of_Extension_list_start_node,
					NULL,
					Option_INPUT_SHA256,
					Mode,
					&FILE_UNIQUE_INDEX

				);
			}
			else {
				Get_or_Set_policy_signature_files_Specified_Node( // ���⼭ ������ ���� �������� SHA256 ��ġ�� ���� !!!!

					current_node->FILEs_of_Extension_list_start_node,
					INPUT_Extension,
					Option_INPUT_SHA256,
					Mode,
					&FILE_UNIQUE_INDEX

				);
			}
			

			return TRUE;


		}
		else {
			/*
				�ʱ� ���� SAVE
			*/
			if (INPUT_Extension == NULL) return FALSE;

			if (wcslen(INPUT_Extension->Buffer) >= wcslen(current_node->Extension.Buffer)) {

				if (DoesEndWith(INPUT_Extension, &current_node->Extension)) {
					/* ��ġ */
	
					/* ���� ���� ID �������� */
					if (Get_FILE_INDEX_INFORMATION(INPUT_Extension, &FILE_UNIQUE_INDEX) != STATUS_SUCCESS || FILE_UNIQUE_INDEX == 0) {
						return FALSE;
					}

					
					


					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " �ñ״�ó ��ġ�մϴ�. ==> %wZ  , �������̵�: %llu \n", *INPUT_Extension, FILE_UNIQUE_INDEX);

					
					



					switch (Mode) {
					case signature_SAVE_Mode: // ��� ����
						NULL;

						/* ���� ���Ḯ��Ʈ���� �ߺ��� �ִ� ��� �������� ����.  */

						if (current_node->FILEs_of_Extension_list_start_node != NULL) {
							Ppolicy_signature_files_struct OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node( // ���⼭ ������ ���� �������� SHA256 ��ġ�� ���� !!!!

								current_node->FILEs_of_Extension_list_start_node,
								INPUT_Extension,
								NULL,
								Mode,
								&FILE_UNIQUE_INDEX

							);

							if (OUTPUT_file_node) {
								/* �ߺ��̹Ƿ� �������. */
								DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "_[Ppolicy_signature_files_struct]�ߺ����� Ȯ�ε�, �������_\n");
								return FALSE;

								// ���� ���� ��� ���� ����
								//if (Remove_Specified_Policy_Signature_files_Node(&current_node->FILEs_of_Extension_list_start_node, &current_node->FILEs_of_Extension_list_current_node, OUTPUT_file_node)) {
								//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " signature_REMOVE_file_node_with_FULL_PATH ���� ���� \n");
								//	return TRUE;
								//}
								//else {
								//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " signature_REMOVE_file_node_with_FULL_PATH ���� ���� \n");
								//	return FALSE;
								//}
							}
							else {
								/* �ߺ��ƴ� Ȯ�� */
							}
						}

						/* �ؽ� ���ϱ�  ���ڿ� ������ ���� �ȱ��ص��� */
						UCHAR SHA256[SHA256_String_Byte_Length] = { 0, };

						PVOID File_Bin = NULL;
						ULONG File_Bin_Length = 0;

						if (Option_INPUT_SHA256 == NULL) {
							/* ���� ���̳ʸ� ���ϱ� */

							if (ALL_in_ONE_FILE_IO(&File_Bin, &File_Bin_Length, *INPUT_Extension, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) return FALSE;

							/* ����  �ؽ� ���ϱ� */

							if (GET_SHA256_HASHING(File_Bin, File_Bin_Length, (PCHAR)&SHA256) != STATUS_SUCCESS) {
								ExFreePoolWithTag(File_Bin, 'FILE');
								return FALSE;
							}
						}
						else {
							/* �̹� ���ڿ� SHA256���� �غ�� ��� */
							memcpy((PUCHAR)&SHA256, Option_INPUT_SHA256, SHA256_String_Byte_Length);
						}



						if (current_node->FILEs_of_Extension_list_start_node == NULL) {



							current_node->FILEs_of_Extension_list_start_node = Create_Policy_Signature_files_Node(NULL, INPUT_Extension, (PUCHAR)&SHA256, &FILE_UNIQUE_INDEX, FILE_DIR_METADATA);

							current_node->FILEs_of_Extension_list_current_node = current_node->FILEs_of_Extension_list_start_node;
						}
						else {
							current_node->FILEs_of_Extension_list_current_node = Append_Policy_Signature_files_Node(

								current_node->FILEs_of_Extension_list_current_node,
								INPUT_Extension,
								(PUCHAR)&SHA256,
								&FILE_UNIQUE_INDEX,
								FILE_DIR_METADATA
							);
						}

						if (File_Bin != NULL) ExFreePoolWithTag(File_Bin, 'FILE');
						break;
					case signature_NORMAL_Mode: // �Ϲ� + SHA256 ���Ե� ��� ���� 
						NULL;
						if (Option_OUTPUT_file_node != NULL) {

							*Option_OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node(

								current_node->FILEs_of_Extension_list_start_node,
								INPUT_Extension,
								NULL,
								Mode,
								&FILE_UNIQUE_INDEX
							);

						}



						break;

					case signature_COMPARE_Mode:
						NULL;
						if (INPUT_Extension == NULL) return FALSE;

						if (Option_OUTPUT_file_node != NULL) {
							*Option_OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node(

								current_node->FILEs_of_Extension_list_start_node,
								NULL,
								NULL,
								Mode,
								&FILE_UNIQUE_INDEX
							);
						}


						break;

					default:
						//if(File_Bin!= NULL) ExFreePoolWithTag(File_Bin, 'FILE');
						return FALSE;
					}


					//print_All_Policy_Signature_files_Node(current_node->FILEs_of_Extension_list_start_node);

					if (Option_OUTPUT_important_node != NULL) *Option_OUTPUT_important_node = current_node;


					
					return TRUE;
				}else{
					/*
						�ñ״�ó Ȯ���ڰ� �޶���� ����� ������.
					*/
					if (Mode == signature_reEDIT_about_file_extension) {
						// ����� IRP_MJ_SET_INFORMATIO���� ����ϴ� ENUM���̴�.

						// ���ϸ� ���ڴ� ��ȿ���� �ʰ�, ������ ������ ����̹Ƿ�, SHA256�� ����Ͽ� Node�� ã���ش�.
						if (Option_OUTPUT_file_node != NULL) {

							*Option_OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node(

								current_node->FILEs_of_Extension_list_start_node,
								NULL,
								Option_INPUT_SHA256,
								Mode,
								NULL // ���� ID �ʿ���� - SHA256�� �����
							);

							/*
								�Ϻη� FALSE�� �����Ͽ� Ȯ���� ������ �ν��ϵ�����.
								������ ȣ�⿵������ Option_OUTPUT_file_node�� Ȯ���ؾ���
							*/
							if (*Option_OUTPUT_file_node != NULL) {
								return FALSE;
							}
						}


					}
					else {
						if (Option_OUTPUT_file_node != NULL) {

							/* ���� ���� ID �������� */
							if (Get_FILE_INDEX_INFORMATION(INPUT_Extension, &FILE_UNIQUE_INDEX) != STATUS_SUCCESS || FILE_UNIQUE_INDEX == 0) {
								return FALSE;
							}

							*Option_OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node(

								current_node->FILEs_of_Extension_list_start_node,
								NULL,
								NULL,
								Mode,
								&FILE_UNIQUE_INDEX
							);

							/*
								�Ϻη� FALSE�� �����Ͽ� Ȯ���� ������ �ν��ϵ�����.
								������ ȣ�⿵������ Option_OUTPUT_file_node�� Ȯ���ؾ���
							*/
							if (*Option_OUTPUT_file_node != NULL) {
								return FALSE;
							}
						}
					}
					
				}
			}
		}
			
		
		
		
		current_node = (Ppolicy_signature_struct)current_node->Next_Node;

	} while (current_node != NULL);

	return FALSE;
}


Ppolicy_signature_struct Create_Policy_Signature_Node(PUCHAR Previcous_Node, PUNICODE_STRING Extension) { //1-1

	Ppolicy_signature_struct New_Node = ExAllocatePoolWithTag(NonPagedPool, sizeof(policy_signature_struct), 'PoSg');
	if (New_Node == NULL) return NULL;

	if (Previcous_Node == NULL) {
		New_Node->Previous_Node = NULL;
	}
	else {
		New_Node->Previous_Node = Previcous_Node;
	}

	New_Node->Extension.Length = (USHORT)wcslen(Extension->Buffer) * sizeof(WCHAR) ;
	New_Node->Extension.MaximumLength = (USHORT)(wcslen(Extension->Buffer)+1) * sizeof(WCHAR) ;
	New_Node->Extension.Buffer = ExAllocatePoolWithTag(NonPagedPool, New_Node->Extension.MaximumLength, 'PoSg');
	if (New_Node->Extension.Buffer == NULL) {
		ExFreePoolWithTag(New_Node, 'PoSg');
		return NULL;
	}

	RtlCopyMemory(New_Node->Extension.Buffer, Extension->Buffer, New_Node->Extension.Length);
	New_Node->Extension.Buffer[Extension->Length / sizeof(WCHAR)] = L'\0'; // NULL ���� ���� �߰�

	New_Node->FILEs_of_Extension_list_start_node = NULL;// ���Ḯ��Ʈ ( �ش� Ȯ���ڸ� ���� ���Ḯ��Ʈ 
	New_Node->FILEs_of_Extension_list_current_node = NULL;// ���Ḯ��Ʈ ( �ش� Ȯ���ڸ� ���� ���Ḯ��Ʈ 

	New_Node->Next_Node = NULL;

	return New_Node;
}

Ppolicy_signature_struct Append_Policy_Signature_Node(Ppolicy_signature_struct Node, PUNICODE_STRING Extension) { // 1-2
	
	Ppolicy_signature_struct New_Node = Create_Policy_Signature_Node((PUCHAR)Node, Extension);
	if (New_Node == NULL) return NULL;

	Node->Next_Node = (PUCHAR)New_Node;

	New_Node->Previous_Node = (PUCHAR)Node;
	
	return New_Node;
}

BOOLEAN Remove_All_Policy_Signature_Node() {// 1-3
	if (Policy_Signature_Start_Node == NULL) return FALSE;

	Ppolicy_signature_struct current_node = Policy_Signature_Start_Node;

	do {
		ExFreePoolWithTag((PVOID)current_node->Extension.Buffer, 'PoSg');

		Remove_All_Policy_Signature_files_Node((Ppolicy_signature_files_struct) current_node->FILEs_of_Extension_list_start_node);
		current_node->FILEs_of_Extension_list_start_node = NULL;
		current_node->FILEs_of_Extension_list_current_node = NULL;


		Ppolicy_signature_struct tmp_next_node = NULL;

		tmp_next_node = (Ppolicy_signature_struct)current_node->Next_Node;
		ExFreePoolWithTag((PVOID)current_node, 'PoSg');

		current_node = tmp_next_node;
	} while (current_node != NULL);

	Policy_Signature_Start_Node = NULL;
	Policy_Signature_Current_Node = NULL;

	return TRUE;

}

//BOOLEAN Specified_Node_Remover(PUCHAR Node, PWCH signature_name, ULONG32 signature_length); // 1-4



BOOLEAN Remove_Specified_Policy_Signature_files_Node(Ppolicy_signature_files_struct* Start_Node, Ppolicy_signature_files_struct* Current_Node,  Ppolicy_signature_files_struct Specified_Node) {
	if (Specified_Node == NULL || Start_Node == NULL || Current_Node == NULL || *Start_Node == NULL ) return FALSE;

	Specified_Node->Next_Node;
	Specified_Node->Previous_Node;

	if (Specified_Node->Previous_Node == NULL && Specified_Node->Next_Node == NULL) {
		/*
			���� ���Ḯ��Ʈ �ּҰ� ó�� �ּ��� ��, 
		*/
		if (*Start_Node == Specified_Node) {
			*Start_Node = (Ppolicy_signature_files_struct)Specified_Node->Next_Node;
		}

		if (*Start_Node == *Current_Node) *Current_Node = *Start_Node;


	} 
	else if (Specified_Node->Previous_Node == NULL && Specified_Node->Next_Node != NULL) {
		((Ppolicy_signature_files_struct)Specified_Node->Next_Node)->Previous_Node = NULL;
		if (*Start_Node == Specified_Node) {
			*Start_Node = (Ppolicy_signature_files_struct)Specified_Node->Next_Node;
		}

		if (*Start_Node == *Current_Node) *Current_Node = *Start_Node;
	}
	else if (Specified_Node->Previous_Node && Specified_Node->Next_Node) {
		/*
			��尡 �߰��� ���ִ� ���.
		*/
		((Ppolicy_signature_files_struct)Specified_Node->Next_Node)->Previous_Node = (PUCHAR)((Ppolicy_signature_files_struct)Specified_Node->Previous_Node);

	}
	else if (Specified_Node->Previous_Node && Specified_Node->Next_Node == NULL) {
		/*
			��� �������� ���, ( Head�ƴ� )
		*/
		((Ppolicy_signature_files_struct)Specified_Node->Previous_Node)->Next_Node = NULL;

		if (*Current_Node == Specified_Node) *Current_Node = (Ppolicy_signature_files_struct)Specified_Node->Previous_Node;

		
	}
	else {
		return FALSE;
	}

	ExFreePoolWithTag(Specified_Node->FULL_PATH.Buffer, 'PoSg');
	ExFreePoolWithTag(Specified_Node, 'PoSg');

	return TRUE;
}



VOID print_All_Policy_Signature_Node() {

	if (Policy_Signature_Start_Node == NULL) return;

	Ppolicy_signature_struct current_node = Policy_Signature_Start_Node;

	do {
		
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "list -> %wZ \n", current_node->Extension);

		Ppolicy_signature_files_struct current_FILES_list = current_node->FILEs_of_Extension_list_start_node;
		do {
			if (current_FILES_list == NULL) break;
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "current_FILES_list address -> %p @ PATHS-> %wZ \n", current_FILES_list, current_FILES_list->FULL_PATH);
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PATHS-> %wZ \n ����_����_�ð�: %llu ����_����������_�ð�: %llu ����_����������_�ð�: %llu ����_����_�ð�: %llu", 
				current_FILES_list, current_FILES_list->FILE_METADATA.CreationTime, current_FILES_list->FILE_METADATA.LastAccessTime, current_FILES_list->FILE_METADATA.LastWriteTime, current_FILES_list->FILE_METADATA.ChangeTime);

			current_FILES_list = (Ppolicy_signature_files_struct)current_FILES_list->Next_Node;
		} while (current_FILES_list != NULL);

		current_node = (Ppolicy_signature_struct)current_node->Next_Node;
	} while (current_node != NULL);

	return;

}


