#ifndef Mutex_with_Lock_H
#define Mutex_with_Lock_H

#include <ntifs.h>
#pragma warning(disable:4996)


typedef struct Util_Mutex_with_Lock {
	PKMUTEX Mutex;
	BOOLEAN is_Lock;
}Util_Mutex_with_Lock, *PUtil_Mutex_with_Lock;

BOOLEAN Initilize_or_Locking_PKmutex(PUtil_Mutex_with_Lock Mutex_Struct, BOOLEAN is_Lock);

VOID Release_PKmutex(PUtil_Mutex_with_Lock Mutex_Struct);

#endif // !
