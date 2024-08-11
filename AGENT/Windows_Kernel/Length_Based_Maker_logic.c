#include "Length_Based_Maker.h"



BOOLEAN Length_Based_MAKER(ULONG32 INPUT_Command, PUCHAR INPUT_RAW_DATA, ULONG32 INPUT_RAW_DATA_SIZE, PUCHAR* APPENDING_DATA, ULONG32* APPENDING_DATA_SIZE) {
	if (APPENDING_DATA == NULL) return FALSE;

	CHAR END_str[] = "_END"; // ������ NULL�� �������� �ʵ��� -1 �ؾ���
	
	// ������ ���ϱ�
	ULONG32 DATA_ALL_SIZE = 0;
	if (*APPENDING_DATA == NULL) {
		DATA_ALL_SIZE = sizeof(INPUT_Command) + (sizeof(INPUT_RAW_DATA_SIZE) + INPUT_RAW_DATA_SIZE) + (sizeof(END_str)-1);
	} 
	else{
		DATA_ALL_SIZE = *APPENDING_DATA_SIZE + (sizeof(INPUT_RAW_DATA_SIZE) + INPUT_RAW_DATA_SIZE); // �̹� Header(INPUT_command) + Tail(_END) 2���� ũ��� �̹� �����ϴϱ� 
	}

	// �����Ҵ�
	PUCHAR LENGTH_BASED_ADDR = ExAllocatePoolWithTag(PagedPool, DATA_ALL_SIZE, 'MAKE');
	if (LENGTH_BASED_ADDR == NULL) {
		return FALSE;
	}


	PUCHAR current_addr = LENGTH_BASED_ADDR;

	// ����
	if (*APPENDING_DATA == NULL) {
		memcpy(current_addr, &INPUT_Command, sizeof(INPUT_Command));
		current_addr += sizeof(INPUT_Command);
	}
	else {
		/*�߰� ���� APPEND*/
		/*
			
			������ ��, ������ _END�� ������ INDEX ���� ���� �ְ� , �������� _END�� �����ؾ��Ѵ�. 
		
		*/
		memcpy(current_addr, *APPENDING_DATA, ( *APPENDING_DATA_SIZE-(sizeof("_END")-1) ) );
		current_addr += (*APPENDING_DATA_SIZE - (sizeof("_END") - 1));

		memset(*APPENDING_DATA, 0, *APPENDING_DATA_SIZE);
		ExFreePoolWithTag(*APPENDING_DATA, 'MAKE');
	}


	//[ ���� ] �߰��� �����͸� ����־��
	memcpy(current_addr, &INPUT_RAW_DATA_SIZE, sizeof(INPUT_RAW_DATA_SIZE));
	current_addr += sizeof(INPUT_RAW_DATA_SIZE);

	memcpy(current_addr, INPUT_RAW_DATA, INPUT_RAW_DATA_SIZE);
	current_addr += INPUT_RAW_DATA_SIZE;

	memcpy(current_addr, END_str, sizeof(END_str) - 1); // Ensure no NULL terminator is copied
	current_addr += sizeof(END_str) - 1;
	

	// OUTPUT
	*APPENDING_DATA = LENGTH_BASED_ADDR;
	*APPENDING_DATA_SIZE = DATA_ALL_SIZE;

	return TRUE;
}