#include "Get_File_Index_INFORMATION.h"

NTSTATUS Get_FILE_INDEX_INFORMATION(
	PUNICODE_STRING INPUT_absolute_FILE_PATH,
	ULONG64* OUTPUT_FILE_INDEX
) {


    NTSTATUS status;
    HANDLE fileHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_INTERNAL_INFORMATION fileInternalInfo;

    InitializeObjectAttributes(&objectAttributes, INPUT_absolute_FILE_PATH, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwCreateFile(
        &fileHandle,
        GENERIC_READ,
        &objectAttributes,
        &ioStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN,
        FILE_NON_DIRECTORY_FILE,
        NULL,
        0
    );

    if (!NT_SUCCESS(status)) {
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[uniq] 파일 열기 실패: %08x\n", status);
        return status;
    }

    status = ZwQueryInformationFile(
        fileHandle,
        &ioStatusBlock,
        &fileInternalInfo,
        sizeof(FILE_INTERNAL_INFORMATION),
        FileInternalInformation
    );

    if (NT_SUCCESS(status)) {
        *OUTPUT_FILE_INDEX = fileInternalInfo.IndexNumber.QuadPart;
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[uniq] 파일 ID: %llu\n", *OUTPUT_FILE_INDEX);
    }
    else {
        *OUTPUT_FILE_INDEX = 0;
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[uniq] 파일 정보 쿼리 실패: %08x \n", status);
    }

    ZwClose(fileHandle);
    return status;
}