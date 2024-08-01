#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "PLIST_Node_Manager.h"
/*
	����� " �� PLIST "�� ������ "����-���" ���·� ��ȯ�ϴ� �۾��� �Ѵ�. 
*/



// PLIST �� ��带 "����-���"���� ������
NTSTATUS Link_node__2__Mem_Alloc(
	ULONG32 input_TYPE,

	PLIST input_LIST_Tail_node_addr,
	ULONG32 input_LIST_Tail_node_addr_SIZE,

	PUCHAR* output_result_allocated_addr,
	ULONG32* output_result_allocated_addr__SIZE

){
	ULONG32 Length_Based_RAW_DATA_len = 0;
	ULONG32 i = 0;

	do {
		/*
			�������� ���̸� �����Ͽ� ���� �Ҵ��ϱ� ���� �ѷ��� ���Ѵ�
		*/
		i++; // �ݺ�ī��Ʈ

		Length_Based_RAW_DATA_len += input_LIST_Tail_node_addr->RAW_DATA_len;//���� �ּҿ��� RAW_DATA ���� �ߺ����� [1/4]
		input_LIST_Tail_node_addr = (PLIST)input_LIST_Tail_node_addr->previous_addr; // �ּ� �������� �����Ͽ� �̵�


		if (input_LIST_Tail_node_addr->previous_addr == NULL) {
			i++; //������ �߰� ī��Ʈ 
			/* �� ������ �����ϸ�, �� ó�� ������ �� ���̴�. */
			Length_Based_RAW_DATA_len += input_LIST_Tail_node_addr->RAW_DATA_len;
			break;
		}


	} while (input_LIST_Tail_node_addr->previous_addr != NULL);
	PLIST RAW_DATA__LIST__BACKUP = input_LIST_Tail_node_addr; // 2�� �ε��� �� �ּ� -> �̶��� ����� ( �� ó�� )�����ּҷ� ����Ѵ�.






	Length_Based_RAW_DATA_len += sizeof(input_TYPE); // TYPE 4����Ʈ �ߺ����� [2/4]
	Length_Based_RAW_DATA_len += (sizeof(ULONG32) * i); // RAW_DATA�� ����Ű�� ���� (4����Ʈ) ���� �ߺ����� [3/4]
	/*
		[_END]
		Ascii -> 5F 45 4E 44
	*/
	Length_Based_RAW_DATA_len += 4;// END 4����Ʈ ���� [4/4]

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[���̱��] �����Ҵ��� �� ���� -> %llu \n", Length_Based_RAW_DATA_len);
	/*

		�����Ҵ� �õ�

	*/
	PUCHAR all_of_data = (PUCHAR)ExAllocatePoolWithTag(PagedPool, Length_Based_RAW_DATA_len, 'ALL'); // tag-> Length Based Raw Data
	if (all_of_data == NULL) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}


	PUCHAR Header_addr = all_of_data;//���� �ּ� ���
	PUCHAR Tail_addr = (PUCHAR)all_of_data + ((SIZE_T)Length_Based_RAW_DATA_len - 4); // �� �ּ� ���ϱ� ( ��! _END �� _ �ּҴ� ����. 

	/* TYPE�� ���� [1/2]*/
	memcpy(all_of_data, &input_TYPE, sizeof(input_TYPE)); // 4����Ʈ �ּ� ���� ++ 
	all_of_data = all_of_data + 4; // �ּ� �̵�

	/* RAW_DATA__LIST__BACKUP == RAW_DATA__LIST */

	do {
		/*
			���� �����ϸ� Ż�� [2/2]
		*/
		if (all_of_data == Tail_addr) {
			DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "�� �ּҿ� �����Ͽ����Ƿ� Ż���մϴ�. \n");
			break;
		}

		memcpy(all_of_data, &RAW_DATA__LIST__BACKUP->RAW_DATA_len, sizeof(ULONG32)); // 4����Ʈ �ּ� ���� ++
		all_of_data = all_of_data + 4; // �ּ� 4 ����

		memcpy(all_of_data, RAW_DATA__LIST__BACKUP->RAW_DATA, RAW_DATA__LIST__BACKUP->RAW_DATA_len);  // �������� RAW_DATA ���� ++
		all_of_data = all_of_data + RAW_DATA__LIST__BACKUP->RAW_DATA_len;//�ּ� n ����



		RAW_DATA__LIST__BACKUP = (PLIST)RAW_DATA__LIST__BACKUP->next_addr; // �ּ� �������� �����Ͽ� �̵�

		if (RAW_DATA__LIST__BACKUP->next_addr == NULL) {
			/* �� ������ �����ϸ�, �� ������ ������ �� ���̴�. */
			memcpy(all_of_data, &RAW_DATA__LIST__BACKUP->RAW_DATA_len, sizeof(ULONG32)); // 4����Ʈ �ּ� ���� ++
			all_of_data = all_of_data + 4; // �ּ� 4 ����

			memcpy(all_of_data, RAW_DATA__LIST__BACKUP->RAW_DATA, RAW_DATA__LIST__BACKUP->RAW_DATA_len);  // �������� RAW_DATA ���� ++
			all_of_data = all_of_data + RAW_DATA__LIST__BACKUP->RAW_DATA_len;//�ּ� n ����

			break;
		}


	} while (RAW_DATA__LIST__BACKUP->next_addr != NULL);

	//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "_END  MEMCPY �ϱ�! -> Tail_add	r �� �ּ� -> %p \n", Tail_addr);
	/*[+] _END */memcpy(Tail_addr, "_END", (sizeof("_END") - 1)); // ������ \x00�� ���� �ʰ� 4����Ʈ�� ����ؼ� �ִ´�.

	*output_result_allocated_addr = Header_addr; // output ����
	*output_result_allocated_addr__SIZE = Length_Based_RAW_DATA_len; // output ����

	/*
		���� ��ȯ�Ǹ�, �ش� ����Ʈ�� ��� �Ҵ������Ǿ�߸��Ѵ�.
	*/

	return STATUS_SUCCESS;
}