#ifndef Agent_Alive_Check_processing_H
#define Agent_Alive_Check_processing_H

#include <ntifs.h>
#include <ntddk.h>

#include "License_Agent_struct.h"// AGENT_ID�� �ĺ��ؼ� ��������� RUST���� ��������
#include "TCP_conn.h"

NTSTATUS Are_You_ALIVE_process(PUCHAR BUFFER, ULONG32 BUFFER_Size);

#endif