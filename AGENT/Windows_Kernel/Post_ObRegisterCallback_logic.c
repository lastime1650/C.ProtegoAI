#include "Post_ObRegisterCallback_.h"
/* 어떤 유형의 오브젝트가 등록된 후 콜백함수 */
void PostOperationCallback(PVOID RegistrationContext, POB_POST_OPERATION_INFORMATION OperationInformation) {
    UNREFERENCED_PARAMETER(RegistrationContext);

    if (OperationInformation->ObjectType == *PsProcessType) {
        if (OperationInformation->Operation == OB_OPERATION_HANDLE_DUPLICATE ||
            OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE) {

            //if (OperationInformation->Parameters->CreateHandleInformation.GrantedAccess & PROCESS_TERMINATE) {
            //PEPROCESS process = (PEPROCESS)OperationInformation->Object;
            //HANDLE PID = PsGetProcessId(process);
            // 프로세스 종료를 감지한 후의 처리
            // 사용자 모드 애플리케이션에 통지하여 창을 제거하는 작업 수행
            //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Process PostOperationCallback [Duplicate]: %llu \n", PID);
            //}
        }
    }
}