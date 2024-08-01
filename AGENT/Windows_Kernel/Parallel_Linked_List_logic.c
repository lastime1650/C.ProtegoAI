#include "Parallel_Linked_List.h"

//PKMUTEX CreateAppendRemove__Parallel_Linked_List_KMutex = NULL;
PKMUTEX GetRemoveParallel_Linked_List_KMutex = NULL;

ULONG64 external_current_NODE_SECTION_INDEX = 0;
PDynamic_NODE external_start_node = NULL;
PDynamic_NODE external_current_node = NULL;


Util_Mutex_with_Lock CreateAppendRemove__Parallel_Linked_List_KMutex = { NULL,0 };


// 노드 총 관리 함수 (노드 최초생성 및 추가 2가지 가능 ) 
BOOLEAN Build_up_Node(
	PUCHAR DATA,
	ULONG32 DATA_SIZE,

	BOOLEAN is_init,
	PDynamic_NODE* output_Start_Node_of_NODE_SECTION, // index 섹션 노드들의 시작주소 

	ULONG32 Node_Search_VALUE

) {

	if (output_Start_Node_of_NODE_SECTION == NULL) return FALSE;
	

	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex,TRUE) == FALSE) return FALSE;

	if (external_current_node == NULL) {
		if (is_init) {
			external_current_NODE_SECTION_INDEX++;
			external_current_node = Create_Node(NULL, NULL, external_current_NODE_SECTION_INDEX, DATA, DATA_SIZE, Node_Search_VALUE);
			external_start_node = external_current_node;
		}
		else {
			external_current_node = Create_Node(NULL, NULL, ((PDynamic_NODE)*output_Start_Node_of_NODE_SECTION)->NODE_SECTION_INDEX, DATA, DATA_SIZE, Node_Search_VALUE);
			external_start_node = external_current_node;
		}

	}
	else {
		if (is_init) {
			external_current_NODE_SECTION_INDEX++;
			external_current_node = Append_Node(NULL, external_current_node, external_current_NODE_SECTION_INDEX, DATA, DATA_SIZE, Node_Search_VALUE);

		}
		else {
			external_current_node = Append_Node(((PDynamic_NODE)*output_Start_Node_of_NODE_SECTION)->NODE_SECTION_START_NODE_ADDRESS, external_current_node, ((PDynamic_NODE)*output_Start_Node_of_NODE_SECTION)->NODE_SECTION_INDEX, DATA, DATA_SIZE, Node_Search_VALUE);
		}
	}

	if (is_init) {
		// 최초 노드생성시에만 해당 SECTION의 노드 시작주소가 저장됨
		*output_Start_Node_of_NODE_SECTION = external_current_node;
		// 그리고 검증도 보장되도록, 다음 호출시, 시작노드값을 가져와서 필요한 정보를 가져와서 노드추가함
	}


	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);

	return TRUE;
}





////////////////////////////////////////////////

PDynamic_NODE Create_Node(PUCHAR SECTION_START_NODE_ADDRESS, PUCHAR Previous_Node, ULONG64 NODE_SECTION_INDEX, PUCHAR DATA, ULONG32 DATA_SIZE, ULONG32 Node_Search_VALUE) {
	
	PDynamic_NODE New_Node = (PDynamic_NODE)ExAllocatePoolWithTag(NonPagedPool,sizeof(Dynamic_NODE),'Para');
	if (New_Node == NULL) return NULL;
	memset(New_Node, 0, sizeof(Dynamic_NODE));

	/*
		항상 모든 노드들은 자신의 섹션에서 "시작노드" 주소 값을 가지고 있어야한다. (바뀌면 안되며, 일관성유지)
	*/
	if (SECTION_START_NODE_ADDRESS == NULL) {
		//최초 노드
		New_Node->NODE_SECTION_START_NODE_ADDRESS = (PUCHAR)New_Node; // 섹션 노드들의 시작 주소 저장
	}
	else {
		//Append 노드 
		New_Node->NODE_SECTION_START_NODE_ADDRESS = (PUCHAR)SECTION_START_NODE_ADDRESS;
	}

	New_Node->Previous_Node = Previous_Node;

	New_Node->NODE_SECTION_INDEX = NODE_SECTION_INDEX;

	// 데이터 새로저장
	New_Node->DATA = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, DATA_SIZE, 'Para');
	if (New_Node->DATA == NULL) {
		ExFreePoolWithTag(New_Node, 'Para');
		return NULL;
	}
	memcpy(New_Node->DATA, DATA, DATA_SIZE);

	New_Node->DATA_SIZE = DATA_SIZE;

	//
	New_Node->Node_Search_VALUE = Node_Search_VALUE;

	New_Node->is_end_node = TRUE; // 일단 마지막 노드일 수 밖에 없으니까,,

	New_Node->Next_Node = NULL;

	return New_Node;

}

PDynamic_NODE Append_Node(PUCHAR SECTION_START_NODE_ADDRESS, PDynamic_NODE NODE, ULONG64 NODE_SECTION_INDEX, PUCHAR DATA, ULONG32 DATA_SIZE, ULONG32 Node_Search_VALUE) {

	BOOLEAN is_SUCCESS = FALSE;
	PDynamic_NODE New_Node = NULL;

	// SECTION_START_NODE_ADDRESS 가 NULL이면, 전혀 새로운 노드의 특정 시작주소임 
	/*
		주의, NODE를 바로 참조하면 일관성 무너짐, 그렇기 때문에, 검증하여 얻어와야함
	*/
	if (SECTION_START_NODE_ADDRESS == NULL) {
		/*
			아예 새로운 노드(새로운 SECTION을 의미) 를 생성하는 것임
			전역적으로 생성되고있는 연결리스트안에서 지속적으로 추가하는 것이므로, Append_Node 함수안에서도 이러한 구현이 필요함.
		*/

		New_Node = Create_Node(SECTION_START_NODE_ADDRESS, (PUCHAR)NODE, NODE_SECTION_INDEX, DATA, DATA_SIZE, Node_Search_VALUE);//APPEND
		if (New_Node == NULL) return NULL;

		is_SUCCESS = TRUE;

	}
	else {
		/*
			이미 연관있는 노드가 존재할 때 처리하는 영역

			참조 기준은 SECTION_START_NODE_ADDRESS 노드 필드를 활용한다.

		*/
		PDynamic_NODE current = (PDynamic_NODE)SECTION_START_NODE_ADDRESS;//전역변수가 아닌, 섹션노드의 시작주소로 하면 비교적 빠름
		do {

			// 같은 섹션내에서만 비교하도록 함
			if (current->NODE_SECTION_INDEX == NODE_SECTION_INDEX && current->is_end_node == TRUE) {

				New_Node = Create_Node(SECTION_START_NODE_ADDRESS, (PUCHAR)NODE, NODE_SECTION_INDEX, DATA, DATA_SIZE, Node_Search_VALUE);//APPEND
				if (New_Node == NULL) return NULL;

				/*
					is_end_node 부울변수를 각각 업데이트한다.
				*/

				//이전의 부울을 FALSE로하여 "끝이 아님"을 설정
				current->is_end_node = FALSE;

				//상대 인덱스 값 (++)하여 추가하기
				New_Node->NODE_RELATION_INDEX = current->NODE_RELATION_INDEX + 1; // 상대 인덱스를 +1 증가함 

				is_SUCCESS = TRUE;

			}

			current = (PDynamic_NODE)(current->Next_Node);
		} while (current != NULL);

	}

	if (is_SUCCESS) {

		New_Node->Previous_Node = (PUCHAR)NODE;

		NODE->Next_Node = (PUCHAR)New_Node;

		return New_Node;
	}
	else {
		return NULL;
	}



}

VOID print_node() {
	if (external_start_node == NULL || external_current_node == NULL) return;

	PDynamic_NODE current_node = external_start_node;

	do {
		//printf("노드인덱스:[%d] /  노드 주소: %p  /  데이터사이즈: %d \n", current_node->NODE_SECTION_INDEX, current_node, current_node->DATA_SIZE);
		current_node = (PDynamic_NODE)current_node->Next_Node;
	} while (current_node != NULL);
	//printf("\n\n");
}

//

BOOLEAN Remove_Node_internal(PDynamic_NODE Specified_Node);

BOOLEAN Remove_Node(PDynamic_NODE NODE_SECTION_Start_Address) {

	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex, TRUE) == FALSE) return FALSE;

	print_node();
	PDynamic_NODE current_node = external_start_node;
	ULONG64 NODE_SECTION_INDEX = 0xFFFFFFFF;

	ULONG64 current_Maximum_of_NODE_SECTION_INDEX = 0;// 전체 Section노드 숫자중 가장 큰 값을 구하고, 거기에서 전역 섹션 번호 업데이트

	do {

		PDynamic_NODE tmp = (PDynamic_NODE)current_node->Next_Node;// 미리 next노드 기억.

		// 시작주소부터 얻어야 index얻음
		if ((PUCHAR)NODE_SECTION_Start_Address == (PUCHAR)current_node) {
			NODE_SECTION_INDEX = NODE_SECTION_Start_Address->NODE_SECTION_INDEX;
		}



		// index얻었을 때 노드 연쇄삭제 가능.
		if (NODE_SECTION_INDEX != 0xFFFFFFFF) {
			if (current_node->NODE_SECTION_INDEX == NODE_SECTION_INDEX) {
				/*
					이중 연결리스트 서로 연결해주고 할당해제하기.
				*/
				Remove_Node_internal(current_node);


			}
			else {
				/* NODE_SECTION_INDEX 파라미터 값보다 큰 경우 - 1 씩 하기.*/
				if (current_node->NODE_SECTION_INDEX > NODE_SECTION_INDEX) {
					current_node->NODE_SECTION_INDEX--;
				}
			}
			// 전역변수 - 노드 섹션 인덱스 전역변수 업데이트를 위한,
			if (current_Maximum_of_NODE_SECTION_INDEX < current_node->NODE_SECTION_INDEX) {
				current_Maximum_of_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}
		}

		


		current_node = tmp;
	} while (current_node != NULL);

	// 성공 검증
	if (NODE_SECTION_INDEX == 0xFFFFFFFF) {
		Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
		return FALSE; // index변화가 없으면 실패임.
	}

	// 섹션 번호 업데이트 
	external_current_NODE_SECTION_INDEX = current_Maximum_of_NODE_SECTION_INDEX;

	print_node();
	
	

	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
	return TRUE;
}

BOOLEAN Remove_1Dim_Node_with_Search_Value(PDynamic_NODE NODE_SECTION_Specified_Start_Address, PUCHAR compare_DATA, ULONG32 compare_DATA_Size) {
	/*
		노드 하나를 삭제하는 것이다.

		단 고려해야할 것.

		1. 해당 SECTIN_INDEX에 있는 값을 가지는 연결리스트에서,
			노드가 단 하나인 경우, 그 한 노드를 지우고, 노드의 INDEX보다 큰 값들은 모두 NODE_SECTION_INDEX를 (--) 처리해야한다.

	*/
	if (NODE_SECTION_Specified_Start_Address == NULL || external_start_node == NULL) return FALSE;

	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex, TRUE) == FALSE) return FALSE;
	
	//ULONG64 remember_NODE_SECTION_INDEX = 0xFFFFFFFF; // 확인용
	ULONG64 NODE_SECTION_INDEX = NODE_SECTION_Specified_Start_Address->NODE_SECTION_INDEX; // 색인

	ULONG32 current_node_count = 0;

	ULONG64 current_Maximum_of_NODE_SECTION_INDEX = 0;

	BOOLEAN status = FALSE;

	// 노드 갯수 얻기
	PDynamic_NODE current_node = external_start_node;
	do {

		if (NODE_SECTION_INDEX == current_node->NODE_SECTION_INDEX) {

			current_node_count++;
		}
	}while (current_node != NULL);

	if (current_node_count == 0) {
		Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
		return FALSE;
	}


	current_node = external_start_node;
	do {

		if (NODE_SECTION_INDEX == current_node->NODE_SECTION_INDEX) {


			if (compare_DATA_Size <= current_node->DATA_SIZE && memcmp(current_node->DATA, compare_DATA, compare_DATA_Size) == 0) {
				Remove_Node_internal(current_node);
				status = TRUE;

				current_node = (PDynamic_NODE)current_node->Next_Node;
				continue;
			}
			
		}
		
		// (--)
		if ( current_node_count <= 1) {
			if (NODE_SECTION_INDEX < current_node->NODE_SECTION_INDEX) {
				current_node->NODE_SECTION_INDEX--;
				current_Maximum_of_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}

			if (current_Maximum_of_NODE_SECTION_INDEX < current_node->NODE_SECTION_INDEX) {
				current_Maximum_of_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}
		}

		
		current_node = (PDynamic_NODE)current_node->Next_Node;

	} while (current_node != NULL);

	if ( current_node_count <= 1)
		external_current_NODE_SECTION_INDEX  = current_Maximum_of_NODE_SECTION_INDEX;

	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
	return status;
}



// 시작주소부터의 노드 모두 삭제 (Search_Value수치 값의 기준인, 2차원 방식)
BOOLEAN Remove_Node_with_Search_Value(PDynamic_NODE NODE_SECTION_Specified_Start_Address, ULONG32 Dir_Search_Value) {
	if (NODE_SECTION_Specified_Start_Address == NULL || Dir_Search_Value != 'Dirs' || external_start_node == NULL) return FALSE;

	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex, TRUE) == FALSE) return FALSE;
	
	// 전역변수를 시작 주소로, 
	PDynamic_NODE current_node = external_start_node;
	ULONG64 NODE_SECTION_INDEX = 0xFFFFFFFF; // 확인용
	ULONG32 NODE_SEARCH_VALUE = NODE_SECTION_Specified_Start_Address->Node_Search_VALUE; // 색인

	ULONG64 current_Maximum_of_NODE_SECTION_INDEX = 0;

	do {
		PDynamic_NODE tmp = (PDynamic_NODE)current_node->Next_Node;// 미리 next노드 기억.

		if (NODE_SECTION_INDEX == 0xFFFFFFFF && NODE_SEARCH_VALUE == current_node->Node_Search_VALUE) {
			//최초감지 
			NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
		}


		if (NODE_SECTION_INDEX != 0xFFFFFFFF ) {
			//이후감지
			ULONG64 tmp_NODE_SECTION_INDEX = 0;

			if (NODE_SEARCH_VALUE == current_node->Node_Search_VALUE) {
				/*
					이중 연결리스트 서로 연결해주고 할당해제하기.
				*/
				if (Remove_Node_internal(current_node) == FALSE) {
					Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
					return FALSE;
				}

				tmp_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}
			else if (current_node->NODE_SECTION_INDEX > NODE_SECTION_INDEX ) {
				// 상관없는 노드지만, INDEX가 더 크다면, -1씩 해야함
				current_node->NODE_SECTION_INDEX--;
				tmp_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}

			// 전역변수 - 노드 섹션 인덱스 전역변수 업데이트를 위한,
			if (current_Maximum_of_NODE_SECTION_INDEX < tmp_NODE_SECTION_INDEX) {
				current_Maximum_of_NODE_SECTION_INDEX = tmp_NODE_SECTION_INDEX;
			}
		}


		current_node = tmp;//미리 기억한 next노드주소 값 저장
	} while (current_node != NULL);

	external_current_NODE_SECTION_INDEX = current_Maximum_of_NODE_SECTION_INDEX;

	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
	if (NODE_SECTION_INDEX == 0xFFFFFFFF) return FALSE;
	return TRUE;
}

BOOLEAN Remove_Node_internal(PDynamic_NODE Specified_Node) {
	if ((external_start_node == NULL) || (external_current_node == NULL))return FALSE;

	if (Specified_Node->Previous_Node == NULL && Specified_Node->Next_Node == NULL) {
		external_start_node = NULL;
		external_current_node = NULL;
	}
	else if (Specified_Node->Previous_Node == NULL && Specified_Node->Next_Node) {
		((PDynamic_NODE)Specified_Node->Next_Node)->Previous_Node = NULL;
		external_start_node = (PDynamic_NODE)Specified_Node->Next_Node;
	}
	else if (Specified_Node->Previous_Node && Specified_Node->Next_Node == NULL) {
		((PDynamic_NODE)Specified_Node->Previous_Node)->Next_Node = NULL;
		external_current_node = (PDynamic_NODE)Specified_Node->Previous_Node;
	}
	else if (Specified_Node->Previous_Node && Specified_Node->Next_Node) {
		((PDynamic_NODE)Specified_Node->Previous_Node)->Next_Node = Specified_Node->Next_Node;
		((PDynamic_NODE)Specified_Node->Next_Node)->Previous_Node = Specified_Node->Previous_Node;
	}

	ExFreePoolWithTag(Specified_Node->DATA, 'Para'); 
	ExFreePoolWithTag(Specified_Node, 'Para'); 

	return TRUE;
}


BOOLEAN is_valid_address(PDynamic_NODE NODE_SECTION_Start_Address) {
	if (external_start_node == NULL || external_current_node == NULL) return FALSE;

	PDynamic_NODE current_node = external_start_node;

	do {

		if (NODE_SECTION_Start_Address == current_node) {
			return TRUE;
		}

		current_node = (PDynamic_NODE)current_node->Next_Node;
	} while (current_node != NULL);

	return FALSE;
}

////////////////////////////////////////////////////////////

PDynamic_NODE Get_Node_1Dim(
	PDynamic_NODE NODE_SECTION_Start_Address,
	ULONG32 node_count_for_field
) {// 노드 가져오기 ( count )

	if (external_start_node == NULL || external_current_node == NULL) return FALSE;
	
	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex, TRUE) == FALSE) return FALSE;

	PDynamic_NODE current_node = external_start_node;
	BOOLEAN is_found_start_node = FALSE;


	ULONG64 remember_NODE_SECTION_INDEX = 0xFFFFFFFF;
	ULONG32 count = 0;

	do {

		if (NODE_SECTION_Start_Address == current_node) {
			is_found_start_node = TRUE;
			remember_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
		}

		if (is_found_start_node && remember_NODE_SECTION_INDEX != 0xFFFFFFFF && current_node->NODE_SECTION_INDEX == remember_NODE_SECTION_INDEX) {
			//printf("node_count_for_field -> %d / %d \n", node_count_for_field, count);
			if (node_count_for_field == count) {
				Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
				return current_node; // 노드 추출 
			}
			count++;
		}

		current_node = (PDynamic_NODE)current_node->Next_Node;
	} while (current_node != NULL);
	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
	return NULL;
}


PDynamic_NODE Get_Node_memcmp_1Dim(
	PDynamic_NODE NODE_SECTION_Start_Address,
	PUCHAR DATA,
	ULONG32 DATA_SIZE
) {

	if (external_start_node == NULL || external_current_node == NULL) return FALSE;
	
	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex, TRUE) == FALSE) return FALSE;

	PDynamic_NODE current_node = external_start_node;
	BOOLEAN is_found_start_node = FALSE;


	ULONG64 remember_NODE_SECTION_INDEX = 0xFFFFFFFF;

	do {

		if (NODE_SECTION_Start_Address == current_node) {
			is_found_start_node = TRUE;
			remember_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
		}

		if (is_found_start_node && remember_NODE_SECTION_INDEX != 0xFFFFFFFF && current_node->NODE_SECTION_INDEX == remember_NODE_SECTION_INDEX) {

			if (current_node->DATA_SIZE >= DATA_SIZE && memcmp(DATA, current_node->DATA, DATA_SIZE) == 0) {
				Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
				return current_node; // 노드 추출 
			}
		}

		current_node = (PDynamic_NODE)current_node->Next_Node;
	} while (current_node != NULL);
	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
	return NULL;
}

/////////////////////////////////////////////////////////


PDynamic_NODE Get_Node_2Dim(
	ULONG32 Node_Search_VALUE,
	PUCHAR DATA,
	ULONG32 DATA_SIZE,

	PDynamic_NODE* OUTPUT_SECTION_START_ADDRESS
) {
	if (external_start_node == NULL || external_current_node == NULL || OUTPUT_SECTION_START_ADDRESS == NULL) return NULL;

	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex, TRUE) == FALSE) return FALSE;

	PDynamic_NODE current_node = external_start_node;


	do {

		if (Node_Search_VALUE == current_node->Node_Search_VALUE) {
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " -- Parelle -- ((PActionProcessNode)current_node->DATA)->SHA256:: %s / %s \n", ((PActionProcessNode)current_node->DATA)->SHA256, ((PActionProcessNode)DATA)->SHA256 );
			if ((current_node->DATA_SIZE >= DATA_SIZE) && memcmp(current_node->DATA, DATA, DATA_SIZE) == 0) {
				/*
				
					일치할 때,,
						1. Return
							> 현재 감지된 노드 주소
						2. Output - PUCHAR
							> 현재 감지된 노드 섹션의 "시작주소" 

				*/

				*OUTPUT_SECTION_START_ADDRESS = (PDynamic_NODE)current_node->NODE_SECTION_START_NODE_ADDRESS; 
				Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
				return current_node;
			}

		}

		current_node = (PDynamic_NODE)current_node->Next_Node;
	} while (current_node != NULL);
	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
	return NULL;
}

PDynamic_NODE Get_Node_2Dim_with_Relation_Index(
	ULONG32 Node_Search_VALUE, // 시작점 
	PUCHAR DATA,
	ULONG32 DATA_SIZE,

	PDynamic_NODE* OUTPUT_SECTION_START_ADDRESS,
	ULONG64 Relation_index
) {

	if (external_start_node == NULL || external_current_node == NULL || OUTPUT_SECTION_START_ADDRESS == NULL) return NULL;

	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex, TRUE) == FALSE) return FALSE;

	PDynamic_NODE current_node = external_start_node;
	ULONG64 remember_NODE_SECTION_INDEX = 0xFFFFFFFF;

	do {
		if (Node_Search_VALUE == current_node->Node_Search_VALUE) {

			// 최초 섹션 -> 인덱스 취득
			if (remember_NODE_SECTION_INDEX == 0xFFFFFFFF) {
				remember_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}

			// 섹션 인덱스 성공적으로 얻었을 때, (같은 Search_Value일 때 작동됨)
			if (remember_NODE_SECTION_INDEX != 0xFFFFFFFF) {

				// 같은 섹션 인덱스 내에서 순환하도록 함 
				PDynamic_NODE detected_section_current_node = current_node;
				do {

					if (remember_NODE_SECTION_INDEX == detected_section_current_node->NODE_SECTION_INDEX) {
						if (detected_section_current_node->is_end_node) {
							break;
						}
						else {
							if (Relation_index == detected_section_current_node->NODE_RELATION_INDEX) {
								/*
									Relation_Index 같을 때 비교할 수 있음
								*/
								if (detected_section_current_node->DATA_SIZE >= DATA_SIZE) {
									if (memcmp(DATA, detected_section_current_node->DATA, DATA_SIZE) == 0) {
										*OUTPUT_SECTION_START_ADDRESS = (PDynamic_NODE)detected_section_current_node->NODE_SECTION_START_NODE_ADDRESS;
										Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
										return detected_section_current_node;
									}
								}

							}
						}
					}

					detected_section_current_node = (PDynamic_NODE)detected_section_current_node->Next_Node;
				} while (detected_section_current_node != NULL);

				remember_NODE_SECTION_INDEX = 0xFFFFFFFF;

				current_node = detected_section_current_node; // 다음 섹션으로 바로 건너뛰도록함. 
			}


		}
		current_node = (PDynamic_NODE)current_node->Next_Node;
	} while (current_node != NULL);

	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
	return NULL;

}