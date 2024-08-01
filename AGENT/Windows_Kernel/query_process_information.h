#ifndef query_process_information_H
#define query_process_information_H

#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>


#include "Initialize_API.h"// ƒı∏Æ API¿÷¿Ω



NTSTATUS Query_Process_info(
	HANDLE* real_process_handle, PROCESSINFOCLASS input_Process_class,
	PUNICODE_STRING output_unicode

);


#endif