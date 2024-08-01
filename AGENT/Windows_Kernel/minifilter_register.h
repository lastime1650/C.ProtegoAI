#ifndef minifilter_register_H
#define minifilter_register_H

#include <fltKernel.h> // 필터 드라이버 헤더 ( 링커 삽입하쇼 ) 

#include "minifilter_handlers.h"// 필터 핸들러 모음집 헤더

// 등록하기

extern PFLT_FILTER gFilterHandle; // 전역 변수 선언만

NTSTATUS Initialize_Mini_Filter_Driver(PDRIVER_OBJECT DriverObject);

#endif 