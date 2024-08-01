#ifndef NotifyRoutine_H
#define NotifyRoutine_H


#include <ntifs.h>
#include <ntddk.h>


//프로세스 알림루틴 로직
#include "ProcessNotifyRoutine.h"


NTSTATUS initialize_NotifyRoutine();


#endif