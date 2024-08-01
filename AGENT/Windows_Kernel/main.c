#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include <ntifs.h>
#include <ntddk.h>



// 바이오스 정보 가져오기
#include "query_system_smbios_information.h"

// ioctl 설정
#include "my_ioctl.h" 

// 라이선스/에이전트 구조체 정의
#include "License_Agent_struct.h"
ID_information Driver_ID = { 0 };

// RUST와 세션 맺기 위한 헤더
#include "TCP_conn.h"

//default) 지속적으로 현재 프로세스의 연결리스트를 UPDATE하는 병렬 스레드 헤더
#include "query_process.h"

//모니터링
#include "NotifyRoutine.h"

//프로세스 생성/삭제 보호조치
#include "ObRegisterCallback_.h"

// 디바이스 검색 for Volume
#include "Get_Volumes.h"

// 디렉터리 검색
//#include "Query_Files_in_Directories.h";

//미니필터 
#include "minifilter_register.h"

/*
	LNK2019 문제는 ( 헤더,함수정의 C 파일 )<- 구성할 때 접할 수 있다. 주로 헤더를 정의하는 C파일을 (삭제)지웠다가 다시 생성해야 해결됨 ㅋㅋ
*/
NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry) {
	NTSTATUS status = STATUS_SUCCESS;

	if (Bring_API() != STATUS_SUCCESS) {
		return STATUS_UNSUCCESSFUL;
	}

	DeviceInitialize(driver_object);


	/*  현재 파일시스템 로드된 드라이브 전체 확인 후 최초 연결리스트 생성 */
	ListMountedDrives(NULL, FALSE); // [1/ ] 디바이스 Volume 연결리스트 생성


	// SM_BIOS
	Query_SMBIOS_information();

	if( Make_ObRegisterCallback() != STATUS_SUCCESS) return STATUS_UNSUCCESSFUL;

	if (Initialize_IOCTL_communicate() == FALSE) {  // 유저모드의 요청 대기.
		return STATUS_UNSUCCESSFUL;
	}

	


	//HANDLE thread_handle2 = 0;
	//status = PsCreateSystemThread(&thread_handle2, THREAD_ALL_ACCESS, NULL, NULL, NULL, Get_ALL_Process_List, NULL); // 무한 서버 연결 체크 스레드 [2/2]
	//ZwClose(thread_handle2);


	/* 스레드 생성 < TCP서버에 연결한다 >*/
	HANDLE thread_handle = 0;
	status = PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, TCP_conn, NULL); // 무한 서버 연결 체크 스레드 [2/2]
	ZwClose(thread_handle);


	/*
		알림루틴 등록 시작!
	*/
	initialize_NotifyRoutine();

	/*
		오브젝트 루틴 등록 시작!
	*/
	/*
	status = Make_ObRegisterCallback();
	if (status != STATUS_SUCCESS) {//최초 프로세스 생성/제거 등록
		//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Make_ObRegisterCallback 등록실패 -> %p\n", status);
		return STATUS_UNSUCCESSFUL;
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Make_ObRegisterCallback 등록성공 \n");
	*/


	/*
		미니필터 과정

		디바이스 볼륨 추출 -> 시그니처 얻어야함 -> 미니필터 등록 후 바로 사용가능. 

	*/

	

	// 시그니처 연결리스트는 TCP 연결될 때 생성되어야함

	// 미니필터 등록
	//Initialize_Mini_Filter_Driver(driver_object);


	return status;
}