#ifndef PLIST_Node_Manager_H
#define PLIST_Node_Manager_H
/*
	RUST���� �����ϱ� ���� ������(���Ḯ��Ʈ-��������)�� �����ϴ� ���
*/

#include <ntifs.h>
#include <ntddk.h>

#include "License_Agent_struct.h" // ������Ʈ ID ����������


typedef struct LIST {

	ULONG32 RAW_DATA_len;
	PUCHAR RAW_DATA; // ���⼭ ���� �����Ҵ����� �ʰ�, �ٷ� ���������� �����ȴ�. (���= ���������� ���� ��, �񵿱� ó�� ���� �Ұ��� ) 

	PUCHAR previous_addr;
	PUCHAR next_addr;
}LIST, * PLIST;

// ������
PLIST CREATE_tag_LIST(
	ULONG32 RAW_DATA_len,
	PUCHAR RAW_DATA,
	PUCHAR Previous_addr,
	ULONG32* inout_SIZE
);

// ��� �߰�
PLIST APPENDLIST(
	ULONG32 RAW_DATA_len,
	PUCHAR RAW_DATA,
	PLIST Node,
	ULONG32* inout_SIZE
);


// ��� ��� ����
VOID REMOVE__ALL_LIST(PLIST Lastest_node);



/////[ ���Ḯ��Ʈ ��� ��带 �� ����������.. ]//////
extern PUCHAR RAW_DATA__external_var;
extern ULONG32 RAW_DATA_SIZE__external_var;



// "�� ���"�� ���Ḯ��Ʈ�� APPEND�ϰ� �� ���������� �����ϴ� �Լ�( �ȿ��� Link_node__2__Mem_Alloc() �� ȣ���� )
NTSTATUS SAVE_DATA_with_Length_based_RAW_DATA(
	ULONG32 TYPE,

	PLIST input_RAW_DATA__LIST_tail_node, // ���� ��� ������ �ּҸ� �޴´�
	ULONG32 input_RAW_DATA__LIST_tail_node_SIZE, // ���� ��� ������ 

	 PUCHAR* inoutput_OLD_RAW_DATA_alloc_addr, /*�����ڿ� �б⾲�� -> ������ ��� NULL��*/
	 ULONG32* inoutput_OLD_RAW_DATA_alloc_addr_SIZE /*�����ڿ� �б⾲��*/
);



// PLIST �� ��带 "����-���"���� ������
NTSTATUS Link_node__2__Mem_Alloc(
	ULONG32 input_TYPE,

	PLIST input_LIST_Tail_node_addr,
	ULONG32 input_LIST_Tail_node_addr_SIZE,

	PUCHAR* output_result_allocated_addr,
	ULONG32* output_result_allocated_addr__SIZE

);


// ���������� "����-���" �����͸� ���� COPY�ع������� �� ( �Ʒ� �Լ��� ȣ���� ��, RUST���� ���� �� �ִ� �ϼ���ü�� �� )
NTSTATUS APPEND_Length_Based_DATA(
	PUCHAR* OLD_input_data, ULONG32* OLD_input_data_size,

	PUCHAR* CURRENT_input_data, ULONG32* CURRENT_input_data_size,

	PUCHAR* output_data, ULONG32* output_data_size);

/////[ �۽�ó�� ]//////

#include "TCP_send_or_Receiver.h"

// ���Ḯ��Ʈ�� �� ���������� ���� ������ RUST������ �����ϴ� �Լ� 
NTSTATUS RAWDATA_SEND__when_request_from_server(
	PUCHAR* RAW_DATA,
	ULONG32* RAW_DATA_SIZE,
	SOCKET_INFORMATION receive_types
);


#endif