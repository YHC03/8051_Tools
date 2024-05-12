#include<stdio.h>
#include<string.h>
#include"8051_Variables.h" // 8051의 각종 상수와, RAM, ROM 변수를 저장함
#include"FileInput.h" // 파일 입력을 진행함
#include"RunFunction.h" // 8051 Simulator 실행을 진행함

/*
* 8051 Simulator
*
* 제한사항 : 외장 메모리 미지원, UART 기능 미지원
* UART 기능의 경우, 송신하는 데이터를 보관하는 변수의 메모리가 Overflow될 우려가 있고, 다양한 종류의 UART 통신을 CLI 환경에서 구현하기에는 한계가 있어 구현하지 않고 있다.
*
* 주의사항 : Regiser Indirect 명령 이용 시, Special Function Register를 읽고 수정한다. 이 프로그램은 8052가 아닌 8051을 기반으로 하였으므로, 이러한 동작은 정상적인 동작이다.
* AC, OV는 ADD, ADDC, SUBB 함수에서만 작동한다. MUL, DIV는 제조사마다 해당 Bit값이 변화하는 형태가 다른 관계로 해당 값이 변화하지 않는다. 또한, DIV 함수에서는 0으로 나누면 아무 일도 일어나지 않는다.
*
* 작성자 : YHC03
*/


// 초기화
void init();


/* init() 함수
*
* 기능 : chip의 내부 메모리를 초기화한다
* 입출력 변수 없음
*/
void init()
{
	// RAM을 0으로 초기화
	for (unsigned short i = 0; i <= 255; i++)
	{
		chip.internal_RAM[i] = 0;
	}

	// SBUF_send 초기화
	chip.SBUF_send = 0;

	// Stack Pointer의 값은 0x07로 초기화
	chip.internal_RAM[SP] = 0x07;

	// Latch 초기화
	for (unsigned short i = 0; i < 4; i++)
	{
		chip.latch[i] = 0;
	}

	// Port의 값을 0xFF로 초기화(Interrupt 무시)
	chip.internal_RAM[0x80] = 0xFF;
	chip.internal_RAM[0x90] = 0xFF;
	chip.internal_RAM[0xA0] = 0xFF;
	chip.internal_RAM[0xB0] = 0xFF;

	// 모든 Port에 대해 Latch값을 RAM의 Port 변수와 연동
	syncLatch(0);
	syncLatch(1);
	syncLatch(2);
	syncLatch(3);

	return;
}


int main(int argc, char* argv[])
{
	// 순서대로 mode와 마지막 Program Counter를 저장하는 변수와, 읽어들일 파일명을 저장하는 변수 선언
	int mode = 0, end_PC = 0;
	char fileName[256] = "";

	// 안내문구 출력
	printf("8051 Simulator by YHC03\n\n");

	// 파일명에 공백이 있는 경우를 처리한다.
	for (int i = 1; i < argc; i++)
	{
		strcat(fileName, argv[i]);
		strcat(fileName, " ");
	}

	// 파일을 추가하지 않은 경우
	if (argc == 1)
	{
		printf("No File Included!\n");
		return 1;
	}

	// 초기화
	init();

	// 파일 읽기
	end_PC = fileReader(fileName);
	
	// Mode 설정
	printf("Debug Mode : 0, Auto-Running Mode : 1. : ");
	scanf("%d", &mode);

	// 프로그램 실행
	RunProgram(mode, end_PC);

	return 0;
}