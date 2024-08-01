#ifndef communication_with_RUST_H
#define communication_with_RUST_H

// RUST������ TCP������ �ΰ�, ó��
#include <ntifs.h>
#include <ntddk.h>

#include "Mutex_with_Lock.h"

#include "TCP_send_or_Receiver.h" // �ۼ���ó�� API

#include "TCP_conn.h" // RUST ������ ��� �������� ( �����ߴ� �� Ȯ���ϰ�, ���� ���н� ��õ��� �䱸�ؾ��ϹǷ� ) 

//#include "PLIST_Node_Manager.h"

#include "RUST_command_process.h"

VOID communication_server(PVOID context);


#endif