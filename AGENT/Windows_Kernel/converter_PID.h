#ifndef converter_PID_H
#define converter_PID_H

#include <ntifs.h>
#include <ntddk.h>

#include "File_io.h"
#include "SHA256.h"

/*PID to real_HANDLE*/
NTSTATUS Get_real_HANDLE_from_pid(HANDLE INPUT_pid, HANDLE* OUTPUT_real_handle, MODE Kernel_or_User_mode);


/*PID to EXE binary*/
NTSTATUS PID_to_EXE(
	HANDLE Input_PID,

	PUCHAR* EXE_binary,
	ULONG* EXE_binary_Size,

	CHAR* SHA256_hashed, 
	MODE Kernel_or_User_mode
);

/*PID to 절대경로*/
NTSTATUS PID_to_ANSI_FULL_PATH(HANDLE PID, PUNICODE_STRING output_fullpath, MODE Kernel_or_User_mode);


#endif