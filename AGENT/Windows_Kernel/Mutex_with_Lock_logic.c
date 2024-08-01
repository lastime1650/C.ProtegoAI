#include "Mutex_with_Lock.h"

BOOLEAN Initilize_or_Locking_PKmutex(PUtil_Mutex_with_Lock Mutex_Struct, BOOLEAN is_Lock) {


	if (Mutex_Struct->Mutex == NULL) {
		Mutex_Struct->Mutex = (PKMUTEX)ExAllocatePoolWithTag(NonPagedPool, sizeof(KMUTEX), 'TCPm');// TCP mutex
		memset(Mutex_Struct->Mutex, 0, sizeof(KMUTEX));
		if (Mutex_Struct->Mutex == NULL) return FALSE;

		KeInitializeMutex(Mutex_Struct->Mutex, 0);
		Mutex_Struct->is_Lock = FALSE;
	}
	 
	if (Mutex_Struct->Mutex && is_Lock) {
		KeWaitForSingleObject(Mutex_Struct->Mutex, Executive, KernelMode, FALSE, NULL);
		Mutex_Struct->is_Lock = TRUE;
	}

	return TRUE;
}


VOID Release_PKmutex(PUtil_Mutex_with_Lock Mutex_Struct) {
	if (Mutex_Struct->Mutex == NULL) return;

	if (Mutex_Struct->is_Lock) {
		KeReleaseMutex(Mutex_Struct->Mutex, FALSE);
		Mutex_Struct->is_Lock = FALSE;
	}

}