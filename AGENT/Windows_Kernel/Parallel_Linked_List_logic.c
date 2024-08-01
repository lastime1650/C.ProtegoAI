#include "Parallel_Linked_List.h"

//PKMUTEX CreateAppendRemove__Parallel_Linked_List_KMutex = NULL;
PKMUTEX GetRemoveParallel_Linked_List_KMutex = NULL;

ULONG64 external_current_NODE_SECTION_INDEX = 0;
PDynamic_NODE external_start_node = NULL;
PDynamic_NODE external_current_node = NULL;


Util_Mutex_with_Lock CreateAppendRemove__Parallel_Linked_List_KMutex = { NULL,0 };


// ��� �� ���� �Լ� (��� ���ʻ��� �� �߰� 2���� ���� ) 
BOOLEAN Build_up_Node(
	PUCHAR DATA,
	ULONG32 DATA_SIZE,

	BOOLEAN is_init,
	PDynamic_NODE* output_Start_Node_of_NODE_SECTION, // index ���� ������ �����ּ� 

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
		// ���� �������ÿ��� �ش� SECTION�� ��� �����ּҰ� �����
		*output_Start_Node_of_NODE_SECTION = external_current_node;
		// �׸��� ������ ����ǵ���, ���� ȣ���, ���۳�尪�� �����ͼ� �ʿ��� ������ �����ͼ� ����߰���
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
		�׻� ��� ������ �ڽ��� ���ǿ��� "���۳��" �ּ� ���� ������ �־���Ѵ�. (�ٲ�� �ȵǸ�, �ϰ�������)
	*/
	if (SECTION_START_NODE_ADDRESS == NULL) {
		//���� ���
		New_Node->NODE_SECTION_START_NODE_ADDRESS = (PUCHAR)New_Node; // ���� ������ ���� �ּ� ����
	}
	else {
		//Append ��� 
		New_Node->NODE_SECTION_START_NODE_ADDRESS = (PUCHAR)SECTION_START_NODE_ADDRESS;
	}

	New_Node->Previous_Node = Previous_Node;

	New_Node->NODE_SECTION_INDEX = NODE_SECTION_INDEX;

	// ������ ��������
	New_Node->DATA = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, DATA_SIZE, 'Para');
	if (New_Node->DATA == NULL) {
		ExFreePoolWithTag(New_Node, 'Para');
		return NULL;
	}
	memcpy(New_Node->DATA, DATA, DATA_SIZE);

	New_Node->DATA_SIZE = DATA_SIZE;

	//
	New_Node->Node_Search_VALUE = Node_Search_VALUE;

	New_Node->is_end_node = TRUE; // �ϴ� ������ ����� �� �ۿ� �����ϱ�,,

	New_Node->Next_Node = NULL;

	return New_Node;

}

PDynamic_NODE Append_Node(PUCHAR SECTION_START_NODE_ADDRESS, PDynamic_NODE NODE, ULONG64 NODE_SECTION_INDEX, PUCHAR DATA, ULONG32 DATA_SIZE, ULONG32 Node_Search_VALUE) {

	BOOLEAN is_SUCCESS = FALSE;
	PDynamic_NODE New_Node = NULL;

	// SECTION_START_NODE_ADDRESS �� NULL�̸�, ���� ���ο� ����� Ư�� �����ּ��� 
	/*
		����, NODE�� �ٷ� �����ϸ� �ϰ��� ������, �׷��� ������, �����Ͽ� ���;���
	*/
	if (SECTION_START_NODE_ADDRESS == NULL) {
		/*
			�ƿ� ���ο� ���(���ο� SECTION�� �ǹ�) �� �����ϴ� ����
			���������� �����ǰ��ִ� ���Ḯ��Ʈ�ȿ��� ���������� �߰��ϴ� ���̹Ƿ�, Append_Node �Լ��ȿ����� �̷��� ������ �ʿ���.
		*/

		New_Node = Create_Node(SECTION_START_NODE_ADDRESS, (PUCHAR)NODE, NODE_SECTION_INDEX, DATA, DATA_SIZE, Node_Search_VALUE);//APPEND
		if (New_Node == NULL) return NULL;

		is_SUCCESS = TRUE;

	}
	else {
		/*
			�̹� �����ִ� ��尡 ������ �� ó���ϴ� ����

			���� ������ SECTION_START_NODE_ADDRESS ��� �ʵ带 Ȱ���Ѵ�.

		*/
		PDynamic_NODE current = (PDynamic_NODE)SECTION_START_NODE_ADDRESS;//���������� �ƴ�, ���ǳ���� �����ּҷ� �ϸ� ���� ����
		do {

			// ���� ���ǳ������� ���ϵ��� ��
			if (current->NODE_SECTION_INDEX == NODE_SECTION_INDEX && current->is_end_node == TRUE) {

				New_Node = Create_Node(SECTION_START_NODE_ADDRESS, (PUCHAR)NODE, NODE_SECTION_INDEX, DATA, DATA_SIZE, Node_Search_VALUE);//APPEND
				if (New_Node == NULL) return NULL;

				/*
					is_end_node �οﺯ���� ���� ������Ʈ�Ѵ�.
				*/

				//������ �ο��� FALSE���Ͽ� "���� �ƴ�"�� ����
				current->is_end_node = FALSE;

				//��� �ε��� �� (++)�Ͽ� �߰��ϱ�
				New_Node->NODE_RELATION_INDEX = current->NODE_RELATION_INDEX + 1; // ��� �ε����� +1 ������ 

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
		//printf("����ε���:[%d] /  ��� �ּ�: %p  /  �����ͻ�����: %d \n", current_node->NODE_SECTION_INDEX, current_node, current_node->DATA_SIZE);
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

	ULONG64 current_Maximum_of_NODE_SECTION_INDEX = 0;// ��ü Section��� ������ ���� ū ���� ���ϰ�, �ű⿡�� ���� ���� ��ȣ ������Ʈ

	do {

		PDynamic_NODE tmp = (PDynamic_NODE)current_node->Next_Node;// �̸� next��� ���.

		// �����ּҺ��� ���� index����
		if ((PUCHAR)NODE_SECTION_Start_Address == (PUCHAR)current_node) {
			NODE_SECTION_INDEX = NODE_SECTION_Start_Address->NODE_SECTION_INDEX;
		}



		// index����� �� ��� ������� ����.
		if (NODE_SECTION_INDEX != 0xFFFFFFFF) {
			if (current_node->NODE_SECTION_INDEX == NODE_SECTION_INDEX) {
				/*
					���� ���Ḯ��Ʈ ���� �������ְ� �Ҵ������ϱ�.
				*/
				Remove_Node_internal(current_node);


			}
			else {
				/* NODE_SECTION_INDEX �Ķ���� ������ ū ��� - 1 �� �ϱ�.*/
				if (current_node->NODE_SECTION_INDEX > NODE_SECTION_INDEX) {
					current_node->NODE_SECTION_INDEX--;
				}
			}
			// �������� - ��� ���� �ε��� �������� ������Ʈ�� ����,
			if (current_Maximum_of_NODE_SECTION_INDEX < current_node->NODE_SECTION_INDEX) {
				current_Maximum_of_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}
		}

		


		current_node = tmp;
	} while (current_node != NULL);

	// ���� ����
	if (NODE_SECTION_INDEX == 0xFFFFFFFF) {
		Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
		return FALSE; // index��ȭ�� ������ ������.
	}

	// ���� ��ȣ ������Ʈ 
	external_current_NODE_SECTION_INDEX = current_Maximum_of_NODE_SECTION_INDEX;

	print_node();
	
	

	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
	return TRUE;
}

BOOLEAN Remove_1Dim_Node_with_Search_Value(PDynamic_NODE NODE_SECTION_Specified_Start_Address, PUCHAR compare_DATA, ULONG32 compare_DATA_Size) {
	/*
		��� �ϳ��� �����ϴ� ���̴�.

		�� ����ؾ��� ��.

		1. �ش� SECTIN_INDEX�� �ִ� ���� ������ ���Ḯ��Ʈ����,
			��尡 �� �ϳ��� ���, �� �� ��带 �����, ����� INDEX���� ū ������ ��� NODE_SECTION_INDEX�� (--) ó���ؾ��Ѵ�.

	*/
	if (NODE_SECTION_Specified_Start_Address == NULL || external_start_node == NULL) return FALSE;

	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex, TRUE) == FALSE) return FALSE;
	
	//ULONG64 remember_NODE_SECTION_INDEX = 0xFFFFFFFF; // Ȯ�ο�
	ULONG64 NODE_SECTION_INDEX = NODE_SECTION_Specified_Start_Address->NODE_SECTION_INDEX; // ����

	ULONG32 current_node_count = 0;

	ULONG64 current_Maximum_of_NODE_SECTION_INDEX = 0;

	BOOLEAN status = FALSE;

	// ��� ���� ���
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



// �����ּҺ����� ��� ��� ���� (Search_Value��ġ ���� ������, 2���� ���)
BOOLEAN Remove_Node_with_Search_Value(PDynamic_NODE NODE_SECTION_Specified_Start_Address, ULONG32 Dir_Search_Value) {
	if (NODE_SECTION_Specified_Start_Address == NULL || Dir_Search_Value != 'Dirs' || external_start_node == NULL) return FALSE;

	if (Initilize_or_Locking_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex, TRUE) == FALSE) return FALSE;
	
	// ���������� ���� �ּҷ�, 
	PDynamic_NODE current_node = external_start_node;
	ULONG64 NODE_SECTION_INDEX = 0xFFFFFFFF; // Ȯ�ο�
	ULONG32 NODE_SEARCH_VALUE = NODE_SECTION_Specified_Start_Address->Node_Search_VALUE; // ����

	ULONG64 current_Maximum_of_NODE_SECTION_INDEX = 0;

	do {
		PDynamic_NODE tmp = (PDynamic_NODE)current_node->Next_Node;// �̸� next��� ���.

		if (NODE_SECTION_INDEX == 0xFFFFFFFF && NODE_SEARCH_VALUE == current_node->Node_Search_VALUE) {
			//���ʰ��� 
			NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
		}


		if (NODE_SECTION_INDEX != 0xFFFFFFFF ) {
			//���İ���
			ULONG64 tmp_NODE_SECTION_INDEX = 0;

			if (NODE_SEARCH_VALUE == current_node->Node_Search_VALUE) {
				/*
					���� ���Ḯ��Ʈ ���� �������ְ� �Ҵ������ϱ�.
				*/
				if (Remove_Node_internal(current_node) == FALSE) {
					Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
					return FALSE;
				}

				tmp_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}
			else if (current_node->NODE_SECTION_INDEX > NODE_SECTION_INDEX ) {
				// ������� �������, INDEX�� �� ũ�ٸ�, -1�� �ؾ���
				current_node->NODE_SECTION_INDEX--;
				tmp_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}

			// �������� - ��� ���� �ε��� �������� ������Ʈ�� ����,
			if (current_Maximum_of_NODE_SECTION_INDEX < tmp_NODE_SECTION_INDEX) {
				current_Maximum_of_NODE_SECTION_INDEX = tmp_NODE_SECTION_INDEX;
			}
		}


		current_node = tmp;//�̸� ����� next����ּ� �� ����
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
) {// ��� �������� ( count )

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
				return current_node; // ��� ���� 
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
				return current_node; // ��� ���� 
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
				
					��ġ�� ��,,
						1. Return
							> ���� ������ ��� �ּ�
						2. Output - PUCHAR
							> ���� ������ ��� ������ "�����ּ�" 

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
	ULONG32 Node_Search_VALUE, // ������ 
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

			// ���� ���� -> �ε��� ���
			if (remember_NODE_SECTION_INDEX == 0xFFFFFFFF) {
				remember_NODE_SECTION_INDEX = current_node->NODE_SECTION_INDEX;
			}

			// ���� �ε��� ���������� ����� ��, (���� Search_Value�� �� �۵���)
			if (remember_NODE_SECTION_INDEX != 0xFFFFFFFF) {

				// ���� ���� �ε��� ������ ��ȯ�ϵ��� �� 
				PDynamic_NODE detected_section_current_node = current_node;
				do {

					if (remember_NODE_SECTION_INDEX == detected_section_current_node->NODE_SECTION_INDEX) {
						if (detected_section_current_node->is_end_node) {
							break;
						}
						else {
							if (Relation_index == detected_section_current_node->NODE_RELATION_INDEX) {
								/*
									Relation_Index ���� �� ���� �� ����
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

				current_node = detected_section_current_node; // ���� �������� �ٷ� �ǳʶٵ�����. 
			}


		}
		current_node = (PDynamic_NODE)current_node->Next_Node;
	} while (current_node != NULL);

	Release_PKmutex(&CreateAppendRemove__Parallel_Linked_List_KMutex);
	return NULL;

}