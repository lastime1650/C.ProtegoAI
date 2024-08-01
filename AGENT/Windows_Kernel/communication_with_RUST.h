#ifndef communication_with_RUST_H
#define communication_with_RUST_H

// RUST서버와 TCP세션을 맺고, 처리
#include <ntifs.h>
#include <ntddk.h>

#include "Mutex_with_Lock.h"

#include "TCP_send_or_Receiver.h" // 송수신처리 API

#include "TCP_conn.h" // RUST 서버와 통신 관련정보 ( 연결했는 지 확인하고, 연결 실패시 재시도를 요구해야하므로 ) 

//#include "PLIST_Node_Manager.h"

#include "RUST_command_process.h"

VOID communication_server(PVOID context);


#endif