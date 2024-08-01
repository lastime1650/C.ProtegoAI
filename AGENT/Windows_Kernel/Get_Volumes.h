#ifndef GET_VOLUMES_H
#define GET_VOLUMES_H

#include <ntifs.h>
#define MAX_DEVICES 256
#pragma warning(disable:4996)

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, * POBJECT_DIRECTORY_INFORMATION;


// is_external_Device 의 리턴용 
typedef enum DEVICE_DECTECT_ENUM {
    DEVICE_None,
    JUST_VOLUME_DISK,
    Internal_DISK,
    External_DISK_USB,// 3
    External_DISK_ISO,
    External_DISK_CDROM,

    USB_DEVICE_from_PNP // PNP 콜백함수의 비동기 스레드에서 호출되었을 때를 인식하는 용도


}DEVICE_DECTECT_ENUM, * PDEVICE_DECTECT_ENUM;

/*
    연결리스트
*/
// 현 디바이스 드라이브 기억하는 연결리스트 
typedef struct ALL_DEVICE_DRIVES {

    PUCHAR Previous_Node;

    UNICODE_STRING DRIVE_ALPHABET;
    UNICODE_STRING DRIVE_NT_PATH; // 기준 
    DEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE;

    UNICODE_STRING USBSTOR_Serial; // USB인 경우, 이게 기준. 
    UNICODE_STRING USB_PDO_obj; // PNP 마운트된 USB 디바이스. ( 주로 Node Remove용 )

    PUCHAR Next_Node;

}ALL_DEVICE_DRIVES, * PALL_DEVICE_DRIVES;

extern PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Start_Node; // 총 드라이브 연결리스트 시작노드 
extern PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Current_Node; // 총 드라이브 연결리스트 현재노드 

PALL_DEVICE_DRIVES Create_ALL_DEVICE_DRIVES_Node(PUCHAR Previous_addr, PUNICODE_STRING DRIVE_ALPHABET, PUNICODE_STRING DRIVE_NT_PATH, PUNICODE_STRING USB_Serial, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE);// 노드 생성
PALL_DEVICE_DRIVES Append_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES current_Node, PUNICODE_STRING DRIVE_ALPHABET, PUNICODE_STRING DRIVE_NT_PATH, PUNICODE_STRING USB_Serial, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE);

//print
VOID Print_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node);

//finder hint를 통하여 노드 추출 없으면 NULL반환
PALL_DEVICE_DRIVES Finder_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node, PUNICODE_STRING IMPORTANT_DRIVE_NT_PATH_hint, PUNICODE_STRING Serial_Number_hint, PDEVICE_DECTECT_ENUM Option_INPUT_DRIVE_DEVICE_TYPE);

//업데이트
PALL_DEVICE_DRIVES Update_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Spectified_Node_valid, PUNICODE_STRING DRIVE_ALPHABET, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE);// 노드 수정

//특정-노드 삭제 
BOOLEAN Remove_Specified_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node, PALL_DEVICE_DRIVES Specified_Node);


// 마무리 함수, 중복 및 필요없는 노드는 지워야함
BOOLEAN Complete_ALL_DEVICE_DRIVED_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node);


/*

    드라이브 추출

*/

extern POBJECT_TYPE* IoDriverObjectType;
extern PKMUTEX Get_Volume_KMUTEX;// 알아서 동적할당하여 사용


/*
    연결리스트를 생성하거나 Update될 수 있음

    [+]is_PNP_call 는 PNP 외부 장치 마운트에 의해호출될떄 감지용

    [+]is_remove_mode 는 노드 삭제용인지.

*/
VOID ListMountedDrives(PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING, BOOLEAN is_remove_node);


DEVICE_DECTECT_ENUM is_external_Device(PUNICODE_STRING Obj_Dir_NAME, PUNICODE_STRING NT_NAME, PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING);// Device인지 확인

BOOLEAN Get_USB_Serial(PUNICODE_STRING INPUT_USB_DIR_INFO, PUNICODE_STRING OUTPUT_USB_SERIAL_NUMBER, ULONG32 Filter_Index, BOOLEAN Filter_with_Ampersand); // PNP를 통해 얻은 USB NT경로에서 시리얼 정보 추출 

// 인수값에 대한 드라이브 문자 존재여부 확인용 / 아웃풋: 드라이브 연결리스트 노드
PALL_DEVICE_DRIVES is_Drives_PATH(PUNICODE_STRING INPUT_ABSOULTE_PATH);

#endif