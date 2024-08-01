#include "Process_ObRegisterCallback_processing.h"

PDynamic_NODE Dynamic_Node_Action_Process_Node = NULL; // 전역변수

/* 비동기 연결리스트 기반 Action */


BOOLEAN Create_or_Append_Action_Process_Node(PActionProcessNode input) {
	/*
		비동기 연결리스트에 노드 추가하기
	*/

	/*
		중복제외 값을 넣어야함

		선 질의 후 삽입 결정

	*/

	//ActionProcessNode Node = { 0, };
	//Node.is_Block = is_Block;
	//memcpy((PUCHAR)&Node.SHA256, SHA256_64Byte, SHA256_String_Byte_Length - 1);
	//Node.feature = feature;



	if (
		Get_Node_2Dim(
			'Actn',
			(PUCHAR)input,
			sizeof(ActionProcessNode),
			&Dynamic_Node_Action_Process_Node) != NULL
		) {
		/* 이미 존재할 때, */
		return FALSE;

	}
	else {
		/* 없을 때, */
		if (Dynamic_Node_Action_Process_Node == NULL) {
			return Build_up_Node((PUCHAR)input, sizeof(ActionProcessNode), TRUE, &Dynamic_Node_Action_Process_Node, 'Actn'); // 노드 최초생성
		}
		else {
			return Build_up_Node((PUCHAR)input, sizeof(ActionProcessNode), FALSE, &Dynamic_Node_Action_Process_Node, 'Actn'); // 노드 추가 
		}


	}



}


BOOLEAN is_exist_program_Action_Process_Node(PActionProcessNode input) {
	if (
		Get_Node_2Dim(
			'Actn',
			(PUCHAR)input,
			sizeof(ActionProcessNode),
			&Dynamic_Node_Action_Process_Node) != NULL
	){
		return TRUE;
	}
	else {
		return FALSE;
	}
}


BOOLEAN Remove_one_node_Action_Process_Node(PActionProcessNode input) {


	return Remove_1Dim_Node_with_Search_Value(Dynamic_Node_Action_Process_Node, (PUCHAR)input, sizeof(ActionProcessNode));


}


BOOLEAN compare_Action_Node_with_PID(HANDLE PID, MODE Kernel_or_User) {
	/* PID -> EXE,SHA */
	PUCHAR EXE_binary = NULL;
	ULONG ExE_binary_Size = 0;
	CHAR SHA256[SHA256_String_Byte_Length] = { 0, };
	if (PID_to_EXE(
		PID,
		&EXE_binary,
		&ExE_binary_Size,
		SHA256,
		Kernel_or_User
	) != STATUS_SUCCESS) {
		return FALSE;
	}

	ExFreePoolWithTag(EXE_binary, 'FILE');
	return TRUE;
}