#include "Post_ObRegisterCallback_.h"
/* � ������ ������Ʈ�� ��ϵ� �� �ݹ��Լ� */
void PostOperationCallback(PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation) {
    UNREFERENCED_PARAMETER(RegistrationContext);

    if (OperationInformation->ObjectType == *PsProcessType) {
        if (OperationInformation->Operation == OB_OPERATION_HANDLE_DUPLICATE ||
            OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE) {

            //if (OperationInformation->Parameters->CreateHandleInformation.GrantedAccess & PROCESS_TERMINATE) {
            //PEPROCESS process = (PEPROCESS)OperationInformation->Object;
            //HANDLE PID = PsGetProcessId(process);
            // ���μ��� ���Ḧ ������ ���� ó��
            // ����� ��� ���ø����̼ǿ� �����Ͽ� â�� �����ϴ� �۾� ����
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Process PostOperationCallback [Duplicate]: %llu \n", PID);
            //}
        }
    }
}