#ifndef EXE_bin_request_processing_H
#define EXE_bin_request_processing_H

#include <ntifs.h>
#include <ntddk.h>

#include "converter_string.h"//RUST���� ���� ANSI(������)�� �����ڵ�� ��ȯ�ؾ��ϹǷ�
#include "File_io.h"//������ �о
#include "TCP_conn.h"

NTSTATUS FILE_bin_process(PUCHAR Absolute_KERNEL_FILE_FULL_PATH__Ansi, ULONG32 FILE_FULL_PATH_STRING_SIZE);

#endif