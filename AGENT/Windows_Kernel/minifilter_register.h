#ifndef minifilter_register_H
#define minifilter_register_H

#include <fltKernel.h> // ���� ����̹� ��� ( ��Ŀ �����ϼ� ) 

#include "minifilter_handlers.h"// ���� �ڵ鷯 ������ ���

// ����ϱ�

extern PFLT_FILTER gFilterHandle; // ���� ���� ����

NTSTATUS Initialize_Mini_Filter_Driver(PDRIVER_OBJECT DriverObject);

#endif 