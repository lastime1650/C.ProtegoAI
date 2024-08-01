#ifndef ObRegisterCallback__H
#define ObRegisterCallback__H

#include <ntifs.h>
#include <ntddk.h>

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


#include "Post_ObRegisterCallback_.h"
#include "Pre_ObRegisterCallback_.h" // 이게 중요하게 씀 



extern OB_CALLBACK_REGISTRATION g_CallbackRegistration;
extern PVOID g_CallbackHandle;

/*선언 부*/
NTSTATUS Make_ObRegisterCallback();

#endif