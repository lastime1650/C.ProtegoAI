#ifndef query_system_smbios_information_H
#define query_system_smbios_information_H

#include <ntifs.h>
#include <ntddk.h>

#include <wdf.h>
#include <ntstrsafe.h>

#include "License_Agent_struct.h" // Driver_ID 전역변수에 TYPE1, 2 두개 집어넣기

#include "Initialize_API.h"// 쿼리 API있음

#include "SHA256.h"// 하드웨어 정보를 기반으로 SHA512 추출

#define SMBIOS_TABLE_SIGNATURE 'RSMB'

typedef struct _SMBIOS_TABLE_ENTRY_POINT {
    UCHAR AnchorString[4]; // "_SM_"
    UCHAR EntryPointStructureChecksum;
    UCHAR EntryPointLength;
    UCHAR MajorVersion;
    UCHAR MinorVersion;
    USHORT MaxStructureSize;
    UCHAR EntryPointRevision;
    UCHAR FormattedArea[5];
    UCHAR IntermediateAnchorString[5]; // "_DMI_"
    UCHAR IntermediateChecksum;
    USHORT TableLength;
    ULONG TableAddress;
    USHORT NumberOfStructures;
    UCHAR BCDRevision;
} SMBIOS_TABLE_ENTRY_POINT, * PSMBIOS_TABLE_ENTRY_POINT;

#include "SMBIOS_type_structs.h"// SMBIOS 파싱용 구조체들이 있음


NTSTATUS Query_SMBIOS_information();

//VOID ParseSMBIOSTable(PVOID tableAddress, USHORT tableLength);



#endif