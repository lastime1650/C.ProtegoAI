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



	if ( is_exist_program_Action_Process_Node(input) ) {
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


BOOLEAN Make_ActionProcessNode_from_length_based_linked_list(PLength_Based_DATA_Node Parsed_RAWDATA, ActionProcessNode* OUTPUT_Node, BOOLEAN* OUTPUT_register_check);

// ���� ��û ó���Լ� ( ��� �Ǵ� ���� )
BOOLEAN processing_action_with_server_Action_Process_Node(PLength_Based_DATA_Node Parsed_RAWDATA) {
	// �� ���Ḯ��Ʈ�� ��ȸ�Ͽ� ACTION ���� ����ü�� ��ȯ

	/*
		[0] [ register_check / is_DELETE ]
		[1] [ Method ]
		[2] [ Sha256 ]
		[3] [ Type ]
	*/


	ActionProcessNode ACTION_NODE = { 0, };
	BOOLEAN is_register;
	if (Make_ActionProcessNode_from_length_based_linked_list(Parsed_RAWDATA, &ACTION_NODE, &is_register) == FALSE) {
		
		return FALSE;
	}


	if (is_register) {
		// ��� . ��, �ߺ��� ��� ����

		//�ߺ��˻� + ����
		if (Create_or_Append_Action_Process_Node(&ACTION_NODE)==FALSE) {
			return FALSE;
		}
	}
	else {
		// ���� . ��, �̹� ���� ��� ����

		//�������˻� 
		if (is_exist_program_Action_Process_Node(&ACTION_NODE)) {
			if (Remove_one_node_Action_Process_Node(&ACTION_NODE) == FALSE) {
				return FALSE;
			}
		}
		else {
			return FALSE;
		}
	}

	// ���� ����
	ULONG32 response = Yes; //1030
	SEND_TCP_DATA(&response, 4, SERVER_DATA_PROCESS);
	return TRUE;
}


BOOLEAN Make_ActionProcessNode_from_length_based_linked_list(PLength_Based_DATA_Node Parsed_RAWDATA, ActionProcessNode* OUTPUT_Node, BOOLEAN* OUTPUT_is_register) {
	/*
		[0] [ register_check / is_DELETE ]
		[1] [ Method ]
		[2] [ Sha256 ]
		[3] [ Type ]
	*/
	
	// [0]
	if (memcmp(Parsed_RAWDATA->RAW_DATA, "DELETE", Parsed_RAWDATA->RAW_DATA_Size) == 0) {
		*OUTPUT_is_register = TRUE;
	}
	else {
		*OUTPUT_is_register = FALSE;
	}

	//[1]
	Parsed_RAWDATA = (PLength_Based_DATA_Node)Parsed_RAWDATA->Next_Address;
	if (memcmp(Parsed_RAWDATA->RAW_DATA, "BLOCK", Parsed_RAWDATA->RAW_DATA_Size) == 0) {
		OUTPUT_Node->is_Block = TRUE;
	}
	else {
		OUTPUT_Node->is_Block = FALSE;
	}

	//[2]
	Parsed_RAWDATA = (PLength_Based_DATA_Node)Parsed_RAWDATA->Next_Address;
	memset(OUTPUT_Node->SHA256, 0, 65);
	
	if (Parsed_RAWDATA->RAW_DATA_Size <= 65) {
		memcpy(OUTPUT_Node->SHA256, Parsed_RAWDATA->RAW_DATA, Parsed_RAWDATA->RAW_DATA_Size);
	}
	else {
		return FALSE;
	}
	

	//[3]
	Parsed_RAWDATA = (PLength_Based_DATA_Node)Parsed_RAWDATA->Next_Address;
	if (memcmp(Parsed_RAWDATA->RAW_DATA, "process_Create", Parsed_RAWDATA->RAW_DATA_Size) == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " Create�� \n ");
		OUTPUT_Node->feature = Create;
	}
	else if (memcmp(Parsed_RAWDATA->RAW_DATA, "process_Remove", Parsed_RAWDATA->RAW_DATA_Size) == 0) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " Remove�� \n ");
		OUTPUT_Node->feature = Remove;
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " �׼� ��� ���� \n ");
		return FALSE;
	}


	return TRUE;
}