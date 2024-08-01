
#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "Process_Action_processing.h"
/*
	���μ��� �ݹ��Լ� Action ó�� �Լ���
*/

PAction_for_process_creation_NODE Action_for_process_routine_node_Start_Address = NULL; // ����� �� �� ���ʷ� Create�� ��
PAction_for_process_creation_NODE Action_for_process_routine_node_Current_Address = NULL; // ���������� APPEND�� �� 

Util_Mutex_with_Lock Action_for_proces_routine_node_KMUTEX = { NULL, FALSE };

PAction_for_process_creation_NODE Create_process_routine_NODE(
	PAction_for_process_creation_NODE Previous_Node,
	BOOLEAN is_Block,
	PUCHAR SHA256,
	Type_Feature feature
) {

	

	PAction_for_process_creation_NODE new_node = ExAllocatePoolWithTag(PagedPool, sizeof(Action_for_process_creation_NODE), 'NoDe');
	if (new_node == NULL){
		Release_PKmutex(&Action_for_proces_routine_node_KMUTEX);
		return NULL;
	}

	if (Previous_Node == NULL) {
		Initilize_or_Locking_PKmutex(&Action_for_proces_routine_node_KMUTEX,TRUE);
		new_node->Previous_Address = NULL;
	}
	else {
		new_node->Previous_Address = (PUCHAR)Previous_Node;
	}

	new_node->is_Block = is_Block;

	memset(new_node->SHA256, 0, SHA256_String_Byte_Length);
	memcpy(new_node->SHA256, SHA256, (SHA256_String_Byte_Length - 1));

	new_node->feature = feature;

	new_node->Next_Address = NULL;

	if (Previous_Node == NULL) {
		Release_PKmutex(&Action_for_proces_routine_node_KMUTEX);
	}
	
	return new_node;
}

PAction_for_process_creation_NODE APPEND_process_routine_NODE(
	PAction_for_process_creation_NODE current_node,
	BOOLEAN is_Block,
	PUCHAR SHA256,
	Type_Feature feature
) {
	if (Initilize_or_Locking_PKmutex(&Action_for_proces_routine_node_KMUTEX, TRUE) == FALSE) return NULL;


	PAction_for_process_creation_NODE new_node = Create_process_routine_NODE(
		current_node,
		is_Block,
		SHA256,
		feature
	);
	if (new_node == NULL) {
		Release_PKmutex(&Action_for_proces_routine_node_KMUTEX);
		return NULL;
	}
	current_node->Next_Address = (PUCHAR)new_node;
	Release_PKmutex(&Action_for_proces_routine_node_KMUTEX);
	return new_node;
}

PAction_for_process_creation_NODE Search_process_routine_NODE(
	PAction_for_process_creation_NODE Start_Node,
	PUCHAR target_SHA256
) {
	if (Start_Node == NULL) {
		return NULL;
	}


	PUCHAR Current_Node = (PUCHAR)Start_Node;

	do {
		if (memcmp(((PAction_for_process_creation_NODE)Current_Node)->SHA256, target_SHA256, 64) == 0) {
			// ����
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Search_process_routine_NODE -> SHA256����!\n");
			return ((PAction_for_process_creation_NODE)Current_Node);
		}

		Current_Node = ((PAction_for_process_creation_NODE)Current_Node)->Next_Address;

	} while (Current_Node != NULL);
	// �ᱹ ������ ����. 
	return NULL;
}

PAction_for_process_creation_NODE Ex_Search_process_routine_NODE(
	PAction_for_process_creation_NODE Start_Node,
	BOOLEAN is_Block,
	PUCHAR target_SHA256,
	Type_Feature feature
) {
	if (Start_Node == NULL) {
		return NULL;
	}

	PUCHAR Current_Node = (PUCHAR)Start_Node;

	do {
		if ( (memcmp(((PAction_for_process_creation_NODE)Current_Node)->SHA256, target_SHA256, 64) == 0) && ( ( (PAction_for_process_creation_NODE)Current_Node )->is_Block == is_Block ) && (((PAction_for_process_creation_NODE)Current_Node)->feature == feature) ) {
			// ����
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Ex_Search_process_routine_NODE -> ������ ã�Ҵ�!\n");
			return ((PAction_for_process_creation_NODE)Current_Node);
		}

		Current_Node = ((PAction_for_process_creation_NODE)Current_Node)->Next_Address;

	} while (Current_Node != NULL);
	// �ᱹ ������ ����. 
	return NULL;
}

VOID DELETE_Action_One_Node(PAction_for_process_creation_NODE One_Node) {
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DELETE_Action_One_Node -> %p \n", One_Node);

	if (One_Node->Previous_Address == NULL && One_Node->Next_Address == NULL) {
		// ���� �����
		/*
			
			���Ḯ��Ʈ ���� ����� �ּҸ� Next_Address�� ���� ��,
			�Ҵ�����
		*/
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DELETE_Action_One_Node 1 \n");
		
		Action_for_process_routine_node_Start_Address = NULL;
		Action_for_process_routine_node_Current_Address = NULL;
		
		ExFreePoolWithTag((PVOID)One_Node, 'NoDe');


	}
	else if (One_Node->Previous_Address == NULL && One_Node->Next_Address != NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DELETE_Action_One_Node 4 \n");
		Action_for_process_routine_node_Start_Address = (PAction_for_process_creation_NODE)One_Node->Next_Address;
		ExFreePoolWithTag((PVOID)One_Node, 'NoDe');
	}
	else if (One_Node->Previous_Address != NULL && One_Node->Next_Address == NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DELETE_Action_One_Node 2 \n");
		// ������ �����
		/*
			
			���� ����� Next_Address ���� NULL�� �ϰ�,
			current�� �� ������ ��� �ּҷ� �����Ѵ�
			�Ҵ����� 

		*/
		((PAction_for_process_creation_NODE)(One_Node->Previous_Address))->Next_Address = NULL;//One_Node->Next_Address;
		Action_for_process_routine_node_Current_Address = (PAction_for_process_creation_NODE)(One_Node->Previous_Address);
		ExFreePoolWithTag((PVOID)One_Node, 'NoDe');

	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DELETE_Action_One_Node 3 \n");
		// �߰��� ���ִ� �����
		/*
			���� ����� ���� ��带 �����ϰ�, ������ ���� ����� Next_Address�� ���� ����� Next_Address������ ���� ��,
			�Ҵ�����.
		*/
		((PAction_for_process_creation_NODE)(One_Node->Previous_Address))->Next_Address = One_Node->Next_Address;
		ExFreePoolWithTag((PVOID)One_Node, 'NoDe');

	}

}

VOID Print_process_routine_NODE(
	PAction_for_process_creation_NODE Start_Node
) {
	PAction_for_process_creation_NODE Current_Node = Start_Node;
	do {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Print_process_routine_NODE] feature: %d / is_Block: %d / SHA256: %s \n", Current_Node->feature, Current_Node->is_Block, Current_Node->SHA256);

		Current_Node = (PAction_for_process_creation_NODE)Current_Node->Next_Address;
	} while (Current_Node != NULL);
}

BOOLEAN Action_for_process_creation(PUCHAR GET_BUFFER, ULONG32 GET_BUFFER_len) {

	
	/*
		[ register_check / is_DELETE ]
		[ Method ]
		[ Sha256 ]
		[ Type ]
	*/

	BOOLEAN is_DELETE = FALSE; // ��Ģ ���� ( ��忡�� ���� ) ���� Ȯ��
	BOOLEAN is_Block = FALSE;
	PUCHAR SHA256 = NULL;// [SHA256_Length] = { 0, };
	Type_Feature feature;

	/* ���̱�� �����͸� Node�� ���� */
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Action_for_process_routine -> GET_BUFFER : %p, SIZE : %lu\n", GET_BUFFER, GET_BUFFER_len);
	PLength_Based_DATA_Node RAW_DATA_NODE_ADDR = Build_RAW_DATA(GET_BUFFER, GET_BUFFER_len); // RUST������ ���� �����͸� �������� ���Ḯ��Ʈ�� ���� ����
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RAW_DATA_NODE_ADDR -> %p\n", RAW_DATA_NODE_ADDR);



	PLength_Based_DATA_Node RAW_DATA_StartNode = RAW_DATA_NODE_ADDR;

	/* ��Ģ �������� �������� �����ϱ� */
	if (memcmp(RAW_DATA_NODE_ADDR->RAW_DATA, "DELETE", RAW_DATA_NODE_ADDR->RAW_DATA_Size) == 0) {
		is_DELETE = TRUE;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " is_DELETE TRUE \n ");
	}
	else {
		is_DELETE = FALSE;
	}


	RAW_DATA_NODE_ADDR = (PLength_Based_DATA_Node)RAW_DATA_NODE_ADDR->Next_Address;

	/* Method ���� */
	if (memcmp(RAW_DATA_NODE_ADDR->RAW_DATA, "BLOCK", RAW_DATA_NODE_ADDR->RAW_DATA_Size) == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " BLOCK TRUE \n ");
		is_Block = TRUE;
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " BLOCK FALSE \n ");
		is_Block = FALSE;
	}

	RAW_DATA_NODE_ADDR = (PLength_Based_DATA_Node)RAW_DATA_NODE_ADDR->Next_Address;

	/* SHA256 ���� */
	//memcpy(SHA256, RAW_DATA_NODE_ADDR->RAW_DATA, RAW_DATA_NODE_ADDR->RAW_DATA_Size);
	SHA256 = RAW_DATA_NODE_ADDR->RAW_DATA;
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " SHA256 ���⼺�� \n ");
	RAW_DATA_NODE_ADDR = (PLength_Based_DATA_Node)RAW_DATA_NODE_ADDR->Next_Address;

	/* TYPE_Feature ���� */
	if (memcmp(RAW_DATA_NODE_ADDR->RAW_DATA, "process_Create", RAW_DATA_NODE_ADDR->RAW_DATA_Size) == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " Create�� \n ");
		feature = Create;
	}
	else if (memcmp(RAW_DATA_NODE_ADDR->RAW_DATA, "process_Remove", RAW_DATA_NODE_ADDR->RAW_DATA_Size) == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " Remove�� \n ");
		feature = Remove;
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " �׼� ��� ���� \n ");
		ULONG32 response = No; //1030
		SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
		return FALSE;
	}

	RAW_DATA_node_FreePool(RAW_DATA_StartNode); // ������ ���������� ���� ����Ʈ�� �����ּҺ����Ͽ�, �����Ҵ� ����

	// PROCESS ��ƾ Action �������� NULL���� �ƴϸ� append���� Ȯ��
	// 
	// �̶� Node�� ����/�߰��Ҷ��� ���������ʹ� �����Ҵ��� �������ؾ��Ѵ�. 
	//

	ULONG32 response = Yes; //1029
	if (Action_for_process_routine_node_Start_Address == NULL) {

		if (is_DELETE) {
			/* �̰��� ���� �� ���� ���� �ٷ� NO!*/
			response = No;
		}
		else {
			Action_for_process_routine_node_Start_Address = Create_process_routine_NODE(
				NULL,
				is_Block,
				(PUCHAR)SHA256,
				feature
			);
			Action_for_process_routine_node_Current_Address = Action_for_process_routine_node_Start_Address;
			response = Yes;
		}
		
	}
	else {

		if (is_DELETE) {

			PAction_for_process_creation_NODE GET_one_Node = Ex_Search_process_routine_NODE(
				Action_for_process_routine_node_Start_Address,
				is_Block,
				(PUCHAR)SHA256,
				feature
			);

			if (GET_one_Node == NULL) {
				/* ã���� ����! */
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "/* ã���� ����! */\n");
				response = No;
			}
			else {
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "/* ã���� �ִ�! */\n");
				DELETE_Action_One_Node(GET_one_Node);
				response = Yes;
			}

		}
		else {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "APPEND_process_routine_NODE -->> %p �� \n", Action_for_process_routine_node_Current_Address);
			Action_for_process_routine_node_Current_Address = APPEND_process_routine_NODE(
				Action_for_process_routine_node_Current_Address,
				is_Block,
				(PUCHAR)SHA256,
				feature
			);
			response = Yes;
		}
		
	}


	if (Action_for_process_routine_node_Start_Address != NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n\n\n ");

		Print_process_routine_NODE(Action_for_process_routine_node_Start_Address); // print

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n\n\n ");
	}
	
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " �׼� ��� ���� \n ");
	
	SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
	return TRUE;
}

/* PAction_for_process_creation_NODE ��� �ϳ��� ã���� */
/* �߰���ȹ: �� SHA256�� ���� �������� ������ �ɾ��� �� �� ����*/
PAction_for_process_creation_NODE Get_Action_Node_With_SHA256(HANDLE Input_PID) {
	/* PID -> EXE,SHA */
	PUCHAR EXE_binary = NULL;
	ULONG ExE_binary_Size = 0;
	CHAR SHA256[SHA256_String_Byte_Length] = { 0, };
	if (PID_to_EXE(
		Input_PID,
		&EXE_binary,
		&ExE_binary_Size,
		SHA256, KernelMode
	) != STATUS_SUCCESS) {
		return NULL;
	}

	ExFreePoolWithTag(EXE_binary, 'FILE');
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\nPRE -> PID: %llu, SHA256: %64s\n", Input_PID, SHA256);
	PAction_for_process_creation_NODE ActionNode = Search_process_routine_NODE(Action_for_process_routine_node_Start_Address, (PUCHAR)SHA256);
	return ActionNode;
}