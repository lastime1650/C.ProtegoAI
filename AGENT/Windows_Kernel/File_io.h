#ifndef File_io_H
#define File_io_H

#include <ntifs.h>
#include <ntddk.h>

typedef enum ALL_in_ONE_FILE_IO_info {
    WRITE_MODE,
    READ_MODE,
    READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH
}ALL_in_ONE_FILE_IO_info, * PALL_in_ONE_FILE_IO_info;



/*파일 핸들 얻기*/
NTSTATUS Create_FILE(PHANDLE File_handle, UNICODE_STRING Create_File_Path, POBJECT_ATTRIBUTES objAttr, PIO_STATUS_BLOCK ioStatusBlock, ACCESS_MASK Desired_access, ULONG ShareOption, ULONG CreateOption);

NTSTATUS Open_FILE(PHANDLE File_handle, UNICODE_STRING Create_File_Path, POBJECT_ATTRIBUTES objAttr, PIO_STATUS_BLOCK ioStatusBlock, ACCESS_MASK Desired_access, ULONG ShareOption);

NTSTATUS Write_FILE(HANDLE File_handle, PVOID INPUT_DATA, ULONG INPUT_DATA_SIZE, PIO_STATUS_BLOCK ioStatusBlock);

NTSTATUS Read_FILE(_In_ HANDLE File_handle, _In_ PVOID OUTPUT_DATA, _In_ ULONG inPUT_DATA_SIZE, PIO_STATUS_BLOCK ioStatusBlock);

// 실 사용 함수
NTSTATUS ALL_in_ONE_FILE_IO(PVOID* RAW_DATA, ULONG* RAW_DATA_SIZE, UNICODE_STRING File_path, ALL_in_ONE_FILE_IO_info custom_MODE);

#endif