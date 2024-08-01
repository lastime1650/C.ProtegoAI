#ifndef converter_string_H
#define converter_string_H

#include <ntifs.h>
#include <ntddk.h>


/*UNICODE_STRING --> ANSI_STRING*/
NTSTATUS UNICODE_to_ANSI(ANSI_STRING* OUTPUT_ansi, UNICODE_STRING* INPUT_unicode);


/*ANSI_STRING --> UNICODE_STRING*/
NTSTATUS ANSI_to_UNICODE(UNICODE_STRING* OUTPUT_unicode, ANSI_STRING INPUT_ansi);

#endif