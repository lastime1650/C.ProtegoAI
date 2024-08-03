#ifndef RUST_DLP_Get_File_SIgnatures_H
#define RUST_DLP_Get_File_SIgnatures_H

#include <ntifs.h>

#include "Length_Based_Data_Parser.h" // ����-���
#include "policy_signature_list_manager.h"

#include "converter_string.h"

/*  RUST �����κ��� ���� CHAR�� �ƽ�Ű���ڿ��� Ȯ���� ���ڿ��� "����-���"���� �޾� ���� ���Ḯ��Ʈ�� ��� */

typedef enum SIG_STATUS {
	is_register = 1,
	is_remove,
	is_get
}SIG_STATUS;


// ������� �������� �Ǵ� ���� ��ϵ� ������ ��û�ϴ� ��������? �ʼ�.
BOOLEAN Signature_append_or_remove_or_get(PLength_Based_DATA_Node INPUT);

#endif