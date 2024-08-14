#ifndef Hooking_H
#define Hooking_H

#include <stdio.h>
#include <Windows.h>

typedef struct HOOKING_move {
	HANDLE PID;
	HANDLE PROCESS_HANDLE;
}HOOKING_move, *PHOOKING_move;

VOID START_HOOKING(HOOKING_move* PROCESS_CONTEXT);

#endif