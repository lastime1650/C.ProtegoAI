#include "Pre_ObRegisterCallback_.h"

#include "util_Delay.h"

void PrintDesiredAccess(ACCESS_MASK DesiredAccess)
{
    // ������Ʈ ID�� ������ �����Ͽ� �α׸� ���
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

/* � ������ ������Ʈ�� ��ϵǱ��� �ݹ��Լ� */
OB_PREOP_CALLBACK_STATUS PreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation) {
    UNREFERENCED_PARAMETER(RegistrationContext);
    if (Dynamic_Node_Action_Process_Node == NULL) return OB_PREOP_SUCCESS;
    /*
        � �۾��� �� ��?
        [ GUI���� ����� ó���� �� ���� ]
        < ���μ��� >
        => Action ���Ḯ��Ʈ���� SHA256�������� ���μ����� ������ ���Ͽ� ����� ��μ� ó���ϴ� ����

        1. �Ź� ȣ�⶧����  CREATE���� DUPLICATE���� Ȯ���ϰ�, ������ "������ Parameter"�� �����ϴ� Desired�� ���� ���� ���μ��� �ൿ�� �ϴ� �� Ȯ���ϰ�, 

        �̸� ������ ���Ḯ��Ʈ�� ����� ��带 ��ȸ�ϸ鼭 ��ġ�� SHA256 ���� Type ENUM���� ������ �����ϵ��� �Ѵ�.

    */

    if (OperationInformation->ObjectType == *PsProcessType) {

        PEPROCESS PRE_eprocess = OperationInformation->Object;
        HANDLE PID = PsGetProcessId(PRE_eprocess);


        if (OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE) {

            

            if (OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess == 0x0 || OperationInformation->Parameters->CreateHandleInformation.DesiredAccess == 0x0) return OB_PREOP_SUCCESS;

            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [ACTION] OB_OPERATION_HANDLE_CREATE PID  PID %llu ���� \n", PID);
            PrintDesiredAccess(OperationInformation->Parameters->CreateHandleInformation.DesiredAccess);


            ActionProcessNode APN = { 0, };
            APN.is_Block = TRUE;
            Type_Feature TMP_feature = 0;
            if (OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_CREATE_PROCESS) {
                /* ���μ��� ���� ��,,, */

                // 1. ������ ����(�нú극��)  -> 2. �۾�(����) -> 3. ���� �ް� ����
                TMP_feature |= Create;
            }



            if (OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) {
                /* ���μ��� ���� ��,,  */

                TMP_feature |= Remove;
            }

            if (TMP_feature & Create || TMP_feature & Remove) {
                PRE_OBREGISTER_CALLBACK_MOVE MOVER = { PID,(PUCHAR)&APN.SHA256, {0,} };
                KeInitializeEvent(&MOVER.EVENT, SynchronizationEvent, FALSE);


                HANDLE thread_handle = 0;
                PsCreateSystemThread(&thread_handle, THREAD_ALL_ACCESS, NULL, NULL, NULL, PRE_OBREGISTER_CALLBACK_PROCESSING_THREAD, &MOVER);
                KeWaitForSingleObject(&MOVER.EVENT, Executive, KernelMode, FALSE, NULL);//���

                // ���ܼ���

                if (TMP_feature & Create) {
                    APN.feature = Create;
                    if (is_exist_program_Action_Process_Node(&APN)) {
                        if (Action_PROCESSING_on_RegisterCallback(&APN, &OperationInformation, Create)) {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "1 [ACTION ����] ����\n");
                        }
                    }

                }

                if (TMP_feature & Remove) {
                    APN.feature = Remove;
                    if (is_exist_program_Action_Process_Node(&APN)) {
                        if (Action_PROCESSING_on_RegisterCallback(&APN, &OperationInformation, Remove)) {
                            DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "2 [ACTION  ����] ����\n");
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
                    KeWaitForSingleObject(&MOVER.EVENT, Executive, KernelMode, FALSE, NULL);//���


                    // ���ܼ���

                    if (TMP_feature & Create) {
                        APN.feature = Create;
                        if (is_exist_program_Action_Process_Node(&APN)) {
                            if (Action_PROCESSING_on_RegisterCallback(&APN, &OperationInformation, Create)) {
                                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " 3[ACTION DUP ����] ����\n");
                            }
                        }

                    }

                    if (TMP_feature & Remove) {
                        APN.feature = Remove;
                        if (is_exist_program_Action_Process_Node(&APN)) {
                            if (Action_PROCESSING_on_RegisterCallback(&APN, &OperationInformation, Remove)) {
                                DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "4 [ACTION DUP ����] ����\n");
                            }
                        }
                    }

                }
            }
        }


    }

    return OB_PREOP_SUCCESS;
}


// PAction_for_process_routine_NODE�� ���������� ����� ���, GUI�� ��ɴ�� ó����
BOOLEAN Action_PROCESSING_on_RegisterCallback(PActionProcessNode Action_Node, POB_PRE_OPERATION_INFORMATION* OperationInformation, Type_Feature typefeature) {
    // �� ��忡�� ������ SHA256�� ��� �ߺ� ó���� ���ΰ�?  
    switch (Action_Node->feature) {
    case Create:
        if (typefeature != Create) return TRUE;

        DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, " [Pre] Create\n");
        if (Action_Node->is_Block) {
            /* ���μ��� Create �� Block (���μ��� ��ȣ�� �ǹ��Ѵ�.) */
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
            /* ���μ��� Remove �� Block (���μ��� "��ȣ"�� �ǹ��Ѵ�.) */
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