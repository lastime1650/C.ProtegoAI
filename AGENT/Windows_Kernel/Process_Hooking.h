#ifndef Process_Hooking_H
#define Process_Hooking_H

#include <ntifs.h>
#include "converter_PID.h"

BOOLEAN Hooking_Start(HANDLE PID, PVOID LoadLibrary_Address, PCHAR DLL_ANSi);


#endif
