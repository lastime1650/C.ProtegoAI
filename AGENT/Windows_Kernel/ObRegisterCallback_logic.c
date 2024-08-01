#include "ObRegisterCallback_.h"

OB_CALLBACK_REGISTRATION g_CallbackRegistration;
PVOID g_CallbackHandle = NULL;

NTSTATUS Make_ObRegisterCallback() {
    // ������� �ۼ�.
    //PsProcessType


    OB_OPERATION_REGISTRATION operations[] = {
        { PsProcessType, OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE , PreOperationCallback, PostOperationCallback } // ���μ��� ����͸�
    };


    g_CallbackRegistration = (OB_CALLBACK_REGISTRATION){
        .Version = OB_FLT_REGISTRATION_VERSION,
        .OperationRegistrationCount = ARRAYSIZE(operations),
        .Altitude = RTL_CONSTANT_STRING(L"321650"),
        .RegistrationContext = NULL,
        .OperationRegistration = operations
    };

    return ObRegisterCallbacks(&g_CallbackRegistration, &g_CallbackHandle);
}