#ifndef GET_VOLUMES_H
#define GET_VOLUMES_H

#include <ntifs.h>
#define MAX_DEVICES 256
#pragma warning(disable:4996)

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, * POBJECT_DIRECTORY_INFORMATION;


// is_external_Device �� ���Ͽ� 
typedef enum DEVICE_DECTECT_ENUM {
    DEVICE_None,
    JUST_VOLUME_DISK,
    Internal_DISK,
    External_DISK_USB,// 3
    External_DISK_ISO,
    External_DISK_CDROM,

    USB_DEVICE_from_PNP // PNP �ݹ��Լ��� �񵿱� �����忡�� ȣ��Ǿ��� ���� �ν��ϴ� �뵵


}DEVICE_DECTECT_ENUM, * PDEVICE_DECTECT_ENUM;

/*
    ���Ḯ��Ʈ
*/
// �� ����̽� ����̺� ����ϴ� ���Ḯ��Ʈ 
typedef struct ALL_DEVICE_DRIVES {

    PUCHAR Previous_Node;

    UNICODE_STRING DRIVE_ALPHABET;
    UNICODE_STRING DRIVE_NT_PATH; // ���� 
    DEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE;

    UNICODE_STRING USBSTOR_Serial; // USB�� ���, �̰� ����. 
    UNICODE_STRING USB_PDO_obj; // PNP ����Ʈ�� USB ����̽�. ( �ַ� Node Remove�� )

    PUCHAR Next_Node;

}ALL_DEVICE_DRIVES, * PALL_DEVICE_DRIVES;

extern PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Start_Node; // �� ����̺� ���Ḯ��Ʈ ���۳�� 
extern PALL_DEVICE_DRIVES ALL_DEVICE_DRIVES_Current_Node; // �� ����̺� ���Ḯ��Ʈ ������ 

PALL_DEVICE_DRIVES Create_ALL_DEVICE_DRIVES_Node(PUCHAR Previous_addr, PUNICODE_STRING DRIVE_ALPHABET, PUNICODE_STRING DRIVE_NT_PATH, PUNICODE_STRING USB_Serial, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE);// ��� ����
PALL_DEVICE_DRIVES Append_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES current_Node, PUNICODE_STRING DRIVE_ALPHABET, PUNICODE_STRING DRIVE_NT_PATH, PUNICODE_STRING USB_Serial, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE);

//print
VOID Print_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node);

//finder hint�� ���Ͽ� ��� ���� ������ NULL��ȯ
PALL_DEVICE_DRIVES Finder_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Start_Node, PUNICODE_STRING IMPORTANT_DRIVE_NT_PATH_hint, PUNICODE_STRING Serial_Number_hint, PDEVICE_DECTECT_ENUM Option_INPUT_DRIVE_DEVICE_TYPE);

//������Ʈ
PALL_DEVICE_DRIVES Update_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES Spectified_Node_valid, PUNICODE_STRING DRIVE_ALPHABET, PDEVICE_DECTECT_ENUM DRIVE_DEVICE_TYPE);// ��� ����

//Ư��-��� ���� 
BOOLEAN Remove_Specified_ALL_DEVICE_DRIVES_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node, PALL_DEVICE_DRIVES Specified_Node);


// ������ �Լ�, �ߺ� �� �ʿ���� ���� ��������
BOOLEAN Complete_ALL_DEVICE_DRIVED_Node(PALL_DEVICE_DRIVES* Start_Node, PALL_DEVICE_DRIVES* Current_Node);


/*

    ����̺� ����

*/

extern POBJECT_TYPE* IoDriverObjectType;
extern PKMUTEX Get_Volume_KMUTEX;// �˾Ƽ� �����Ҵ��Ͽ� ���


/*
    ���Ḯ��Ʈ�� �����ϰų� Update�� �� ����

    [+]is_PNP_call �� PNP �ܺ� ��ġ ����Ʈ�� ����ȣ��ɋ� ������

    [+]is_remove_mode �� ��� ����������.

*/
VOID ListMountedDrives(PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING, BOOLEAN is_remove_node);


DEVICE_DECTECT_ENUM is_external_Device(PUNICODE_STRING Obj_Dir_NAME, PUNICODE_STRING NT_NAME, PUNICODE_STRING PNP_Device_Name_from_PNP_UNICODE_STRING);// Device���� Ȯ��

BOOLEAN Get_USB_Serial(PUNICODE_STRING INPUT_USB_DIR_INFO, PUNICODE_STRING OUTPUT_USB_SERIAL_NUMBER, ULONG32 Filter_Index, BOOLEAN Filter_with_Ampersand); // PNP�� ���� ���� USB NT��ο��� �ø��� ���� ���� 

// �μ����� ���� ����̺� ���� ���翩�� Ȯ�ο� / �ƿ�ǲ: ����̺� ���Ḯ��Ʈ ���
PALL_DEVICE_DRIVES is_Drives_PATH(PUNICODE_STRING INPUT_ABSOULTE_PATH);

#endif