#ifndef query_process_H
#define query_process_H

#include <ntifs.h>
#include <ntddk.h>
#include <ntstrsafe.h>

#include "Initialize_API.h"
#include "SHA256.h"

#include "converter_PID.h"

#include "ioctl_process_sender.h"

/* PID 로 프로세스 정보 다 얻기 */
typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset; // 다음 프로세스 정보 구조체까지의 바이트 오프셋
    ULONG NumberOfThreads; // 이 프로세스의 스레드 수
    LARGE_INTEGER WorkingSetPrivateSize; // 프라이빗 작업 세트 크기
    ULONG HardFaultCount; // 하드 페이지 폴트 수
    ULONG NumberOfThreadsHighWatermark; // 스레드의 최대 수
    ULONGLONG CycleTime; // 이 프로세스의 사이클 시간
    LARGE_INTEGER CreateTime; // 프로세스 생성 시간
    LARGE_INTEGER UserTime; // 사용자 모드에서 사용된 시간
    LARGE_INTEGER KernelTime; // 커널 모드에서 사용된 시간
    UNICODE_STRING ImageName; // 프로세스의 이미지 이름
    ULONG BasePriority; // 기본 우선순위
    HANDLE UniqueProcessId; // 프로세스 식별자
    HANDLE InheritedFromUniqueProcessId; // 부모 프로세스 식별자
    ULONG HandleCount; // 프로세스가 소유한 핸들 수
    ULONG SessionId; // 프로세스가 속한 세션 ID
    ULONG_PTR UniqueProcessKey; // 프로세스의 고유 키
    SIZE_T PeakVirtualSize; // 가상 메모리 최대 크기
    SIZE_T VirtualSize; // 현재 가상 메모리 크기
    ULONG PageFaultCount; // 페이지 폴트 수
    SIZE_T PeakWorkingSetSize; // 작업 세트의 최대 크기
    SIZE_T WorkingSetSize; // 현재 작업 세트 크기
    SIZE_T QuotaPeakPagedPoolUsage; // 페이지 풀 사용량의 최대 크기
    SIZE_T QuotaPagedPoolUsage; // 현재 페이지 풀 사용량
    SIZE_T QuotaPeakNonPagedPoolUsage; // 비페이징 풀 사용량의 최대 크기
    SIZE_T QuotaNonPagedPoolUsage; // 현재 비페이징 풀 사용량
    SIZE_T PagefileUsage; // 페이지 파일 사용량
    SIZE_T PeakPagefileUsage; // 페이지 파일 사용량의 최대 크기
    SIZE_T PrivatePageCount; // 프로세스의 프라이빗 페이지 수
    LARGE_INTEGER ReadOperationCount; // 읽기 작업 수
    LARGE_INTEGER WriteOperationCount; // 쓰기 작업 수
    LARGE_INTEGER OtherOperationCount; // 기타 작업 수
    LARGE_INTEGER ReadTransferCount; // 읽기 전송량
    LARGE_INTEGER WriteTransferCount; // 쓰기 전송량
    LARGE_INTEGER OtherTransferCount; // 기타 전송량
    // 이 구조체에는 추가적으로 더 많은 필드가 있을 수 있습니다.
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

typedef struct Get_Process_List { // Get_Process_List 함수의 output 형 ( 동적 할당용 ) 

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



/*  많이 쓰임   */
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