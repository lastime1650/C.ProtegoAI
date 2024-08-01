#ifndef SEND_or_SAVE_H
#define SEND_or_SAVE_H

#include <ntifs.h>
#include <ntddk.h>

#include "Mutex_with_Lock.h"

#include "PLIST_Node_Manager.h" // {{ ���(����,����,����) + �۽� + ���̱�� ��ȯ ������ + �������� }}����
//#include "TCP_send_or_Receiver.h"

#include "Build_Socket.h"

typedef enum SEND_or_SAVE {
	SEND_RAW_DATA, // ������ RAW_DATA__external_var �����͸� ����
	SAVE_RAW_DATA // RAW_DATA__external_var �����Ϳ� �����͸� �����Ͽ� �� �Ҵ��ϸ鼭 ������ 
}SEND_or_SAVE_enum;


//PUCHAR RAW_DATA__external_var = NULL;
//ULONG32 RAW_DATA_SIZE__external_var = 0;
//KMUTEX MUTEX_for_External_var_Read_Write = { 0, };


//extern PKMUTEX MUTEX_for_External_var_Read_Write;

NTSTATUS Send_or_Save(SEND_or_SAVE_enum select, ULONG32* TYPE, PUCHAR* RAW_DATA, ULONG32* RAW_DATA_SIZE, SOCKET_INFORMATION receive_types);

#endif