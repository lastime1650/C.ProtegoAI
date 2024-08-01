#ifndef EXE_bin_request_processing_H
#define EXE_bin_request_processing_H

#include <ntifs.h>
#include <ntddk.h>

#include "converter_string.h"//RUST에서 보낸 ANSI(절대경로)를 유니코드로 변환해야하므로
#include "File_io.h"//파일을 읽어냄
#include "TCP_conn.h"

NTSTATUS FILE_bin_process(PUCHAR Absolute_KERNEL_FILE_FULL_PATH__Ansi, ULONG32 FILE_FULL_PATH_STRING_SIZE);

#endif