#include "License_Agent_struct.h"
#include "TCP_send_or_Receiver.h"
#include "util_Delay.h"

#include "my_ioctl.h"
BOOLEAN Initialize_IOCTL_communicate() {


	//ULONG32 limit_delay_count = 1000;

	if (IOCTL_share_structure.is_init) { // �ʱ��ΰ�? 
		while (1) {

			if (IOCTL_share_structure.is_usermode_request && (IOCTL_share_structure.ioctl_BUFFER != NULL)) {
				// ���⼭�� ������忡�� ��û�� ���� �� ����Ǵ� ����

				/*
					[NEW] Ioctl�� User_Mode ���α׷��� PID�� �׻� �޾ƾ���. (���۸���)
					�ش� PID�� Ŀ�θ�忡�� �ܺλ�����û���κ��� "��ȣ"�޵��� �ؾ���. -> ObRegisterCallbacks �������Ḯ��Ʈ�� �߰� 
				*/


				switch (IOCTL_share_structure.ioctl_BUFFER->information) {
				case REQUEST_all:
					// ���̼��� + ������Ʈ ID �� �� ���� ��,
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IOCTL ��û -> REQUEST_all\n");

					Driver_ID.information = IOCTL_share_structure.ioctl_BUFFER->information;

					memcpy(Driver_ID.LICENSE_ID, IOCTL_share_structure.ioctl_BUFFER->license_ID, 128); // �̽�

					memcpy(Driver_ID.AGENT_ID, IOCTL_share_structure.ioctl_BUFFER->Agent_ID, 128); // �̽�

					return TRUE;

				case REQUESET_without_AGENT_ID:
					// ���̼����� ���� ��  (���� AGENT_ID �߱��� �ʿ��ؿ�!)

					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IOCTL ��û -> REQUESET_without_AGENT_ID\n");

					Driver_ID.information = IOCTL_share_structure.ioctl_BUFFER->information;

					memcpy(Driver_ID.LICENSE_ID, IOCTL_share_structure.ioctl_BUFFER->license_ID, 128); // �̽�

					return TRUE;

				default:
					return FALSE; // �̻��� ���� �޾Ұų�, ���е� ���
				}

			}
			else {
				// ���ѽð��� �ϰ�, limit Ƚ����ŭ ���ѽð��� �ݺ��ϰ�, �̷��� else ��, ���� ����.
				//DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "IOCTL ��û ��� ���Դϴ�.\n");
				Delays(-1); // Default 3�� ���.

				//limit_delay_count -= 1;
				//if (limit_delay_count == 0) return FALSE;

				continue;
			}


		}
	}
	else {
		// �ʱ� ������ ��û�� 
		return FALSE; 
	}



	return FALSE;
}

//


NTSTATUS GET_AGENT_ID() {

	/*
		���� TCP ������ ���� ��, Agent ID �� Ȯ���Ѵ�. ( SMBIOS ���� Query_SMBIOS_information() �Լ����� �̹� �޾ƿԴ� ) 
		[ (SHA512) + (FULL-PATH)   ]
	*/

	PVOID Generated_AGENT_ID_by_RUST = NULL;
	ULONG32 Generated_AGENT_ID_by_RUST_len = 0;
	RECEIVE_TCP_DATA__with_alloc(&Generated_AGENT_ID_by_RUST, &Generated_AGENT_ID_by_RUST_len, TCP_DATA_RECEIVE);

	if (Generated_AGENT_ID_by_RUST_len < 128 || Generated_AGENT_ID_by_RUST == NULL) { // SHA512 ���̰� �ƴϸ� ����!
		return STATUS_UNSUCCESSFUL;
	}
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[AGENT_ID()] Generated_AGENT_ID_by_RUST_len ��� -> %lu", Generated_AGENT_ID_by_RUST_len);




	// {AGENT_ID} + {LICENSE_ID} ���� ���� ���� ����( Stack ) 
	UCHAR SEND_RESULT[( sizeof(Driver_ID)-4 )] = {0,}; // 4�� ���� ������ ENUM ����ü ����(��512����Ʈ)�� ���Ͽ�


	if (IOCTL_share_structure.is_init) {

		switch (IOCTL_share_structure.ioctl_BUFFER->information) {
		case REQUEST_all:
			// ���̼��� + ������Ʈ ID �� �� ���� ��, ( ���� Initialize_ID �Լ��� ���ؼ� ��ϵ� �������� Driver_ID�� ID 2���� RUST���� ����. )

			Driver_ID.information = IOCTL_share_structure.ioctl_BUFFER->information;

			memcpy(Driver_ID.LICENSE_ID, IOCTL_share_structure.ioctl_BUFFER->license_ID, 128); // �̽�

			memcpy(Driver_ID.AGENT_ID, IOCTL_share_structure.ioctl_BUFFER->Agent_ID, 128); // �̽�

			break;

		case REQUESET_without_AGENT_ID:
			// ���̼����� ���� ��  (���� AGENT_ID �߱��� �ʿ��ؿ�!)

			Driver_ID.information = IOCTL_share_structure.ioctl_BUFFER->information;

			memcpy(Driver_ID.LICENSE_ID, IOCTL_share_structure.ioctl_BUFFER->license_ID, 128); // �̽�

			memcpy(Driver_ID.AGENT_ID, Generated_AGENT_ID_by_RUST, 128); // **�̽� ( RUST���� �⺻ Generate�� ���� �ݿ��Ѵ�. )

			break;

		default:
			return STATUS_UNSUCCESSFUL; // �̻��� ���� �޾Ұų�, ���е� ��� ( �̶��� ����̹��� �����ϵ��� �ϰų� �ؾ��� ��
		}

	}
	else {
		// �� ����, �̹� �ʱ�ȭ�� ������ �߾���, �߰��� ������ ����ٰ� �ٽ� �����ϴ� �����. 

	}



	/*���� ���� �� copy */
	memcpy(SEND_RESULT, Driver_ID.AGENT_ID, sizeof(Driver_ID.AGENT_ID));
	memcpy(SEND_RESULT + sizeof(Driver_ID.AGENT_ID), Driver_ID.LICENSE_ID, sizeof(Driver_ID.LICENSE_ID));
	memcpy(SEND_RESULT + sizeof(Driver_ID.AGENT_ID) + sizeof(Driver_ID.LICENSE_ID) , Driver_ID.SMBIOS_TYPE_1, sizeof(Driver_ID.SMBIOS_TYPE_1));
	memcpy(SEND_RESULT + sizeof(Driver_ID.AGENT_ID) + sizeof(Driver_ID.LICENSE_ID) + sizeof(Driver_ID.SMBIOS_TYPE_1), Driver_ID.SMBIOS_TYPE_2, sizeof(Driver_ID.SMBIOS_TYPE_2));



	/* �������� ������ �ۼ��� �غ�Ǿ��ٰ� �����Ѵ�. */
	if (SEND_TCP_DATA(SEND_RESULT, sizeof(SEND_RESULT), TCP_DATA_SEND) != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[AGENT - FAIL] SEND_TCP_DATA ����!");
		return STATUS_UNSUCCESSFUL;
	}


	/* ������ ����� �޾ƾ��Ѵ�.-> IOCTL�� ������̸� ����� �˷������ */
	
	PVOID result_from_server_addr = NULL;
	ULONG32 result_from_server_addr_size = 0;
	if (RECEIVE_TCP_DATA__with_alloc((PVOID*) &result_from_server_addr, (ULONG32*)&result_from_server_addr_size, TCP_DATA_RECEIVE) == STATUS_UNSUCCESSFUL ) {
		// ioctl���� ������� ����
		if (IOCTL_share_structure.is_init) {
			IOCTL_share_structure.ioctl_BUFFER->information = FAIL_id;

			KeSetEvent(&IOCTL_share_structure.ioctl_event, 0, FALSE); //ioctl�� ������ �簳
		}


		ExFreePoolWithTag(Generated_AGENT_ID_by_RUST, 'RcDt');
		return STATUS_UNSUCCESSFUL;
	}


	// ioctl���� ������� �����ϱ�
	COMMUNICATION_IOCTL_ENUM result =  (COMMUNICATION_IOCTL_ENUM)(*(ULONG32*)result_from_server_addr);
	if (result == SUCCESS_id) {
		if (IOCTL_share_structure.is_init) {

			// Driver_ID���� AGENT_ID�� ���� ��ϼ����� ��� Driver_ID ���������� memcpy�ؾ��Ѵ�.(�ý��ۿ� �����ؼ� ���)
			if (IOCTL_share_structure.ioctl_BUFFER->information == REQUESET_without_AGENT_ID) {
				memcpy(Driver_ID.AGENT_ID, Generated_AGENT_ID_by_RUST, 128);
			}

			// ������� ����.
			IOCTL_share_structure.ioctl_BUFFER->information = SUCCESS_id;

			KeSetEvent(&IOCTL_share_structure.ioctl_event, 0, FALSE); //ioctl�� ������ �簳
		}
	}
	else {
		// �������� ���еǾ��� ���;; 
		if (IOCTL_share_structure.is_init) {
		

			// ������� ����.
			IOCTL_share_structure.ioctl_BUFFER->information = FAIL_id;

			KeSetEvent(&IOCTL_share_structure.ioctl_event, 0, FALSE); //ioctl�� ������ �簳
		}
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[AGENT - FAILED]");
		ExFreePoolWithTag(Generated_AGENT_ID_by_RUST, 'RcDt');
		return STATUS_UNSUCCESSFUL;
	}
	


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "\n[AGENT_ID��� SUCCESSED]-> AGENT_ID: %128s \n LICENSE_ID: %128s\n", Driver_ID.AGENT_ID, Driver_ID.LICENSE_ID);


	ExFreePoolWithTag(Generated_AGENT_ID_by_RUST, 'RcDt');

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[AGENT - SUCCESS]");
	return STATUS_SUCCESS;

}