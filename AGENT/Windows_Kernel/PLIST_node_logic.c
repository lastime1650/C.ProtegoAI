#include "PLIST_Node_Manager.h"
/*
	����� PLIST ��� ����/����/����  ������ �����Ѵ�.
*/

// ������
PLIST CREATE_tag_LIST(
	ULONG32 RAW_DATA_len,
	PUCHAR RAW_DATA,
	PUCHAR Previous_addr,
	ULONG32* inout_SIZE
) {
	PLIST NEW = (PLIST)ExAllocatePoolWithTag(NonPagedPool, sizeof(LIST), 'NoDe');
	if (NEW == NULL) {
		return NULL;
	}

	memset(NEW, 0, sizeof(LIST));

	if (Previous_addr == NULL) {
		/*
			���ʻ���
		*/
		NEW->previous_addr = NULL;
	}
	else {
		/*
			�߰�����
		*/
		NEW->previous_addr = Previous_addr;
	}
	NEW->next_addr = NULL;
	NEW->RAW_DATA = RAW_DATA; //�Ҵ����� �ʰ� �˷��ֱ⸸�ض�! ���� �����忡�� �����Ҵ��ϸ� ��
	NEW->RAW_DATA_len = RAW_DATA_len;

	*inout_SIZE += sizeof(LIST);
	return NEW;
}



// ��� �߰�
PLIST APPENDLIST(
	ULONG32 RAW_DATA_len,
	PUCHAR RAW_DATA,
	PLIST Node,
	ULONG32* inout_SIZE
) {
	if (Node->next_addr != NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "APPEND ����! �Ķ���� Node�� next_addr����  NULL�� �ƴմϴ�. \n");
		return NULL;
	}
	PLIST APPEND_node = CREATE_tag_LIST( RAW_DATA_len, RAW_DATA, (PUCHAR)Node, &(*inout_SIZE)); // �߰��� ���
	if (APPEND_node == NULL) {
		return NULL;
	}

	Node->next_addr = (PUCHAR)APPEND_node;

	return APPEND_node;
}


// ��� ��� ����
VOID REMOVE__ALL_LIST(PLIST Lastest_node) {
	PLIST Prv_node_tmp = NULL;
	do {
		/*
			�����ü�� �����Ҵ� �����ؾ���.
		*/
		Prv_node_tmp = (PLIST)Lastest_node->previous_addr; //��� �Ҵ����� ��, �� ��尡 ( ����Ű�� ������� �ּ� ) ���.


		ExFreePoolWithTag(Lastest_node, 'NoDe');///////////////////////////// �Ҵ�����
		Lastest_node = Prv_node_tmp;

	} while (Lastest_node->previous_addr != NULL);
}