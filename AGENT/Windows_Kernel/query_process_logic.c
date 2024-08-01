#pragma warning(disable:4100)
#pragma warning(disable:4996)
#include "query_process.h"

KMUTEX Get_Process_List_MUTEX = { 0, };
PGet_Process_List Get_Process_List_Start_Address = NULL;
PGet_Process_List Get_Process_List_Current_Address = NULL;

PGet_Process_List Create_Process_List_Node(
    PGet_Process_List Previous_Address,

    HANDLE PID,
    CHAR* SHA256
) {
    PGet_Process_List new_node = ExAllocatePoolWithTag(NonPagedPool, sizeof(Get_Process_List), 'NoDe');
    memset(new_node, 0, sizeof(Get_Process_List));


    new_node->PID = PID;
    //memcpy(&new_node->PID, &PID, 8);

    memcpy(new_node->SHA256, SHA256, 65);

    new_node->Next_Address = NULL;

    if (Previous_Address == NULL) {
        new_node->Previous_Address = NULL;
    }
    else {
        new_node->Previous_Address = (PUCHAR)Previous_Address;
    }

    return new_node;
}

PGet_Process_List APPEND_Process_List_Node(
    PGet_Process_List Current_Node,

    HANDLE PID,
    CHAR* SHA256
) {

    PGet_Process_List new_node = Create_Process_List_Node(
        Current_Node,
        PID,
        SHA256
    );

    Current_Node->Next_Address = (PUCHAR)new_node;

    return new_node;

}

VOID print_Process_Node(PGet_Process_List Start_Node) {
    PGet_Process_List Current_Node = Start_Node;
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n\n");
    do {
        // PGet_Process_List tmp_node = Current_Node;

         //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [프로세스노드]PI , SHA256: %s \n", tmp_node->SHA256);

        Current_Node = (PGet_Process_List)Current_Node->Next_Address;

    } while (Current_Node != NULL);
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n\n");
}

PGet_Process_List Search_Process_Node(
    PGet_Process_List Start_Node,
    HANDLE target_PID
) {
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 서치주소 %p\n", Start_Node);
    PGet_Process_List Current_Node = Start_Node;
    do {

        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [프로세스노드]PID: %llu / %llu \n", (HANDLE)Current_Node->PID, target_PID);
        if ((HANDLE)Current_Node->PID == (HANDLE)target_PID) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " PID 찾음!\n");
            return Current_Node;
        }

        Current_Node = (PGet_Process_List)Current_Node->Next_Address;

    } while (Current_Node != NULL);

    return NULL;
}

VOID REMOVE_Process_Node(
    PGet_Process_List Start_Node
) {
    PGet_Process_List Current_Node = Start_Node;
    do {
        PGet_Process_List tmp_node = Current_Node;


        Current_Node = (PGet_Process_List)Current_Node->Next_Address;
        ExFreePoolWithTag(tmp_node, 'NoDe');

    } while (Current_Node != NULL);
}

// 지속적으로 실행중인 프로그램을 가져옴 ( 가져오면 해제했다가 다시 가져옴 )
// 사용하려면, 점유를 해서 가져와야함!!!!!!
VOID Get_ALL_Process_List(PVOID context) {

    UNREFERENCED_PARAMETER(context);
    KeInitializeMutex(&Get_Process_List_MUTEX, 0);// 뮤텍스 초기화

    while (TRUE) {

        KeWaitForSingleObject(&Get_Process_List_MUTEX, Executive, KernelMode, FALSE, NULL);

        if (Get_Process_List_Start_Address != NULL) {

            // 이전에 연결리스트를 만든적이 있다면 전부 할당해제 해서 초기화해야함 
            REMOVE_Process_Node(
                Get_Process_List_Start_Address
            );


            Get_Process_List_Start_Address = NULL;
        }

        NTSTATUS status;
        PVOID buffer = NULL;
        ULONG bufferSize = 0;


        PGet_Process_List Start_Node = NULL;
        PGet_Process_List Current_Node = NULL;

        /*
            초기 길이얻기 -> continue -> 제대로 얻음
        */
        while ((status = ZwQuerySystemInformation(SystemProcessInformation, buffer, bufferSize, &bufferSize)) == STATUS_INFO_LENGTH_MISMATCH) {
            // 버퍼가 충분하지 않을 경우, 새로운 크기로 재할당
            buffer = ExAllocatePoolWithTag(NonPagedPool, bufferSize, 'Info');

            if (!buffer) {
                continue;
            }
        }



        if (NT_SUCCESS(status)) {
            // 성공적으로 정보를 얻었을 경우
            PSYSTEM_PROCESS_INFORMATION current = (PSYSTEM_PROCESS_INFORMATION)buffer;
            while (TRUE) {

                if (current->NextEntryOffset == 0) {
                    break;
                }

                HANDLE PID = current->UniqueProcessId;



                UNICODE_STRING string = { 0, };
                PID_to_ANSI_FULL_PATH(PID, &string, KernelMode);


                PUCHAR EXE_binary = NULL;
                ULONG ExE_binary_Size = 0;
                CHAR SHA256[SHA256_String_Byte_Length] = { 0, };

                if (PID_to_EXE(
                    PID,
                    &EXE_binary,
                    &ExE_binary_Size,
                    SHA256,
                    KernelMode
                ) != STATUS_SUCCESS) {
                    current = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)current + current->NextEntryOffset);
                    continue;

                }

                ExFreePoolWithTag(EXE_binary, 'FILE');

                if (Start_Node == NULL) {
                    Start_Node = Create_Process_List_Node(
                        NULL,
                        PID,
                        SHA256
                    );
                    Current_Node = Start_Node;
                }
                else {
                    Current_Node = APPEND_Process_List_Node(
                        Current_Node,
                        PID,
                        SHA256
                    );

                }
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [프로세스노드]PID: %llu  , SHA256: %s , FileName %wZ\n", PID, SHA256, current->ImageName);


                current = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)current + current->NextEntryOffset);
            }
        }



        Get_Process_List_Start_Address = Start_Node;
        //print_Process_Node(Get_Process_List_Start_Address);


        if (buffer) {
            ExFreePoolWithTag(buffer, 'Info');
        }

        KeReleaseMutex(&Get_Process_List_MUTEX, FALSE);


    }

}
