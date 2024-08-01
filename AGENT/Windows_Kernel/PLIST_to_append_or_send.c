#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "PLIST_Node_Manager.h"
/*
	�� ������ 
	
		1."����-���"�����͸� RUST�� ���� ���������� APPEND,

		2.�������� RUST���� �����ϴ� ���� ����

	�Ǿ��ִ�.
*/



//
NTSTATUS APPEND_Length_Based_DATA(
	PUCHAR* OLD_input_data, ULONG32* OLD_input_data_size,

	PUCHAR* CURRENT_input_data, ULONG32* CURRENT_input_data_size,

	PUCHAR* output_data, ULONG32* output_data_size) {
	/*
		[���ǻ���]
			OLD - input_data�� src DATA�� memcpy ������ ��, ���� SHA512(128) ���� �����ϰ� �־����;;
	*/

	if (OLD_input_data == NULL && OLD_input_data_size == NULL) { // �������뺯���� ���ʷ� ���� ���� �� 
		PUCHAR NEW_addr = NULL;
		ULONG32 NEW_addr_size = *CURRENT_input_data_size + 128;

		NEW_addr = ExAllocatePoolWithTag(PagedPool, NEW_addr_size, 'ALL');
		if (NEW_addr == NULL) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		memcpy((PVOID)NEW_addr, (PVOID)Driver_ID.AGENT_ID, 128);

		memcpy((PUCHAR)NEW_addr + 128, *CURRENT_input_data, *CURRENT_input_data_size);

		ExFreePoolWithTag(*CURRENT_input_data, 'ALL'); // Link_node__2__Mem_Alloc �Ҵ� ���� 
		*CURRENT_input_data_size = 0;

		*output_data = NEW_addr; // �������뺯�� �Ҵ� �ּ� �̵�. 
		*output_data_size = NEW_addr_size;

		return STATUS_SUCCESS;
	}
	else { // �������뺯���� �������� �Ҵ�Ǿ� �־�����, �߰��� ���� ���� �� 
		PUCHAR NEW_addr = NULL;
		ULONG32 NEW_addr_size = *OLD_input_data_size + *CURRENT_input_data_size;// +128;

		NEW_addr = ExAllocatePoolWithTag(PagedPool, NEW_addr_size, 'ALL');
		if (NEW_addr == NULL) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		//memcpy((PVOID)NEW_addr, (PVOID)AGENT_ID, 128);

		memcpy((PUCHAR)NEW_addr, *OLD_input_data, *OLD_input_data_size); // SHA512 + ���������� �̹� OLD�� ����

		memcpy((PUCHAR)((PUCHAR)NEW_addr + *OLD_input_data_size), *CURRENT_input_data, *CURRENT_input_data_size);

		ExFreePoolWithTag(*CURRENT_input_data, 'ALL'); // Link_node__2__Mem_Alloc �Ҵ� ���� 
		*CURRENT_input_data_size = 0;
		ExFreePoolWithTag(*OLD_input_data, 'ALL'); // �������뺯�� �Ҵ�����
		*CURRENT_input_data_size = 0;

		*output_data = NEW_addr; // �������뺯�� �Ҵ� �ּ� �̵�. 
		*output_data_size = NEW_addr_size;

		return STATUS_SUCCESS;
	}



}



//
NTSTATUS SAVE_DATA_with_Length_based_RAW_DATA(
	ULONG32 TYPE,

	PLIST input_RAW_DATA__LIST_tail_node, // ���� ��� ������ �ּҸ� �޴´�
	ULONG32 input_RAW_DATA__LIST_tail_node_SIZE, // ���� ��� ������ 

	PUCHAR* inoutput_OLD_RAW_DATA_alloc_addr, /*�����ڿ� �б⾲�� -> ������ ��� NULL��*/
	ULONG32* inoutput_OLD_RAW_DATA_alloc_addr_SIZE /*�����ڿ� �б⾲��*/
) {

	PUCHAR Parsed__all_of_data = NULL; // <===== ���� ����Ʈ ������ ��� �� ������ �ϳ��� �����Ϳ� ����. ( ����-��� ���� ��������.,, ) 
	ULONG32 Length_Based_RAW_DATA_len = 0;

	PLIST Node_Lastest_addr_BACKUP = input_RAW_DATA__LIST_tail_node;
	// [1/2] �Ľ� �� �����޸� �Ҵ�
	if (Link_node__2__Mem_Alloc(TYPE, input_RAW_DATA__LIST_tail_node, input_RAW_DATA__LIST_tail_node_SIZE, &Parsed__all_of_data, &Length_Based_RAW_DATA_len) != STATUS_SUCCESS) {
		return STATUS_UNSUCCESSFUL;
	}


	/*
		Old�� null�� �ƴ϶��, �� �Ҵ��Ͽ� ��� ���� ���Ͽ�  �� �����Ҵ� �Ѵ�. ( ���� )
		�Ʒ� if else ������ else�� ��
	*/
	if (*inoutput_OLD_RAW_DATA_alloc_addr != NULL && *inoutput_OLD_RAW_DATA_alloc_addr_SIZE > 0) { // [2/2] 


		APPEND_Length_Based_DATA(inoutput_OLD_RAW_DATA_alloc_addr, inoutput_OLD_RAW_DATA_alloc_addr_SIZE, &Parsed__all_of_data, &Length_Based_RAW_DATA_len, inoutput_OLD_RAW_DATA_alloc_addr, inoutput_OLD_RAW_DATA_alloc_addr_SIZE);


	}
	else {
		/*
			NEW ���ʷ� ����Ǵ� ����� ����
		*/

		APPEND_Length_Based_DATA(NULL, NULL, &Parsed__all_of_data, &Length_Based_RAW_DATA_len, inoutput_OLD_RAW_DATA_alloc_addr, inoutput_OLD_RAW_DATA_alloc_addr_SIZE);
	}

	REMOVE__ALL_LIST(Node_Lastest_addr_BACKUP/* ������ ��� �ּ� */); // ���Ḯ��Ʈ ��� �ϴ� ����

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[���̱��] output_NEW_RAW_DATA_alloc_addr_SIZE �� ���� -> %llu \n", *inoutput_OLD_RAW_DATA_alloc_addr_SIZE);

	return STATUS_SUCCESS;
}






/////[send �κ�]////
/*
	�����κ��� ��� �����͸� �����϶�� ����� ������ ȣ��Ǵ� �Լ�
	�����͸� �������� �����Ѵ�
*/
NTSTATUS RAWDATA_SEND__when_request_from_server(_Inout_ PUCHAR* RAW_DATA, _Inout_ ULONG32* RAW_DATA_SIZE, SOCKET_INFORMATION receive_types) {

	if (*RAW_DATA == NULL || *RAW_DATA_SIZE == 0) {

		return STATUS_UNSUCCESSFUL;
	}





	PUCHAR remember_RAW_DATA_addr = *RAW_DATA;
	ULONG32 remember_RAW_DATA_size = *RAW_DATA_SIZE;

	NTSTATUS status = STATUS_SUCCESS;

	/* ������ ���� */
	if (SEND_TCP_DATA(*RAW_DATA, *RAW_DATA_SIZE, receive_types) != STATUS_SUCCESS) {
		/* ���������� �������� ��,,,,*/
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> Length-based ������ ���� ���� ����!.. \n");

		/* �ּ�,������ �� ���Ἲ �˻� */
		if (remember_RAW_DATA_addr != *RAW_DATA && remember_RAW_DATA_size != *RAW_DATA_SIZE) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> ���Ἲ ������ (1) \n");
		}

		status = STATUS_UNSUCCESSFUL;
	}
	else {
		/* ���������� �������� ��,,,,*/
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> Length-based ������ ���� ���� ����!!!!.. \n");

		/* �ּ�,������ �� ���Ἲ �˻� */
		if (remember_RAW_DATA_addr != *RAW_DATA && remember_RAW_DATA_size != *RAW_DATA_SIZE) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> ���Ἲ ������ (2) \n");
			status = STATUS_UNSUCCESSFUL;
		}
		else {
			/* ����Ʈ ���� �Ҵ����� */
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "RAWDATA_SEND__when_request_from_server -> ȣ�� �� FREE_pool____Dynamic_Allocation_mem ==>> RAW_DATA_addr: %p , RAW_DATA_size: %p , %d\n", RAW_DATA__external_var, &RAW_DATA_SIZE__external_var, RAW_DATA_SIZE__external_var);
			ExFreePoolWithTag((PVOID)*RAW_DATA, (ULONG)'ALL'); //���������� �����Ͽ�����, �ƿ� �ϴ� �ʱ�ȭ
			*RAW_DATA = NULL;
			*RAW_DATA_SIZE = 0;
			//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[Length_RAWDATA_SEND__when_request_from_server] -> �Ҵ�����\n");
			status = STATUS_SUCCESS;
		}


	}

	return status;
}