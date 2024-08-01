#include "PLIST_Node_Manager.h"
/*
	여기는 PLIST 노드 생성/축적/삭제  로직을 정의한다.
*/

// 노드생성
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
			최초생성
		*/
		NEW->previous_addr = NULL;
	}
	else {
		/*
			추가생성
		*/
		NEW->previous_addr = Previous_addr;
	}
	NEW->next_addr = NULL;
	NEW->RAW_DATA = RAW_DATA; //할당하지 않고 알려주기만해라! 최초 스레드에서 동적할당하면 댐
	NEW->RAW_DATA_len = RAW_DATA_len;

	*inout_SIZE += sizeof(LIST);
	return NEW;
}



// 노드 추가
PLIST APPENDLIST(
	ULONG32 RAW_DATA_len,
	PUCHAR RAW_DATA,
	PLIST Node,
	ULONG32* inout_SIZE
) {
	if (Node->next_addr != NULL) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "APPEND 실패! 파라미터 Node의 next_addr값이  NULL이 아닙니다. \n");
		return NULL;
	}
	PLIST APPEND_node = CREATE_tag_LIST( RAW_DATA_len, RAW_DATA, (PUCHAR)Node, &(*inout_SIZE)); // 추가된 노드
	if (APPEND_node == NULL) {
		return NULL;
	}

	Node->next_addr = (PUCHAR)APPEND_node;

	return APPEND_node;
}


// 모든 노드 삭제
VOID REMOVE__ALL_LIST(PLIST Lastest_node) {
	PLIST Prv_node_tmp = NULL;
	do {
		/*
			노드자체를 동적할당 해제해야함.
		*/
		Prv_node_tmp = (PLIST)Lastest_node->previous_addr; //노드 할당해제 전, 현 노드가 ( 가르키는 이전노드 주소 ) 기억.


		ExFreePoolWithTag(Lastest_node, 'NoDe');///////////////////////////// 할당해제
		Lastest_node = Prv_node_tmp;

	} while (Lastest_node->previous_addr != NULL);
}