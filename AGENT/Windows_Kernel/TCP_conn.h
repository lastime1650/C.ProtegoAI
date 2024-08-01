#ifndef TCP_conn_H
#define TCP_conn_H

#include <ntifs.h>
#include <ntddk.h>

#include "TCP_send_or_Receiver.h" // 유틸리티 -> 송수신

// RUST 명령 구조체들
typedef enum Type_Feature {
	Create = 1,
	Remove = 2,
	Inbound = 4,
	Outbound = 8,
}Type_Feature;

typedef enum server_cmd {
	Err = 0,
	GET_file_bin = 1, // Server의 요청임 "파일을 보내라!..."( 이때, ANSI 문자열 ( 절대경로 ) 를 받는다 )

	GET_collected_data = 1650,

	GET_action_process_creation = 100,
	GET_action_process_network = 101,
	GET_action_network = 200,


	Are_you_ALIVE = 999, // 너 살아있니?



	Yes = 1029, // 긍정
	No = 1030 // 부정

}SERVER_COMMAND;


typedef enum TYPE_enum {
	process_routine = 1,
	network_filter = 10

}TYPE_for_DB;


// 전역변수
extern BOOLEAN is_connected_2_SERVER;
extern KEVENT is_connected_event;



// TCP 세션을 서버와 맺기
VOID TCP_conn(PVOID context);


// TCP 성공하면 호출되는 함수
//VOID communication_server(PVOID context);
/*
	이전됨 -> communication_with_RUST.h
*/


#endif