#include "minifilter_handlers.h"
#include <ntstrsafe.h>


// PRE ���ؽ�Ʈ
pre_filter_struct Pre_struct_Share_variable = { 0, };


const FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_CREATE_NAMED_PIPE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_CLOSE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_READ, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_WRITE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_INFORMATION, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_INFORMATION, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_EA, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_EA, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_FLUSH_BUFFERS, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_VOLUME_INFORMATION, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_VOLUME_INFORMATION, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_DIRECTORY_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_FILE_SYSTEM_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_DEVICE_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SHUTDOWN, 0, Pre_filter_Handler, NULL }, // No post-operation callback
    { IRP_MJ_LOCK_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_CLEANUP, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_CREATE_MAILSLOT, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_SECURITY, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_SECURITY, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_POWER, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SYSTEM_CONTROL, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_DEVICE_CHANGE, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_QUERY_QUOTA, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_SET_QUOTA, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_PNP, 0, Pre_filter_Handler, NULL },
    { IRP_MJ_OPERATION_END } // Array termination
};



NTSTATUS is_file(PFLT_CALLBACK_DATA Input_Data, PFLT_FILE_NAME_INFORMATION* Output_fileNameInfo) {

    
    // ��������ΰ�? 
    if (Input_Data->RequestorMode != UserMode) return STATUS_UNSUCCESSFUL;

    if (FltGetFileNameInformation(Input_Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, Output_fileNameInfo) != STATUS_SUCCESS) return STATUS_UNSUCCESSFUL;


    // ���� �̸� ����
    if (FltParseFileNameInformation(*Output_fileNameInfo) != STATUS_SUCCESS) {

        FltReleaseFileNameInformation(*Output_fileNameInfo);
        return STATUS_UNSUCCESSFUL;
    }





    
    // �ñ״�ó ���� ( �����κ��� ��å ������ ) - test
    UNICODE_STRING abc[] = { { 0, }, { 0, },{ 0, }, { 0, } };
    RtlInitUnicodeString(&abc[0], L".hwp");
    RtlInitUnicodeString(&abc[1], L".zip");
    RtlInitUnicodeString(&abc[2], L".pdf");
    RtlInitUnicodeString(&abc[3], L".hwpx");
    //RtlInitUnicodeString(&abc[2], L".exe");


    BOOLEAN is_TRUE = FALSE;
    for (ULONG32 index = 0; index < sizeof(abc) / 16; index++) {
        if (ContainsStringInsensitive(&((PFLT_FILE_NAME_INFORMATION)*Output_fileNameInfo)->Name, &abc[index])) {
            is_TRUE = TRUE;
            break;
        }
    }
    if (!is_TRUE) {
        FltReleaseFileNameInformation(*Output_fileNameInfo);
        return STATUS_UNSUCCESSFUL;
    }
    


    return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS return_pre(PFLT_FILE_NAME_INFORMATION fileNameInfo, FLT_PREOP_CALLBACK_STATUS Return_Value) {
    FltReleaseFileNameInformation(fileNameInfo);
    KeReleaseMutex(&MUTEX_signature, FALSE);
    return Return_Value;
}


// �ݹ� �Լ� ����


FLT_PREOP_CALLBACK_STATUS
Pre_filter_Handler(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    
    // [1] ���� ���� ��� ( �����ΰ�? )
    /*
        1. ��������ΰ�?
        2. �����ΰ�?
        3. ��å���� ���� �ñ״�ó�ΰ�?
    
    */
    PFLT_FILE_NAME_INFORMATION fileNameInfo;
    if (is_file(Data, &fileNameInfo) != STATUS_SUCCESS) return FLT_PREOP_SUCCESS_NO_CALLBACK;

    KeWaitForSingleObject(&MUTEX_signature, Executive, KernelMode, FALSE, NULL);

    /*
        [2] ���� ���� ���� ( ���͸��� ���� ���� ) 
        **����ȭ**
        * 
        * IRP_MJ_CREATE �� ����������, READ �� WRITE�� ȣ��Ǵ� ����� ( ��������(CREATE�ϰ� �� CREATEȣ��� �� ����). ) 
        * 
        * IRP_MJ_CREATE�� ACCESS_MASK�� �������, ���� ȣ�� IRP��  READ���� WRITE�������� ������ �� �� ���� ��
        * 
    */
    if (Data->Iopb->MajorFunction == IRP_MJ_CREATE) {

        HANDLE ph = is_valid_FILE(&fileNameInfo->Name);
        if (ph > 0) {
            ZwClose(ph);
            Pre_struct_Share_variable.is_Success_open_that_FILE_when_IRP_MJ_CREATE = TRUE;
        }
        else {
            Pre_struct_Share_variable.is_Success_open_that_FILE_when_IRP_MJ_CREATE = FALSE;
            // ���⼭ ���� ��Ȳ
            // �ٸ� ��ο� �ٿ��ֱ� �õ��� �� ����.
            /*
                �ٸ� ��ΰ� ���ѵ� �����̸� ������ �ʿ䰡 ����. ( Drives���Ḯ��Ʈ ��ȸ�Ͽ� ������ ���ѵ� ����̺��� ��� ���� ) 
            */
        }
        


        /*
            ACCESS_MASK ����
            ���� �� ���� ���� IRP�� ������ �� ����
        */
        ULONG FILE_ACCESS_MASK = (ULONG)Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess;
        

        if (Pre_struct_Share_variable.is_IRP_monitoring_active == TRUE && Pre_struct_Share_variable.is_get_something == TRUE) {
            /*
            * [����]
                ������� �м� ( �ʵ尡 TRUE�� �� �۵��Ǿ���� )
            */
            // ACCESS_MASK�� �߿��� IRP �� & ���� �ϳ��� Ȯ���Ͽ� ����м� ����

            /*
                ������ ��� ������ �ϳ��� �־� �ݺ������� Ȯ��
            */
            /*
            FILE_behavior_ENUM result = (FILE_behavior_ENUM)None;
            for (ULONG32 index = 0; index < 6; index++) {

                result = Get_FILE_behavior(Pre_struct_Share_variable.PreCreate_Context.ACCESS_MASK, Pre_struct_Share_variable.PreCreate_Context.IRP_MajorFunction_append_value_for_sum, file_behavior_list[index]);
                if (result != (FILE_behavior_ENUM)None) {
                    break;
                }
            }
            if (result != (FILE_behavior_ENUM)None) {
                // �˾Ƽ� ó�� 
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "CREATE: ������� -> %d \n", result);
            }
            */
            
            

        }
        else if (Pre_struct_Share_variable.is_IRP_monitoring_active == TRUE && Pre_struct_Share_variable.is_get_something == FALSE) {
            /*
            *  * [����]
                CREATE�� ���� 2���� ���,,
            */
            // ACCESS_MASK�� ������ �м��� �� �ۿ� ����


           
        }


        /*
        *  * [����]
            ���� �ʱ�ȭ set
        */
        Pre_struct_Share_variable.is_IRP_monitoring_active = FALSE; Pre_struct_Share_variable.is_get_something = FALSE;
        
        

        

        // ������ ���⼭ Ȯ�ΰ���


        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "CREATE: FileName = %wZ  ACCESS_MASK %lu\n", &fileNameInfo->Name, FILE_ACCESS_MASK);

        
        if (Pre_struct_Share_variable.is_IRP_monitoring_active == FALSE) {
            Pre_struct_Share_variable.keep_DENIED = FALSE;

            /*
            *  * [����]
                �������� �ʱ�ȭ
            */
            if (fileNameInfo->Name.MaximumLength > sizeof(Pre_struct_Share_variable.PreCreate_Context.FULL_PATH)) goto LABEL_0; // ���� �Ѿ�� Ż��


            Pre_struct_Share_variable.PreCreate_Context.is_changed_FULL_PATH = FALSE;
            memcpy(Pre_struct_Share_variable.PreCreate_Context.FULL_PATH, fileNameInfo->Name.Buffer, fileNameInfo->Name.MaximumLength);
            Pre_struct_Share_variable.PreCreate_Context.FULL_PATH_LENGTH = fileNameInfo->Name.MaximumLength;
            Pre_struct_Share_variable.PreCreate_Context.ACCESS_MASK = FILE_ACCESS_MASK;
            Pre_struct_Share_variable.PreCreate_Context.IRP_MajorFunction_append_value_for_sum = 0;


            Pre_struct_Share_variable.is_get_something = FALSE;
            Pre_struct_Share_variable.is_IRP_monitoring_active = TRUE;


            Pre_struct_Share_variable.is_get_SHA256 = FALSE;
            Pre_struct_Share_variable.PreCreate_Context.is_changed_SHA256 = FALSE;
            memset(Pre_struct_Share_variable.PreCreate_Context.SHA256, 0, SHA256_String_Byte_Length);

        }


        /* �ٸ� ������ �����̵� ���� if��  */
        if ( is_copy_move(FILE_ACCESS_MASK, Pre_struct_Share_variable.is_Success_open_that_FILE_when_IRP_MJ_CREATE) ) {

            /*
                �����̵��� �´ٸ� ���Ḯ��Ʈ�� �߰�
            */


            /*
                �����ϵ忡 �����Ŷ�� ����. (�⺻��)
            */
            if (is_External_NT_PATH(&fileNameInfo->Name)) {
                goto LABEL_1;
            }
            
        }

    }

    // ���� ������ ��� ������.
    if (Pre_struct_Share_variable.keep_DENIED) goto LABEL_1;


    if (Data->Iopb->MajorFunction == IRP_MJ_READ) {
        /*
            ���� ����
        */
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "READ: FileName = %wZ\n", &fileNameInfo->Name);

        
        
    }
    else if (Data->Iopb->MajorFunction == IRP_MJ_WRITE) {
        if (Pre_struct_Share_variable.keep_DENIED) goto LABEL_1;
        /*
            ���� ���� �� ����
        */
        //FILE_ACCESS_MASK = (ULONG)Data->Iopb->Parameters.Write.SecurityContext->DesiredAccess;
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "WRTIE: FileName = %wZ\n", &fileNameInfo->Name);

        /*
            �������� Ȯ���� �ʿ䰡 ���� ( �ؽñ��ϱ� ) 
        */

    }
    else if (Data->Iopb->MajorFunction == IRP_MJ_CLOSE) {
        /*
            ���� Ŭ��/����?
        */
        //FILE_ACCESS_MASK = (ULONG)Data->Iopb->Parameters.Read.SecurityContext->DesiredAccess;
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "CLOSE: FileName = %wZ\n", &fileNameInfo->Name);


    }
    else if (Data->Iopb->MajorFunction == IRP_MJ_SET_INFORMATION) {
        
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_SET_INFORMATION: FileName = %wZ , Data->Iopb->Parameters.SetFileInformation.FileInformationClass -> %d\n", &fileNameInfo->Name, Data->Iopb->Parameters.SetFileInformation.FileInformationClass);

        // ���� �̸� ���� ���͸�
        if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileRenameInformation ||
            Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileRenameInformationEx)
        {
            
            UNICODE_STRING newFileName = { 0, };
            PFILE_RENAME_INFORMATION renameInfo = (PFILE_RENAME_INFORMATION)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
            if (renameInfo != NULL)
            {
                // Extract the new file name
                
                newFileName.Length = (USHORT)renameInfo->FileNameLength;
                newFileName.MaximumLength = (USHORT)renameInfo->FileNameLength;
                newFileName.Buffer = renameInfo->FileName;
            }
            /*1. ���ϸ� ������ ��� ����?*/
            /*2. ���� �̵���, �� ��ġ�� ��� ����? ( �Ľ� �ʿ� ) */
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " ���� �̸� ���� ����@@ ����� �� -> %wZ\n", newFileName);

            // ���Ͽ���
            HANDLE FileHandle = is_valid_FILE(&fileNameInfo->Name);
            if (FileHandle > 0) {

                /*
                *   �ؽ� ���� �˾Ƴ��� ( ���� ���� )
                    �ϴ� �ؽ� ���� [2/3]
                */
                /* ���� ���̳ʸ� ���ϱ� */
                PVOID File_Bin = NULL;
                ULONG File_Bin_Length = 0;
                if (ALL_in_ONE_FILE_IO(&File_Bin, &File_Bin_Length, fileNameInfo->Name, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) {
                    ZwClose(FileHandle);
                    goto LABEL_0;
                }

                /* ����  �ؽ� ���ϱ� */
                UCHAR SHA256[SHA256_String_Byte_Length] = { 0, };
                if (GET_SHA256_HASHING(File_Bin, File_Bin_Length, (PCHAR)&SHA256) != STATUS_SUCCESS) {
                    ExFreePoolWithTag(File_Bin, 'FILE');
                    ZwClose(FileHandle);
                    goto LABEL_0;
                }
                ExFreePoolWithTag(File_Bin, 'FILE');


                /*
                    [1/2] ���� �ܼ� �̵��� ��� ��� ��ġ�� �����ϵ��� ���, �����ϱ� 
                */


                /*
                   [2/2]Ȯ���� ������ ��� �����ϱ�. 
               */
                Ppolicy_signature_struct SIG = NULL; // �ñ״�ó ���
                Ppolicy_signature_files_struct SIG_file = NULL; // �ش� �ñ״�ó���� ����� ���� ����Ʈ
                if (!Policy_Signature_Compare(

                    (PUCHAR)Policy_Signature_Start_Node,
                    &newFileName, // ������ ���ϸ����� �����Ѵ�. �̴� Ȯ���ڸ� �˻��ϱ� ����.( ��ȿ���� ���� ���ϸ���  )
                    (PUCHAR)&SHA256, // ���� ID�� ����Ͽ� ��带 ã���� 
                    &SIG,
                    signature_reEDIT_about_file_extension,// IRP_MJ_SET_INFORMATIO ����
                    &SIG_file,
                    NULL

                )) {
                    if (SIG_file != NULL) goto LABEL_1; // FALSE ����, SIG_file ���������� ������ ���ϸ� ������ ������ �� ����.
                }

            }

           
        }
        // ���� �ٿ��ֱ� Ȯ�� 
        else if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileEndOfFileInformation) {
            // ������ IRP_MJ_CRAETE��� ���� �õ��� ��   "����"�� ��쿩����. 
            if (Pre_struct_Share_variable.is_Success_open_that_FILE_when_IRP_MJ_CREATE == FALSE) { // ����

                //UNICODE_STRING newFileName = { 0, };
                FILE_END_OF_FILE_INFORMATION* endOfFileInfo = (FILE_END_OF_FILE_INFORMATION*)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
                if (endOfFileInfo != NULL)
                {
                    // Extract the new endOfFileInfo
                    endOfFileInfo->EndOfFile.QuadPart;
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "���� ��ġ ���� ���� IRP_MJ_SET_INFORMATION %llu\n ������ ��ġ %wZ ", endOfFileInfo->EndOfFile.QuadPart, &fileNameInfo->Name);


                    /* Defualt - �����ϵ� �� ��� ����. - �̶��� IRP_MJ_CREATE�� ���� ���� ��ȿ���� ���� �������� ����̺� ���ڿ��� ���� �Ǵ�. */

                    //Pre_struct_Share_variable.keep_DENIED = TRUE;
                    //goto LABEL_1;
                }
                
            }

            
        }
    }
    else if (Data->Iopb->MajorFunction == IRP_MJ_QUERY_INFORMATION) {

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_QUERY_INFORMATION , FILE_NAME = %wZ  FileInformationClass-> %d \n", &fileNameInfo->Name, Data->Iopb->Parameters.QueryFileInformation.FileInformationClass);

        if (Data->Iopb->Parameters.QueryFileInformation.FileInformationClass == FileBasicInformation) {
                /*
                typedef struct _FILE_BASIC_INFORMATION {
                    LARGE_INTEGER CreationTime;
                    LARGE_INTEGER LastAccessTime;
                    LARGE_INTEGER LastWriteTime;
                    LARGE_INTEGER ChangeTime;
                    ULONG         FileAttributes;
                } FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

                ���� �ٿ��ֱ� ����, �� ��,,,
                */

                
            if ((Pre_struct_Share_variable.is_Success_open_that_FILE_when_IRP_MJ_CREATE == FALSE) && (Data->Iopb->TargetFileObject && FltObjects->Instance)) {

                FILE_BASIC_INFORMATION basicInfo;
                if (FltQueryInformationFile(FltObjects->Instance, Data->Iopb->TargetFileObject, &basicInfo, sizeof(basicInfo), FileBasicInformation, NULL) == STATUS_SUCCESS) {
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[IRP_MJ_QUERY_INFORMATION - FileBasicInformation] ����_����_�ð�: %llu ����_����������_�ð�: %llu ����_����������_�ð�: %llu ����_����_�ð�: %llu\n",
                        basicInfo.CreationTime.QuadPart, basicInfo.LastAccessTime.QuadPart, basicInfo.LastWriteTime.QuadPart, basicInfo.ChangeTime.QuadPart);
               
                    /* ���� �ϵ� ��� ����. */

                    // USB ����̺� �ΰ�?
                    if (is_External_NT_PATH(&fileNameInfo->Name)) {
                        Pre_struct_Share_variable.keep_DENIED = TRUE;
                        goto LABEL_1; // ���� ���� ����.
                    }
                    
                }
            }
            //FileFsAttributeInformation




        }
        else if (Data->Iopb->Parameters.QueryFileInformation.FileInformationClass == FileStandardInformation) {

        }
            
        
        
    }
    else if(Data->Iopb->MajorFunction == IRP_MJ_QUERY_VOLUME_INFORMATION){
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_QUERY_VOLUME_INFORMATION , FILE_NAME = %wZ  FsInformationClass-> %d \n", &fileNameInfo->Name, Data->Iopb->Parameters.QueryVolumeInformation.FsInformationClass);
        
        if ( (Pre_struct_Share_variable.is_Success_open_that_FILE_when_IRP_MJ_CREATE == FALSE) || Data->Iopb->Parameters.QueryVolumeInformation.FsInformationClass == FileFsAttributeInformation) {

            /* ���� �ϵ� ��� ����. */
            PALL_DEVICE_DRIVES FOUND_DRIVE_NODE = is_Drives_PATH(&fileNameInfo->Name); // ����̺� ���ڰ� ��ȿ���� Ȯ�� 
            if (FOUND_DRIVE_NODE != NULL) {

                // USB ����̺� �ΰ�?
                if (FOUND_DRIVE_NODE->DRIVE_DEVICE_TYPE == External_DISK_USB) {
                    Pre_struct_Share_variable.keep_DENIED = TRUE;
                    goto LABEL_1; // ���� ���� ����.
                }
            }

            
        }

        
    
    }
    else {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Other [ %d ] :  FileName = %wZ\n", (ULONG32)Data->Iopb->MajorFunction, &fileNameInfo->Name);

    }
   
    // ���� ����
    if (Data->Iopb->MajorFunction != IRP_MJ_CREATE) {
        
        /*
        *  * [����]
            ��������
        */
        



        /*
            �ñ״�ó�� �˻��ؾ���
        */
        Ppolicy_signature_struct SIG = NULL; // �ñ״�ó ���
        Ppolicy_signature_files_struct SIG_file = NULL; // �ش� �ñ״�ó���� ����� ���� ����Ʈ
        if (Policy_Signature_Compare(

            (PUCHAR)Policy_Signature_Start_Node,
            &fileNameInfo->Name,
            NULL,
            &SIG,
            signature_COMPARE_Mode,
            &SIG_file,
            NULL

        )) {
            if (SIG_file == NULL) {// SIG_file �� �ʼ��� �����;��� ( ��, ���ο� ������ ��� NULL�� 
                /*
                    �ñ״�ó�� ������,, ���� ���� 2���� ���Ḯ��Ʈ�� ��带 �������Դ�?
                    �̴� ���ο� ������ �ǹ��Ѵ�. 
                */
                
                if ( Policy_Signature_Compare(
                    (PUCHAR)Policy_Signature_Start_Node,
                    &fileNameInfo->Name,
                    NULL,
                    NULL,
                    signature_SAVE_Mode,
                    NULL,
                    NULL // ���Ⱑ NULL�̸� ��Ÿ������ ���� ���ϰ� �� 
                ) == FALSE ) {
                    goto LABEL_0;
                }

                /*
                    ���ο� ��� �߰� �Ϸ�
                */
                if (Policy_Signature_Compare(

                    (PUCHAR)Policy_Signature_Start_Node,
                    &fileNameInfo->Name,
                    NULL,
                    &SIG,
                    signature_COMPARE_Mode,
                    &SIG_file,
                    NULL

                )) {
                    if (SIG_file == NULL) goto LABEL_0;
                }
                
            }
            Pre_struct_Share_variable.PreCreate_Context.IRP_MajorFunction_append_value_for_sum |= (ULONG32)Data->Iopb->MajorFunction;
            Pre_struct_Share_variable.is_get_something = TRUE;

            /*

                �Ʒ� �ڵ�� ���� ���� ���� Ž�� Ȯ�ο�

            */


            HANDLE FileHandle = is_valid_FILE(&fileNameInfo->Name);
            if (FileHandle>0) {
                /* ���� ���� ���� */
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile ���� ���� ���� \n" );
                /*
                    ���� ������ ���� �˾Ƴ��� [1/3]
                */
                if (wcscmp(Pre_struct_Share_variable.PreCreate_Context.FULL_PATH, fileNameInfo->Name.Buffer) == 0) {
                    // ����
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile -> wcscmp ���� \n");
                }
                else {
                    // ���� ������ �޶���
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile -> wcscmp �ٸ� \n");
                    Pre_struct_Share_variable.PreCreate_Context.is_changed_FULL_PATH = TRUE;
                }




                /*
                *   �ؽ� ���� �˾Ƴ��� ( ���� ���� )
                    �ϴ� �ؽ� ���� [2/3]
                */
                /* ���� ���̳ʸ� ���ϱ� */
                PVOID File_Bin = NULL;
                ULONG File_Bin_Length = 0;
                if (ALL_in_ONE_FILE_IO(&File_Bin, &File_Bin_Length, fileNameInfo->Name, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) {
                    ZwClose(FileHandle);
                    goto LABEL_0;
                }

                /* ����  �ؽ� ���ϱ� */
                UCHAR SHA256[SHA256_String_Byte_Length] = { 0, };
                if (GET_SHA256_HASHING(File_Bin, File_Bin_Length, (PCHAR)&SHA256) != STATUS_SUCCESS) {
                    ExFreePoolWithTag(File_Bin, 'FILE');
                    ZwClose(FileHandle);
                    goto LABEL_0;
                }
                ExFreePoolWithTag(File_Bin, 'FILE');



                if (Pre_struct_Share_variable.is_get_SHA256 == FALSE) {
                    /*
                        ���� SHA256 ���� �� ���� ��
                    */
                    memcpy((PUCHAR)&Pre_struct_Share_variable.PreCreate_Context.SHA256, (PUCHAR)&SHA256, SHA256_String_Byte_Length);


                    Pre_struct_Share_variable.is_get_SHA256 = TRUE;
                }
                else {
                    /*
                        SHA256 ������ �޾��� ��
                    */
                    // ���Ͽ� ' is_changed ' �Ǿ��� �� Ȯ��  
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " memcmp( %s, %s  )\n", Pre_struct_Share_variable.PreCreate_Context.SHA256, SHA256);

                    if (memcmp(Pre_struct_Share_variable.PreCreate_Context.SHA256, SHA256, SHA256_String_Byte_Length) == 0) {
                        // ���� ��
                        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile -> memcmp ���� \n");
                    }
                    else {
                        // �ٸ� ��
                        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile -> memcmp �ٸ� \n");
                        Pre_struct_Share_variable.PreCreate_Context.is_changed_SHA256 = TRUE;
                    }
                }
                // �ؽ� ���� �� 


                /* [3/3]

                    ��å �ñ״�ó ����� ��� ���Ḯ��Ʈ ��� �ϳ��� �ȿ� �ִ� ���� ���� ������ + SHA256�� ��� �� �ϳ��� ���Ḯ��Ʈ ��带 �������� ��.

                    2���� ���Ḯ��Ʈ ������

                    �̸� ���Ͽ� ���� �����ʹ� ������, ���ϸ��� ����Ǿ��� �� Ȯ�� ����

                */

                BOOLEAN is_changed_SHA256 = FALSE;
                BOOLEAN is_changed_FULL_PATH = FALSE;


                SIG = NULL; // �ñ״�ó ���
                SIG_file = NULL; // �ش� �ñ״�ó���� ����� ���� ����Ʈ
                if (Policy_Signature_Compare(

                    (PUCHAR)Policy_Signature_Start_Node,
                    &fileNameInfo->Name,
                    (PUCHAR)&SHA256,
                    &SIG,
                    signature_get_file_node_with__SHA256___but_idc_about_signature,
                    &SIG_file,
                    NULL

                )) {
                    /*
                        TRUE��, SHA256�� ����!
                    */
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Policy_Signature_Compare -> SHA256 TRUE\n");

                    is_changed_SHA256 = FALSE;
                }
                else {
                    is_changed_SHA256 = TRUE;
                }

                if (Policy_Signature_Compare(

                    (PUCHAR)Policy_Signature_Start_Node,
                    &fileNameInfo->Name,
                    (PUCHAR)&SHA256,
                    &SIG,
                    signature_get_file_node_with__FULL_PATH___but_idc_about_signature,
                    &SIG_file,
                    NULL

                )) {
                    /*
                        TRUE��, ���ϸ��� ����!
                    */

                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Policy_Signature_Compare -> fileNameInfo->Name TRUE\n");

                    is_changed_FULL_PATH = FALSE;
                }
                else {
                    is_changed_FULL_PATH = TRUE;
                }


                // ���� ���� 4���� ���¸� ���� 

                if (is_changed_SHA256 == FALSE && is_changed_FULL_PATH == FALSE) {
                    /*
                        ���� �������
                    */
                }
                else if (is_changed_SHA256 == TRUE && is_changed_FULL_PATH == FALSE) {
                    /*
                        SHA256�� �����

                        ���ϸ��� ������, ���� �����Ͱ� �ٲ�

                        (���)
                        �̷� ���� �ٽ� �ؽ� ���� ���ϰ�, �����ϰų� �˾ƺ� �� �ְ� �ؾ���

                    */
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] (����) ���ϸ��� ������, SHA256���� �ٸ��� \n");
                    Policy_Signature_Compare(
                        (PUCHAR)Policy_Signature_Start_Node,
                        &fileNameInfo->Name,
                        (PUCHAR)&SHA256,
                        &SIG,
                        signature_set_file_node_with__SHA256__but_idc_about_signature, // �������
                        &SIG_file,
                        NULL
                    );


                }
                else if (is_changed_SHA256 == FALSE && is_changed_FULL_PATH == TRUE) {
                    /*
                        ���ϸ� �����

                        SHA256�� ������, ���� ��� �� �̸��� �ٲ� ( Ȯ���� ������ �ִ� ��쵵 ���� ó���ؾ��� )

                        �� ���, 2������ ĳġ�ؾ��� -> ( ���� �� ����  �Ǵ� �����̵� )




                    */
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] (����) ���ϸ��� �ٸ���, SHA256���� ���� \n");


                    Policy_Signature_Compare(
                        (PUCHAR)Policy_Signature_Start_Node,
                        &fileNameInfo->Name,
                        NULL,
                        &SIG,
                        signature_set_file_node_with__FULL_PATH__but_idc_about_signature, // �������
                        &SIG_file,
                        NULL
                    );


                }
                else if (is_changed_SHA256 == TRUE && is_changed_FULL_PATH == TRUE) {
                    /*
                        

                        �Ѵ� ���ÿ� �����ؾ���

                    */
                    Policy_Signature_Compare(
                        (PUCHAR)Policy_Signature_Start_Node,
                        &fileNameInfo->Name,
                        (PUCHAR)&SHA256,
                        &SIG,
                        signature_set_file_node_with__FULL_PATH__and__SHA256_but_idc_about_signature, // �������
                        &SIG_file,
                        NULL
                    );
                }

                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n\n");
                ZwClose(FileHandle);
            }
            else {
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile ���� ���� ���� \n");
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n\n");
            }

        }else{
 
            /*
                �ñ״�ó Ȯ���ڰ� �ƴ� ������.

                ���� " SIG_file " ���� NULL�� �ƴ϶��, Ȯ���� ������ �ǹ��� 
            
            */

        }

        
    }



    
    // �ڵ鷯 ����
    goto LABEL_0;


LABEL_1:
    /*
        ����
    */
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "- DENIED -\n");
    Data->IoStatus.Status = STATUS_ACCESS_DENIED; // �Ǵ� �ٸ� ������ NTSTATUS ��
    Data->IoStatus.Information = 0;
    return return_pre(fileNameInfo, FLT_PREOP_COMPLETE);

LABEL_0:
    /*
        ���
    */
    return return_pre(fileNameInfo, FLT_PREOP_SUCCESS_NO_CALLBACK);
}


/*
IRP_MJ_CREATE: 0
IRP_MJ_CREATE_NAMED_PIPE: 1
IRP_MJ_CLOSE: 2
IRP_MJ_READ: 3
IRP_MJ_WRITE: 4
IRP_MJ_QUERY_INFORMATION: 5
IRP_MJ_SET_INFORMATION: 6
IRP_MJ_QUERY_EA: 7
IRP_MJ_SET_EA: 8
IRP_MJ_FLUSH_BUFFERS: 9
IRP_MJ_QUERY_VOLUME_INFORMATION: 10
IRP_MJ_SET_VOLUME_INFORMATION: 11
IRP_MJ_DIRECTORY_CONTROL: 12
IRP_MJ_FILE_SYSTEM_CONTROL: 13
IRP_MJ_DEVICE_CONTROL: 14
IRP_MJ_INTERNAL_DEVICE_CONTROL: 15
IRP_MJ_SHUTDOWN: 16
IRP_MJ_LOCK_CONTROL: 17
IRP_MJ_CLEANUP: 18
IRP_MJ_CREATE_MAILSLOT: 19
IRP_MJ_QUERY_SECURITY: 20
IRP_MJ_SET_SECURITY: 21
IRP_MJ_POWER: 22
IRP_MJ_SYSTEM_CONTROL: 23
IRP_MJ_DEVICE_CHANGE: 24
IRP_MJ_QUERY_QUOTA: 25
IRP_MJ_SET_QUOTA: 26
IRP_MJ_PNP: 27
*/



// �����̵� ����
BOOLEAN is_copy_move(ULONG FILE_ACCESS_MASK, BOOLEAN is_Success_open_that_FILE_when_IRP_MJ_CREATE) {

    if (


        (
            FILE_ACCESS_MASK == 1507743 || FILE_ACCESS_MASK == 1507734 || FILE_ACCESS_MASK == 1507735 ||
            FILE_ACCESS_MASK == 1180054 || FILE_ACCESS_MASK == 1180055 ||
            FILE_ACCESS_MASK == 1442199 || FILE_ACCESS_MASK == 1442198

            ) // ����Ȯ��

        && is_Success_open_that_FILE_when_IRP_MJ_CREATE == FALSE // �ʱ� ���� ��� ��ȿ���� ��, 

        ) {
        return TRUE;
    }
    else {
        return FALSE;
    }

}

// ���� ���� ��ȿ��
HANDLE is_valid_FILE(PUNICODE_STRING FILE_ABSOULTE_PATH) {

    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE filehandle = 0;
    NTSTATUS status;
    // OBJECT_ATTRIBUTES ����ü �ʱ�ȭ
    InitializeObjectAttributes(&objectAttributes,
        FILE_ABSOULTE_PATH,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    // ���� ����+ �߰��� 
    status = ZwOpenFile(&filehandle,
        SYNCHRONIZE | GENERIC_READ,
        &objectAttributes,
        &ioStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] -> IRP_MJ_CREATE ���� ���� �õ� %p  \n", status);
    if (status != STATUS_SUCCESS || filehandle == 0) {
        return 0;
    }
    else {
        return filehandle;
    }

}


BOOLEAN is_External_NT_PATH(PUNICODE_STRING FILE_ABSOULTE_PATH) {

    /* ���� �ϵ� ��� ����. */
    PALL_DEVICE_DRIVES FOUND_DRIVE_NODE = is_Drives_PATH(FILE_ABSOULTE_PATH); // ����̺� ���ڰ� ��ȿ���� Ȯ�� 
    if (FOUND_DRIVE_NODE != NULL) {

        // USB ����̺� �ΰ�?
        if (FOUND_DRIVE_NODE->DRIVE_DEVICE_TYPE == External_DISK_USB) {
            return TRUE;
        }
        else {
            return FALSE;
        }
    }
    return FALSE;
}

// ���� �ܼ� �̵� ���� ( rename �� �̸� ����� ��ΰ� �޶����� ���� �̸��� �� ����. ) 
BOOLEAN is_move(PUNICODE_STRING original_PATH, PUNICODE_STRING renaming_PATH) {

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n");

    if (original_PATH->Length != renaming_PATH->Length) {

        if (original_PATH->Length >= renaming_PATH->Length) {

            if (memcmp(original_PATH->Buffer, renaming_PATH->Buffer, renaming_PATH->Length) == 0) {
                return FALSE;
            }
            else {
                return TRUE;
            }
        }
        else {
            if (memcmp(original_PATH->Buffer, renaming_PATH->Buffer, original_PATH->Length) == 0) {
                return FALSE;
            }
            else {
                return TRUE;
            }
        }
    }
    else {
        if (memcmp(original_PATH->Buffer, renaming_PATH->Buffer, renaming_PATH->Length) == 0) {
            return FALSE;
        }
        else {
            return TRUE;
        }
    }

}