#include "Process_ObRegisterCallback_processing.h"

PDynamic_NODE Dynamic_Node_Action_Process_Node = NULL; // ��������

/* �񵿱� ���Ḯ��Ʈ ��� Action */


BOOLEAN Create_or_Append_Action_Process_Node(PActionProcessNode input) {
	/*
		�񵿱� ���Ḯ��Ʈ�� ��� �߰��ϱ�
	*/

	/*
		�ߺ����� ���� �־����

		�� ���� �� ���� ����

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
		/* �̹� ������ ��, */
		return FALSE;

	}
	else {
		/* ���� ��, */
		if (Dynamic_Node_Action_Process_Node == NULL) {
			return Build_up_Node((PUCHAR)input, sizeof(ActionProcessNode), TRUE, &Dynamic_Node_Action_Process_Node, 'Actn'); // ��� ���ʻ���
		}
		else {
			return Build_up_Node((PUCHAR)input, sizeof(ActionProcessNode), FALSE, &Dynamic_Node_Action_Process_Node, 'Actn'); // ��� �߰� 
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