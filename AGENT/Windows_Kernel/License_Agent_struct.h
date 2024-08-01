#ifndef License_Agent_struct_H
#define License_Agent_struct_H

#include <ntifs.h>
#include <ntddk.h>
#include "my_ioctl.h"

// COMMUNICATION_IOCTL_ENUM <- my_ioctl

typedef struct ID_info {// [NEW]
	COMMUNICATION_IOCTL_ENUM information; // ID 상태 확인용
	UCHAR AGENT_ID[128]; // ANsi 
	UCHAR LICENSE_ID[128]; // ANsi 
	
	/*[하드웨어 검증용-필드]*/
	UCHAR SMBIOS_TYPE_1[256]; // 시스템 정보
	UCHAR SMBIOS_TYPE_2[256]; // 마더보드 정보

}ID_information, * PID_information;


//전역변수
extern ID_information Driver_ID;


// 먼저 유저모드에서 라이선스 + 에이전트 ID를 전송했을 때까지 기다렸다가(이벤트-점유) Driver_ID 전역변수에 정보 저장. 
BOOLEAN Initialize_IOCTL_communicate();


// AGENT_ID, LICENSE_ID를 RUST와 통신하여 교류함.
NTSTATUS GET_AGENT_ID();

#endif