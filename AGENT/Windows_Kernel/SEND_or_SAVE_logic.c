#include "SEND_or_SAVE.h"

PUCHAR RAW_DATA__external_var = NULL; // PLIST ������ ����� �����
ULONG32 RAW_DATA_SIZE__external_var = 0; // PLIST ������ ����� �����

Util_Mutex_with_Lock MUTEX_for_External_var_Read_Write = { NULL,0 }; //���� ���� ���������� ��


NTSTATUS Send_or_Save(SEND_or_SAVE_enum select, ULONG32* TYPE, PUCHAR* RAW_DATA, ULONG32* RAW_DATA_SIZE, SOCKET_INFORMATION receive_types) {

	if( Initilize_or_Locking_PKmutex(&MUTEX_for_External_var_Read_Write, TRUE)==FALSE) return STATUS_UNSUCCESSFUL; // �ڿ� ���� ( RAW_DATA__external_var �� ���� ��ȣ���� ���� )

	NTSTATUS status = STATUS_SUCCESS;

	switch (select) {
	case SEND_RAW_DATA:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SEND_RAW_DATA\n");

		/*
			�Ʒ��� ȣ���ؼ�
				RUST���� ���������� �������� ����..
					=> "RAW_DATA__external_var" , "RAW_DATA_SIZE__external_var" �� NULL�ʱ�ȭ �ȴ�.
		*/

		status = RAWDATA_SEND__when_request_from_server(
			&(*RAW_DATA), /*������ ������ ( �ߺ��� ��� ������ ��� ) */
			&(*RAW_DATA_SIZE), /*������ �������� ��� �������� ����*/

			receive_types // TCP Server���� ���� ��, Mutex �������� ���� ����
		);

		break;



	case SAVE_RAW_DATA:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "SAVE_RAW_DATA\n");
		/*
		
			���Ḯ��Ʈ ����ü(PLIST) -> 1���� ����-��� ��ȯ -> ���������� ���� �ϴ� ������
		
		*/


		//RAW_DATA__external_var; RAW_DATA_SIZE__external_var;
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TYPE -> %lu \n", *TYPE);
		if (SAVE_DATA_with_Length_based_RAW_DATA( // �� �Լ� ȣ�� �� ���������� ������ BSOD �߻�
			*TYPE,
			(PLIST)*RAW_DATA/*���Ḯ��Ʈ�� ��� ������ addr*/,
			*RAW_DATA_SIZE/*��� ���Ḯ��Ʈ�� ��� ���� ( �������� ULONG32 + PUCHAR �� 1set ) */,

			/*
				�Ʒ� 2�� ������ ��� ���������ε�, ������ ���� �Ķ���� �ѱ�
			*/
			&RAW_DATA__external_var,  /*�������� �ּ�*/
			&RAW_DATA_SIZE__external_var//, /*�������� Ulong32*/


		) != STATUS_SUCCESS) {

			status = STATUS_UNSUCCESSFUL;
			break;
		}



	default:
		status = STATUS_UNSUCCESSFUL;
	}

	Release_PKmutex(&MUTEX_for_External_var_Read_Write); // �ڿ� ���� ����
	return status;
}