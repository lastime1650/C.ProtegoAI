#include "Hooking.h"

CHAR DLL_NAME[] = "C:\\Users\\Administrator\\Desktop\\DLL_START.dll";

VOID START_HOOKING(HOOKING_move* PROCESS_CONTEXT) {

	if (PROCESS_CONTEXT->PID <= (HANDLE)1000) return;


	printf("\n후크시작 ->PID %lu / HANDLE %lu  + OpenProcess -> %lu\n", PROCESS_CONTEXT->PID, PROCESS_CONTEXT->PROCESS_HANDLE, OpenProcess(PROCESS_ALL_ACCESS, FALSE, PROCESS_CONTEXT->PID));

	HANDLE process_handle = 0;

	// 원격으로 실행중인 프로세스에 DLL삽입시도 


	// 타겟 프로세스에 DLL 문자열 동적할당
	LPVOID get_allocated_mem_addr = NULL;
	
	
	if((get_allocated_mem_addr = VirtualAllocEx(PROCESS_CONTEXT->PROCESS_HANDLE, NULL, sizeof(DLL_NAME), MEM_COMMIT, PAGE_READWRITE)) != NULL){
		process_handle = PROCESS_CONTEXT->PROCESS_HANDLE;
		printf("1 get_allocated_mem_addr 가 성공!\n");
	}
	else if ((get_allocated_mem_addr = VirtualAllocEx(OpenProcess(PROCESS_ALL_ACCESS, FALSE, PROCESS_CONTEXT->PID), NULL, sizeof(DLL_NAME), MEM_COMMIT, PAGE_READWRITE)) != NULL) {
		process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PROCESS_CONTEXT->PID);
		printf("2 get_allocated_mem_addr 가 성공!\n");
	}
	else {
		printf("실패!\n");
		return;
	}

	// 삽입
	if (WriteProcessMemory(process_handle, get_allocated_mem_addr, (LPVOID)DLL_NAME, sizeof(DLL_NAME), NULL) == FALSE) {
		printf("WriteProcessMemory 가 FALSE!\n");
		return;
	}


	// DLL 로드할 주소 알아내기
	LPTHREAD_START_ROUTINE LOADLIBRARYW_ADDR = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

	// 스레드 실행
	HANDLE hThread = CreateRemoteThread(process_handle, NULL, 0, LOADLIBRARYW_ADDR, get_allocated_mem_addr, 0, NULL);
	//if (hThread == 0) return;
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	printf("\nworked!\n");



	return;
}