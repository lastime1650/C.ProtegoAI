#ifndef TCP_conn_H
#define TCP_conn_H

#include <ntifs.h>
#include <ntddk.h>

#include "TCP_send_or_Receiver.h" // ��ƿ��Ƽ -> �ۼ���

// RUST ��� ����ü��
typedef enum Type_Feature {
	Create = 1,
	Remove = 2,
	Inbound = 4,
	Outbound = 8,
}Type_Feature;

typedef enum server_cmd {
	Err = 0,
	GET_file_bin = 1, // Server�� ��û�� "������ ������!..."( �̶�, ANSI ���ڿ� ( ������ ) �� �޴´� )

	GET_collected_data = 1650,

	GET_action_process_creation = 100,
	GET_action_process_network = 101,
	GET_action_network = 200,

	Signature_Management = 300, // DLP �ñ״�ó ����

	Are_you_ALIVE = 999, // �� ����ִ�?



	Yes = 1029, // ����
	No = 1030 // ����

}SERVER_COMMAND;


typedef enum TYPE_enum {
	process_routine = 1,
	network_filter = 10

}TYPE_for_DB;


// ��������
extern BOOLEAN is_connected_2_SERVER;
extern KEVENT is_connected_event;



// TCP ������ ������ �α�
VOID TCP_conn(PVOID context);

// �߰��� TCP ������ ������ ��� ó��
/*
	is_connected_event KEVENT �������� �����带 �ٽ� �簳 �ϵ��� �ؾ���.

	����, �̹� ��å�� ����� ��찡 �ִ� �ݹ��Լ��� �̾ ����� �� �ִ�.
	��, ����͸��� �������� ������ �� �� ����
	PsSetCreateProcessNotifyCAllbackRoutine()

*/
VOID TCP_Disconnected_conn();

// TCP �����ϸ� ȣ��Ǵ� �Լ�
//VOID communication_server(PVOID context);
/*
	������ -> communication_with_RUST.h
*/


#endif