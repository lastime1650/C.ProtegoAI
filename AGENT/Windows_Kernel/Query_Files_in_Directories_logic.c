#include "Query_Files_in_Directories.h"


VOID AppendBackslashToDirectoryPath(PUNICODE_STRING DirectoryPath);

ULONG32 ListDirectories(PUNICODE_STRING DirectoryPath, BOOLEAN is_init, PUNICODE_STRING INPUT_Hint_Data, PDynamic_NODE* Output_Node) {
    if (Output_Node == NULL) return 0;
    OBJECT_ATTRIBUTES objAttr;
    HANDLE hDirectory;
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS status;
    PVOID buffer;
    ULONG bufferSize = 1024;

    if (is_init && *Output_Node == NULL) {
        AppendBackslashToDirectoryPath(DirectoryPath);
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " is_init -> %wZ \n", *DirectoryPath);
    }

    InitializeObjectAttributes(&objAttr, DirectoryPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    status = ZwOpenFile(&hDirectory, FILE_LIST_DIRECTORY | SYNCHRONIZE, &objAttr, &ioStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to open directory: %wZ, Status: 0x%08X\n", DirectoryPath, status);
        return 0;
    }

    buffer = ExAllocatePoolWithTag(NonPagedPool, bufferSize, 'dirT');
    if (buffer == NULL) {
        ZwClose(hDirectory);
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to allocate memory\n");
        return 0;
    }

    while (TRUE) {
        status = ZwQueryDirectoryFile(hDirectory, NULL, NULL, NULL, &ioStatusBlock, buffer, bufferSize,
            FileDirectoryInformation, TRUE, NULL, FALSE);

        if (status == STATUS_NO_MORE_FILES) {
            break;
        }

        if (!NT_SUCCESS(status)) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to query directory: %wZ, Status: 0x%08X\n", DirectoryPath, status);
            break;
        }

        PFILE_DIRECTORY_INFORMATION dirInfo = (PFILE_DIRECTORY_INFORMATION)buffer;
        while (TRUE) {
            UNICODE_STRING fileName;
            fileName.Length = (USHORT)dirInfo->FileNameLength;
            fileName.MaximumLength = (USHORT)dirInfo->FileNameLength;
            fileName.Buffer = dirInfo->FileName;



            // "."와 ".." 문자열 정의
            WCHAR deny_filter[2][3] = {
                L".",
                L".."
            };

            // "."와 ".."로 시작하는지 확인
            BOOLEAN is_same = FALSE;
            for (int i = 0; i < 2; i++) {
                if (wcsncmp(fileName.Buffer, deny_filter[i], wcslen(deny_filter[i])) == 0) {
                    is_same = TRUE;
                    break;
                }
            }

            // 심볼릭 링크나 정크션 포인트 무시
            if (is_same || (dirInfo->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
                if (dirInfo->NextEntryOffset == 0) {
                    break;
                }

                dirInfo = (PFILE_DIRECTORY_INFORMATION)((PUCHAR)dirInfo + dirInfo->NextEntryOffset);
                continue;
            }


            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Found file: %wZ\n", &fileName);

            WCHAR path_conn[] = L"\\";

            ULONG32 unicode_size = DirectoryPath->Length + sizeof(path_conn) + fileName.Length + sizeof(WCHAR); // 널 문자를 위한 공간 포함
            PWCH unc_buffer = (PWCH)ExAllocatePoolWithTag(PagedPool, unicode_size, 'dirT');
            if (unc_buffer == NULL) {
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Memory allocation failed\n");
                return 0;
            }
            RtlZeroMemory(unc_buffer, unicode_size);

            // [1/3] DirectoryPath 복사
            RtlCopyMemory(unc_buffer, DirectoryPath->Buffer, DirectoryPath->Length);

            // [2/3] Path separator 복사
            if (is_init == FALSE) {
                RtlCopyMemory((PUCHAR)unc_buffer + DirectoryPath->Length, path_conn, sizeof(path_conn) - sizeof(WCHAR));


                // [3/3] fileName 복사
                RtlCopyMemory((PUCHAR)unc_buffer + DirectoryPath->Length + sizeof(path_conn) - sizeof(WCHAR), fileName.Buffer, fileName.Length);

                // 널 문자 추가
                unc_buffer[(DirectoryPath->Length + sizeof(path_conn) - sizeof(WCHAR) + fileName.Length) / sizeof(WCHAR)] = L'\0';
            }
            else {

                // [3/3] fileName 복사
                RtlCopyMemory((PUCHAR)unc_buffer + DirectoryPath->Length, fileName.Buffer, fileName.Length);

                // 널 문자 추가
                unc_buffer[(DirectoryPath->Length + fileName.Length) / sizeof(WCHAR)] = L'\0';

            }



            // UNICODE_STRING 구조체로 출력
            UNICODE_STRING fullPath = { 0, };
            RtlInitUnicodeString(&fullPath, unc_buffer);


            if (dirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                /* 일반 디렉터리 */

                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[전체_디스크_스캔] [디렉터리] -> %wZ\n", &fullPath);
                ListDirectories(&fullPath, FALSE, INPUT_Hint_Data, Output_Node);


            }
            else {
                /*일반파일 - 유틸리티 */
                
                if (INPUT_Hint_Data != NULL) {

                    if (ContainsStringInsensitive(&fullPath, INPUT_Hint_Data)) {
                        Build_up_Node((PUCHAR)fullPath.Buffer, fullPath.MaximumLength, is_init, Output_Node, 'Dirs');
                    }
                    
                }
                else {
                    Build_up_Node((PUCHAR)fullPath.Buffer, fullPath.MaximumLength, is_init, Output_Node, 'Dirs');
                }
               
                

               // File_Dir_METADATA FILE_METADATA = { 0, };
               //FILE_METADATA.CreationTime = dirInfo->CreationTime.QuadPart;
               // FILE_METADATA.LastAccessTime = dirInfo->LastAccessTime.QuadPart;
               // FILE_METADATA.LastWriteTime = dirInfo->LastWriteTime.QuadPart;
               // FILE_METADATA.ChangeTime = dirInfo->ChangeTime.QuadPart;


                /* 일반 파일 */
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[전체_디스크_스캔] [파일] -> %wZ\n", &fullPath);
                /* 정책에 따른 파일만 추출 */
               // KeWaitForSingleObject(&MUTEX_signature, Executive, KernelMode, FALSE, NULL);

                //UNICODE_STRING abc = { 0, };
                //RtlInitUnicodeString(&abc, L".hwpx");
                //if (ContainsStringInsensitive_2(&fullPath, &abc)) {
                //    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ContainsStringInsensitive_2 -> %wZ \n", fullPath);
                //}

                // 현재 중복으로 값이 넣어지는 현상 발견.
                // 중복제외 저장으로 로직 재수정.
                //추가
               // Policy_Signature_Compare((PUCHAR)Policy_Signature_Start_Node, &fullPath, NULL, NULL, signature_SAVE_Mode, NULL, &FILE_METADATA);


               // KeReleaseMutex(&MUTEX_signature, FALSE);

            }




            ExFreePoolWithTag(unc_buffer, 'dirT');

            if (dirInfo->NextEntryOffset == 0) {
                break;
            }


            dirInfo = (PFILE_DIRECTORY_INFORMATION)((PUCHAR)dirInfo + dirInfo->NextEntryOffset);
        }
    }

    ExFreePool(buffer);
    ZwClose(hDirectory);
    return 'Dirs';
}



VOID AppendBackslashToDirectoryPath(PUNICODE_STRING DirectoryPath)
{
    // 현재 길이와 최대 길이를 저장합니다.
    USHORT currentLength = DirectoryPath->Length;
    USHORT maxLength = DirectoryPath->MaximumLength;

    // 문자열이 이미 백슬래시로 끝나는지 확인합니다.
    if (currentLength >= sizeof(WCHAR) &&  DirectoryPath->Buffer[(currentLength / sizeof(WCHAR)) - 1] == L'\\') {
        return; // 이미 백슬래시로 끝나는 경우 아무것도 하지 않음
    }

    // 추가할 공간이 충분한지 확인합니다.
    if (currentLength + sizeof(WCHAR) > maxLength) {
        // 공간이 부족한 경우, 새로운 버퍼를 할당합니다.
        USHORT newMaxLength = currentLength + sizeof(WCHAR);
        PWCHAR newBuffer = (PWCHAR)ExAllocatePoolWithTag(PagedPool, newMaxLength, 'test');

        if (newBuffer == NULL) {
            // 메모리 할당 실패 시 반환
            return;
        }

        // 기존 버퍼의 내용을 새로운 버퍼로 복사합니다.
        RtlCopyMemory(newBuffer, DirectoryPath->Buffer, currentLength);

        // 기존 버퍼를 해제합니다.
        ExFreePoolWithTag(DirectoryPath->Buffer, 'test');

        // 새로운 버퍼를 설정합니다.
        DirectoryPath->Buffer = newBuffer;
        DirectoryPath->MaximumLength = newMaxLength;
    }

    // 백슬래시를 추가합니다.
    DirectoryPath->Buffer[currentLength / sizeof(WCHAR)] = L'\\';
    DirectoryPath->Length += sizeof(WCHAR);
    DirectoryPath->Buffer[DirectoryPath->Length / sizeof(WCHAR)] = L'\0'; // 널 종단 문자 추가
}



BOOLEAN ListDirectories_PoolFree(PDynamic_NODE Node_for_PoolFREE, ULONG32 Dir_Search_Value) {
    if (Node_for_PoolFREE == NULL) return FALSE;
    return Remove_Node_with_Search_Value(Node_for_PoolFREE, Dir_Search_Value);
}