#pragma once
#ifndef Pre_ObRegisterCallback__H
#define Pre_ObRegisterCallback__H

#define PROCESS_CREATE_PROCESS            0x0080
#define PROCESS_CREATE_THREAD             0x0002
//#define PROCESS_DUP_HANDLE                0x0040
#define PROCESS_SET_QUOTA                 0x0100
#define PROCESS_SET_INFORMATION           0x0200
#define PROCESS_SUSPEND_RESUME            0x0800
#define PROCESS_TERMINATE                 0x0001
#define PROCESS_VM_OPERATION              0x0008
#define PROCESS_VM_READ                   0x0010
#define PROCESS_VM_WRITE                  0x0020


#include <ntifs.h>
#include <ntddk.h>

//#include "Process_Action_processing.h" // 전역변수 연결리스트에 등록된 액션 처리
#include "SHA256.h"

#include "Process_ObRegisterCallback_processing.h" // 연결리스트 

OB_PREOP_CALLBACK_STATUS PreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation);


// 액션 처리
BOOLEAN Action_PROCESSING_on_RegisterCallback(PActionProcessNode Action_Node, POB_PRE_OPERATION_INFORMATION* OperationInformation, Type_Feature typefeature);


#endif