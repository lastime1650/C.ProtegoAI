#include "Get_Volumes.h"

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDirectoryObject(
    _In_ HANDLE DirectoryHandle,
    _Out_opt_ PVOID Buffer,
    _In_ ULONG Length,
    _In_ BOOLEAN ReturnSingleEntry,
    _In_ BOOLEAN RestartScan,
    _Inout_ PULONG Context,
    _Out_opt_ PULONG ReturnLength
);

PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Start_Node = NULL; // �� ����̺� ���Ḯ��Ʈ ���۳�� 
PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Current_Node = NULL; // �� ����̺� ���Ḯ��Ʈ ������ 
PKMUTEX Get_Volume_KMUTEX = NULL;

VOID ListMountedDrives(PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING, BOOLEAN is_remove_node)
{
    if (Get_Volume_KMUTEX == NULL) {
        Get_Volume_KMUTEX = ExAllocatePoolWithTag(NonPagedPool, sizeof(KMUTEX), 'MUTX');
        if (Get_Volume_KMUTEX == NULL) return;
        KeInitializeMutex(Get_Volume_KMUTEX, 0);
    }

    KeWaitForSingleObject(Get_Volume_KMUTEX, Executive, KernelMode, FALSE, NULL);

    is_remove_node;

    UNICODE_STRING uniString;
    OBJECT_ATTRIBUTES objAttr;
    HANDLE dirHandle;
    NTSTATUS status;
    PVOID buffer;
    ULONG context = 0;
    ULONG returnedLength;
    POBJECT_DIRECTORY_INFORMATION dirInfo;
    UNICODE_STRING ntPathPrefix = RTL_CONSTANT_STRING(L"\\Device\\");

    RtlInitUnicodeString(&uniString, L"\\??");
    InitializeObjectAttributes(&objAttr, &uniString, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);

    status = ZwOpenDirectoryObject(&dirHandle, DIRECTORY_QUERY, &objAttr);
    if (!NT_SUCCESS(status)) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to open directory object: %x\n", status);
        KeReleaseMutex(Get_Volume_KMUTEX, FALSE);
        return;
    }

    buffer = ExAllocatePoolWithTag(PagedPool, 1024 * 1024, 'dirb');
    if (!buffer) {
        ZwClose(dirHandle);
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to allocate buffer\n");
        KeReleaseMutex(Get_Volume_KMUTEX, FALSE);
        return;
    }

    while (TRUE) {
        status = ZwQueryDirectoryObject(dirHandle, buffer, 1024 * 1024, TRUE, FALSE, &context, &returnedLength);
        if (status == STATUS_NO_MORE_ENTRIES) {
            break;
        }

        if (!NT_SUCCESS(status)) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to query directory object: %x\n", status);
            break;
        }

        dirInfo = (POBJECT_DIRECTORY_INFORMATION)buffer;

        for (; ; dirInfo++) {
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "dirInfo -> %d\n", dirInfo);
            if (dirInfo->Name.Length == 0) {
                break;
            }

            HANDLE linkHandle;
            UNICODE_STRING linkTarget;
            WCHAR targetBuffer[MAXIMUM_FILENAME_LENGTH];
            linkTarget.Buffer = targetBuffer;
            linkTarget.Length = 0;
            linkTarget.MaximumLength = sizeof(targetBuffer);

            InitializeObjectAttributes(&objAttr, &dirInfo->Name, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, dirHandle, NULL);
            status = ZwOpenSymbolicLinkObject(&linkHandle, SYMBOLIC_LINK_QUERY, &objAttr);
            if (NT_SUCCESS(status)) {
                status = ZwQuerySymbolicLinkObject(linkHandle, &linkTarget, NULL);
                if (NT_SUCCESS(status)) {
                    if (RtlPrefixUnicodeString(&ntPathPrefix, &linkTarget, TRUE)) {
                        //

                        DEVICE_DECTECT_ENUM result = is_external_Device(&dirInfo->Name, &linkTarget, PNP_Device_Name_from_PNP_UNICODE_STRING);

                        switch (result) {
                        case JUST_VOLUME_DISK:
                            NULL;
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Mounted JUST_VOLUME_DISK Drive: %wZ -> %wZ\n", &dirInfo->Name, &linkTarget);
                            break;
                        case Internal_DISK:
                            NULL;
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Mounted Internal_DISK Drive: %wZ -> %wZ\n", &dirInfo->Name, &linkTarget);
                            break;
                        case External_DISK_USB:
                            NULL;
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Mounted External_DISK_USB Drive: %wZ -> %wZ\n", &dirInfo->Name, &linkTarget);
                            break;
                        case USB_DEVICE_from_PNP:
                            NULL;
                            break;
                        default:
                            NULL;
                            break;
                        }
                        if (result != DEVICE_None && result != USB_DEVICE_from_PNP && result != External_DISK_USB) {
                            // �������� ���� �� ���� UPDATE


                            PALL_DEVICE_DRIVES Node_from_Finder = NULL;

                            //if (is_PNP_call == FALSE) {
                            Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, &linkTarget, NULL, NULL);
                            // }
                             //else {
                                 //Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, &linkTarget, &result);
                             //}




                            if (Node_from_Finder == NULL) {
                                // ���� ��忡 ���� �� ���� ��� ����
                                if (ALL_DEVICE_DRIVES_Start_Node == NULL) {

                                    ALL_DEVICE_DRIVES_Start_Node = Create_ALL_DEVICE_DRIVES_Node(NULL, &dirInfo->Name, &linkTarget, NULL, &result);

                                    ALL_DEVICE_DRIVES_Current_Node = ALL_DEVICE_DRIVES_Start_Node;
                                }
                                else {
                                    ALL_DEVICE_DRIVES_Current_Node = Append_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Current_Node, &dirInfo->Name, &linkTarget, NULL, &result);
                                }


                            }
                            else {
                                // �̹����� ��, UPDATE��
                                Update_ALL_DEVICE_DRIVES_Node(Node_from_Finder, &dirInfo->Name, &result);
                            }


                            //Print_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node);


                        }
                        Complete_ALL_DEVICE_DRIVED_Node(&ALL_DEVICE_DRIVES_Start_Node, &ALL_DEVICE_DRIVES_Current_Node);

                    }

                    /*
                        USB���� Ȯ��.
                    */



                }
                ZwClose(linkHandle);
            }
        }
    }

    KeReleaseMutex(Get_Volume_KMUTEX, FALSE);
    ExFreePoolWithTag(buffer, 'dirb');
    ZwClose(dirHandle);
}


// USB���� �����ϵ����� ... ENUM���� ��ȯ
DEVICE_DECTECT_ENUM is_external_Device(PUNICODE_STRING Obj_Dir_NAME, PUNICODE_STRING NT_NAME, PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING) {
    PNP_Device_Name_from_PNP_UNICODE_STRING;
    // ���͸� �����ڵ��
    UNICODE_STRING filter_string[] = {
        {0,},
        {0,}
    };
    RtlInitUnicodeString(&filter_string[0], L"STORAGE#Volume#{"); // �ϵ��ũ
    RtlInitUnicodeString(&filter_string[1], L"STORAGE#Volume#_??_USBSTOR#"); // USB ����

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " �ϱ��� %wZ\n", *Obj_Dir_NAME);
    // ����Ʈ�� ��ġ���� ( ���ĺ�Volume�� ���� �͵��̶�� ��� �ش�ȴ�. ) 
    if (Obj_Dir_NAME->Length == 4 && Obj_Dir_NAME->Buffer[1] == L':' &&
        ((Obj_Dir_NAME->Buffer[0] >= L'A' && Obj_Dir_NAME->Buffer[0] <= L'Z') ||
            (Obj_Dir_NAME->Buffer[0] >= L'a' && Obj_Dir_NAME->Buffer[0] <= L'z'))) {

        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Mounted_MSSS_DOSSS Drive: %wZ -> %wZ\n", *Obj_Dir_NAME, *NT_PATH);

        return JUST_VOLUME_DISK;
    }

    // �����ϵ�����
    if (memcmp(Obj_Dir_NAME->Buffer, filter_string[0].Buffer, sizeof(L"STORAGE#Volume#{") - sizeof(WCHAR)) == 0) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Internal_DISK �Դϴ�.\n");
        return Internal_DISK;
    }


    // USB �����ϵ� ���� Ȯ�� ( �̶��� ���� ���� "��" �� )
    PWCH filter = L"USB#";
    PWCH filter2 = L"\\??\\USB#"; // PNP���� ������ �ɺ��� ��ũ

    // USB#�� �����ϴ��� Ȯ��
    if (
        (memcmp(Obj_Dir_NAME->Buffer, filter, sizeof(L"USB#") - sizeof(WCHAR)) == 0) ||
        (memcmp(Obj_Dir_NAME->Buffer, filter2, sizeof(L"\\??\\USB#") - sizeof(WCHAR)) == 0)

        ) {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PNP���κ����� USB�̸�, ������ ��� �� �Դϴ�.\n");
        /*
            �� ������  ����Ʈ�� ��ġ�� ������ ��� ���̹Ƿ�

            1. �ø��� ����
            2. ��忡 �ø��� ������� Ž��
            3. ��� ������ �ø��� ���� ��� ����

             ����� ��κ� PNP���� ���Ǵ� ����.
            4. �ʿ�� ������Ʈ
        */

        UNICODE_STRING get_SERIAL_NUM = { 0, }; // USB ����̽��κ��� �ø����� ������ ������ 
        DEVICE_DECTECT_ENUM result = USB_DEVICE_from_PNP;
        if (PNP_Device_Name_from_PNP_UNICODE_STRING != NULL) {
            /*
                ���⼭ ���� PNP �ݹ��Լ��� �񵿱� �����忡�� ȣ���� ���

            */
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [�ø���] USB_DEVICE_from_PNP memcmp( %wZ vs %wZ )\n", PNP_Device_Name_from_PNP_UNICODE_STRING, NT_NAME);
            if (memcmp(PNP_Device_Name_from_PNP_UNICODE_STRING->Buffer, NT_NAME->Buffer, PNP_Device_Name_from_PNP_UNICODE_STRING->Length) == 0) {
                if (Get_USB_Serial(Obj_Dir_NAME, &get_SERIAL_NUM, 2, FALSE)) {// �ø��� ���� �� �ø���ȿ� "&" �� ����, �ʿ���� 
                    /*
                       USB �ø��� ���������� ���� ��
                   */
                    PALL_DEVICE_DRIVES Node_from_Finder = NULL;
                    Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, NULL, &get_SERIAL_NUM, NULL);
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [�ø���] Finder_ALL_DEVICE_DRIVES_Node -> %p\n", Node_from_Finder);
                    if (Node_from_Finder == NULL) {
                        // ���� ��忡 ���� �� ���� ��� ����
                        if (ALL_DEVICE_DRIVES_Start_Node == NULL) {

                            ALL_DEVICE_DRIVES_Start_Node = Create_ALL_DEVICE_DRIVES_Node(NULL, Obj_Dir_NAME, NT_NAME, &get_SERIAL_NUM, &result);

                            ALL_DEVICE_DRIVES_Current_Node = ALL_DEVICE_DRIVES_Start_Node;
                        }
                        else {
                            ALL_DEVICE_DRIVES_Current_Node = Append_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Current_Node, Obj_Dir_NAME, NT_NAME, &get_SERIAL_NUM, &result);
                        }


                    }
                    else {
                        // �̹����� ��, UPDATE��
                        //Update_ALL_DEVICE_DRIVES_Node(Node_from_Finder, &dirInfo->Name, &result);
                    }
                    //////////////Print_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node);

                }
            }






        }
        return USB_DEVICE_from_PNP;
    }



    // USB �����ϵ� ���� Ȯ�� ( �̶��� ���� ���� "��" �� )
    if (memcmp(Obj_Dir_NAME->Buffer, filter_string[1].Buffer, sizeof(L"STORAGE#Volume#_??_USBSTOR#") - sizeof(WCHAR)) == 0) {
        /*
            �� ������ �̹� ����Ʈ�� ��ġ�� ������ ���� ���̱� ������

            1. �ø��� ����
            2. ��忡 �ø��� ������� Ž��
            3. ��� ������ �ø��� ���� ��� ����
            4. ��� ������ ������Ʈ

        */
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "External_DISK_USB �Դϴ�. %wZ -> %wZ\n", *Obj_Dir_NAME, *NT_NAME);
        /*
            �ø��� ���� �� �̹� �ø����� ��忡 �ִ� ��� UPDATE / ������ ����
        */
        UNICODE_STRING get_SERIAL_NUM;
        DEVICE_DECTECT_ENUM result = External_DISK_USB;
        if (Get_USB_Serial(Obj_Dir_NAME, &get_SERIAL_NUM, 4, TRUE)) {// �ø��� ���� �� �ø���ȿ� "&" �� �߰��� �ִٸ�, ���͸���
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " ���� USB: %wZ \n", get_SERIAL_NUM);



            PALL_DEVICE_DRIVES Node_from_Finder = NULL;
            Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, NULL, &get_SERIAL_NUM, NULL);
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [�ø���] Finder_ALL_DEVICE_DRIVES_Node ///  External_DISK_USB -> %p\n", Node_from_Finder);
            if (Node_from_Finder == NULL) {

                // NT ��η� ��Ž��
                Node_from_Finder = Finder_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node, NT_NAME, NULL, NULL);
                if (Node_from_Finder == NULL) {
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [�ø���] �ι����� Ʋ��\n");
                    // ���� ��忡 ���� �� ���� ��� ����
                    if (ALL_DEVICE_DRIVES_Start_Node == NULL) {

                        ALL_DEVICE_DRIVES_Start_Node = Create_ALL_DEVICE_DRIVES_Node(NULL, Obj_Dir_NAME, NT_NAME, &get_SERIAL_NUM, &result);

                        ALL_DEVICE_DRIVES_Current_Node = ALL_DEVICE_DRIVES_Start_Node;
                    }
                    else {
                        ALL_DEVICE_DRIVES_Current_Node = Append_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Current_Node, Obj_Dir_NAME, NT_NAME, &get_SERIAL_NUM, &result);
                    }
                }
                else {
                    // �̹����� ��, UPDATE��
                    Update_ALL_DEVICE_DRIVES_Node(Node_from_Finder, Obj_Dir_NAME, &result);
                }

            }
            else {
                // �̹����� ��, UPDATE��
                Update_ALL_DEVICE_DRIVES_Node(Node_from_Finder, Obj_Dir_NAME, &result);
            }
            //////////////Print_ALL_DEVICE_DRIVES_Node(ALL_DEVICE_DRIVES_Start_Node);

        }
        return External_DISK_USB;
    }





    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Drive: %wZ -> %wZ\n", *Obj_Dir_NAME, *NT_PATH);
    return DEVICE_None;
}

BOOLEAN Get_USB_Serial(PUNICODE_STRING INPUT_USB_DIR_INFO, PUNICODE_STRING OUTPUT_USB_SERIAL_NUMBER, ULONG32 Filter_Index, BOOLEAN Filter_with_Ampersand) {
    if (OUTPUT_USB_SERIAL_NUMBER == NULL) return FALSE;

    PWCH filter = L"USB#"; // �׳� �Ϲ� �Լ����� ������ ������Ʈ ���
    PWCH filter2 = L"\\??\\USB#"; // PNP������ ������ �ɺ��� ��ũ
    PWCH filter3 = L"STORAGE#Volume#_??_USBSTOR#"; // USB�� �ý��ۿ� ��ϵ� ���� ( ������ �Ҵ���� )

    // USB#�� �����ϴ��� Ȯ��
    if (

        (memcmp(INPUT_USB_DIR_INFO->Buffer, filter, sizeof(L"USB#") - sizeof(WCHAR)) == 0) || // �׳� �Ϲ� �Լ����� ������ ������Ʈ ����ΰ�?
        (memcmp(INPUT_USB_DIR_INFO->Buffer, filter2, sizeof(L"\\??\\USB#") - sizeof(WCHAR)) == 0) ||// PNP���� ������ �ɺ��� ��ũ�ΰ�?
        (memcmp(INPUT_USB_DIR_INFO->Buffer, filter3, sizeof(L"STORAGE#Volume#_??_USBSTOR#") - sizeof(WCHAR)) == 0)

        ) {

        ULONG32 detect_count = Filter_Index;// 2; // 2��° "#" ã��
        ULONG32 current_detect_count = 0;
        BOOLEAN is_SERIAL_GETTING = FALSE;

        // �ʱ�ȭ
        OUTPUT_USB_SERIAL_NUMBER->Length = 0;
        OUTPUT_USB_SERIAL_NUMBER->MaximumLength = INPUT_USB_DIR_INFO->MaximumLength;
        OUTPUT_USB_SERIAL_NUMBER->Buffer = ExAllocatePoolWithTag(NonPagedPool, OUTPUT_USB_SERIAL_NUMBER->MaximumLength, 'Seri');
        if (OUTPUT_USB_SERIAL_NUMBER->Buffer == NULL) return FALSE;
        memset(OUTPUT_USB_SERIAL_NUMBER->Buffer, 0, OUTPUT_USB_SERIAL_NUMBER->MaximumLength);

        ULONG32 SERIAL_current_offset = 0;

        // USB ���丮 ������ �� ���ھ� Ȯ��
        for (ULONG32 count = 0; count < INPUT_USB_DIR_INFO->Length / sizeof(WCHAR); count++) {
            if (is_SERIAL_GETTING) {
                // �ø��� ��ȣ �� �˻�
                if ((INPUT_USB_DIR_INFO->Buffer[count] == L'#') || (Filter_with_Ampersand && (INPUT_USB_DIR_INFO->Buffer[count] == L'&'))) {
                    is_SERIAL_GETTING = FALSE;

                    OUTPUT_USB_SERIAL_NUMBER->Buffer[SERIAL_current_offset] = L'\0';
                    OUTPUT_USB_SERIAL_NUMBER->Length = (USHORT)(SERIAL_current_offset * sizeof(WCHAR)); // Length�� ���� �� ���� �ؾ���
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " ----SERIAL----- %wZ \n", OUTPUT_USB_SERIAL_NUMBER);

                    return TRUE;
                }

                // �ø��� ��ȣ ����
                if (SERIAL_current_offset < OUTPUT_USB_SERIAL_NUMBER->MaximumLength / sizeof(WCHAR) - 1) {
                    OUTPUT_USB_SERIAL_NUMBER->Buffer[SERIAL_current_offset] = INPUT_USB_DIR_INFO->Buffer[count];
                    SERIAL_current_offset += 1;
                }
                else {
                    break; // ���� �����÷� ����
                }
                continue;
            }

            // "#" ã��
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "%wc\n", INPUT_USB_DIR_INFO->Buffer[count]);
            if (INPUT_USB_DIR_INFO->Buffer[count] == L'#') {
                current_detect_count += 1;

                if (current_detect_count == detect_count) {
                    is_SERIAL_GETTING = TRUE;
                }
            }
        }

        // �ø��� ��ȣ�� ã�� ���� ��� �޸� ����
        ExFreePoolWithTag(OUTPUT_USB_SERIAL_NUMBER->Buffer, 'Seri');
        OUTPUT_USB_SERIAL_NUMBER->Buffer = NULL;
    }

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " --- serial FALSE \n");
    return FALSE;
}



/*



    ���Ḯ��Ʈ �κ�




*/
PALL_DEVICE_DRIVES Create_ALL_DEVICE_DRIVES_Node(
    PUCHAR Previous_addr,

    PUNICODE_STRING DRIVE_ALPHABET,
    PUNICODE_STRING DRIVE_NT_PATH,

    PUNICODE_STRING USB_Serial,

    PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE
) {
    PALL_DEVICE_DRIVES New_Node = ExAllocatePoolWithTag(NonPagedPool, sizeof(ALL_DEVICE_DRIVES), 'DVDR');
    if (New_Node == NULL) return NULL;
    memset(New_Node, 0, sizeof(ALL_DEVICE_DRIVES));

    if (Previous_addr == NULL) {
        New_Node->Previous_Node = NULL;
    }
    else {
        New_Node->Previous_Node = Previous_addr;
    }



    //DRIVE_ALPHABET
    if (DRIVE_ALPHABET != NULL) {
        New_Node->DRIVE_ALPHABET.Length = DRIVE_ALPHABET->Length;
        New_Node->DRIVE_ALPHABET.MaximumLength = DRIVE_ALPHABET->MaximumLength;
        New_Node->DRIVE_ALPHABET.Buffer = ExAllocatePoolWithTag(NonPagedPool, New_Node->DRIVE_ALPHABET.MaximumLength, 'DVDR');
        if (New_Node->DRIVE_ALPHABET.Buffer == NULL) {
            ExFreePoolWithTag(New_Node, 'DVDR');
            return NULL;
        }
        memcpy(New_Node->DRIVE_ALPHABET.Buffer, DRIVE_ALPHABET->Buffer, New_Node->DRIVE_ALPHABET.MaximumLength);
    }



    //DRIVE_NT_PATH
    if (DRIVE_NT_PATH != NULL) {
        New_Node->DRIVE_NT_PATH.Length = DRIVE_NT_PATH->Length;
        New_Node->DRIVE_NT_PATH.MaximumLength = DRIVE_NT_PATH->MaximumLength;
        New_Node->DRIVE_NT_PATH.Buffer = ExAllocatePoolWithTag(NonPagedPool, New_Node->DRIVE_NT_PATH.MaximumLength, 'DVDR');
        if (New_Node->DRIVE_NT_PATH.Buffer == NULL) {
            ExFreePoolWithTag(New_Node->DRIVE_ALPHABET.Buffer, 'DVDR');
            ExFreePoolWithTag(New_Node, 'DVDR');
            return NULL;
        }
        memcpy(New_Node->DRIVE_NT_PATH.Buffer, DRIVE_NT_PATH->Buffer, New_Node->DRIVE_NT_PATH.MaximumLength);
    }

    //USB_Serial
    if (USB_Serial != NULL) {
        New_Node->USBSTOR_Serial.Length = USB_Serial->Length;
        New_Node->USBSTOR_Serial.MaximumLength = USB_Serial->MaximumLength;
        New_Node->USBSTOR_Serial.Buffer = ExAllocatePoolWithTag(NonPagedPool, New_Node->USBSTOR_Serial.MaximumLength, 'DVDR');
        if (New_Node->USBSTOR_Serial.Buffer == NULL) {
            ExFreePoolWithTag(New_Node->DRIVE_ALPHABET.Buffer, 'DVDR');
            ExFreePoolWithTag(New_Node->DRIVE_NT_PATH.Buffer, 'DVDR');
            ExFreePoolWithTag(New_Node, 'DVDR');
            return NULL;
        }
        memcpy(New_Node->USBSTOR_Serial.Buffer, USB_Serial->Buffer, New_Node->USBSTOR_Serial.MaximumLength);
    }


    // ENUM
    if (DRIVE_DEVICE_TYPE != NULL) {
        New_Node->DRIVE_DEVICE_TYPE = *DRIVE_DEVICE_TYPE;
    }





    New_Node->Next_Node = NULL;

    return New_Node;
}



// APPEND
PALL_DEVICE_DRIVES Append_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES current_Node, PUNICODE_STRING DRIVE_ALPHABET, PUNICODE_STRING DRIVE_NT_PATH, PUNICODE_STRING USB_Serial, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE) {

    PALL_DEVICE_DRIVES New_Node = Create_ALL_DEVICE_DRIVES_Node((PUCHAR)current_Node, DRIVE_ALPHABET, DRIVE_NT_PATH, USB_Serial, DRIVE_DEVICE_TYPE);
    if (New_Node == NULL) return NULL;

    current_Node->Next_Node = (PUCHAR)New_Node;

    New_Node->Previous_Node = (PUCHAR)current_Node;

    return New_Node;
}


// PRINT
VOID Print_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node) {
    if (Start_Node == NULL) return;

    PALL_DEVICE_DRIVES CURRENT_Node = Start_Node;
    do {

        if (CURRENT_Node->USBSTOR_Serial.Length > 1) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " DRIVE_ALPHABET => %wZ / DRIVE_NT_PATH => %wZ / USBSTOR_Serial => %wZ / DRIVE_DEVICE_TYPE => %d \n", CURRENT_Node->DRIVE_ALPHABET, CURRENT_Node->DRIVE_NT_PATH, CURRENT_Node->USBSTOR_Serial, CURRENT_Node->DRIVE_DEVICE_TYPE);
        }
        else {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " DRIVE_ALPHABET => %wZ / DRIVE_NT_PATH => %wZ / DRIVE_DEVICE_TYPE => %d \n", CURRENT_Node->DRIVE_ALPHABET, CURRENT_Node->DRIVE_NT_PATH, CURRENT_Node->DRIVE_DEVICE_TYPE);
        }





        CURRENT_Node = (PALL_DEVICE_DRIVES)CURRENT_Node->Next_Node; // ���� ��� �̵�
    } while (CURRENT_Node != NULL);


    return;
}




//Finder
PALL_DEVICE_DRIVES Finder_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node, PUNICODE_STRING IMPORTANT_DRIVE_NT_PATH_hint, PUNICODE_STRING Serial_Number_hint, PDEVICE_DECTECT_ENUM Option_INPUT_DRIVE_DEVICE_TYPE) {
    if (Start_Node == NULL) return NULL;

    PALL_DEVICE_DRIVES CURRENT_Node = Start_Node;
    do {

        if (Option_INPUT_DRIVE_DEVICE_TYPE == NULL && IMPORTANT_DRIVE_NT_PATH_hint != NULL) {
            // NT_PATH
            if (RtlCompareUnicodeString(&CURRENT_Node->DRIVE_NT_PATH, IMPORTANT_DRIVE_NT_PATH_hint, TRUE) == 0) {
                return CURRENT_Node;
            }

        }
        else if (Option_INPUT_DRIVE_DEVICE_TYPE != NULL && IMPORTANT_DRIVE_NT_PATH_hint != NULL) {
            // NT_PATH + ENUM
            if ((RtlCompareUnicodeString(&CURRENT_Node->DRIVE_NT_PATH, IMPORTANT_DRIVE_NT_PATH_hint, TRUE) == 0) && (CURRENT_Node->DRIVE_DEVICE_TYPE == *Option_INPUT_DRIVE_DEVICE_TYPE)) {
                return CURRENT_Node;
            }
        }
        else if (Option_INPUT_DRIVE_DEVICE_TYPE == NULL && IMPORTANT_DRIVE_NT_PATH_hint == NULL && Serial_Number_hint != NULL) {
            // USB ������ ������
            // USB_Serial
            if (RtlCompareUnicodeString(&CURRENT_Node->USBSTOR_Serial, Serial_Number_hint, TRUE) == 0) {
                return CURRENT_Node;
            }
        }


        CURRENT_Node = (PALL_DEVICE_DRIVES)CURRENT_Node->Next_Node; // ���� ��� �̵�
    } while (CURRENT_Node != NULL);

    return NULL;
}



//UPDATE
PALL_DEVICE_DRIVES Update_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Spectified_Node_valid, PUNICODE_STRING DRIVE_ALPHABET, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE) {
    if (Spectified_Node_valid == NULL) return NULL;


    if (DRIVE_ALPHABET != NULL) {
        if (

            DRIVE_ALPHABET->Length == 4 && DRIVE_ALPHABET->Buffer[1] == L':' &&
            ((DRIVE_ALPHABET->Buffer[0] >= L'A' && DRIVE_ALPHABET->Buffer[0] <= L'Z') ||
                (DRIVE_ALPHABET->Buffer[0] >= L'a' && DRIVE_ALPHABET->Buffer[0] <= L'z'))

            ) {
            // C: D: �� ���� ���ڿ��ΰ�?

            memset(Spectified_Node_valid->DRIVE_ALPHABET.Buffer, 0, Spectified_Node_valid->DRIVE_ALPHABET.MaximumLength);
            ExFreePoolWithTag(Spectified_Node_valid->DRIVE_ALPHABET.Buffer, 'DVDR');

            Spectified_Node_valid->DRIVE_ALPHABET.Length = DRIVE_ALPHABET->Length;
            Spectified_Node_valid->DRIVE_ALPHABET.MaximumLength = DRIVE_ALPHABET->MaximumLength;
            Spectified_Node_valid->DRIVE_ALPHABET.Buffer = ExAllocatePoolWithTag(NonPagedPool, Spectified_Node_valid->DRIVE_ALPHABET.MaximumLength, 'DVDR');
            if (Spectified_Node_valid->DRIVE_ALPHABET.Buffer == NULL) return NULL;

            memcpy(Spectified_Node_valid->DRIVE_ALPHABET.Buffer, DRIVE_ALPHABET->Buffer, Spectified_Node_valid->DRIVE_ALPHABET.MaximumLength);

        }
    }


    // ���͸� �����ڵ��
    UNICODE_STRING filter_string[] = {
        {0,},
        {0,}

    };
    RtlInitUnicodeString(&filter_string[0], L"STORAGE#Volume#{"); // �ϵ��ũ
    RtlInitUnicodeString(&filter_string[1], L"STORAGE#Volume#_??_USBSTOR#"); // USB ����



    // ENUM����
    if (DRIVE_DEVICE_TYPE != NULL) {
        Spectified_Node_valid->DRIVE_DEVICE_TYPE = *DRIVE_DEVICE_TYPE;
    }




    return Spectified_Node_valid;
}

PALL_DEVICE_DRIVES is_Drives_PATH(PUNICODE_STRING INPUT_ABSOULTE_PATH) {
    if (ALL_DEVICE_DRIVES_Start_Node == NULL || ALL_DEVICE_DRIVES_Current_Node == NULL || INPUT_ABSOULTE_PATH == NULL) return NULL;



    PALL_DEVICE_DRIVES current_NODE = ALL_DEVICE_DRIVES_Start_Node;
    do {

        if (

            current_NODE->DRIVE_ALPHABET.Length == 4 && current_NODE->DRIVE_ALPHABET.Buffer[1] == L':' &&
            ((current_NODE->DRIVE_ALPHABET.Buffer[0] >= L'A' && current_NODE->DRIVE_ALPHABET.Buffer[0] <= L'Z') ||
                (current_NODE->DRIVE_ALPHABET.Buffer[0] >= L'a' && current_NODE->DRIVE_ALPHABET.Buffer[0] <= L'z'))

            ) {
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "is_Drives_PATH memcmp(%wZm %wZ) \n", INPUT_ABSOULTE_PATH, current_NODE->DRIVE_NT_PATH);
            if (memcmp(INPUT_ABSOULTE_PATH->Buffer, current_NODE->DRIVE_NT_PATH.Buffer, current_NODE->DRIVE_NT_PATH.Length - 2) == 0) {
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "is_Drives_PATH ���ĺ� ����̺� ���� ���� ! %wZ TYPE -> %d \n", current_NODE->DRIVE_ALPHABET, current_NODE->DRIVE_DEVICE_TYPE);
                return current_NODE;
            }

        }



        current_NODE = (PALL_DEVICE_DRIVES)current_NODE->Next_Node;
    } while (current_NODE != NULL);


    return NULL;
}



//Ư��-��� ���� 
BOOLEAN Remove_Specified_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node, PALL_DEVICE_DRIVES Specified_Node) {
    if (Specified_Node == NULL || Start_Node == NULL || Current_Node == NULL || *Start_Node == NULL) return FALSE;

    Specified_Node->Next_Node;
    Specified_Node->Previous_Node;

    if (Specified_Node->Previous_Node == NULL && Specified_Node->Next_Node == NULL) {
        /*
            ���� ���Ḯ��Ʈ �ּҰ� ó�� �ּ��� ��,
        */
        if (*Start_Node == Specified_Node) {
            *Start_Node = (PALL_DEVICE_DRIVES)Specified_Node->Next_Node;
        }

        if (*Start_Node == *Current_Node) *Current_Node = *Start_Node;


    }
    else if (Specified_Node->Previous_Node == NULL && Specified_Node->Next_Node != NULL) {
        ((PALL_DEVICE_DRIVES)Specified_Node->Next_Node)->Previous_Node = NULL;
        if (*Start_Node == Specified_Node) {
            *Start_Node = (PALL_DEVICE_DRIVES)Specified_Node->Next_Node;
        }

        if (*Start_Node == *Current_Node) *Current_Node = *Start_Node;
    }
    else if (Specified_Node->Previous_Node && Specified_Node->Next_Node) {
        /*
            ��尡 �߰��� ���ִ� ���.
        */
        ((PALL_DEVICE_DRIVES)Specified_Node->Next_Node)->Previous_Node = (PUCHAR)((PALL_DEVICE_DRIVES)Specified_Node->Previous_Node);

    }
    else if (Specified_Node->Previous_Node && Specified_Node->Next_Node == NULL) {
        /*
            ��� �������� ���, ( Head�ƴ� )
        */
        ((PALL_DEVICE_DRIVES)Specified_Node->Previous_Node)->Next_Node = NULL;

        if (*Current_Node == Specified_Node) *Current_Node = (PALL_DEVICE_DRIVES)Specified_Node->Previous_Node;


    }
    else {
        return FALSE;
    }

    if (Specified_Node->DRIVE_ALPHABET.Buffer != NULL) {
        ExFreePoolWithTag(Specified_Node->DRIVE_ALPHABET.Buffer, 'DVDR');
    }

    if (Specified_Node->DRIVE_NT_PATH.Buffer != NULL) {
        ExFreePoolWithTag(Specified_Node->DRIVE_NT_PATH.Buffer, 'DVDR');
    }

    if (Specified_Node->USBSTOR_Serial.Buffer != NULL) {
        ExFreePoolWithTag(Specified_Node->USBSTOR_Serial.Buffer, 'DVDR');
    }

    ExFreePoolWithTag(Specified_Node, 'DVDR');

    return TRUE;
}




BOOLEAN Complete_ALL_DEVICE_DRIVED_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node) {
    if (*Start_Node == NULL || *Current_Node == NULL) return FALSE;

    PALL_DEVICE_DRIVES current_Node = *Start_Node;

    do {
        PALL_DEVICE_DRIVES tmp = (PALL_DEVICE_DRIVES)current_Node->Next_Node;
        if (

            !(
                current_Node->DRIVE_ALPHABET.Length == 4 && current_Node->DRIVE_ALPHABET.Buffer[1] == L':' &&
                ((current_Node->DRIVE_ALPHABET.Buffer[0] >= L'A' && current_Node->DRIVE_ALPHABET.Buffer[0] <= L'Z') ||
                    (current_Node->DRIVE_ALPHABET.Buffer[0] >= L'a' && current_Node->DRIVE_ALPHABET.Buffer[0] <= L'z'))
                )

            ) {

            // 2�� ���� - USB�ø����� ��ȿ�� ��� �������� ����.  ( ����̺� ���ڴ� ������, USB�ø����� ��ȿ�ϸ� �ش� ��� ���� ���� )
            if (current_Node->USBSTOR_Serial.Buffer == NULL || current_Node->USBSTOR_Serial.Length == 0) {

                if (!Remove_Specified_ALL_DEVICE_DRIVES_Node(Start_Node, Current_Node, current_Node)) return FALSE;

            }



        }
        else {
            // current_Node ����̺� ���ڰ� ������, �ߺ��� ���� ���� �����ؾ���. 

            PALL_DEVICE_DRIVES current_Node_for_drives_char_node = current_Node;
            do {


                current_Node_for_drives_char_node = (PALL_DEVICE_DRIVES)current_Node_for_drives_char_node->Next_Node;
                if (current_Node_for_drives_char_node == NULL) break;


                if (memcmp(current_Node_for_drives_char_node->DRIVE_ALPHABET.Buffer, current_Node->DRIVE_ALPHABET.Buffer, 4) == 0) {
                    if (!Remove_Specified_ALL_DEVICE_DRIVES_Node(Start_Node, Current_Node, current_Node_for_drives_char_node)) return FALSE;
                }

            } while (current_Node_for_drives_char_node != NULL);

        }


        current_Node = tmp;
    } while (current_Node != NULL);



    Print_ALL_DEVICE_DRIVES_Node(*Start_Node);
    return TRUE;

}