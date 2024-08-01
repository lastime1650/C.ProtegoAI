#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "File_io.h"


NTSTATUS Create_FILE(PHANDLE File_handle, UNICODE_STRING Create_File_Path, POBJECT_ATTRIBUTES objAttr, PIO_STATUS_BLOCK ioStatusBlock, ACCESS_MASK Desired_access, ULONG ShareOption, ULONG CreateOption) {


    InitializeObjectAttributes(objAttr,
        &Create_File_Path,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, NULL);


    NTSTATUS status = ZwCreateFile(File_handle,
        Desired_access,
        objAttr, ioStatusBlock, 0,
        FILE_ATTRIBUTE_NORMAL,
        ShareOption,
        CreateOption,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL, 0);


    return status;
}

NTSTATUS Open_FILE(PHANDLE File_handle, UNICODE_STRING Create_File_Path, POBJECT_ATTRIBUTES objAttr, PIO_STATUS_BLOCK ioStatusBlock, ACCESS_MASK Desired_access, ULONG ShareOption) {


    InitializeObjectAttributes(objAttr,
        &Create_File_Path,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        NULL, NULL);


    NTSTATUS status = ZwOpenFile(File_handle,
        Desired_access,
        objAttr, ioStatusBlock,
        ShareOption,
        FILE_SYNCHRONOUS_IO_NONALERT);


    return status;
}

/*INPUT_DATA�� �����ͷ� ���� ����*/
NTSTATUS Write_FILE(HANDLE File_handle, PVOID INPUT_DATA, ULONG INPUT_DATA_SIZE, PIO_STATUS_BLOCK ioStatusBlock) {

    NTSTATUS status = ZwWriteFile(File_handle, NULL, NULL, NULL, ioStatusBlock,
        INPUT_DATA, INPUT_DATA_SIZE, NULL, NULL);

    return status;
}
/*���� ���� ������*/
NTSTATUS Read_FILE(_In_ HANDLE File_handle, _In_ PVOID OUTPUT_DATA, _In_ ULONG inPUT_DATA_SIZE, PIO_STATUS_BLOCK ioStatusBlock) {

    NTSTATUS status = ZwReadFile(File_handle, NULL, NULL, NULL, ioStatusBlock,
        OUTPUT_DATA, inPUT_DATA_SIZE, NULL, NULL);

    return status;
}



NTSTATUS ALL_in_ONE_FILE_IO(PVOID* RAW_DATA, ULONG* RAW_DATA_SIZE, UNICODE_STRING File_path, ALL_in_ONE_FILE_IO_info custom_MODE) {

    NTSTATUS status = STATUS_SUCCESS;

    HANDLE filehandle = 0;


    OBJECT_ATTRIBUTES objAttr = { 0, };
    IO_STATUS_BLOCK ioStatusBlock = { 0, };




    switch (custom_MODE) {
    case    WRITE_MODE: //���� �� ��
        status = Create_FILE(&filehandle, File_path, &objAttr, &ioStatusBlock, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OVERWRITE_IF);
        if (status != STATUS_SUCCESS) {
            // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE - WRITE -> Create_FILE ��� %p ", status);
            return status;
        }

        status = Write_FILE(filehandle, *RAW_DATA, *RAW_DATA_SIZE, &ioStatusBlock);
        if (status != STATUS_SUCCESS) {
            ZwClose(filehandle);
            // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Write_FILE ->  ��� %p ", status);
            return status;
        }
        break;



    case    READ_MODE: //�б� �� ��
        status = Create_FILE(&filehandle, File_path, &objAttr, &ioStatusBlock, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN);
        if (status != STATUS_SUCCESS) {
            //  DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE - READ -> Create_FILE ��� %p ", status);
            return status;
        }

        status = Read_FILE(filehandle, *RAW_DATA, *RAW_DATA_SIZE, &ioStatusBlock);
        if (status != STATUS_SUCCESS) {
            ZwClose(filehandle);
            // DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Read_FILE ->  ��� %p\n ", status);
            return status;
        }
        break;


        // �Ʒ� ����� �� ENUM ��,,, FULL-PATH�� ������ ���� EXE ���̳ʸ��� ���������� �Ѵ�. 
    case    READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH: // ���� ���� ���� ��
        //status = Create_FILE(&filehandle, File_path, &objAttr, &ioStatusBlock, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE , FILE_OPEN);
        status = Open_FILE(&filehandle, File_path, &objAttr, &ioStatusBlock, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE);
        if (status != STATUS_SUCCESS) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE - READ -> Create_FILE ��� %p ", status);
            return status;
        }


        /*���� STANDARD ���*/
        FILE_STANDARD_INFORMATION fileStandardInfo; // ���� ��ü ��û ����
        status = ZwQueryInformationFile(filehandle, &ioStatusBlock, &fileStandardInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation);
        if (status != STATUS_SUCCESS) {
            ZwClose(filehandle);
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Create_FILE - READ -> ZwQueryInformationFile ��� %p ", status);
            return STATUS_UNSUCCESSFUL;
        }

        /*���� ���� ����*/
        *RAW_DATA_SIZE = (ULONG)fileStandardInfo.EndOfFile.QuadPart;

        /* ���� ���̿� ���� �����Ҵ� <- �̰����� EXE ���α׷��� ���*/
        *RAW_DATA = ExAllocatePoolWithTag(PagedPool, *RAW_DATA_SIZE, 'FILE'); // ���� �Ҵ� �õ��� �ϹǷ�, NonPaged�� �ϸ� ��罺ũ�� -> �޸� �̽�
        if (*RAW_DATA == NULL) {
            ZwClose(filehandle);
            return STATUS_UNSUCCESSFUL;
        }
        memset(*RAW_DATA, 0, *RAW_DATA_SIZE); // 0���� set


        status = Read_FILE(filehandle, *RAW_DATA, *RAW_DATA_SIZE, &ioStatusBlock);
        if (status != STATUS_SUCCESS) {
            ZwClose(filehandle);
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Read_FILE ->  ��� %p ", status);
            return status;
        }
        break;

    default:
        return STATUS_UNSUCCESSFUL;
    }
    ZwClose(filehandle);
    return status;
}