#include "Length_Based_Maker.h"



BOOLEAN Length_Based_MAKER(ULONG32 INPUT_Command, PUCHAR INPUT_RAW_DATA, ULONG32 INPUT_RAW_DATA_SIZE, PUCHAR* APPENDING_DATA, ULONG32* APPENDING_DATA_SIZE) {
	if (APPENDING_DATA == NULL) return FALSE;

	CHAR END_str[] = "_END"; // 마지먹 NULL은 존재하지 않도록 -1 해야함
	
	// 사이즈 정하기
	ULONG32 DATA_ALL_SIZE = 0;
	if (*APPENDING_DATA == NULL) {
		DATA_ALL_SIZE = sizeof(INPUT_Command) + (sizeof(INPUT_RAW_DATA_SIZE) + INPUT_RAW_DATA_SIZE) + (sizeof(END_str)-1);
	} 
	else{
		DATA_ALL_SIZE = *APPENDING_DATA_SIZE + (sizeof(INPUT_RAW_DATA_SIZE) + INPUT_RAW_DATA_SIZE); // 이미 Header(INPUT_command) + Tail(_END) 2가지 크기는 이미 존재하니까 
	}

	// 동적할당
	PUCHAR LENGTH_BASED_ADDR = ExAllocatePoolWithTag(PagedPool, DATA_ALL_SIZE, 'MAKE');
	if (LENGTH_BASED_ADDR == NULL) {
		return FALSE;
	}


	PUCHAR current_addr = LENGTH_BASED_ADDR;

	// 삽입
	if (*APPENDING_DATA == NULL) {
		memcpy(current_addr, &INPUT_Command, sizeof(INPUT_Command));
		current_addr += sizeof(INPUT_Command);
	}
	else {
		/*추가 삽입 APPEND*/
		/*
			
			주의할 점, 마지막 _END는 제외한 INDEX 부터 값을 넣고 , 마지막에 _END를 삽입해야한다. 
		
		*/
		memcpy(current_addr, *APPENDING_DATA, ( *APPENDING_DATA_SIZE-(sizeof("_END")-1) ) );
		current_addr += (*APPENDING_DATA_SIZE - (sizeof("_END") - 1));

		memset(*APPENDING_DATA, 0, *APPENDING_DATA_SIZE);
		ExFreePoolWithTag(*APPENDING_DATA, 'MAKE');
	}


	//[ 공통 ] 추가될 데이터만 집어넣어라
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