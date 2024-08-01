#include "minifilter_handlers.h"
#include <ntstrsafe.h>


// PRE 컨텍스트
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

    
    // 유저모드인가? 
    if (Input_Data->RequestorMode != UserMode) return STATUS_UNSUCCESSFUL;

    if (FltGetFileNameInformation(Input_Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, Output_fileNameInfo) != STATUS_SUCCESS) return STATUS_UNSUCCESSFUL;


    // 파일 이름 추출
    if (FltParseFileNameInformation(*Output_fileNameInfo) != STATUS_SUCCESS) {

        FltReleaseFileNameInformation(*Output_fileNameInfo);
        return STATUS_UNSUCCESSFUL;
    }





    
    // 시그니처 추출 ( 서버로부터 정책 얻어야함 ) - test
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


// 콜백 함수 선언


FLT_PREOP_CALLBACK_STATUS
Pre_filter_Handler(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    
    // [1] 파일 정보 얻기 ( 파일인가? )
    /*
        1. 유저모드인가?
        2. 파일인가?
        3. 정책에서 얻은 시그니처인가?
    
    */
    PFLT_FILE_NAME_INFORMATION fileNameInfo;
    if (is_file(Data, &fileNameInfo) != STATUS_SUCCESS) return FLT_PREOP_SUCCESS_NO_CALLBACK;

    KeWaitForSingleObject(&MUTEX_signature, Executive, KernelMode, FALSE, NULL);

    /*
        [2] 파일 행위 추출 ( 디렉터리는 포함 안함 ) 
        **세분화**
        * 
        * IRP_MJ_CREATE 가 먼저나오고, READ 및 WRITE가 호출되는 방식임 ( 예외존재(CREATE하고 또 CREATE호출될 수 있음). ) 
        * 
        * IRP_MJ_CREATE의 ACCESS_MASK를 기반으로, 다음 호출 IRP가  READ인지 WRITE인지등의 예측도 할 수 있을 듯
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
            // 여기서 있을 상황
            // 다른 경로에 붙여넣기 시도일 수 있음.
            /*
                다른 경로가 제한된 공간이면 차단할 필요가 있음. ( Drives연결리스트 조회하여 접근이 제한된 드라이브인 경우 차단 ) 
            */
        }
        


        /*
            ACCESS_MASK 추출
            다음 올 실제 동작 IRP를 예견할 수 있음
        */
        ULONG FILE_ACCESS_MASK = (ULONG)Data->Iopb->Parameters.Create.SecurityContext->DesiredAccess;
        

        if (Pre_struct_Share_variable.is_IRP_monitoring_active == TRUE && Pre_struct_Share_variable.is_get_something == TRUE) {
            /*
            * [추적]
                추적결과 분석 ( 필드가 TRUE일 때 작동되어야함 )
            */
            // ACCESS_MASK와 중요한 IRP 를 & 으로 하나씩 확인하여 정상분석 가능

            /*
                가능한 모든 행위를 하나씩 넣어 반복문으로 확인
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
                // 알아서 처리 
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "CREATE: 추적결과 -> %d \n", result);
            }
            */
            
            

        }
        else if (Pre_struct_Share_variable.is_IRP_monitoring_active == TRUE && Pre_struct_Share_variable.is_get_something == FALSE) {
            /*
            *  * [추적]
                CREATE가 연속 2번인 경우,,
            */
            // ACCESS_MASK만 가지고 분석할 수 밖에 없음


           
        }


        /*
        *  * [추적]
            추적 초기화 set
        */
        Pre_struct_Share_variable.is_IRP_monitoring_active = FALSE; Pre_struct_Share_variable.is_get_something = FALSE;
        
        

        

        // 삭제는 여기서 확인가능


        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "CREATE: FileName = %wZ  ACCESS_MASK %lu\n", &fileNameInfo->Name, FILE_ACCESS_MASK);

        
        if (Pre_struct_Share_variable.is_IRP_monitoring_active == FALSE) {
            Pre_struct_Share_variable.keep_DENIED = FALSE;

            /*
            *  * [추적]
                전역변수 초기화
            */
            if (fileNameInfo->Name.MaximumLength > sizeof(Pre_struct_Share_variable.PreCreate_Context.FULL_PATH)) goto LABEL_0; // 길이 넘어서면 탈출


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


        /* 다른 공간에 복사이동 감지 if문  */
        if ( is_copy_move(FILE_ACCESS_MASK, Pre_struct_Share_variable.is_Success_open_that_FILE_when_IRP_MJ_CREATE) ) {

            /*
                복사이동이 맞다면 연결리스트에 추가
            */


            /*
                외장하드에 넣은거라면 차단. (기본값)
            */
            if (is_External_NT_PATH(&fileNameInfo->Name)) {
                goto LABEL_1;
            }
            
        }

    }

    // 지속 차단인 경우 차단함.
    if (Pre_struct_Share_variable.keep_DENIED) goto LABEL_1;


    if (Data->Iopb->MajorFunction == IRP_MJ_READ) {
        /*
            파일 열기
        */
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "READ: FileName = %wZ\n", &fileNameInfo->Name);

        
        
    }
    else if (Data->Iopb->MajorFunction == IRP_MJ_WRITE) {
        if (Pre_struct_Share_variable.keep_DENIED) goto LABEL_1;
        /*
            파일 수정 및 쓰기
        */
        //FILE_ACCESS_MASK = (ULONG)Data->Iopb->Parameters.Write.SecurityContext->DesiredAccess;
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "WRTIE: FileName = %wZ\n", &fileNameInfo->Name);

        /*
            수정인지 확인할 필요가 있음 ( 해시구하기 ) 
        */

    }
    else if (Data->Iopb->MajorFunction == IRP_MJ_CLOSE) {
        /*
            파일 클린/삭제?
        */
        //FILE_ACCESS_MASK = (ULONG)Data->Iopb->Parameters.Read.SecurityContext->DesiredAccess;
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "CLOSE: FileName = %wZ\n", &fileNameInfo->Name);


    }
    else if (Data->Iopb->MajorFunction == IRP_MJ_SET_INFORMATION) {
        
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IRP_MJ_SET_INFORMATION: FileName = %wZ , Data->Iopb->Parameters.SetFileInformation.FileInformationClass -> %d\n", &fileNameInfo->Name, Data->Iopb->Parameters.SetFileInformation.FileInformationClass);

        // 파일 이름 변경 필터링
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
            /*1. 파일명 감지는 어떻게 알지?*/
            /*2. 파일 이동시, 그 위치는 어떻게 알지? ( 파싱 필요 ) */
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 파일 이름 변경 감지@@ 변경될 명 -> %wZ\n", newFileName);

            // 파일열기
            HANDLE FileHandle = is_valid_FILE(&fileNameInfo->Name);
            if (FileHandle > 0) {

                /*
                *   해시 변경 알아내기 ( 파일 변경 )
                    일단 해시 구해 [2/3]
                */
                /* 파일 바이너리 구하기 */
                PVOID File_Bin = NULL;
                ULONG File_Bin_Length = 0;
                if (ALL_in_ONE_FILE_IO(&File_Bin, &File_Bin_Length, fileNameInfo->Name, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) {
                    ZwClose(FileHandle);
                    goto LABEL_0;
                }

                /* 파일  해시 구하기 */
                UCHAR SHA256[SHA256_String_Byte_Length] = { 0, };
                if (GET_SHA256_HASHING(File_Bin, File_Bin_Length, (PCHAR)&SHA256) != STATUS_SUCCESS) {
                    ExFreePoolWithTag(File_Bin, 'FILE');
                    ZwClose(FileHandle);
                    goto LABEL_0;
                }
                ExFreePoolWithTag(File_Bin, 'FILE');


                /*
                    [1/2] 파일 단순 이동의 경우 대상 위치가 외장하드인 경우, 차단하기 
                */


                /*
                   [2/2]확장자 변경의 경우 차단하기. 
               */
                Ppolicy_signature_struct SIG = NULL; // 시그니처 목록
                Ppolicy_signature_files_struct SIG_file = NULL; // 해당 시그니처에서 저장된 파일 리스트
                if (!Policy_Signature_Compare(

                    (PUCHAR)Policy_Signature_Start_Node,
                    &newFileName, // 수정될 파일명으로 전달한다. 이는 확장자를 검사하기 위함.( 유효하지 않은 파일명임  )
                    (PUCHAR)&SHA256, // 파일 ID를 대신하여 노드를 찾아줌 
                    &SIG,
                    signature_reEDIT_about_file_extension,// IRP_MJ_SET_INFORMATIO 전용
                    &SIG_file,
                    NULL

                )) {
                    if (SIG_file != NULL) goto LABEL_1; // FALSE 지만, SIG_file 성공적으로 얻어오면 파일명 변경을 감지할 수 있음.
                }

            }

           
        }
        // 파일 붙여넣기 확인 
        else if (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileEndOfFileInformation) {
            // 파일이 IRP_MJ_CRAETE당시 열기 시도할 때   "실패"된 경우여야함. 
            if (Pre_struct_Share_variable.is_Success_open_that_FILE_when_IRP_MJ_CREATE == FALSE) { // 실패

                //UNICODE_STRING newFileName = { 0, };
                FILE_END_OF_FILE_INFORMATION* endOfFileInfo = (FILE_END_OF_FILE_INFORMATION*)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
                if (endOfFileInfo != NULL)
                {
                    // Extract the new endOfFileInfo
                    endOfFileInfo->EndOfFile.QuadPart;
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "파일 위치 변경 감지 IRP_MJ_SET_INFORMATION %llu\n 변경할 위치 %wZ ", endOfFileInfo->EndOfFile.QuadPart, &fileNameInfo->Name);


                    /* Defualt - 외장하드 인 경우 차단. - 이때는 IRP_MJ_CREATE때 얻은 아직 유효하지 않은 절대경로의 드라이브 문자열을 보고 판단. */

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

                파일 붙여넣기 감지, 할 때,,,
                */

                
            if ((Pre_struct_Share_variable.is_Success_open_that_FILE_when_IRP_MJ_CREATE == FALSE) && (Data->Iopb->TargetFileObject && FltObjects->Instance)) {

                FILE_BASIC_INFORMATION basicInfo;
                if (FltQueryInformationFile(FltObjects->Instance, Data->Iopb->TargetFileObject, &basicInfo, sizeof(basicInfo), FileBasicInformation, NULL) == STATUS_SUCCESS) {
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[IRP_MJ_QUERY_INFORMATION - FileBasicInformation] 파일_생성_시간: %llu 파일_마지막접근_시간: %llu 파일_마지막쓰기_시간: %llu 파일_수정_시간: %llu\n",
                        basicInfo.CreationTime.QuadPart, basicInfo.LastAccessTime.QuadPart, basicInfo.LastWriteTime.QuadPart, basicInfo.ChangeTime.QuadPart);
               
                    /* 외장 하드 모두 차단. */

                    // USB 드라이브 인가?
                    if (is_External_NT_PATH(&fileNameInfo->Name)) {
                        Pre_struct_Share_variable.keep_DENIED = TRUE;
                        goto LABEL_1; // 볼륨 접근 차단.
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

            /* 외장 하드 모두 차단. */
            PALL_DEVICE_DRIVES FOUND_DRIVE_NODE = is_Drives_PATH(&fileNameInfo->Name); // 드라이브 문자가 유효한지 확인 
            if (FOUND_DRIVE_NODE != NULL) {

                // USB 드라이브 인가?
                if (FOUND_DRIVE_NODE->DRIVE_DEVICE_TYPE == External_DISK_USB) {
                    Pre_struct_Share_variable.keep_DENIED = TRUE;
                    goto LABEL_1; // 볼륨 접근 차단.
                }
            }

            
        }

        
    
    }
    else {
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Other [ %d ] :  FileName = %wZ\n", (ULONG32)Data->Iopb->MajorFunction, &fileNameInfo->Name);

    }
   
    // 전역 조건
    if (Data->Iopb->MajorFunction != IRP_MJ_CREATE) {
        
        /*
        *  * [추적]
            추적저장
        */
        



        /*
            시그니처를 검사해야함
        */
        Ppolicy_signature_struct SIG = NULL; // 시그니처 목록
        Ppolicy_signature_files_struct SIG_file = NULL; // 해당 시그니처에서 저장된 파일 리스트
        if (Policy_Signature_Compare(

            (PUCHAR)Policy_Signature_Start_Node,
            &fileNameInfo->Name,
            NULL,
            &SIG,
            signature_COMPARE_Mode,
            &SIG_file,
            NULL

        )) {
            if (SIG_file == NULL) {// SIG_file 는 필수로 가져와야함 ( 단, 새로운 파일인 경우 NULL임 
                /*
                    시그니처는 같은데,, 파일 정보 2차원 연결리스트의 노드를 못가져왔다?
                    이는 새로운 파일을 의미한다. 
                */
                
                if ( Policy_Signature_Compare(
                    (PUCHAR)Policy_Signature_Start_Node,
                    &fileNameInfo->Name,
                    NULL,
                    NULL,
                    signature_SAVE_Mode,
                    NULL,
                    NULL // 여기가 NULL이면 메타데이터 저장 안하게 됨 
                ) == FALSE ) {
                    goto LABEL_0;
                }

                /*
                    새로운 노드 추가 완료
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

                아래 코드는 파일 세부 행위 탐지 확인용

            */


            HANDLE FileHandle = is_valid_FILE(&fileNameInfo->Name);
            if (FileHandle>0) {
                /* 파일 열기 성공 */
                //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile 파일 열기 성공 \n" );
                /*
                    파일 절대경로 변경 알아내기 [1/3]
                */
                if (wcscmp(Pre_struct_Share_variable.PreCreate_Context.FULL_PATH, fileNameInfo->Name.Buffer) == 0) {
                    // 같음
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile -> wcscmp 같음 \n");
                }
                else {
                    // 파일 절대경로 달라짐
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile -> wcscmp 다름 \n");
                    Pre_struct_Share_variable.PreCreate_Context.is_changed_FULL_PATH = TRUE;
                }




                /*
                *   해시 변경 알아내기 ( 파일 변경 )
                    일단 해시 구해 [2/3]
                */
                /* 파일 바이너리 구하기 */
                PVOID File_Bin = NULL;
                ULONG File_Bin_Length = 0;
                if (ALL_in_ONE_FILE_IO(&File_Bin, &File_Bin_Length, fileNameInfo->Name, READ_MODE_with_FILE_STANDARD_INFORMATION__get__FILE_LENGTH) != STATUS_SUCCESS) {
                    ZwClose(FileHandle);
                    goto LABEL_0;
                }

                /* 파일  해시 구하기 */
                UCHAR SHA256[SHA256_String_Byte_Length] = { 0, };
                if (GET_SHA256_HASHING(File_Bin, File_Bin_Length, (PCHAR)&SHA256) != STATUS_SUCCESS) {
                    ExFreePoolWithTag(File_Bin, 'FILE');
                    ZwClose(FileHandle);
                    goto LABEL_0;
                }
                ExFreePoolWithTag(File_Bin, 'FILE');



                if (Pre_struct_Share_variable.is_get_SHA256 == FALSE) {
                    /*
                        최초 SHA256 얻은 적 없을 떄
                    */
                    memcpy((PUCHAR)&Pre_struct_Share_variable.PreCreate_Context.SHA256, (PUCHAR)&SHA256, SHA256_String_Byte_Length);


                    Pre_struct_Share_variable.is_get_SHA256 = TRUE;
                }
                else {
                    /*
                        SHA256 이전에 받았을 때
                    */
                    // 비교하여 ' is_changed ' 되었는 지 확인  
                    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " memcmp( %s, %s  )\n", Pre_struct_Share_variable.PreCreate_Context.SHA256, SHA256);

                    if (memcmp(Pre_struct_Share_variable.PreCreate_Context.SHA256, SHA256, SHA256_String_Byte_Length) == 0) {
                        // 같을 때
                        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile -> memcmp 같음 \n");
                    }
                    else {
                        // 다를 때
                        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile -> memcmp 다름 \n");
                        Pre_struct_Share_variable.PreCreate_Context.is_changed_SHA256 = TRUE;
                    }
                }
                // 해시 구역 끝 


                /* [3/3]

                    정책 시그니처 목록이 담긴 연결리스트 노드 하나씩 안에 있는 실제 파일 절대경로 + SHA256가 담긴 또 하나의 연결리스트 노드를 가져오는 것.

                    2차원 연결리스트 구조임

                    이를 통하여 파일 데이터는 같지만, 파일명이 변경되었는 지 확인 가능

                */

                BOOLEAN is_changed_SHA256 = FALSE;
                BOOLEAN is_changed_FULL_PATH = FALSE;


                SIG = NULL; // 시그니처 목록
                SIG_file = NULL; // 해당 시그니처에서 저장된 파일 리스트
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
                        TRUE면, SHA256은 같다!
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
                        TRUE면, 파일명은 같다!
                    */

                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Policy_Signature_Compare -> fileNameInfo->Name TRUE\n");

                    is_changed_FULL_PATH = FALSE;
                }
                else {
                    is_changed_FULL_PATH = TRUE;
                }


                // 수정 검증 4가지 상태를 가짐 

                if (is_changed_SHA256 == FALSE && is_changed_FULL_PATH == FALSE) {
                    /*
                        전혀 변경없음
                    */
                }
                else if (is_changed_SHA256 == TRUE && is_changed_FULL_PATH == FALSE) {
                    /*
                        SHA256만 변경됨

                        파일명은 같지만, 파일 데이터가 바뀜

                        (방안)
                        이런 경우는 다시 해시 값을 구하고, 갱신하거나 알아볼 수 있게 해야함

                    */
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] (수정) 파일명은 같지만, SHA256값이 다르다 \n");
                    Policy_Signature_Compare(
                        (PUCHAR)Policy_Signature_Start_Node,
                        &fileNameInfo->Name,
                        (PUCHAR)&SHA256,
                        &SIG,
                        signature_set_file_node_with__SHA256__but_idc_about_signature, // 수정모드
                        &SIG_file,
                        NULL
                    );


                }
                else if (is_changed_SHA256 == FALSE && is_changed_FULL_PATH == TRUE) {
                    /*
                        파일명만 변경됨

                        SHA256은 같지만, 파일 경로 및 이름이 바뀜 ( 확장자 변경이 있는 경우도 따로 처리해야함 )

                        이 경우, 2가지를 캐치해야함 -> ( 파일 명 변경  또는 파일이동 )




                    */
                    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] (수정) 파일명은 다르고, SHA256값은 같다 \n");


                    Policy_Signature_Compare(
                        (PUCHAR)Policy_Signature_Start_Node,
                        &fileNameInfo->Name,
                        NULL,
                        &SIG,
                        signature_set_file_node_with__FULL_PATH__but_idc_about_signature, // 수정모드
                        &SIG_file,
                        NULL
                    );


                }
                else if (is_changed_SHA256 == TRUE && is_changed_FULL_PATH == TRUE) {
                    /*
                        

                        둘다 동시에 수정해야함

                    */
                    Policy_Signature_Compare(
                        (PUCHAR)Policy_Signature_Start_Node,
                        &fileNameInfo->Name,
                        (PUCHAR)&SHA256,
                        &SIG,
                        signature_set_file_node_with__FULL_PATH__and__SHA256_but_idc_about_signature, // 수정모드
                        &SIG_file,
                        NULL
                    );
                }

                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n\n");
                ZwClose(FileHandle);
            }
            else {
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] ZwOpenFile 파일 열기 실패 \n");
                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n\n");
            }

        }else{
 
            /*
                시그니처 확장자가 아닌 영역임.

                만약 " SIG_file " 값이 NULL이 아니라면, 확장자 변경을 의미함 
            
            */

        }

        
    }



    
    // 핸들러 리턴
    goto LABEL_0;


LABEL_1:
    /*
        차단
    */
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "- DENIED -\n");
    Data->IoStatus.Status = STATUS_ACCESS_DENIED; // 또는 다른 적절한 NTSTATUS 값
    Data->IoStatus.Information = 0;
    return return_pre(fileNameInfo, FLT_PREOP_COMPLETE);

LABEL_0:
    /*
        허용
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



// 복사이동 감지
BOOLEAN is_copy_move(ULONG FILE_ACCESS_MASK, BOOLEAN is_Success_open_that_FILE_when_IRP_MJ_CREATE) {

    if (


        (
            FILE_ACCESS_MASK == 1507743 || FILE_ACCESS_MASK == 1507734 || FILE_ACCESS_MASK == 1507735 ||
            FILE_ACCESS_MASK == 1180054 || FILE_ACCESS_MASK == 1180055 ||
            FILE_ACCESS_MASK == 1442199 || FILE_ACCESS_MASK == 1442198

            ) // 권한확인

        && is_Success_open_that_FILE_when_IRP_MJ_CREATE == FALSE // 초기 파일 경로 유효안할 때, 

        ) {
        return TRUE;
    }
    else {
        return FALSE;
    }

}

// 파일 열기 유효성
HANDLE is_valid_FILE(PUNICODE_STRING FILE_ABSOULTE_PATH) {

    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE filehandle = 0;
    NTSTATUS status;
    // OBJECT_ATTRIBUTES 구조체 초기화
    InitializeObjectAttributes(&objectAttributes,
        FILE_ABSOULTE_PATH,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    // 파일 열기+ 추가됨 
    status = ZwOpenFile(&filehandle,
        SYNCHRONIZE | GENERIC_READ,
        &objectAttributes,
        &ioStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[flt] -> IRP_MJ_CREATE 파일 열기 시도 %p  \n", status);
    if (status != STATUS_SUCCESS || filehandle == 0) {
        return 0;
    }
    else {
        return filehandle;
    }

}


BOOLEAN is_External_NT_PATH(PUNICODE_STRING FILE_ABSOULTE_PATH) {

    /* 외장 하드 모두 차단. */
    PALL_DEVICE_DRIVES FOUND_DRIVE_NODE = is_Drives_PATH(FILE_ABSOULTE_PATH); // 드라이브 문자가 유효한지 확인 
    if (FOUND_DRIVE_NODE != NULL) {

        // USB 드라이브 인가?
        if (FOUND_DRIVE_NODE->DRIVE_DEVICE_TYPE == External_DISK_USB) {
            return TRUE;
        }
        else {
            return FALSE;
        }
    }
    return FALSE;
}

// 파일 단순 이동 감지 ( rename 시 미리 변경될 경로가 달라지는 것을 미리알 수 있음. ) 
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