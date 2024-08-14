#pragma warning(disable:4996)


#include <stdio.h>
#include <Windows.h>

#include "ioctl.h";


COMMUNICATION_IOCTL_ENUM input_license_agent(
	PUCHAR License_ID,
	PUCHAR Agent_ID
) {

	ULONG32 yes_or_no = 0;

	while (1) {


		// 라이선스 ID 입력 검사
		printf("라이선스 ID를 입력해주십시오: ");
		scanf("%128s", License_ID);

		if (strlen((PCHAR)License_ID) != 128) {
			printf(" 라이선스 입력한 값이 128이 아닙니다. 이보다 크거나 작을 수 없습니다;;\n ");
			continue;
		}


		// 에이전트 ID입력 검사
		while (1) {
			printf("에이전트 ID를 입력해주십시오: ");
			scanf("%128s", Agent_ID);

			if (strlen((PCHAR)Agent_ID) < 128) {
				printf(" 에이전트 입력한 값이 128이 아닙니다. 이런 경우에는 AGENT_ID를 새로 등록할 것입니다. [다시 입력하시겠습니까? -> (1): yes / (2) no(register)]\n ");
				scanf("%ld", &yes_or_no);
				if (yes_or_no == 1) {
					continue;
				}
				else {
					return REQUESET_without_AGENT_ID; // 에이전트 새로 할당요구
				}
			}
			if (strlen((PCHAR)Agent_ID) > 128) {
				printf(" 에이전트 입력한 값이 128를 초과합니다. 이런 경우에는 AGENT_ID를 새로 등록하거나 다시 입력해야만합니다. [다시 입력하시겠습니까? -> (1): yes / (2) no(register)]\n ");
				scanf("%ld", &yes_or_no);
				if (yes_or_no == 1) {
					continue;
				}
				else {
					return REQUESET_without_AGENT_ID; // 에이전트 새로 할당요구
				}
			}
			else {
				return REQUEST_all; //에이전트 기존 꺼 사용
			}
		}

	}

}

