#ifndef Length_Based_Maker_H
#define Length_Based_Maker_H

#include <ntifs.h>

#pragma warning(disable:4996)
#pragma warning(disable:6386)

BOOLEAN Length_Based_MAKER(ULONG32 INPUT_Command, PUCHAR INPUT_RAW_DATA, ULONG32 INPUT_RAW_DATA_SIZE, PUCHAR* APPENDING_DATA, ULONG32* APPENDING_DATA_SIZE);

#endif