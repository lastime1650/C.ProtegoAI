#ifndef NotifyRoutine_H
#define NotifyRoutine_H


#include <ntifs.h>
#include <ntddk.h>


//���μ��� �˸���ƾ ����
#include "ProcessNotifyRoutine.h"


NTSTATUS initialize_NotifyRoutine();


#endif