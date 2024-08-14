#pragma warning(disable:4996)

#include "ioctl.h"

BOOLEAN Initialize_communicate(


	PCOMMUNICATION_IOCTL BUFFER,
	HANDLE* hDevice

) {

	BUFFER->Ioctl_User_Mode_ProcessId = (HANDLE)GetCurrentProcessId();
	printf("내 PID -> %llu\n", BUFFER->Ioctl_User_Mode_ProcessId);

	*hDevice = 0;
	while (1) {

		*hDevice = CreateFile(TEXT("\\\\.\\My_AGENT_Device"),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (*hDevice == INVALID_HANDLE_VALUE) {
			printf("Failed to obtain handle to driver. Error: %d\n", GetLastError());
			Sleep(5000); //5초 멈춤
			continue;
		}
		else {
			break;
		}
	}




	DWORD bytesReturned;

	// IOCTL 요청 보내기
	BOOL success = DeviceIoControl(*hDevice,
		IOCTL_TEST,
		BUFFER,
		sizeof(COMMUNICATION_IOCTL),
		BUFFER,
		sizeof(COMMUNICATION_IOCTL),
		&bytesReturned,
		NULL
	);

	if (success) {
		printf("통신최종성공!");
		return TRUE;
	}
	else {
		printf("커널과 통신실패!");
		return FALSE;
	}

}



BOOLEAN Keeping_communicate(PCOMMUNICATION_IOCTL BUFFER, HANDLE hDevice) {
	while (1) {
		printf("[대기중]hDevice -> %lu\n", hDevice);
		BUFFER->Ioctl_User_Mode_ProcessId = (HANDLE)GetCurrentProcessId();

		BUFFER->information = CHECK;

		DWORD bytesReturned;

		// IOCTL 요청 보내기
		BOOL success = DeviceIoControl(hDevice,
			IOCTL_TEST,
			BUFFER,
			sizeof(COMMUNICATION_IOCTL),
			BUFFER,
			sizeof(COMMUNICATION_IOCTL),
			&bytesReturned,
			NULL
		);

		if (success) {
			printf("통신성공!\n");
			

			switch (BUFFER->information) {
			case CHECK:
				/*
					CHECK를 서로 주고 받은 경우는 그냥 스킵임
				*/
				continue;
			case HOOKING_request:
				/*
					커널에서 후킹 요청을 한 경우
				*/
				NULL; 
				HANDLE THREAD_ID = 0;

				HOOKING_move move_data = { 
					BUFFER->HOOK_DATA.PID,
					BUFFER->HOOK_DATA.Process_HANDLE 
				};
				printf("HOOKING_request ! -> PID : %lu \n", BUFFER->HOOK_DATA.PID);
				CreateThread(NULL, 0, START_HOOKING, &move_data, 0, &THREAD_ID);
			default:
				continue;
			}

			Sleep(2000);
			continue;
		}
		else {
			printf("커널과 통신실패!");
			return FALSE;
		}

	}
}