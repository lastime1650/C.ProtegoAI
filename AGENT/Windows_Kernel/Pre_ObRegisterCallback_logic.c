#include "Pre_ObRegisterCallback_.h"

#include "util_Delay.h"

void PrintDesiredAccess(ACCESS_MASK DesiredAccess)
{
    // 컴포넌트 ID와 레벨을 지정하여 로그를 출력
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DesiredAccess: 0x%08X\n", DesiredAccess);

    if (DesiredAccess & PROCESS_CREATE_PROCESS)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " - PROCESS_CREATE_PROCESS\n");
    if (DesiredAccess & PROCESS_CREATE_THREAD)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " - PROCESS_CREATE_THREAD\n");
    if (DesiredAccess & PROCESS_DUP_HANDLE)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " - PROCESS_DUP_HANDLE\n");
    if (DesiredAccess & PROCESS_SET_QUOTA)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " - PROCESS_SET_QUOTA\n");
    if (DesiredAccess & PROCESS_SET_INFORMATION)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " - PROCESS_SET_INFORMATION\n");
    if (DesiredAccess & PROCESS_SUSPEND_RESUME)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " - PROCESS_SUSPEND_RESUME\n");
    if (DesiredAccess & PROCESS_TERMINATE)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " - PROCESS_TERMINATE\n");
    if (DesiredAccess & PROCESS_VM_OPERATION)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " - PROCESS_VM_OPERATION\n");
    if (DesiredAccess & PROCESS_VM_WRITE)
        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " - PROCESS_VM_WRITE\n");
}

typedef struct PRE_OBREGISTER_CALLBACK_MOVE {
    HANDLE PID;
    PUCHAR SHA256_OUTPUT_ADDR;
    KEVENT EVENT;
}PRE_OBREGISTER_CALLBACK_MOVE, *PPRE_OBREGISTER_CALLBACK_MOVE;

VOID PRE_OBREGISTER_CALLBACK_PROCESSING_THREAD(PPRE_OBREGISTER_CALLBACK_MOVE PARM_MOVE) {


    PUCHAR EXE = NULL;
    ULONG EXE_Size = 0;
    if (PID_to_EXE(PARM_MOVE->PID, &EXE, &EXE_Size, (PCHAR)PARM_MOVE->SHA256_OUTPUT_ADDR, KernelMode) != STATUS_SUCCESS) {
        if (EXE != NULL) {
            ExFreePoolWithTag(EXE, 'FILE');
        }
        KeSetEvent(&PARM_MOVE->EVENT, 0, FALSE);
        return;

    }
    ExFreePoolWithTag(EXE, 'FILE');

    KeSetEvent(&PARM_MOVE->EVENT, 0, FALSE);

    return;
}

/* 어떤 유형의 오브젝트가 등록되기전 콜백함수 */
OB_PREOP_CALLBACK_STATUS PreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
    UNREFERENCED_PARAMETER(RegistrationContext);
    if (Dynamic_Node_Action_Process_Node == NULL) return OB_PREOP_SUCCESS;
    /*
        어떤 작업을 할 것?
        [ GUI에서 명령을 처리할 수 있음 ]
        < 프로세스 >
        => Action 연결리스트에서 SHA256기준으로 프로세스의 파일을 비교하여 명령을 비로소 처리하는 것임

        1. 매번 호출때마다  CREATE인지 DUPLICATE인지 확인하고, 각각에 "유요한 Parameter"에 존재하는 Desired를 통해 무슨 프로세스 행동을 하는 지 확인하고, 

        미리 사전에 연결리스트에 적용된 노드를 순회하면서 일치한 SHA256 값과 Type ENUM값이 있으면 차단하도록 한다.

    */

    if (OperationInformation->ObjectType == *PsProcessType) {

        PEPROCESS PRE_eprocess = OperationInformation->Object;
        HANDLE PID = PsGetProcessId(PRE_eprocess);


        if (OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE) {

            

            if (OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess == 0x0 || OperationInformation->Parameters->CreateHandleInformation.DesiredAccess == 0x0) return OB_PREOP_SUCCESS;

            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [ACTION] OB_OPERATION_HANDLE_CREATE PID  PID %llu 감지 \n", PID);
            PrintDesiredAccess(OperationInformation->Parameters->CreateHandleInformation.DesiredAccess);


            ActionProcessNode APN = { 0, };
            APN.is_Block = TRUE;
            Type_Feature TMP_feature = 0;
            if (OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_CREATE_PROCESS) {
                /* 프로세스 생성 시,,, */

                // 1. 스레드 생성(패시브레벨)  -> 2. 작업(동기) -> 3. 리턴 받고 리턴
                TMP_feature |= Create;
            }



            if (OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) {
                /* 프로세스 삭제 시,,  */

                TMP_feature |= Remove;
            }

            if (TMP_feature & Create || TMP_feature & Remove) {
                PRE_OBREGISTER_CALLBACK_MOVE MOVER = { PID,(PUCHAR)&APN.SHA256, {0,} };
                KeInitializeEvent(&MOVER.EVENT, SynchronizationEvent, FALSE);


                HANDLE thread_handle = 0;
                PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, PRE_OBREGISTER_CALLBACK_PROCESSING_THREAD, &MOVER);
                KeWaitForSingleObject(&MOVER.EVENT, Executive, KernelMode, FALSE, NULL);//대기

                // 차단수행

                if (TMP_feature & Create) {
                    APN.feature = Create;
                    if (is_exist_program_Action_Process_Node(&APN)) {
                        if (Action_PROCESSING_on_RegisterCallback(&APN, &OperationInformation, Create)) {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 [ACTION 차단] 성공\n");
                        }
                    }

                }

                if (TMP_feature & Remove) {
                    APN.feature = Remove;
                    if (is_exist_program_Action_Process_Node(&APN)) {
                        if (Action_PROCESSING_on_RegisterCallback(&APN, &OperationInformation, Remove)) {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "2 [ACTION  차단] 성공\n");
                        }
                    }
                }
            }



        }
        else if (OperationInformation->Operation == OB_OPERATION_HANDLE_DUPLICATE) {

            ActionProcessNode APN = { 0, };
            APN.is_Block = TRUE;
            Type_Feature TMP_feature = 0;
            if (OperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess != 0x0) {

                PrintDesiredAccess(OperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess);
                //PrintDesiredAccess(OperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess);

                if (OperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess & PROCESS_CREATE_PROCESS) {
                    TMP_feature |= Create;
                }

                if (OperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) {
                    TMP_feature |= Remove;
                }

                if (TMP_feature & Create || TMP_feature & Remove) {
                    PRE_OBREGISTER_CALLBACK_MOVE MOVER = { PID,(PUCHAR)&APN.SHA256, {0,} };
                    KeInitializeEvent(&MOVER.EVENT, SynchronizationEvent, FALSE);


                    HANDLE thread_handle = 0;
                    PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, PRE_OBREGISTER_CALLBACK_PROCESSING_THREAD, &MOVER);
                    KeWaitForSingleObject(&MOVER.EVENT, Executive, KernelMode, FALSE, NULL);//대기


                    // 차단수행

                    if (TMP_feature & Create) {
                        APN.feature = Create;
                        if (is_exist_program_Action_Process_Node(&APN)) {
                            if (Action_PROCESSING_on_RegisterCallback(&APN, &OperationInformation, Create)) {
                                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 3[ACTION DUP 차단] 성공\n");
                            }
                        }

                    }

                    if (TMP_feature & Remove) {
                        APN.feature = Remove;
                        if (is_exist_program_Action_Process_Node(&APN)) {
                            if (Action_PROCESSING_on_RegisterCallback(&APN, &OperationInformation, Remove)) {
                                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "4 [ACTION DUP 차단] 성공\n");
                            }
                        }
                    }

                }
            }
        }


    }

    return OB_PREOP_SUCCESS;
}


// PAction_for_process_routine_NODE를 성공적으로 얻었을 경우, GUI의 명령대로 처리함
BOOLEAN Action_PROCESSING_on_RegisterCallback(PActionProcessNode Action_Node, POB_PRE_OPERATION_INFORMATION* OperationInformation, Type_Feature typefeature) {
    // 두 노드에서 동일한 SHA256은 어떻게 중복 처리할 것인가?  
    switch (Action_Node->feature) {
    case Create:
        if (typefeature != Create) return TRUE;

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [Pre] Create\n");
        if (Action_Node->is_Block) {
            /* 프로세스 Create 를 Block (프로세스 보호를 의미한다.) */
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [Pre] Create->Block\n");
            if ((*OperationInformation)->Operation == OB_OPERATION_HANDLE_CREATE && (*OperationInformation)->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_CREATE_PROCESS) { //       1/4

                (*OperationInformation)->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_CREATE_PROCESS;

                if ((*OperationInformation)->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_OPERATION) {//      2/4
                    (*OperationInformation)->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
                }

                if ((*OperationInformation)->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_READ) {//      3/4
                    (*OperationInformation)->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_READ;
                }

                if ((*OperationInformation)->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_WRITE) {//      4/4
                    (*OperationInformation)->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_WRITE;
                }

                break;
            }
            else if ((*OperationInformation)->Operation == OB_OPERATION_HANDLE_DUPLICATE && (*OperationInformation)->Parameters->DuplicateHandleInformation.OriginalDesiredAccess & PROCESS_CREATE_PROCESS) {
                (*OperationInformation)->Parameters->DuplicateHandleInformation.OriginalDesiredAccess &= ~PROCESS_CREATE_PROCESS;

                if ((*OperationInformation)->Parameters->DuplicateHandleInformation.OriginalDesiredAccess & PROCESS_VM_OPERATION) {//      2/4
                    (*OperationInformation)->Parameters->DuplicateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
                }

                if ((*OperationInformation)->Parameters->DuplicateHandleInformation.OriginalDesiredAccess & PROCESS_VM_READ) {//      3/4
                    (*OperationInformation)->Parameters->DuplicateHandleInformation.DesiredAccess &= ~PROCESS_VM_READ;
                }

                if ((*OperationInformation)->Parameters->DuplicateHandleInformation.OriginalDesiredAccess & PROCESS_VM_WRITE) {//      4/4
                    (*OperationInformation)->Parameters->DuplicateHandleInformation.DesiredAccess &= ~PROCESS_VM_WRITE;
                }

                break;
            }
            else {
                return FALSE;
            }
        }
        else {
            /* Permit */
            return TRUE;
        }


    case Remove:
        if (typefeature != Remove) return TRUE;

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [Pre] Remove\n");
        if (Action_Node->is_Block) {
            /* 프로세스 Remove 를 Block (프로세스 "보호"를 의미한다.) */
            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [Pre] Remove->Block\n");
            if ((*OperationInformation)->Operation == OB_OPERATION_HANDLE_CREATE && (*OperationInformation)->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) { //        1/4

                (*OperationInformation)->Parameters->CreateHandleInformation.DesiredAccess = 0x0;
                (*OperationInformation)->Parameters->CreateHandleInformation.OriginalDesiredAccess = 0x0;

                //(*OperationInformation)->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;

                //if ((*OperationInformation)->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_OPERATION) {//      2/4
                //    (*OperationInformation)->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
                //}

                /*
                if ((*OperationInformation)->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_READ) {//      3/4
                    (*OperationInformation)->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_READ;
                }

                if ((*OperationInformation)->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_WRITE) {//      4/4
                    (*OperationInformation)->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_WRITE;
                }
                */

                break;
            }
            else if ((*OperationInformation)->Operation == OB_OPERATION_HANDLE_DUPLICATE && (*OperationInformation)->Parameters->DuplicateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) {
                (*OperationInformation)->Parameters->DuplicateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;

                if ((*OperationInformation)->Parameters->DuplicateHandleInformation.OriginalDesiredAccess & PROCESS_VM_OPERATION) {//      2/4
                    (*OperationInformation)->Parameters->DuplicateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
                }

                (*OperationInformation)->Parameters->DuplicateHandleInformation.DesiredAccess = 0;

                break;
            }
            else {
                return FALSE;
            }
        }
        else {
            /* Permit */
            return TRUE;
        }

    default:
        return FALSE;
    }

    return TRUE;

}