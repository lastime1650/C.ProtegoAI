#include "RUST_DLP_Get_File_SIgnatures.h"


BOOLEAN Signature_append_or_remove_or_get(PLength_Based_DATA_Node INPUT) {
	/*
		1. status -> 4ULONG ( enum으로.. ) 
		2. 문자열~
	
	*/
	if (INPUT->RAW_DATA_Size != sizeof(SIG_STATUS)) {
		return FALSE;
	}

	PLength_Based_DATA_Node current = NULL;
	
	switch (*((SIG_STATUS*)(INPUT->RAW_DATA))) {
	case is_register:
		NULL;
		current = (PLength_Based_DATA_Node)INPUT->Next_Address;
		while (current != NULL) {
			ANSI_STRING FILE_NAME_ANSI = { 0, };
			FILE_NAME_ANSI.Length = (USHORT)current->RAW_DATA_Size;
			FILE_NAME_ANSI.MaximumLength = (USHORT)current->RAW_DATA_Size;
			FILE_NAME_ANSI.Buffer = (PCHAR)current->RAW_DATA;


			UNICODE_STRING FILE_NAME_UNI = { 0, };

			ANSI_to_UNICODE(&FILE_NAME_UNI, FILE_NAME_ANSI);


			// 중복검사
			if (Policy_Signature_Start_Node) {
				BOOLEAN is_same = TRUE;
				Ppolicy_signature_struct current_Sig = Policy_Signature_Start_Node;
				while (current_Sig != NULL) {
					
					if (RtlCompareUnicodeString(&FILE_NAME_UNI, &current_Sig->Extension, TRUE)) {
						is_same = TRUE;
						break;
					}
					
					current_Sig = (Ppolicy_signature_struct)current_Sig->Next_Node;
				}

				if (is_same) {
					// 중복이 있는 경우 continue
					current = (PLength_Based_DATA_Node)INPUT->Next_Address;
					continue;
				}
			}

			if (Policy_Signature_Start_Node == NULL) {
				Policy_Signature_Start_Node = Create_Policy_Signature_Node(NULL, &FILE_NAME_UNI);
				Policy_Signature_Current_Node = Policy_Signature_Start_Node;
			}
			else {
				Policy_Signature_Current_Node = Append_Policy_Signature_Node(Policy_Signature_Current_Node, &FILE_NAME_UNI);
			}
			

			current = (PLength_Based_DATA_Node)INPUT->Next_Address;
		}

		break;





	case is_remove:
		NULL;

		if (Policy_Signature_Start_Node == NULL || Policy_Signature_Current_Node == NULL)
			return FALSE;

		current = (PLength_Based_DATA_Node)INPUT->Next_Address;
		while (current != NULL) {
			ANSI_STRING FILE_NAME_ANSI = { 0, };
			FILE_NAME_ANSI.Length = (USHORT)current->RAW_DATA_Size;
			FILE_NAME_ANSI.MaximumLength = (USHORT)current->RAW_DATA_Size;
			FILE_NAME_ANSI.Buffer = (PCHAR)current->RAW_DATA;

			UNICODE_STRING FILE_NAME_UNI = { 0, };

			ANSI_to_UNICODE(&FILE_NAME_UNI, FILE_NAME_ANSI);

			if (Policy_Signature_Start_Node) {
				Ppolicy_signature_struct current_Sig = Policy_Signature_Start_Node;
				while (current_Sig != NULL) {

					if (RtlCompareUnicodeString(&FILE_NAME_UNI, &current_Sig->Extension, TRUE)) {

						Remove_Specified_Policy_Signature_Node(Policy_Signature_Start_Node, &current_Sig->Extension);

						return TRUE;
					}

					current_Sig = (Ppolicy_signature_struct)current_Sig->Next_Node;
				}


				current = (PLength_Based_DATA_Node)INPUT->Next_Address;
			}
		}
		return FALSE;




	case is_get:
		NULL;

		if (Policy_Signature_Start_Node == NULL || Policy_Signature_Current_Node == NULL) 
			return FALSE;

		current = (PLength_Based_DATA_Node)INPUT->Next_Address;
		while (current != NULL) {
			/* 아직 */
			current = (PLength_Based_DATA_Node)INPUT->Next_Address;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

