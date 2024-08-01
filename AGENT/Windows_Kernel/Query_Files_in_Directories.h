#ifndef Query_Files_in_Directories_H
#define Query_Files_in_Directories_H

#include <ntifs.h>
#include "Get_Volumes.h" // ������ ������ �� �۵�������.

/*
	��ƿ��Ƽ �������� ��ȯ�Ͽ����Ƿ�, 
	���� ���� ���Ḯ��Ʈ�� �����ؾ���.

	1. ���� ������ (UNICODE_STRING.Buffer) 

	-> Search_Value = 'Dirs'
*/
#include "Parallel_Linked_List.h"
#define Node_Search_Value = 'Dirs';


/*
	���͸� ��ȸ�Ͽ� ���� �� ���͸� ã�� �� ����

	DirectoryPath �μ����� �������� \\ �� �߰����� �ʾƵ� ��

*/

// 3��° ���ڰ��� ���� ȣ�� �ÿ� �� ��ȿ�� �ּҸ� �־����
// ȣ���� ��� ���ϵǸ�, 3��° ���ڰ��� ���Ͽ� ��忡 ������ �� ����
// ��, �ܺο��� �Ҵ������ؾ���

/*
	��ü �˻��� ��ģ ����.PWCH�� ���͸�����
	-> ���͸������?
	> ��ҹ��� ������� ���ԵǸ� TRUE ��
	
	���� �μ����� NULL�̸� ��ü ��ĵ�� �ǹ��Ѵ�. 

	
*/
#include "test_Unicode_word_include.h" // ContainsStringInsensitive()
ULONG32 ListDirectories(
	
	PUNICODE_STRING DirectoryPath, BOOLEAN is_init, 
	
	PUNICODE_STRING INPUT_Hint_Data,
	
	PDynamic_NODE* Output_Node);

BOOLEAN ListDirectories_PoolFree(PDynamic_NODE Node_for_PoolFREE, ULONG32 Dir_Search_Value);

#endif // !Query_Files_in_Directories_H
