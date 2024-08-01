#include "policy_signature_list_manager.h"

Ppolicy_signature_struct Policy_Signature_Start_Node = NULL;
Ppolicy_signature_struct Policy_Signature_Current_Node = NULL;

KMUTEX MUTEX_signature = { 0, };

// 유니코드 문자열 맨 뒷자리 기준으로 일치한지 
BOOLEAN DoesEndWith(PUNICODE_STRING A, PUNICODE_STRING B) {
	if (A->Length < B->Length) {
		return FALSE; // A의 길이가 B보다 짧으면 일치할 수 없음
	}

	UNICODE_STRING subString;
	subString.Buffer = A->Buffer + ( (A->Length - B->Length) / sizeof(WCHAR) );
	subString.Length = B->Length;
	subString.MaximumLength = B->Length;

	return RtlEqualUnicodeString(&subString, B, TRUE);
}

// [ 2. 시그니처 매니저 ]
/*
BOOLEAN Policy_Signature_Register_Remove(PWCH data, BOOLEAN is_register) { // 2-1
	

	if (is_register) {
		if (Policy_Signature_Start_Node == NULL) {


			Policy_Signature_Start_Node = Create_Policy_Signature_Node(NULL, data, (ULONG32)(wcslen(data) * sizeof(WCHAR)));
			Policy_Signature_Current_Node = Policy_Signature_Start_Node;

		}
		else {
			/ 중복 검사하고 넣어야함 
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
	New_Node->FULL_PATH.Buffer[FULL_PATH->Length / sizeof(WCHAR)] = L'\0'; // NULL 종단 문자 추가

	RtlCopyMemory(New_Node->SHA256, SHA256_p, SHA256_String_Byte_Length); // SHA256 저장

	// 파일 메타데이터 추가
	if (FILE_METADATA != NULL) {
		New_Node->FILE_METADATA.CreationTime = FILE_METADATA->CreationTime;
		New_Node->FILE_METADATA.LastAccessTime = FILE_METADATA->LastAccessTime;
		New_Node->FILE_METADATA.LastWriteTime = FILE_METADATA->LastWriteTime;
		New_Node->FILE_METADATA.ChangeTime = FILE_METADATA->ChangeTime;
	}
	



	New_Node->FILE_unique_index = *FILE_UNIQUE_INDEX; // 파일 시스템에서의 고유 파일 번호

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


// hint를 얻어서 특정 1개 노드를 반환
Ppolicy_signature_files_struct Get_or_Set_policy_signature_files_Specified_Node(Ppolicy_signature_files_struct Start_NODE , PUNICODE_STRING Option_INPUT_FULL_PATH, PUCHAR Option_INPUT_SHA256, policy_signature_struct_ENUM Mode, ULONG64* File_unique_index) {
	
	
	if (Start_NODE == NULL) return NULL;

	Ppolicy_signature_files_struct current_node = Start_NODE;

	do {

		// 특수 필터링
		if (Mode == signature_reEDIT_about_file_extension) {
			/*
				IRP_MJ_SET_INFORMATION에서 << 확장자 변경 >> 을 탐지용으로 특수 영역임.

				SHA256으로 노드를 반환한다. 

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
			파일 ID INDEX값이 같아야함 
		*/
		if (current_node->FILE_unique_index == *File_unique_index) {

			if (Option_INPUT_FULL_PATH != NULL && Mode != signature_set_file_node_with__SHA256__but_idc_about_signature) {
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RtlEqualUnicodeString ( %wZ vs %wZ ) %d \n", *Option_INPUT_FULL_PATH, current_node->FULL_PATH, Mode);


				if (Mode == signature_set_file_node_with__FULL_PATH__but_idc_about_signature || Mode == signature_set_file_node_with__FULL_PATH__and__SHA256_but_idc_about_signature) {

					// SET
					/*
						FULL_PATH를 변경할 것임
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
					current_node->FULL_PATH.Buffer[Option_INPUT_FULL_PATH->Length / sizeof(WCHAR)] = L'\0'; // NULL 종단 문자 추가

					ExFreePoolWithTag(tmp_remember, 'PoSg');
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [SET] 파일명 변경완료 -> %wZ\n ", current_node->FULL_PATH);
				}
				else {


					// GET
					if (RtlEqualUnicodeString(Option_INPUT_FULL_PATH, &current_node->FULL_PATH, TRUE)) {
						DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\nGet_policy_signature_files_Specified_Node -> %wZ 일치함!! \n", current_node->FULL_PATH);
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
						SHA256값을 수정할 것임
					*/
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [ SET ] SHA256 수정 \n");
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

			return current_node; // 어차피 맨 마지막 파일 ID 는 같았으므로 리턴함 ( 단 로직에 따라 예외가 있음 )
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
	제일 중요한 함수임 

	파라미터 Mode 인자에 따라 실제 파일 + SHA256 노드를 추가할 수 있고,

	일치하였을 때의 노드를 가져오도록 할 수 있음

*/
BOOLEAN Policy_Signature_Compare(PUCHAR Start_Node, PUNICODE_STRING INPUT_Extension, PUCHAR Option_INPUT_SHA256, Ppolicy_signature_struct* Option_OUTPUT_important_node, policy_signature_struct_ENUM Mode, Ppolicy_signature_files_struct* Option_OUTPUT_file_node, PFile_Dir_METADATA FILE_DIR_METADATA) { // 2-2
	
	// Start_Node == Policy_Signature_Start_Node ! 

	if (Policy_Signature_Start_Node == NULL ) return FALSE;

	ULONG64 FILE_UNIQUE_INDEX = 0;


	

	Ppolicy_signature_struct current_node = (Ppolicy_signature_struct)Start_Node;

	do {

		
		if (Mode == signature_get_file_node_with__SHA256___but_idc_about_signature) {
			/*
				호출자가 SHA256데이터만 가지고 SHA256 노드 가지고 오려고 함
			*/
			/* 파일 고유 ID 가져오기 */
			if (Get_FILE_INDEX_INFORMATION(INPUT_Extension, &FILE_UNIQUE_INDEX) != STATUS_SUCCESS || FILE_UNIQUE_INDEX == 0) {
				return FALSE;
			}


			if (Option_INPUT_SHA256 == NULL || Option_OUTPUT_file_node == NULL) return FALSE; // 이 Mode에 필요한 인자값이 NULL이면 FALSE 

			if (current_node->FILEs_of_Extension_list_start_node == NULL) return FALSE; // 시그니처 탐지된 파일 연결리스트 시작 주소가 NULL이면 FALSE


			*Option_OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node( // 여기서 포인터 값을 가져오면 SHA256 일치한 것임 !!!!

				current_node->FILEs_of_Extension_list_start_node,
				NULL,
				Option_INPUT_SHA256,
				Mode,
				&FILE_UNIQUE_INDEX
			);

			if (Option_OUTPUT_important_node != NULL) *Option_OUTPUT_important_node = current_node; // Option 처리 

			if (*Option_OUTPUT_file_node != NULL) { // SHA256 파일 노드 주소 가져오면 정상처리 
				return TRUE;
			}




		}
		else if (Mode == signature_get_file_node_with__FULL_PATH___but_idc_about_signature) {
			/*
				호출자가 PUNICODE_STRIN input 데이터만 가지고 파일  노드 가지고 오려고 함
			*/
			/* 파일 고유 ID 가져오기 */
			if (Get_FILE_INDEX_INFORMATION(INPUT_Extension, &FILE_UNIQUE_INDEX) != STATUS_SUCCESS || FILE_UNIQUE_INDEX == 0) {
				return FALSE;
			}

			if (INPUT_Extension == NULL || Option_OUTPUT_file_node == NULL) return FALSE; // 이 Mode에 필요한 인자값이 NULL이면 FALSE 

			if (current_node->FILEs_of_Extension_list_start_node == NULL) return FALSE; // 시그니처 탐지된 파일 연결리스트 시작 주소가 NULL이면 FALSE


			*Option_OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node( // 여기서 포인터 값을 가져오면 SHA256 일치한 것임 !!!!

				current_node->FILEs_of_Extension_list_start_node,
				INPUT_Extension,
				NULL,
				Mode,
				&FILE_UNIQUE_INDEX

			);

			if (Option_OUTPUT_important_node != NULL) *Option_OUTPUT_important_node = current_node; // Option 처리 

			if (*Option_OUTPUT_file_node != NULL) { // SHA256 파일 노드 주소 가져오면 정상처리 
				return TRUE;
			}
		}
		else if (Mode == signature_set_file_node_with__SHA256__but_idc_about_signature || Mode == signature_set_file_node_with__FULL_PATH__but_idc_about_signature || Mode == signature_set_file_node_with__FULL_PATH__and__SHA256_but_idc_about_signature) {
			/*
				호출자가 특정 시그니처 노드가 가진 2차원 연결리스트에 대하여 수정을 하는 것 
				
				파라미터가 NULL이면 자동으로 수정안함. 정상적인 파라미터의 경우에만 처리함

			*/
			/* 파일 고유 ID 가져오기 */
			if (Get_FILE_INDEX_INFORMATION(INPUT_Extension, &FILE_UNIQUE_INDEX) != STATUS_SUCCESS || FILE_UNIQUE_INDEX == 0) {
				return FALSE;
			}


			if (current_node->FILEs_of_Extension_list_start_node == NULL) return FALSE; // 시그니처 탐지된 파일 연결리스트 시작 주소가 NULL이면 FALSE
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [ SET ] 도입 \n ");
			
			if (Mode == signature_set_file_node_with__FULL_PATH__but_idc_about_signature) {
				Get_or_Set_policy_signature_files_Specified_Node( // 여기서 포인터 값을 가져오면 SHA256 일치한 것임 !!!!

					current_node->FILEs_of_Extension_list_start_node,
					INPUT_Extension,
					NULL,
					Mode,
					&FILE_UNIQUE_INDEX

				);
			}
			else if (Mode == signature_set_file_node_with__SHA256__but_idc_about_signature) {
				Get_or_Set_policy_signature_files_Specified_Node( // 여기서 포인터 값을 가져오면 SHA256 일치한 것임 !!!!

					current_node->FILEs_of_Extension_list_start_node,
					NULL,
					Option_INPUT_SHA256,
					Mode,
					&FILE_UNIQUE_INDEX

				);
			}
			else {
				Get_or_Set_policy_signature_files_Specified_Node( // 여기서 포인터 값을 가져오면 SHA256 일치한 것임 !!!!

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
				초기 생성 SAVE
			*/
			if (INPUT_Extension == NULL) return FALSE;

			if (wcslen(INPUT_Extension->Buffer) >= wcslen(current_node->Extension.Buffer)) {

				if (DoesEndWith(INPUT_Extension, &current_node->Extension)) {
					/* 일치 */
	
					/* 파일 고유 ID 가져오기 */
					if (Get_FILE_INDEX_INFORMATION(INPUT_Extension, &FILE_UNIQUE_INDEX) != STATUS_SUCCESS || FILE_UNIQUE_INDEX == 0) {
						return FALSE;
					}

					
					


					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 시그니처 일치합니다. ==> %wZ  , 고유아이디: %llu \n", *INPUT_Extension, FILE_UNIQUE_INDEX);

					
					



					switch (Mode) {
					case signature_SAVE_Mode: // 노드 축적
						NULL;

						/* 파일 연결리스트에서 중복이 있는 경우 저장하지 않음.  */

						if (current_node->FILEs_of_Extension_list_start_node != NULL) {
							Ppolicy_signature_files_struct OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node( // 여기서 포인터 값을 가져오면 SHA256 일치한 것임 !!!!

								current_node->FILEs_of_Extension_list_start_node,
								INPUT_Extension,
								NULL,
								Mode,
								&FILE_UNIQUE_INDEX

							);

							if (OUTPUT_file_node) {
								/* 중복이므로 저장안함. */
								DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "_[Ppolicy_signature_files_struct]중복저장 확인됨, 저장실패_\n");
								return FALSE;

								// 파일 정보 노드 삭제 수행
								//if (Remove_Specified_Policy_Signature_files_Node(&current_node->FILEs_of_Extension_list_start_node, &current_node->FILEs_of_Extension_list_current_node, OUTPUT_file_node)) {
								//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " signature_REMOVE_file_node_with_FULL_PATH 삭제 성공 \n");
								//	return TRUE;
								//}
								//else {
								//	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " signature_REMOVE_file_node_with_FULL_PATH 삭제 실패 \n");
								//	return FALSE;
								//}
							}
							else {
								/* 중복아님 확인 */
							}
						}

						/* 해시 구하기  인자에 있으면 직접 안구해도됨 */
						UCHAR SHA256[SHA256_String_Byte_Length] = { 0, };

						PVOID File_Bin = NULL;
						ULONG File_Bin_Length = 0;

						if (Option_INPUT_SHA256 == NULL) {
							/* 파일 바이너리 구하기 */

							if (ALL_in_ONE_FILE_IO(&File_Bin, &File_Bin_Length, *INPUT_Extension, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) return FALSE;

							/* 파일  해시 구하기 */

							if (GET_SHA256_HASHING(File_Bin, File_Bin_Length, (PCHAR)&SHA256) != STATUS_SUCCESS) {
								ExFreePoolWithTag(File_Bin, 'FILE');
								return FALSE;
							}
						}
						else {
							/* 이미 인자에 SHA256값이 준비된 경우 */
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
					case signature_NORMAL_Mode: // 일반 + SHA256 포함된 노드 추출 
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
						시그니처 확장자가 달라버린 경우의 영역임.
					*/
					if (Mode == signature_reEDIT_about_file_extension) {
						// 여기는 IRP_MJ_SET_INFORMATIO에서 사용하는 ENUM값이다.

						// 파일명 인자는 유효하지 않고, 앞으로 수정될 경로이므로, SHA256이 대신하여 Node를 찾아준다.
						if (Option_OUTPUT_file_node != NULL) {

							*Option_OUTPUT_file_node = Get_or_Set_policy_signature_files_Specified_Node(

								current_node->FILEs_of_Extension_list_start_node,
								NULL,
								Option_INPUT_SHA256,
								Mode,
								NULL // 파일 ID 필요없음 - SHA256이 대신함
							);

							/*
								일부러 FALSE로 리턴하여 확장자 변경을 인식하도록함.
								무조건 호출영역에서 Option_OUTPUT_file_node을 확인해야함
							*/
							if (*Option_OUTPUT_file_node != NULL) {
								return FALSE;
							}
						}


					}
					else {
						if (Option_OUTPUT_file_node != NULL) {

							/* 파일 고유 ID 가져오기 */
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
								일부러 FALSE로 리턴하여 확장자 변경을 인식하도록함.
								무조건 호출영역에서 Option_OUTPUT_file_node을 확인해야함
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
	New_Node->Extension.Buffer[Extension->Length / sizeof(WCHAR)] = L'\0'; // NULL 종단 문자 추가

	New_Node->FILEs_of_Extension_list_start_node = NULL;// 연결리스트 ( 해당 확장자를 가진 연결리스트 
	New_Node->FILEs_of_Extension_list_current_node = NULL;// 연결리스트 ( 해당 확장자를 가진 연결리스트 

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
			전역 연결리스트 주소가 처음 주소일 때, 
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
			노드가 중간에 껴있는 경우.
		*/
		((Ppolicy_signature_files_struct)Specified_Node->Next_Node)->Previous_Node = (PUCHAR)((Ppolicy_signature_files_struct)Specified_Node->Previous_Node);

	}
	else if (Specified_Node->Previous_Node && Specified_Node->Next_Node == NULL) {
		/*
			노드 마지막인 경우, ( Head아님 )
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
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PATHS-> %wZ \n 파일_생성_시간: %llu 파일_마지막접근_시간: %llu 파일_마지막쓰기_시간: %llu 파일_수정_시간: %llu", 
				current_FILES_list, current_FILES_list->FILE_METADATA.CreationTime, current_FILES_list->FILE_METADATA.LastAccessTime, current_FILES_list->FILE_METADATA.LastWriteTime, current_FILES_list->FILE_METADATA.ChangeTime);

			current_FILES_list = (Ppolicy_signature_files_struct)current_FILES_list->Next_Node;
		} while (current_FILES_list != NULL);

		current_node = (Ppolicy_signature_struct)current_node->Next_Node;
	} while (current_node != NULL);

	return;

}


