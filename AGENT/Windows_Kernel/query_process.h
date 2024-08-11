#ifndef query_process_H
#define query_process_H

#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>

#include "Initialize_API.h"
#include "SHA256.h"

#include "converter_PID.h"

#include "ioctl_process_sender.h"

/* PID �� ���μ��� ���� �� ��� */
typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset; // ���� ���μ��� ���� ����ü������ ����Ʈ ������
    ULONG NumberOfThreads; // �� ���μ����� ������ ��
    LARGE_INTEGER WorkingSetPrivateSize; // �����̺� �۾� ��Ʈ ũ��
    ULONG HardFaultCount; // �ϵ� ������ ��Ʈ ��
    ULONG NumberOfThreadsHighWatermark; // �������� �ִ� ��
    ULONGLONG CycleTime; // �� ���μ����� ����Ŭ �ð�
    LARGE_INTEGER CreateTime; // ���μ��� ���� �ð�
    LARGE_INTEGER UserTime; // ����� ��忡�� ���� �ð�
    LARGE_INTEGER KernelTime; // Ŀ�� ��忡�� ���� �ð�
    UNICODE_STRING ImageName; // ���μ����� �̹��� �̸�
    ULONG BasePriority; // �⺻ �켱����
    HANDLE UniqueProcessId; // ���μ��� �ĺ���
    HANDLE InheritedFromUniqueProcessId; // �θ� ���μ��� �ĺ���
    ULONG HandleCount; // ���μ����� ������ �ڵ� ��
    ULONG SessionId; // ���μ����� ���� ���� ID
    ULONG_PTR UniqueProcessKey; // ���μ����� ���� Ű
    SIZE_T PeakVirtualSize; // ���� �޸� �ִ� ũ��
    SIZE_T VirtualSize; // ���� ���� �޸� ũ��
    ULONG PageFaultCount; // ������ ��Ʈ ��
    SIZE_T PeakWorkingSetSize; // �۾� ��Ʈ�� �ִ� ũ��
    SIZE_T WorkingSetSize; // ���� �۾� ��Ʈ ũ��
    SIZE_T QuotaPeakPagedPoolUsage; // ������ Ǯ ��뷮�� �ִ� ũ��
    SIZE_T QuotaPagedPoolUsage; // ���� ������ Ǯ ��뷮
    SIZE_T QuotaPeakNonPagedPoolUsage; // ������¡ Ǯ ��뷮�� �ִ� ũ��
    SIZE_T QuotaNonPagedPoolUsage; // ���� ������¡ Ǯ ��뷮
    SIZE_T PagefileUsage; // ������ ���� ��뷮
    SIZE_T PeakPagefileUsage; // ������ ���� ��뷮�� �ִ� ũ��
    SIZE_T PrivatePageCount; // ���μ����� �����̺� ������ ��
    LARGE_INTEGER ReadOperationCount; // �б� �۾� ��
    LARGE_INTEGER WriteOperationCount; // ���� �۾� ��
    LARGE_INTEGER OtherOperationCount; // ��Ÿ �۾� ��
    LARGE_INTEGER ReadTransferCount; // �б� ���۷�
    LARGE_INTEGER WriteTransferCount; // ���� ���۷�
    LARGE_INTEGER OtherTransferCount; // ��Ÿ ���۷�
    // �� ����ü���� �߰������� �� ���� �ʵ尡 ���� �� �ֽ��ϴ�.
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

typedef struct Get_Process_List { // Get_Process_List �Լ��� output �� ( ���� �Ҵ�� ) 

    PUCHAR Previous_Address;

    HANDLE PID;
    UCHAR SHA256[SHA256_String_Byte_Length];

    PUCHAR Next_Address;

}Get_Process_List, * PGet_Process_List;

extern KMUTEX Get_Process_List_MUTEX;
extern PGet_Process_List Get_Process_List_Start_Address;
extern PGet_Process_List Get_Process_List_Current_Address;


PGet_Process_List Create_Process_List_Node(
    PGet_Process_List Previous_Address,

    HANDLE PID,
    CHAR* SHA256
);

PGet_Process_List APPEND_Process_List_Node(
    PGet_Process_List Current_Node,

    HANDLE PID,
    CHAR* SHA256
);



/*  ���� ����   */
PGet_Process_List Search_Process_Node(
    PGet_Process_List Start_Node,
    HANDLE target_PID
);



VOID print_Process_Node(PGet_Process_List Start_Node);

VOID REMOVE_Process_Node(
    PGet_Process_List Start_Node
);

// THread
VOID Get_ALL_Process_List(PVOID context);

#endif