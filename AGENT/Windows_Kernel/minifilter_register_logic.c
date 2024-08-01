#include "minifilter_register.h"

PFLT_FILTER gFilterHandle = NULL;

// �ݹ� �Լ� ����
NTSTATUS
InstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(VolumeDeviceType);
    UNREFERENCED_PARAMETER(VolumeFilesystemType);

    return STATUS_SUCCESS;
}


VOID
InstanceTeardownCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Reason
) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Reason);

}


NTSTATUS Initialize_Mini_Filter_Driver(PDRIVER_OBJECT DriverObject) {

    /// Callbacks �ڵ鷯 ������ minifilter_handlers.h �� ����Ǿ� �ֵ�. 



    const FLT_REGISTRATION FilterRegistration = {
        sizeof(FLT_REGISTRATION),       // Size
        FLT_REGISTRATION_VERSION,       // Version
        0,                              // Flags
        NULL,                           // Context
        Callbacks,                      // Operation callbacks
        NULL,                           // MiniFilterUnload
        InstanceSetupCallback,          // InstanceSetup
        NULL,                           // InstanceQueryTeardown
        InstanceTeardownCallback,       // InstanceTeardownStart
        InstanceTeardownCallback,       // InstanceTeardownComplete
        NULL,                           // GenerateFileName
        NULL,                           // GenerateDestinationFileName
        NULL                            // NormalizeNameComponent
    };

    NTSTATUS status = FltRegisterFilter(DriverObject, &FilterRegistration, &gFilterHandle);
    if (NT_SUCCESS(status)) {
        /*
            ���� ���� �ʱ�ȭ
        */
        memset(&Pre_struct_Share_variable, 0, sizeof(pre_filter_struct));
        Pre_struct_Share_variable.is_IRP_monitoring_active = FALSE;
        Pre_struct_Share_variable.is_get_something = FALSE;


        /*
            �񵿱Ⱓ ��ȣ���� �߰�

            �Ŵ����� �ñ״�ó �������׿� ������ �� ����

        */
        //KeInitializeMutex(&MUTEX_signature, 0);


        status = FltStartFiltering(gFilterHandle);
        if (!NT_SUCCESS(status)) {
            FltUnregisterFilter(gFilterHandle);
        }
    }

    


    return status;
}

