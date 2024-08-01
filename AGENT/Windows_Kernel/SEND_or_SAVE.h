#ifndef SEND_or_SAVE_H
#define SEND_or_SAVE_H

#include <ntifs.h>
#include <ntddk.h>

#include "Mutex_with_Lock.h"

#include "PLIST_Node_Manager.h" // {{ 노드(생성,삭제,축적) + 송신 + 길이기반 변환 모음집 + 전역변수 }}까지
//#include "TCP_send_or_Receiver.h"

#include "Build_Socket.h"

typedef enum SEND_or_SAVE {
	SEND_RAW_DATA, // 서버에 RAW_DATA__external_var 데이터를 전달
	SAVE_RAW_DATA // RAW_DATA__external_var 데이터에 데이터를 축적하여 재 할당하면서 갱신함 
}SEND_or_SAVE_enum;


//PUCHAR RAW_DATA__external_var = NULL;
//ULONG32 RAW_DATA_SIZE__external_var = 0;
//KMUTEX MUTEX_for_External_var_Read_Write = { 0, };


//extern PKMUTEX MUTEX_for_External_var_Read_Write;

NTSTATUS Send_or_Save(SEND_or_SAVE_enum select, ULONG32* TYPE, PUCHAR* RAW_DATA, ULONG32* RAW_DATA_SIZE, SOCKET_INFORMATION receive_types);

#endif