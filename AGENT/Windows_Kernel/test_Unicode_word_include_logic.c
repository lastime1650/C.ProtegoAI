#include "test_Unicode_word_include.h"

BOOLEAN ContainsStringInsensitive(PCUNICODE_STRING SourceString, PCUNICODE_STRING SearchString) {
    if (SourceString == NULL || SearchString == NULL) {
        return FALSE;
    }

    for (USHORT i = 0; i <= SourceString->Length / sizeof(WCHAR) - SearchString->Length / sizeof(WCHAR); ++i) {
        /*수동매핑.*/
        UNICODE_STRING SubString;
        SubString.Buffer = &SourceString->Buffer[i];
        SubString.Length = SearchString->Length;
        SubString.MaximumLength = SearchString->Length;

        if (RtlEqualUnicodeString(&SubString, SearchString, TRUE)) {
            return TRUE;
        }

    }

    return FALSE;
}