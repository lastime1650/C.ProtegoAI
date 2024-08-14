#pragma warning(disable:4996)

#include <stdio.h>
#include <Windows.h>
#include "tchar.h"

#include "ioctl.h"// IOCTL 커널과 통신하기 위한 구조체, 정보등 포함됨
#include "INPUT_license_and_agent.h" // 라이선스 + 에이전트 얻기
 

BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);

int main() {

	PUCHAR License_ID = (PUCHAR)malloc(128);// [128] = { 0, };
	/*
		c4512bed8db9b164735129ad467da7d346f044cba9b4485939a6c24cfb96c7112cf9d56929d54dde7ba86c4d38854eceabe132a29ad7d05ee5b80be640b161dd
	*/

	PUCHAR Agent_ID = (PUCHAR)malloc(128);
	/*
		ebcff5bc26b09a873d206ebfdf74c288eb946dddd06c745b01e7716e825af0475f77363346df17dc71ba9462a2282a324804fa3e9a775f2803ee1832f0e57351
	*/
	printf("라이선스주소: %p , 에이전트 주소: %p / 현재 PID %llu", License_ID, Agent_ID, (HANDLE)GetCurrentProcessId());

	

	

	// 라이선스 AGENT받기
	COMMUNICATION_IOCTL_ENUM result_of_REQUEST = input_license_agent(
		License_ID,
		Agent_ID
	);
	printf("라이선스 ID : %128s\n에이전트 ID: %128s\n맞습니까??\n", License_ID, Agent_ID);
	//system("pause");
	


	


	//
	PCOMMUNICATION_IOCTL BUFFER = (PCOMMUNICATION_IOCTL)malloc(sizeof(PCOMMUNICATION_IOCTL));
	BUFFER->information = result_of_REQUEST;
	memcpy(BUFFER->Agent_ID, Agent_ID, 128);
	memcpy(BUFFER->license_ID, License_ID, 128);

	

	//free(BUFFER);
	//free(Agent_ID);	
	//free(License_ID);

	HANDLE hDevice = 0;
	Initialize_communicate(
		BUFFER,
		&hDevice
	);

	printf("결과->\n\ninformation: %d\nAGENT_ID: %128s , LICENSE_ID: %128s\n", BUFFER->information, BUFFER->Agent_ID, BUFFER->license_ID);

	/*
		지속적인 커뮤니케이션
	*/

	if (SetPrivilege(SE_DEBUG_NAME, TRUE) == FALSE) {
		printf("SetPrivilege 실패");
		return-1;
	}

	Keeping_communicate(
		BUFFER,
		hDevice
	);

	system("pause");

	free(Agent_ID);
	free(License_ID);
	free(BUFFER);

	
	return 0;
}


BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
		&hToken))
	{
		_tprintf(L"OpenProcessToken error: %u\n", GetLastError());
		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL,           // lookup privilege on local system
		lpszPrivilege,  // privilege to lookup 
		&luid))        // receives LUID of privilege
	{
		_tprintf(L"LookupPrivilegeValue error: %u\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.
	if (!AdjustTokenPrivileges(hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES)NULL,
		(PDWORD)NULL))
	{
		_tprintf(L"AdjustTokenPrivileges error: %u\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		_tprintf(L"The token does not have the specified privilege. \n");
		return FALSE;
	}

	return TRUE;
}

