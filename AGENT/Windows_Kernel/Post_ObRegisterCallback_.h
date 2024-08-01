#ifndef Post_ObRegisterCallback__H
#define Post_ObRegisterCallback__H

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

// 아직 안쓸거임
void PostOperationCallback(PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation);

#endif