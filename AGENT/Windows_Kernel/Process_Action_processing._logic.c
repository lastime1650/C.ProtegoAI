
#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "Process_Action_processing.h"
/*
	프로세스 콜백함수 Action 처리 함수들
*/

PAction_for_process_creation_NODE Action_for_process_routine_node_Start_Address = NULL; // 사용할 때 나 최초로 Create할 때
PAction_for_process_creation_NODE Action_for_process_routine_node_Current_Address = NULL; // 지속적으로 APPEND할 때 

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
			// 같다
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Search_process_routine_NODE -> SHA256같다!\n");
			return ((PAction_for_process_creation_NODE)Current_Node);
		}

		Current_Node = ((PAction_for_process_creation_NODE)Current_Node)->Next_Address;

	} while (Current_Node != NULL);
	// 결국 같은게 없다. 
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
			// 같다
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Ex_Search_process_routine_NODE -> 같은거 찾았다!\n");
			return ((PAction_for_process_creation_NODE)Current_Node);
		}

		Current_Node = ((PAction_for_process_creation_NODE)Current_Node)->Next_Address;

	} while (Current_Node != NULL);
	// 결국 같은게 없다. 
	return NULL;
}

VOID DELETE_Action_One_Node(PAction_for_process_creation_NODE One_Node) {
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DELETE_Action_One_Node -> %p \n", One_Node);

	if (One_Node->Previous_Address == NULL && One_Node->Next_Address == NULL) {
		// 시작 노드임
		/*
			
			연결리스트 시작 노드의 주소를 Next_Address로 변경 후,
			할당해제
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
		// 마지막 노드임
		/*
			
			이전 노드의 Next_Address 값을 NULL로 하고,
			current를 맨 마지막 노드 주소로 변경한다
			할당해제 

		*/
		((PAction_for_process_creation_NODE)(One_Node->Previous_Address))->Next_Address = NULL;//One_Node->Next_Address;
		Action_for_process_routine_node_Current_Address = (PAction_for_process_creation_NODE)(One_Node->Previous_Address);
		ExFreePoolWithTag((PVOID)One_Node, 'NoDe');

	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DELETE_Action_One_Node 3 \n");
		// 중간에 껴있는 노드임
		/*
			인자 노드의 이전 노드를 참조하고, 참조된 이전 노드의 Next_Address를 인자 노드의 Next_Address값으로 변경 후,
			할당해제.
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

	BOOLEAN is_DELETE = FALSE; // 규칙 삭제 ( 노드에서 제거 ) 인지 확인
	BOOLEAN is_Block = FALSE;
	PUCHAR SHA256 = NULL;// [SHA256_Length] = { 0, };
	Type_Feature feature;

	/* 길이기반 데이터를 Node로 추출 */
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Action_for_process_routine -> GET_BUFFER : %p, SIZE : %lu\n", GET_BUFFER, GET_BUFFER_len);
	PLength_Based_DATA_Node RAW_DATA_NODE_ADDR = Build_RAW_DATA(GET_BUFFER, GET_BUFFER_len); // RUST서버가 보낸 데이터를 동적으로 연결리스트로 만들어서 추출
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RAW_DATA_NODE_ADDR -> %p\n", RAW_DATA_NODE_ADDR);



	PLength_Based_DATA_Node RAW_DATA_StartNode = RAW_DATA_NODE_ADDR;

	/* 규칙 생성인지 삭제인지 구별하기 */
	if (memcmp(RAW_DATA_NODE_ADDR->RAW_DATA, "DELETE", RAW_DATA_NODE_ADDR->RAW_DATA_Size) == 0) {
		is_DELETE = TRUE;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " is_DELETE TRUE \n ");
	}
	else {
		is_DELETE = FALSE;
	}


	RAW_DATA_NODE_ADDR = (PLength_Based_DATA_Node)RAW_DATA_NODE_ADDR->Next_Address;

	/* Method 추출 */
	if (memcmp(RAW_DATA_NODE_ADDR->RAW_DATA, "BLOCK", RAW_DATA_NODE_ADDR->RAW_DATA_Size) == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " BLOCK TRUE \n ");
		is_Block = TRUE;
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " BLOCK FALSE \n ");
		is_Block = FALSE;
	}

	RAW_DATA_NODE_ADDR = (PLength_Based_DATA_Node)RAW_DATA_NODE_ADDR->Next_Address;

	/* SHA256 추출 */
	//memcpy(SHA256, RAW_DATA_NODE_ADDR->RAW_DATA, RAW_DATA_NODE_ADDR->RAW_DATA_Size);
	SHA256 = RAW_DATA_NODE_ADDR->RAW_DATA;
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " SHA256 추출성공 \n ");
	RAW_DATA_NODE_ADDR = (PLength_Based_DATA_Node)RAW_DATA_NODE_ADDR->Next_Address;

	/* TYPE_Feature 추출 */
	if (memcmp(RAW_DATA_NODE_ADDR->RAW_DATA, "process_Create", RAW_DATA_NODE_ADDR->RAW_DATA_Size) == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " Create암 \n ");
		feature = Create;
	}
	else if (memcmp(RAW_DATA_NODE_ADDR->RAW_DATA, "process_Remove", RAW_DATA_NODE_ADDR->RAW_DATA_Size) == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " Remove임 \n ");
		feature = Remove;
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 액션 등록 실패 \n ");
		ULONG32 response = No; //1030
		SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
		return FALSE;
	}

	RAW_DATA_node_FreePool(RAW_DATA_StartNode); // 추출한 동적데이터 연결 리스트를 시작주소부터하여, 동적할당 해제

	// PROCESS 루틴 Action 전역변수 NULL인지 아니면 append인지 확인
	// 
	// 이때 Node를 생성/추가할때의 동적데이터는 동적할당을 무조건해야한다. 
	//

	ULONG32 response = Yes; //1029
	if (Action_for_process_routine_node_Start_Address == NULL) {

		if (is_DELETE) {
			/* 이것은 있을 수 없는 일임 바로 NO!*/
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
				/* 찾은게 없다! */
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "/* 찾은게 없다! */\n");
				response = No;
			}
			else {
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "/* 찾은게 있다! */\n");
				DELETE_Action_One_Node(GET_one_Node);
				response = Yes;
			}

		}
		else {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "APPEND_process_routine_NODE -->> %p 전 \n", Action_for_process_routine_node_Current_Address);
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
	
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 액션 등록 성공 \n ");
	
	SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
	return TRUE;
}

/* PAction_for_process_creation_NODE 노드 하나를 찾아줌 */
/* 추가계획: 한 SHA256에 여러 개별적인 조건을 걸었을 수 도 있음*/
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