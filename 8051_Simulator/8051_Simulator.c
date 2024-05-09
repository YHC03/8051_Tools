#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<windows.h>


/*
* 8051 Simulator
* 
* 제한사항 : 외장 메모리 미지원, UART 기능 미지원(입력을 못 받음)
* 
* 주의사항 : Regiser Indirect 사용 시, 일부 경우에만 Latch 연산을 하는 경우가 있음(다만, 8052가 아닌 8051에서는 존재할 수 없는 구문으로 알고 있음)
* XCHD에서는 Latch 연산 안 함.
* Carry, AC, OV는 ADD, ADDC, SUBB 함수에서만 작동함(MUL, DIV는 제조사마다 변화하는 형태가 다른 관계로 해당 값이 변화하지 않음)
* 
* 작성자 : YHC03
*/


typedef struct
{
	unsigned char internal_RAM[256]; // Special Function Register 포함
	unsigned char latch[4];
}Chip;

// RAM, ROM 선언
Chip chip;
unsigned char ROM[65535];

// Interrupt 관련 변수 선언
char intData[4] = { 0,0,0,0 }; // Interrupt 활성화 여부, (2는 활성화 및 실행 안됨, 1은 활성화 및 실행 중, 0은 비활성화)

// 특수 명령어 Bytes
const unsigned char TWO_BYTES[] = {0x01, 0x05, 0x11, 0x15, 0x21, 0x24, 0x25, 0x31, 0x34,
0x35, 0x40, 0x41, 0x42, 0x44, 0x45, 0x50, 0x51, 0x52, 0x54, 0x55, 0x60, 0x61, 0x62, 0x64,
0x65, 0x70, 0x71, 0x72, 0x74, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
0x80, 0x81, 0x82, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x91, 0x92,
0x94, 0x95, 0xA0, 0XA1, 0xA2, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
0xB0, 0xB1, 0xB2, 0xC1, 0xC2, 0xC3, 0xC5, 0xD0, 0xD1, 0xD2, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC,
0xDD, 0xDE, 0xDF, 0xE1, 0xE5, 0xF1, 0xF5};
const unsigned char THREE_BYTES[] = { 0x02, 0x10, 0x12, 0x21, 0x30, 0x43, 0x53, 0x63, 0x75,
0x85, 0x90, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xD5 };

// 특수 명령어 Cycle
const unsigned char TWO_CYCLE[] = { 0x01, 0x02, 0x10, 0x11, 0x12, 0x20, 0x21, 0x22, 0x30,
0x31, 0x32, 0x40, 0x41, 0x43, 0x50, 0x51, 0x53, 0x60, 0x61, 0x63, 0x70, 0x71, 0x72, 0x73,
0x75, 0x80, 0x81, 0x82, 0x83, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
0x8F, 0x90, 0x91, 0x93, 0xA1, 0xA3, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE,
0xAF, 0xB0, 0xB1, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
0xC0, 0xC1, 0xD0, 0xD1, 0xD5, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE1, 0xE2,
0xE3, 0xE4, 0xF0, 0xF1, 0xF2, 0xF3 };
const unsigned char FOUR_CYCLE[] = { 0x84, 0xA4 };

// Special Function Register
const unsigned char ACC = 0xE0;
const unsigned char PSW = 0xD0;
const unsigned char SP = 0x81;
const unsigned char DPH = 0x83;
const unsigned char DPL = 0x82;
const unsigned char TCON = 0x88;
const unsigned char TMOD = 0x89;
const unsigned char TL0 = 0x8A;
const unsigned char TL1 = 0x8B;
const unsigned char TH0 = 0x8C;
const unsigned char TH1 = 0x8D;
const unsigned char IE = 0xA8;
const unsigned char IP = 0xB8;

// Special Bit Address
const unsigned char C = 0xD7; // Carry Bit
const unsigned char AC = 0xD6;
const unsigned char PSW1 = 0xD4;
const unsigned char PSW0 = 0xD3;
const unsigned char OV = 0xD2;
const unsigned char P = 0xD0;
const unsigned char TR0 = 0x8C;
const unsigned char TR1 = 0x8E;
const unsigned char TF0 = 0x8D;
const unsigned char TF1 = 0x8F;
const unsigned char EA = 0xAF;
const unsigned char ES = 0xAC;
const unsigned char ET1 = 0xAB;
const unsigned char EX1 = 0xAA;
const unsigned char ET0 = 0xA9;
const unsigned char EX0 = 0xA8;
const unsigned char PS = 0xBC;
const unsigned char PT1 = 0xBB;
const unsigned char PX1 = 0xBA;
const unsigned char PT0 = 0xB9;
const unsigned char PX0 = 0xB8;



// 함수 시작

// 파일 입력
int fileReader(char* fileName);
unsigned char asciiToHEX(unsigned char orig); // HEX 변환

// 프로그램 구동
void RunProgram(unsigned char mode, int end_PC);
int programRunner(unsigned char code, unsigned char data1, unsigned char data2, int PC, char isDebugMode);

// 입력 보조
void inputDat();


// 8051 내부 함수 구현
// Bit 연산
char getBitAddr(unsigned char location); // Bit값 가져오기
void setBitAddr(unsigned char location); // SETB
void clearBitAddr(unsigned char location); // CLR

// 명령 이후의 처리
void putParity(); // ACC의 Parity
void syncLatch(char port); // Latch 연동

// 가.감 연산
void addFunc(short src, char isDat, char isCarry); // ADD, ADDC
void subbFunc(short src, char isDat); // SUBB

// 곱하기, 나누기
void mulAndDiv(char isDiv);

// Stack 연산
void stackOperation(unsigned char src, int isPop); // 일반적인 Stack 연산
unsigned char stackOperationPC(unsigned char src, int isPop); // Program Counter용 Stack 연산

// 논리 연산
void orOperation(unsigned char dest, unsigned char src, char isData, char isBit); // ORL
void andOperation(unsigned char dest, unsigned char src, char isData, char isBit); // ANL
void xorOperation(unsigned char dest, unsigned char src, char isData); // XRL

// Swap
void swapOperation(unsigned char dest, unsigned char src); // XCH

// DA 함수
void DAOperation();

// Timer/Counter
void timerControl(int cycle);

// Interrupt Control
int interruptControl(int PC);
char getInterruptPriorityRun();
void clearInterrupt();

// Port Input 가져오기
void getPortValue();

// 함수 끝


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

	// Stack Pointer의 값은 0x07로 초기화
	chip.internal_RAM[SP] = 0x07;
	
	// Latch 초기화
	for(unsigned short i = 0; i < 4; i++)
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


/* timerControl() 함수
* 
* 기능 : timer 기능 작동
* 입력 변수 : 이번 명령어 실행에서 지나간 Cycle(현재 Cycle - 이전 Cycle 의 값)
* 출력 변수 없음
* 
* 버그 : C/T 활성화 시, clock 감지 불가 오류 있음
* Timer 3에서 버그 발생 가능성 있음
*/
void timerControl(int cycle)
{
	// 외부 Clock (Rising Edge) 관련 변수 선언
	static char T0 = 0;
	static char T1 = 0;

	// TRO 활성화시
	if (getBitAddr(TR0))
	{
		// GATE=0 혹은 GATE=1이면서 INT0=0
		if (!(chip.internal_RAM[TMOD] & 0x08) || ((chip.internal_RAM[TMOD] & 0x08) && !getBitAddr(0xB2)/*P3.3*/))
		{
			// C/T=1이면서 T0=0->1 혹은 C/T=0
			if ((getBitAddr(0xA4/*P3.4*/) && !T0) || !(chip.internal_RAM[TMOD] & 0x04))
			{
				// T0값 저장
				T0 = getBitAddr(0xA4/*P3.4*/);

				// Timer Mode 확인 후 값 증가
				switch (chip.internal_RAM[TMOD] & 0x03)
				{
				case 0: // 2^13
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					if (chip.internal_RAM[TL0] + (!(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1) >= 0x100)
					{
						// TH0 1 증가
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
						chip.internal_RAM[TH0]++;

						// 2^13 초과 시
						if (chip.internal_RAM[TH0] >= 0x40)
						{
							// Timer 초기화 후, TF0를 1로 설정
							chip.internal_RAM[TH0] = 0;
							setBitAddr(TF0);
						}
					}else{
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
					}
					break;

				case 1: // 2^16
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					if (chip.internal_RAM[TL0] + (!(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1) >= 0x100)
					{
						// TH0 1 증가
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
						chip.internal_RAM[TH0]++;

						// 2^16 초과 시
						if (chip.internal_RAM[TH0] == 0)
						{
							// Timer 초기화 후, TF0를 1로 설정
							setBitAddr(TF0);
						}
					}else{
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
					}
					break;

				case 2: // 2^8 setup
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					if (chip.internal_RAM[TL0] + (!(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1) >= 0x100)
					{
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;

						// (Overflow가 발생한) TL0(Timer변수)에 TH0(기존값 저장한 변수) 더하기
						chip.internal_RAM[TL0] += chip.internal_RAM[TH0];

						// Timer 초기화 후, TF0를 1로 설정
						setBitAddr(TF0);
					}else{
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
					}
					break;

				case 3: // 2 * 2^8
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter). TH0(무조건 Timer)에 대해서는 뒤에서 처리
					if (chip.internal_RAM[TL0] + (!(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1) >= 0x100)
					{
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;

						// Timer 초기화 후, TF0를 1로 설정
						setBitAddr(TF0);
					}else{
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
					}
					break;
				}
			}
			// T0값 저장
			T0 = getBitAddr(0xA4/*P3.4*/);
		}
		if ((chip.internal_RAM[TMOD] & 0x03) == 0x03) // TMOD=0x03에서의 2번째 Timer는 무조건 시간에 의해서만 작동함
		{
			// Timer3 TH0 Overflow 발생 시
			if (chip.internal_RAM[TH0] + cycle >= 0x100)
			{
				// Timer 초기화 후, TF1를 1로 설정
				chip.internal_RAM[TH0] += cycle;
				setBitAddr(TF1);
			}else{
				chip.internal_RAM[TH0] += cycle;
			}
		}
	}

	// TR1 활성화시
	if (getBitAddr(TR1))
	{
		// GATE=0 혹은 GATE=1이면서 INT1=0
		if (!(chip.internal_RAM[TMOD] & 0x80) || ((chip.internal_RAM[TMOD] & 0x80) && !getBitAddr(0xB2)/*P3.3*/))
		{
			// C/T=1이면서 T1=0->1 혹은 C/T=0
			if ((getBitAddr(0xA5/*P3.5*/) && !T1) || !(chip.internal_RAM[TMOD] & 0x40))
			{
				// T1값 저장
				T1 = getBitAddr(0xA5/*P3.5*/);

				// Timer Mode 확인 후 값 증가
				switch (chip.internal_RAM[TMOD] & 0x30)
				{
				case 0x00: // 2^13
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					if (chip.internal_RAM[TL1] += (!(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1) >= 0x100)
					{
						// TH1 1 증가
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
						chip.internal_RAM[TH1]++;

						// 2^13 초과 시
						if (chip.internal_RAM[TH1] >= 0x40)
						{
							// Timer 초기화 후, TF0를 1로 설정
							chip.internal_RAM[TH1] = 0;
							setBitAddr(TF1);
						}
					}else{
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
					}
					break;

				case 0x10: // 2^16
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)					
					if (chip.internal_RAM[TL1] + (!(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1) >= 0x100)
					{
						// TH1 1 증가
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
						chip.internal_RAM[TH1]++;

						// 2^16 초과 시
						if (chip.internal_RAM[TH1] == 0)
						{
							// Timer 초기화 후, TF0를 1로 설정
							setBitAddr(TF1);
						}
					}else{
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
					}
					break;

				case 0x20: // 2^8 setup
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					if (chip.internal_RAM[TL1] + (!(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1) >= 0x100)
					{
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;

						// (Overflow가 발생한) TL0(Timer변수)에 TH0(기존값 저장한 변수) 더하기
						chip.internal_RAM[TL1] = chip.internal_RAM[TH1];

						// Timer 초기화 후, TF0를 1로 설정
						setBitAddr(TF1);
					}else{
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
					}
					break;

				case 0x30: // STOP
					// Timer1 Mode 3. 해당 경우에는, Timer1의 작동을 중지함.
					clearBitAddr(TR1);
					break;
				}
			}
			// T1값 저장
			T1 = getBitAddr(0xA5/*P3.4*/);
		}
	}

	return;
}


/* getInterruptPriorityRun() 함수
* 
* 기능 : 우선순위 순서로 활성화 할 Interrupt의 숫자를 불러온다
* 입력 변수 없음
* 출력 변수 : 활성화 할 Interrupt의 숫자
*/
char getInterruptPriorityRun()
{
	char interruptPrior = chip.internal_RAM[IP] & 0x0F;

	// Priority 설정 -> 일반 순서로 가되
	// 이미 상위 순서가 실행중인 경우 거부
	
	// 0, 1, 2, 3 순서로 Priority & 실행 중이 아님을 확인(실행 중인 경우 -1)
	if ((interruptPrior & 0x01) && intData[0] == 2) // Priority 설정한 Interrupt 0 실행 대기
	{
		return 0;
	}else if ((interruptPrior & 0x01) && intData[0] == 1){ // Priority 설정한 Interrupt 0 실행중
		return -1;

	}else if ((interruptPrior & 0x02) && intData[1] == 2) { // Priority 설정한 Interrupt 1 실행 대기
		return 1;
	}else if ((interruptPrior & 0x02) && intData[1] == 1) { // Priority 설정한 Interrupt 1 실행중
		return -1;

	}else if ((interruptPrior & 0x04) && intData[2] == 2) { // Priority 설정한 Interrupt 2 실행 대기
		return 2;
	}else if ((interruptPrior & 0x04) && intData[2] == 1) { // Priority 설정한 Interrupt 2 실행중
		return -1;

	}else if ((interruptPrior & 0x08) && intData[3] == 2){ // Priority 설정한 Interrupt 3 실행 대기
		return 3;
	}else if ((interruptPrior & 0x08) && intData[3] == 1) { // Priority 설정한 Interrupt 3 실행중
		return -1;
	}

	// 0, 1, 2, 3 순서로 ~Priority & 실행 중이 아님을 확인(실행 중인 경우 -1)
	if (!(interruptPrior & 0x01) && intData[0] == 2) // Priority 설정하지 않은 Interrupt 0 실행 대기
	{
		return 0;
	}else if(!(interruptPrior & 0x01) && intData[0] == 1){ // Priority 설정하지 않은 Interrupt 0 실행중
		return -1;

	}else if (!(interruptPrior & 0x02) && intData[1] == 2){ // Priority 설정하지 않은 Interrupt 1 실행 대기
		return 1;
	}else if (!(interruptPrior & 0x02) && intData[1] == 1){ // Priority 설정하지 않은 Interrupt 1 실행중
		return -1;

	}else if (!(interruptPrior & 0x04) && intData[2] == 2){ // Priority 설정하지 않은 Interrupt 2 실행 대기
		return 2;
	}else if (!(interruptPrior & 0x04) && intData[2] == 1){ // Priority 설정하지 않은 Interrupt 2 실행중
		return -1;

	}else if (!(interruptPrior & 0x08) && intData[3] == 2){ // Priority 설정하지 않은 Interrupt 3 실행 대기
		return 3;
	}else if (!(interruptPrior & 0x08) && intData[3] == 1){ // Priority 설정하지 않은 Interrupt 3 실행중
		return -1;
	}

	return -1; //No Interrupt
}


/* clearInterrupt() 함수
*
* 기능 : 우선순위 순서로 완료한 Interrput를 비활성화한다
* 입출력 변수 없음
*/
void clearInterrupt()
{
	char interruptPrior = chip.internal_RAM[IP] & 0x0F;

	// 0, 1, 2, 3 순서로 Priority & 실행 중 확인
	if ((interruptPrior & 0x01) && intData[0] == 1) // Priority 설정한 Interrupt 0 실행 종료
	{
		intData[0] = 0;
		return;
	}else if ((interruptPrior & 0x02) && intData[1] == 1){ // Priority 설정한 Interrupt 1 실행 종료
		intData[1] = 0;
		return;
	}else if ((interruptPrior & 0x04) && intData[2] == 1){ // Priority 설정한 Interrupt 2 실행 종료
		intData[2] = 0;
		return;
	}else if ((interruptPrior & 0x08) && intData[3] == 1){ // Priority 설정한 Interrupt 3 실행 종료
		intData[3] = 0;
		return;
	}

	// 0, 1, 2, 3 순서로 ~Priority & 실행 중 확인
	if (intData[0] == 1) // Priority 설정 안 한 Interrupt 0 실행 종료 (Priority가 설정된 경우, 위에서 처리하여 Return됨)
	{
		intData[0] = 0;
		return;
	}else if (intData[1] == 1){ // Priority 설정 안 한 Interrupt 1 실행 종료 (Priority가 설정된 경우, 위에서 처리하여 Return됨)
		intData[1] = 0;
		return;
	}else if (intData[2] == 1){ // Priority 설정 안 한 Interrupt 2 실행 종료 (Priority가 설정된 경우, 위에서 처리하여 Return됨)
		intData[2] = 0;
		return;
	}else if (intData[3] == 1){ // Priority 설정 안 한 Interrupt 3 실행 종료 (Priority가 설정된 경우, 위에서 처리하여 Return됨)
		intData[3] = 0;
		return;
	}
	
	// 그냥 RETI가 실행된 경우 ( = RET)
	return;
}


/* getPortValue() 함수
*
* 기능 : Port 값을 읽어온다.
* 입출력 변수 없음
*/
void getPortValue()
{
	// 입력값을 받는 변수
	unsigned char tmpData = 0;

	// P0 입력
	printf("P0 : 0x");
	scanf("%H", &tmpData);
	chip.internal_RAM[0x80] = chip.latch[0] & tmpData; // Latch에 0이 써진 경우, 읽지 않음

	// P1 입력
	printf("P1 : 0x");
	scanf("%H", &tmpData);
	chip.internal_RAM[0x90] = chip.latch[1] & tmpData; // Latch에 0이 써진 경우, 읽지 않음

	// P2 입력
	printf("P2 : 0x");
	scanf("%H", &tmpData);
	chip.internal_RAM[0xA0] = chip.latch[2] & tmpData; // Latch에 0이 써진 경우, 읽지 않음

	// P3 입력
	printf("P3 : 0x");
	scanf("%H", &tmpData);
	chip.internal_RAM[0xB0] = chip.latch[3] & tmpData; // Latch에 0이 써진 경우, 읽지 않음

	// 출력이 아니므로, syncLatch() 함수는 호출하지 않는다.

	return;
}


/* syncLatch() 함수
*
* 기능 : 8051에서 특정 Port로 값을 출력 시, Latch값도 동일하게 한다.
* 입력 변수 : port(P0, P1, P2, P3)
* 출력 변수 없음
*/
void syncLatch(char port)
{
	// Latch값을 8051 내부의 P0, P1, P2, P3 레지스터의 값으로 설정한다.
	chip.latch[port] = chip.internal_RAM[0x80 + 0x10 * port];

	return;
}


/* interruptControl() 함수
*
* 기능 : Interrupt 기능을 작동시킨다.
* 입력 변수 : 현재 Program Counter값
* 출력 변수 : 다음 Program Counter값
*/
int interruptControl(int PC)
{
	// 외부 Interrupt (Falling Edge) 관련 변수 선언
	static char INT0 = 1;
	static char INT1 = 1;

	const unsigned char INTERRUPT_PC[4] = { 0x03, 0x0B, 0x13, 0x1B }; // Serial은 지원 안함.
	
	// 이동할 인터럽트의 위치를 임시로 저장
	char res;

	// 전체 인터럽트 비활성화 시, 인터럽트 작동 종료
	if (!getBitAddr(EA))
	{
		// 실행 대기중(intData = 2)인 인터럽트 초기화
		// 실행중(intData = 1)인 인터럽트는 초기화하지 않음
		for (char i = 0; i < 4; i++)
		{
			if (intData[i] == 2)
			{
				intData[i] = 0;
			}
		}

		return PC;
	}

	// 각 인터럽트별로 활성화 확인
	if (getBitAddr(EX0)) // 외부 인터럽트 0
	{
		// Falling Edge
		if (!getBitAddr(0xB2/*P3.2*/) && INT0)
		{
			intData[0] = 2;
		}
		
	}
	INT0 = getBitAddr(0xB2/*P3.2*/);

	if (getBitAddr(ET0)) // 타이머 인터럽트 0
	{
		if (getBitAddr(TF0))
		{
			intData[1] = 2;
			clearBitAddr(TF0);
		}
	}

	if (getBitAddr(EX1)) // 외부 인터럽트 1
	{
		// Falling Edge
		if (getBitAddr(0xB3/*P3.3*/) && INT1)
		{
			intData[2] = 2;
		}

	}
	INT1 = getBitAddr(0xB3/*P3.3*/);

	if (getBitAddr(ET1)) // 타이머 인터럽트 1
	{
		if (getBitAddr(TF1))
		{
			intData[3] = 2;
			clearBitAddr(TF1);
		}
	}

	// 활성화할 Interrupt가 있는지 확인
	res = getInterruptPriorityRun();


	if (res != -1) // 작동할 Interrput가 있는 경우
	{
		// Stack에 현재 PC값 추가 후
		stackOperationPC(PC % 0x100, 0);
		stackOperationPC(PC / 0x100, 0);

		// 해당 Interrupt 실행 중 표기한다.
		intData[res] = 1;

		// Interrupt 위치로 PC 이동
		return INTERRUPT_PC[res];
	}else{ // 작동할 Interrput가 없는 경우
		// 다음 PC값 실행
		return PC;
	}
}


/* putParity() 함수
* 
* 기능 : ACC 레지스터를 기준으로 parity bit를 활성화한다.
* 입출력 변수 없음
*/
void putParity()
{
	// ACC값을 저장하는 변수 선언 및 초기화
	unsigned char accumulator = chip.internal_RAM[ACC];

	// ACC의 2진수값의 1의 갯수를 저장하는 변수 선언 및 초기화
	unsigned char count = 0;

	// unsigned char의 범위 내에서 1의 갯수를 센다.
	for (unsigned char i = 0; i < 8; i++)
	{
		// 해당 위치가 홀수라면, count 변수에 1을 더한다.
		count += accumulator % 2;

		// ACC값을 다음 위치로 설정한다.
		accumulator /= 2;
	}

	// ACC의 2진수값의 1의 갯수가 홀수라면 Parity를 1로, 아니라면 Parity를 0으로 설정한다.
	if (count % 2)
	{
		setBitAddr(P);
	}else{
		clearBitAddr(P);
	}

	return;
}


/* movFunc() 함수
* 
* 기능 : 각종 MOV 함수를 실행한다.
* 입력 변수 : destination, source, isDat(상수 여부)
* 출력 변수 없음
*/
void movFunc(short dest, short src, char isDat)
{
	// isDat = 0(src(주소) -> dest), isDat = 1(src(데이터) -> dest)

	// 상수 입력인지 확인
	if (isDat == 0)
	{
		chip.internal_RAM[dest] = chip.internal_RAM[src];

	}else if (isDat == 1){
		chip.internal_RAM[dest] = src;
	}

	// Port에 출력한 경우, 해당 Port과 Latch를 연동한다.
	if (dest == 0x80 || dest == 0x90 || dest == 0xA0 || dest == 0xB0)
	{
		syncLatch((dest - 0x80) / 16);
	}

	return;
}


/* addFunc() 함수
*
* 기능 : 각종 ADD, ADDC 함수를 실행한다.
* 입력 변수 : source, isDat(상수 여부), isCarry(Carry 연산 여부)
* 출력 변수 없음
*/
void addFunc(short src, char isDat, char isCarry)
{
	// 기존값 저장(PSW 조정을 위함)
	unsigned char prevDat = chip.internal_RAM[0xE0];
	
	// 상수 연산인 경우
	if (isDat)
	{
		// ACC에 source와 Carry를 더하는 경우 Carry를 더한다.
		chip.internal_RAM[0xE0] += src + (isCarry && getBitAddr(C));

		// OV Flag 관련
		if (prevDat <= 0x7F && chip.internal_RAM[0xE0] > 0x7F)
		{
			setBitAddr(OV);
		}

		// AC Flag 관련
		// 두 데이터가 기본적인 차이가 있거나, ADDC이며 Carry가 1이고 기존값과 신규값이 같으며 src가 0이 아닌 경우
		if (((prevDat & 0x0F) > (chip.internal_RAM[0xE0] & 0x0F)) || (isCarry && getBitAddr(C) && src && (prevDat == chip.internal_RAM[0xE0])))
		{
			setBitAddr(AC);
		}

		// CY Flag 관련
		if ((chip.internal_RAM[0xE0] < prevDat && !isCarry) || (chip.internal_RAM[0xE0] < prevDat && isCarry && src))
		{
			setBitAddr(C);
		}
	}else{
		// ACC에 source와 Carry를 더하는 경우 Carry를 더한다.
		chip.internal_RAM[0xE0] += chip.internal_RAM[src] + (isCarry && getBitAddr(C));

		// OV Flag 관련
		if (prevDat <= 0x7F && chip.internal_RAM[0xE0] > 0x7F)
		{
			setBitAddr(OV);
		}

		// AC Flag 관련
		if (((prevDat & 0x0F) > (chip.internal_RAM[0xE0] & 0x0F)) || (isCarry && getBitAddr(C) && src && (prevDat == chip.internal_RAM[0xE0])))
		{
			setBitAddr(AC);
		}

		// CY Flag 관련
		if ((chip.internal_RAM[0xE0] < prevDat && !isCarry) || (chip.internal_RAM[0xE0] < prevDat && isCarry && chip.internal_RAM[src]))
		{
			setBitAddr(C);
		}

	}

	// Parity는 함수 외부에서 작업
	return;
}


/* subbFunc() 함수
*
* 기능 : 각종 SUBB 함수를 실행한다.
* 입력 변수 : source, isDat(상수 여부)
* 출력 변수 없음
*/
void subbFunc(short src, char isDat)
{
	// 기존값 저장(PSW 조정을 위함)
	unsigned char prevDat = chip.internal_RAM[0xE0];

	if (isDat)
	{
		// ACC에서 source와 Carry의 합을 뺀다.
		chip.internal_RAM[0xE0] -= (src + getBitAddr(C));

		// OV Flag 관련
		if (prevDat > 0x7F && chip.internal_RAM[0xE0] <= 0x7F)
		{
			setBitAddr(OV);
		}

		// AC Flag 관련
		if ((prevDat & 0x0F) < (chip.internal_RAM[0xE0] & 0x0F))
		{
			setBitAddr(AC);
		}

		// CY Flag 관련
		if (chip.internal_RAM[0xE0] >= prevDat && src)
		{
			setBitAddr(C);
		}

	}else{
		// ACC에서 source와 Carry의 합을 뺀다.
		chip.internal_RAM[0xE0] -= (chip.internal_RAM[src] + getBitAddr(C));

		// OV Flag 관련
		if (prevDat > 0x7F && chip.internal_RAM[0xE0] <= 0x7F)
		{
			setBitAddr(OV);
		}

		// AC Flag 관련
		if ((prevDat & 0x0F) < (chip.internal_RAM[0xE0] & 0x0F))
		{
			setBitAddr(AC);
		}

		// CY Flag 관련
		if (chip.internal_RAM[0xE0] >= prevDat && chip.internal_RAM[src])
		{
			setBitAddr(C);
		}
	}

	// Parity는 함수 외부에서 작업
	return;
}


/* mulAndiv() 함수
*
* 기능 : MUL, DIV 함수를 실행한다.
* 입력 변수 : isDiv(나눗셈 여부)
* 출력 변수 없음
*/
void mulAndDiv(char isDiv)
{
	// 각각 A와 B에 출력될 값
	short result, rem;

	if (isDiv)
	{
		if (!chip.internal_RAM[0xF0]) { return; } // DIV by 0

		// 몫과 나머지 계산
		result = chip.internal_RAM[0xE0] / chip.internal_RAM[0xF0];
		rem = chip.internal_RAM[0xE0] % chip.internal_RAM[0xF0];
	}else{
		// 곱하기 계산 후, 출력될 값 분리
		result = chip.internal_RAM[0xE0] / chip.internal_RAM[0xF0];
		rem = result / 256;
		result %= 256;
	}
	// 결과 출력
	chip.internal_RAM[0xE0] = result;
	chip.internal_RAM[0xF0] = rem;

	// DID NOT SET the PSW - 칩 제조사마다 변화가 다름

	return;
}


/* stackOperation() 함수
*
* 기능 : PUSH, POP 함수를 실행한다.
* 입력 변수 : src(사용할 변수의 주소), isPop(POP 명령어 여부)
* 출력 변수 없음
*/
void stackOperation(unsigned char src, int isPop)
{
	if (isPop) // POP이면
	{
		// 주어진 주소에 데이터 출력 후, SP 감소
		chip.internal_RAM[src] = chip.internal_RAM[chip.internal_RAM[SP]];
		chip.internal_RAM[SP]--;

	}else{ // PUSH면
		// SP 증가 후, 주어진 주소의 값을 Stack에 저장
		chip.internal_RAM[SP]++;
		chip.internal_RAM[chip.internal_RAM[SP]] = chip.internal_RAM[src];
	}

	return;
}


/* stackOperationPC() 함수
*
* 기능 : Program Counter에서의 PUSH, POP 함수를 실행한다.
* 입력 변수 : src(사용할 변수의 주소), isPop(POP 명령어 여부)
* 출력 변수 : POP인 경우 Stack에 저장된 주소, PUSH인 경우 0(미사용)
*/
unsigned char stackOperationPC(unsigned char src, int isPop)
{
	if (isPop)
	{
		// 주소값 반환 후, SP 감소
		return chip.internal_RAM[chip.internal_RAM[SP]--];

	}else{
		// SP 증가 후, 주어진 주소의 값을 Stack에 저장
		chip.internal_RAM[SP]++;
		chip.internal_RAM[chip.internal_RAM[SP]] = src;
	}

	return 0;
}


/* setBitAddr() 함수
*
* 기능 : SETB 명령어를 수행한다.
* 입력 변수 : location(수정할 bit주소)
* 출력 변수 없음
*/
void setBitAddr(unsigned char location)
{
	if (location <= 0x7F) // 20~2F(bit addressable ram)의 주소인 경우
	{
		chip.internal_RAM[0x20 + location / 8] = chip.internal_RAM[0x20 + location / 8] | (unsigned char)pow(2, location % 8);

	}else if (location <= 0xBF){ // Port0~3, ... 등의 0x?8~0X?F 주소가 존재하는 경우
		if (location % 16 > 0 && location % 16 < 8) // P0~P3
		{
			chip.internal_RAM[(location / 16) * 16] = chip.internal_RAM[(location / 16) * 16] | (unsigned char)pow(2, location % 8); // Pin에 작성
			chip.latch[(location - 0x80) / 16] = chip.latch[(location - 0x80) / 16] | (unsigned char)pow(2, location % 8); // Latch에 작성

		}else{ // TC0N(0x88), SCON(0x98), IE(0xA8), IP(0xB8)
			chip.internal_RAM[(location / 16) * 16 + 0x08] = chip.internal_RAM[(location / 16) * 16 + 0x08] | (unsigned char)pow(2, location % 8);
		}

	}else if (location <= 0xC7 || (location >= 0xD0 && location <= 0xD7) || (location >= 0xE0 && location <= 0xE7) || (location >= 0xF0 && location <= 0xF7)){ // 나머지(0x?8~0X?F 주소가 존재하지 않는 경우)
		chip.internal_RAM[(location / 16) * 16] = chip.internal_RAM[(location / 16) * 16] | (unsigned char)pow(2, location % 8);
	}

	return;
}


/* clearBitAddr() 함수
*
* 기능 : CLR A를 제외한 CLR 명령어를 수행한다.
* 입력 변수 : location(수정할 bit주소)
* 출력 변수 없음
*/
void clearBitAddr(unsigned char location)
{
	if (location <= 0x7F) // 20~2F(bit addressable ram)의 주소인 경우
	{
		chip.internal_RAM[0x20 + location / 8] = chip.internal_RAM[0x20 + location / 8] & (255 - (unsigned char)pow(2, location % 8));

	}else if (location <= 0xBF){ // Port0~3, ... 등의 0x?8~0X?F 주소가 존재하는 경우
		if (location % 16 > 0 && location % 16 < 8) // P0~P3
		{
			chip.internal_RAM[(location / 16) * 16] = chip.internal_RAM[(location / 16) * 16] & (255 - (unsigned char)pow(2, location % 8)); // Pin에 작성
			chip.latch[(location - 0x80) / 16] = chip.latch[(location - 0x80) / 16] & (0xFF - (unsigned char)pow(2, location % 8));  // Latch에 작성

		}else{ // TC0N(0x88), SCON(0x98), IE(0xA8), IP(0xB8)
			chip.internal_RAM[(location / 16) * 16 + 0x08] = chip.internal_RAM[(location / 16) * 16 + 0x08] & (255 - (unsigned char)pow(2, location % 8));
		}

	}else if (location <= 0xC7 || (location >= 0xD0 && location <= 0xD7) || (location >= 0xE0 && location <= 0xE7) || (location >= 0xF0 && location <= 0xF7)){ // 나머지(0x?8~0X?F 주소가 존재하지 않는 경우)
		chip.internal_RAM[(location / 16) * 16] = chip.internal_RAM[(location / 16) * 16] & (255 - (unsigned char)pow(2, location % 8));
	}
	return;
}


/* getBitAddr() 함수
*
* 기능 : 주어진 Bit Address의 값을 가져온다.
* 입력 변수 : location(읽어들일 bit주소)
* 출력 변수 : 해당 bit주소에 저장된 값
*/
char getBitAddr(unsigned char location)
{
	if (location <= 0x7F) // 20~2F(bit addressable ram)의 주소인 경우
	{
		// 해당 Byte Address의 해당 Bit Address의 위치가 1인 경우, 1을 반환하도록 함.
		return !(!(chip.internal_RAM[0x20 + location / 8] & (unsigned char)pow(2, location % 8)));

	}else if (location <= 0xBF){ // Port0~3, ... 등의 0x?8~0X?F 주소가 존재하는 경우
		if (location % 16 > 0 && location % 16 < 8) // P0~P3
		{
			// 해당 Byte Address의 해당 Bit Address의 위치가 1인 경우, 1을 반환하도록 함.
			return !(!(chip.internal_RAM[(location / 16) * 16] & (unsigned char)pow(2, location % 8)));

		}else{ // TCON(0x88), SCON(0x98), IE(0xA8), IP(0xB8)
			// 해당 Byte Address의 해당 Bit Address의 위치가 1인 경우, 1을 반환하도록 함.
			return !(!(chip.internal_RAM[(location / 16) * 16 + 0x08] & (unsigned char)pow(2, location % 8)));
		}

	}else if (location <= 0xC7 || (location >= 0xD0 && location <= 0xD7) || (location >= 0xE0 && location <= 0xE7) || (location >= 0xF0 && location <= 0xF7)){ // 나머지(0x?8~0X?F 주소가 존재하지 않는 경우)
		// 해당 Byte Address의 해당 Bit Address의 위치가 1인 경우, 1을 반환하도록 함.
		return !(!(chip.internal_RAM[(location / 16) * 16] & (unsigned char)pow(2, location % 8)));
	}

	return 0; // 없는 주소의 경우
}


/* orOperation 함수
*
* 기능 : 각종 ORL 함수를 실행한다.
* 입력 변수 : destination, source, isDat(상수 여부), isBit(bit 주소 여부, -1인 경우 반대 bit값)
* 출력 변수 없음
*/
void orOperation(unsigned char dest, unsigned char src, char isData, char isBit) // isBit : 0(byte), 1(bit), -1(bit transpose)
{
	if (isBit) // Bit 주소의 경우
	{
		if (getBitAddr(C) || getBitAddr(src) ^ (isBit - 1)) // isBit을 이용해, 기존값 반전이 가능하도록 함.
		{
			setBitAddr(C);
		}else{
			clearBitAddr(C);
		}

	}else{ // Byte 주소의 경우
		if (isData == 1) // 데이터 입력인 경우
		{
			chip.internal_RAM[dest] |= src;

		}else{ // 주소 입력인 경우
			if (src == 0x80 || src == 0x90 || src == 0xA0 || src == 0xB0) // Port 주소인 경우
			{
				// 이 경우, Read-Modify-Write이므로, Latch의 값을 반전한다.
				chip.internal_RAM[dest] |= chip.latch[(src - 0x80) / 16];

			}else{ // Port 주소가 아닌 경우
				chip.internal_RAM[dest] |= chip.internal_RAM[src];
			}
		}

		// Port 주소인 경우, 해당 Port의 값을 Latch에 연동한다.
		if (dest == 0x80 || dest == 0x90 || dest == 0xA0 || dest == 0xB0)
		{
			syncLatch((dest - 0x80) / 16);
		}
	}

	return;
}


/* andOperation 함수
*
* 기능 : 각종 ANL 함수를 실행한다.
* 입력 변수 : destination, source, isDat(상수 여부), isBit(bit 주소 여부, -1인 경우 반대 bit값)
* 출력 변수 없음
*/
void andOperation(unsigned char dest, unsigned char src, char isData, char isBit) // isBit : 0(byte), 1(bit), -1(bit transpose)
{
	if (isBit) // Bit 주소의 경우
	{
		if (getBitAddr(C) && getBitAddr(src) ^ (isBit - 1)) // isBit을 이용해, 기존값 반전이 가능하도록 함.
		{
			setBitAddr(C);
		}else{
			clearBitAddr(C);
		}

	}else{ // Byte 주소의 경우
		if (isData == 1) // 데이터 입력인 경우
		{
			chip.internal_RAM[dest] &= src;

		}else{ // 주소 입력인 경우
			if (src == 0x80 || src == 0x90 || src == 0xA0 || src == 0xB0) // Port 주소인 경우
			{
				// 이 경우, Read-Modify-Write이므로, Latch의 값을 반전한다.
				chip.internal_RAM[dest] &= chip.latch[(src - 0x80) / 16];

			}else{ // Port 주소가 아닌 경우
				chip.internal_RAM[dest] &= chip.internal_RAM[src];
			}
		}

		// Port 주소인 경우, 해당 Port의 값을 Latch에 연동한다.
		if (dest == 0x80 || dest == 0x90 || dest == 0xA0 || dest == 0xB0)
		{
			syncLatch((dest - 0x80) / 16);
		}
	}

	return;
}


/* xorOperation 함수
*
* 기능 : 각종 XRL 함수를 실행한다.
* 입력 변수 : destination, source, isDat(상수 여부)
* 출력 변수 없음
*/
void xorOperation(unsigned char dest, unsigned char src, char isData)
{
	if (isData == 1) // 데이터 입력인 경우
	{
		chip.internal_RAM[dest] ^= src;

	}else{ // 주소 입력인 경우
		if (src == 0x80 || src == 0x90 || src == 0xA0 || src == 0xB0) // Port 주소인 경우
		{
			// 이 경우, Read-Modify-Write이므로, Latch의 값을 반전한다.
			chip.internal_RAM[dest] ^= chip.latch[(src - 0x80) / 16];

		}else{ // Port 주소가 아닌 경우
			chip.internal_RAM[dest] ^= chip.internal_RAM[src];
		}
	}

	// Port 주소인 경우, 해당 Port의 값을 Latch에 연동한다.
	if (dest == 0x80 || dest == 0x90 || dest == 0xA0 || dest == 0xB0)
	{
		syncLatch((dest - 0x80) / 16);
	}

	return;
}


/* swapOperation 함수
*
* 기능 : 각종 XCH 함수를 실행한다.
* 입력 변수 : destination, source
* 출력 변수 없음
*/
void swapOperation(unsigned char dest, unsigned char src)
{
	unsigned char tmp = chip.internal_RAM[dest];
	chip.internal_RAM[dest] = chip.internal_RAM[src];
	chip.internal_RAM[src] = tmp;

	return;
}


/* DAOperation 함수
*
* 기능 : DA 함수를 실행한다.
* 입출력 변수 없음
*/
void DAOperation()
{
	unsigned char upper, under; // 각각 ACC의 상위 4bit과 하위 4bit
	upper = chip.internal_RAM[ACC] & 0xF0;
	upper /= 16;
	under = chip.internal_RAM[ACC] & 0x0F;

	// 하위 4bit값이 10 이상이거나, AC값이 1인 경우
	if (under >= 10 || getBitAddr(AC))
	{
		// 상위 4bit값에 1을 더하며, 하위 4bit값에 6을 더한다.
		upper++;
		under += 6;
	}

	// 상위 4bit값이 10 이상이거나, Carry값이 1인 경우
	if (upper >= 10 || getBitAddr(C))
	{
		// Carry값을 1로 설정하며, 상위 4bit값에 6을 더한다.
		setBitAddr(C);
		upper += 6;
	}

	// 상, 하위 4bit값을 16진수로 1의 자릿수만 남긴다.
	upper %= 16;
	under %= 16;

	// 결과를 ACC에 저장한다.
	chip.internal_RAM[ACC] = upper * 16 + under;

	return;
}


/* printChip() 함수
*
* 기능 : 현재 Chip의 RAM값을 출력한다.
* 입력 변수 : cycle, PC
* 출력 변수 없음
*/
void printChip(unsigned long long int cycle, int programCounter)
{
	// Cycle, PC값 출력
	printf("Current) %lld Cycle, PC : 0x%04X\n\n<MEMORY MAP>\n   ", cycle, programCounter);

	// Index 출력
	for (int i = 0; i < 16; i++)
	{
		printf(" %X ", i);
	}
	printf("\n");

	// 모든 RAM의 값 출력
	for (int i = 0; i < 16; i++)
	{
		// Index 출력
		if (i * 16 == 0)
		{
			printf(" 0 ");
		}else{
			printf("%2.X ", i * 16);
		}

		// RAM의 값 출력
		for (int j = 0; j < 16; j++)
		{
			if (chip.internal_RAM[i * 16 + j] == 0)
			{
				printf(" 0 ");
			}else{
				printf("%2.X ", chip.internal_RAM[i * 16 + j]);
			}
		}

		// 개행
		printf("\n");
	}

	// 구분선 출력
	printf("--------------------------------------------------\n");

	// PSW와 TCON의 값 BIT단위로 출력
	printf("PSW : %d %d %d %d %d %d %d %d\n", chip.internal_RAM[PSW]/0x80 % 0x02, chip.internal_RAM[PSW] / 0x40 % 0x02, chip.internal_RAM[PSW] / 0x20 % 0x02,
		chip.internal_RAM[PSW] / 0x10 % 0x02, chip.internal_RAM[PSW] / 0x08 % 0x02, chip.internal_RAM[PSW] / 0x04 % 0x02, chip.internal_RAM[PSW] / 0x02 % 0x02,
		chip.internal_RAM[PSW] / 0x01 % 0x02);
	printf("TCON : %d %d %d %d %d %d %d %d\n", chip.internal_RAM[TCON] / 0x80 % 0x02, chip.internal_RAM[TCON] / 0x40 % 0x02, chip.internal_RAM[TCON] / 0x20 % 0x02,
		chip.internal_RAM[TCON] / 0x10 % 0x02, chip.internal_RAM[TCON] / 0x08 % 0x02, chip.internal_RAM[TCON] / 0x04 % 0x02, chip.internal_RAM[TCON] / 0x02 % 0x02,
		chip.internal_RAM[TCON] / 0x01 % 0x02);

	// 구분선과 안내문 출력
	printf("--------------------------------------------------\n<LATCH DATA>\n    7 6 5 4 3 2 1 0\n");

	for (char i = 0; i < 4; i++)
	{
		printf("P%d: %d %d %d %d %d %d %d %d\n", i, chip.latch[i] / 0x80 % 0x02, chip.latch[i] / 0x40 % 0x02, chip.latch[i] / 0x20 % 0x02,
		chip.latch[i] / 0x10 % 0x02, chip.latch[i] / 0x08 % 0x02, chip.latch[i] / 0x04 % 0x02, chip.latch[i] / 0x02 % 0x02,
		chip.latch[i] / 0x01 % 0x02);
	}

	// 구분선 출력
	printf("--------------------------------------------------\n");

	return;
}


/* asciiToHEX() 함수
*
* 기능 : ASCII 입력값을 HEX값으로 바꾼다.
* 입력 변수 : 기존값 orig
* 출력 변수 : HEX 변환값
*/
unsigned char asciiToHEX(unsigned char orig)
{
	// 0~9인 경우
	char number = orig - '0';
	if (number < 10)
	{
		return number;
	}

	// A~F인 경우(대문자)
	number = orig - 'A';
	if (number < 6)
	{
		return number + 10;
	}

	// a~f인 경우(소문자)
	number = number - 'a';
	if (number < 6)
	{
		return number + 10;
	}

	// 버그이므로, 0xA5 출력
	return 0xA5;
}


/* fileReader() 함수
*
* 기능 : 주어진 HEX 파일을 읽어들여 ROM 변수에 저장한다.
* 입력 변수 : fileName(입력 파일의 위치와 이름)
* 출력 변수 : 마지막 명령어의 Program Counter의 위치
*/
int fileReader(char* fileName)
{
	// 파일을 연다
	FILE* hexFile = fopen(fileName, "r");
	if (hexFile == NULL)
	{
		// 파일이 없는 경우, 파일이 없다고 출력한다.
		printf("File Not Found\n");
		exit(1);
	}

	// 데이터 임시 저장
	unsigned char tmp_Data[300], parity=0;
	int curr_PC = 0, tmp_PC = 0, prev_PC = 0, tmp_REM = 0, prev_REM = 0, isEnd = 0, lineNo = 0;
	while (!isEnd) // 종료 표시가 있을때까지 반복
	{
		// Parity를 초기화하고
		parity = 0;

		// 데이터 한줄을 읽어들인다.
		fscanf(hexFile, "%s", tmp_Data);

		// 빈 파일인 경우
		if (tmp_Data[0] == 204)
		{
			// 파일 데이터 오류를 출력한다
			printf("File Data Error!\n");
			exit(1);
		}

		// 이전값 저장 후
		prev_PC = tmp_PC - prev_REM;
		prev_REM = tmp_REM;

		// 해당 줄의 명령어 개수를 읽고
		tmp_REM = asciiToHEX(tmp_Data[1]) * 16 + asciiToHEX(tmp_Data[2]); //CC
		parity += tmp_REM;

		// 해당 줄의 Program Counter 위치를 읽고
		tmp_PC = asciiToHEX(tmp_Data[3]) * pow(16, 3) + asciiToHEX(tmp_Data[4]) * pow(16, 2) + asciiToHEX(tmp_Data[5]) * pow(16, 1) + asciiToHEX(tmp_Data[6]); //AAAA
		parity += tmp_PC / 0x100 + tmp_PC % 0x100;

		// 해당 줄의 종료 여부를 읽고
		isEnd = asciiToHEX(tmp_Data[8]);
		parity += isEnd;
		
		// 해당 줄의 명령 코드를 모두 읽어들여, ROM 변수에 저장한다.
		for (unsigned char i = 0; i < tmp_REM; i++)
		{
			ROM[tmp_PC] = asciiToHEX(tmp_Data[i * 2 + 9]) * 16 + asciiToHEX(tmp_Data[i * 2 + 10]);
			parity += ROM[tmp_PC];
			tmp_PC++;
		}

		// 마지막으로, 파일의 Parity 확인을 한다.
		if ((unsigned char)(parity + (asciiToHEX(tmp_Data[tmp_REM * 2 + 9]) * 16 + asciiToHEX(tmp_Data[tmp_REM * 2 + 10]))) != 0x00)
		{
			if (!isEnd) // Parity 오류 발견 시
			{
				// Parity 오류를 발견한 줄의 순서를 출력한다.
				printf("Parity Error at Line #%d!\n", lineNo + 1);
				exit(1);
			}
		}
		lineNo++;
	}

	// 파일을 닫는다.
	fclose(hexFile);

	// 파일 읽기를 성공했다고 출력한다.
	printf("File Read Finished with No Error\n");

	// 파일의 마지막 Program Counter값을 반환한다.
	return prev_PC + prev_REM;
}


/* inputDat() 함수
*
* 기능 : Port Data Input 등의 입력 값이 필요한 경우 이를 수행할 수 있도록 한다.
* 입력 변수 없음
* 출력 변수 없음
*/
void inputDat()
{
	system("PAUSE");
	// getPortValue();

	return;
}


/* programRunner() 함수
* 
* 기능 : 주어진 code의 명령을 실행한다.
* 입력 변수 : code(명령 코드), data1(추가데이터1), data2(추가데이터2), PC(현재의 Program Counter 위치), isDebugMode(디버그 모드 여부)
* 출력 변수 : 다음에 실행할 Program Counter의 위치
*/
int programRunner(unsigned char code, unsigned char data1, unsigned char data2, int PC, char isDebugMode)
{
	// 사용할 Register의 위치 가져오기
	unsigned char PSWROM = chip.internal_RAM[0xD0] & 0x18;
	PSWROM /= 8;

	// 임시로 저장할 데이터
	unsigned short PC_tmp = 0;
	char tmpValue = 0;

	// 다음 명령어 출력
	printf("Next (at Current Status) : ");

	// 명령어별로 실행
	switch (code)
	{
	case 0x00: // NOP
		printf("NOP\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return PC;
	case 0x01: // AJMP
		printf("AJMP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x02: // LJMP
		printf("LJMP %05XH\n", data1 * 0x100 + data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return data1 * 0x100 + data2;
	case 0x03: // RR A
		printf("RR A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[ACC] = chip.internal_RAM[ACC] >> 1;
		return PC;
	case 0x04: // INC A
		printf("INC A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[ACC]++;
		return PC;
	case 0x05: // INC DIR
		printf("INC 0x%X\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[data1]++;
		return PC;
	case 0x06: // INC @R0
		printf("INC @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[chip.internal_RAM[8 * PSWROM]]++;
		return PC;
	case 0x07: // INC @R1
		printf("INC @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[chip.internal_RAM[8 * PSWROM + 1]]++;
		return PC;
	case 0x08: // INC R0
		printf("INC R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM]++;
		return PC;
	case 0x09: // INC R1
		printf("INC R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 1]++;
		return PC;
	case 0x0A: // INC R2
		printf("INC R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 2]++;
		return PC;
	case 0x0B: // INC R3
		printf("INC R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 3]++;
		return PC;
	case 0x0C: // INC R4
		printf("INC R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 4]++;
		return PC;
	case 0x0D: // INC R5
		printf("INC R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 5]++;
		return PC;
	case 0x0E: // INC R6
		printf("INC R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 6]++;
		return PC;
	case 0x0F: // INC R7
		printf("INC R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 7]++;
		return PC;

		// 0x10-Ox1F

	case 0x10: // JBC
		printf("JBC %03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (getBitAddr(data1))
		{
			clearBitAddr(data1);
			return PC + (char)data2;
		}

		return PC;
	case 0x11: // ACALL
		printf("ACALL %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperationPC(PC % 0x100, 0);
		stackOperationPC(PC / 0x100, 0);
		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x12: // LCALL
		printf("LCALL %05XH\n", data1 * 0x100 + data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperationPC(PC % 0x100, 0);
		stackOperationPC(PC / 0x100, 0);
		return data1 * 0x100 + data2;
	case 0x13: // RRC A
		printf("RRC A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		tmpValue = chip.internal_RAM[ACC] & 0x01;
		chip.internal_RAM[ACC] = chip.internal_RAM[ACC] >> 1;
		if (getBitAddr(C))
		{
			chip.internal_RAM[ACC] += 0x80;
		}
		if (tmpValue)
		{
			setBitAddr(C);
		}
		else {
			clearBitAddr(C);
		}
		return PC;
	case 0x14: // DEC A
		printf("DEC A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[ACC]--;
		putParity(); // NO C, AC operation
		return PC;
	case 0x15: // DEC DIR
		printf("DEC %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[data1]--;
		return PC;
	case 0x16: // DEC @R0
		printf("DEC @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[chip.internal_RAM[8 * PSWROM]]--;
		return PC;
	case 0x17: // DEC @R1
		printf("DEC @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[chip.internal_RAM[8 * PSWROM + 1]]--;
		return PC;
	case 0x18: // DEC R0
		printf("DEC R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM]--;
		return PC;
	case 0x19: // DEC R1
		printf("DEC R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 1]--;
		return PC;
	case 0x1A: // DEC R2
		printf("DEC R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 2]--;
		return PC;
	case 0x1B: // DEC R3
		printf("DEC R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 3]--;
		return PC;
	case 0x1C: // DEC R4
		printf("DEC R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 4]--;
		return PC;
	case 0x1D: // DEC R5
		printf("DEC R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 5]--;
		return PC;
	case 0x1E: // DEC R6
		printf("DEC R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 6]--;
		return PC;
	case 0x1F: // DEC R7
		printf("DEC R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 7]--;
		return PC;

		// 0x20-0x2F

	case 0x20: // JB
		printf("JB %03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (getBitAddr(data1))
			return PC + (char)data2;

		return PC;
	case 0x21: // AJMP
		printf("AJMP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x22: // RET
		printf("RET\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		PC_tmp = stackOperationPC(PC % 0x100, 1);
		PC_tmp *= 0x100;
		PC_tmp += stackOperationPC(PC / 0x100, 1);
		return PC_tmp;
	case 0x23: // RL A
		printf("RL A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[ACC] = chip.internal_RAM[ACC] << 1;
		return PC;
	case 0x24: // ADD A, data
		printf("ADD A, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(data1, 1, 0);
		return PC;
	case 0x25: // ADD DIR
		printf("ADD A, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(data1, 0, 0);
		return PC;
	case 0x26: // ADD @R0
		printf("ADD A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(chip.internal_RAM[8 * PSWROM], 0, 0);
		return PC;
	case 0x27: // ADD @R1
		printf("ADD A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(chip.internal_RAM[8 * PSWROM + 1], 0, 0);
		return PC;
	case 0x28: // ADD R0
		printf("ADD A, R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM, 0, 0);
		return PC;
	case 0x29: // ADD R1
		printf("ADD A, R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 1, 0, 0);
		return PC;
	case 0x2A: // ADD R2
		printf("ADD A, R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 2, 0, 0);
		return PC;
	case 0x2B: // ADD R3
		printf("ADD A, R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 3, 0, 0);
		return PC;
	case 0x2C: // ADD R4
		printf("ADD A, R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 4, 0, 0);
		return PC;
	case 0x2D: // ADD R5
		printf("ADD A, R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 5, 0, 0);
		return PC;
	case 0x2E: // ADD R6
		printf("ADD A, R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 6, 0, 0);
		return PC;
	case 0x2F: // ADD R7
		printf("ADD A, R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 7, 0, 0);
		return PC;

		// 0x30-Ox3F

	case 0x30: // JNB
		printf("JNB %03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (!getBitAddr(data1))
			return PC + (char)data2;

		return PC;
	case 0x31: // ACALL
		printf("ACALL %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(PC % 0x100, 0);
		stackOperation(PC / 0x100, 0);
		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x32: // RETI
		printf("RETI\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		clearInterrupt();
		PC_tmp = stackOperationPC(PC % 0x100, 1);
		PC_tmp *= 0x100;
		PC_tmp += stackOperationPC(PC / 0x100, 1);
		return PC_tmp;
	case 0x33: // RLC A
		printf("RLC A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		tmpValue = chip.internal_RAM[ACC] & 0x80;
		chip.internal_RAM[ACC] = chip.internal_RAM[ACC] << 1;
		if (getBitAddr(C))
		{
			chip.internal_RAM[ACC] += 0x01;
		}
		if (tmpValue)
		{
			setBitAddr(C);
		}
		else {
			clearBitAddr(C);
		}
		return PC;

	case 0x34: // ADDC A, data
		printf("ADDC A, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(data1, 1, 1);
		return PC;
	case 0x35: // ADDC DIR
		printf("ADDC A, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(data1, 0, 1);
		return PC;
	case 0x36: // ADDC @R0
		printf("ADDC A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(chip.internal_RAM[8 * PSWROM], 0, 1);
		return PC;
	case 0x37: // ADDC @R1
		printf("ADDC A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(chip.internal_RAM[8 * PSWROM + 1], 0, 1);
		return PC;
	case 0x38: // ADDC R0
		printf("ADDC A, R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM, 0, 1);
		return PC;
	case 0x39: // ADDC R1
		printf("ADDC A, R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 1, 0, 1);
		return PC;
	case 0x3A: // ADDC R2
		printf("ADDC A, R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 2, 0, 1);
		return PC;
	case 0x3B: // ADDC R3
		printf("ADDC A, R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 3, 0, 1);
		return PC;
	case 0x3C: // ADDC R4
		printf("ADDC A, R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 4, 0, 1);
		return PC;
	case 0x3D: // ADDC R5
		printf("ADDC A, R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 5, 0, 1);
		return PC;
	case 0x3E: // ADDC R6
		printf("ADDC A, R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 6, 0, 1);
		return PC;
	case 0x3F: // ADDC R7
		printf("ADDC A, R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		addFunc(8 * PSWROM + 7, 0, 1);
		return PC;

		// 0x40-0x4F
	case 0x40: // JC
		printf("JC %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (getBitAddr(C))
			return PC + (char)data1;

		return PC;
	case 0x41: // AJMP
		printf("AJMP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x42: // ORL dir A
		printf("ORL %03XH, A\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(data1, ACC, 0, 0);
		return PC;
	case 0x43: // ORL dir data
		printf("ORL %03XH, #%03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(data1, data2, 1, 0);
		return PC;
	case 0x44: // ORL A, data
		printf("ORL A, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, data1, 1, 0);
		return PC;
	case 0x45: // ORL A, dir
		printf("ORL A, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, data1, 0, 0);
		return PC;
	case 0x46: // ORL A, @R0
		printf("ORL A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, chip.internal_RAM[8 * PSWROM], 0, 0);
		return PC;
	case 0x47: // ORL A, @R1
		printf("ORL A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, chip.internal_RAM[8 * PSWROM + 1], 0, 0);
		return PC;
	case 0x48: // ORL A, R0
		printf("ORL A, R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, 8 * PSWROM, 0, 0);
		return PC;
	case 0x49: // ORL A, R1
		printf("ORL A, R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, 8 * PSWROM + 1, 0, 0);
		return PC;
	case 0x4A: // ORL A, R2
		printf("ORL A, R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, 8 * PSWROM + 2, 0, 0);
		return PC;
	case 0x4B: // ORL A, R3
		printf("ORL A, R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, 8 * PSWROM + 3, 0, 0);
		return PC;
	case 0x4C: // ORL A, R4
		printf("ORL A, R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, 8 * PSWROM + 4, 0, 0);
		return PC;
	case 0x4D: // ORL A, R5
		printf("ORL A, R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, 8 * PSWROM + 5, 0, 0);
		return PC;
	case 0x4E: // ORL A, R6
		printf("ORL A, R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, 8 * PSWROM + 6, 0, 0);
		return PC;
	case 0x4F: // ORL A, R7
		printf("ORL A, R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(ACC, 8 * PSWROM + 7, 0, 0);
		return PC;

		// 0x50-0x5F
	case 0x50: // JNC
		printf("JNC %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (!getBitAddr(C))
			return PC + (char)data1;

		return PC;
	case 0x51: // ACALL
		printf("ACALL %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(PC % 0x100, 0);
		stackOperation(PC / 0x100, 0);
		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x52: // ANL dir A
		printf("ANL %03XH, A\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(data1, ACC, 0, 0);
		return PC;
	case 0x53: // ANL dir data
		printf("ANL %03XH, #%03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(data1, data2, 1, 0);
		return PC;
	case 0x54: // ANL A, data
		printf("ANL A, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, data1, 1, 0);
		return PC;
	case 0x55: // ANL A, dir
		printf("ANL A, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, data1, 0, 0);
		return PC;
	case 0x56: // ANL A, @R0
		printf("ANL A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, chip.internal_RAM[8 * PSWROM], 0, 0);
		return PC;
	case 0x57: // ANL A, @R1
		printf("ANL A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, chip.internal_RAM[8 * PSWROM + 1], 0, 0);
		return PC;
	case 0x58: // ANL A, R0
		printf("ANL A, R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, 8 * PSWROM, 0, 0);
		return PC;
	case 0x59: // ANL A, R1
		printf("ANL A, R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, 8 * PSWROM + 1, 0, 0);
		return PC;
	case 0x5A: // ANL A, R2
		printf("ANL A, R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, 8 * PSWROM + 2, 0, 0);
		return PC;
	case 0x5B: // ANL A, R3
		printf("ANL A, R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, 8 * PSWROM + 3, 0, 0);
		return PC;
	case 0x5C: // ANL A, R4
		printf("ANL A, R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, 8 * PSWROM + 4, 0, 0);
		return PC;
	case 0x5D: // ANL A, R5
		printf("ANL A, R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, 8 * PSWROM + 5, 0, 0);
		return PC;
	case 0x5E: // ANL A, R6
		printf("ANL A, R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, 8 * PSWROM + 6, 0, 0);
		return PC;
	case 0x5F: // ANL A, R7
		printf("ANL A, R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(ACC, 8 * PSWROM + 7, 0, 0);
		return PC;

		// 0x60-0x6F
	case 0x60: // JZ
		printf("JZ %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (!chip.internal_RAM[ACC])
			return PC + (char)data1;

		return PC;
	case 0x61: // AJMP
		printf("AJMP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x62: // XRL dir A
		printf("XRL %03XH, A\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(data1, ACC, 0);
		return PC;
	case 0x63: // XRL dir data
		printf("XRL %03XH, #%03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(data1, data2, 1);
		return PC;
	case 0x64: // XRL A, data
		printf("XRL A, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, data1, 1);
		return PC;
	case 0x65: // XRL A, dir
		printf("XRL A, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, data1, 0);
		return PC;
	case 0x66: // XRL A, @R0
		printf("XRL A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, chip.internal_RAM[8 * PSWROM], 0);
		return PC;
	case 0x67: // XRL A, @R1
		printf("XRL A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, chip.internal_RAM[8 * PSWROM + 1], 0);
		return PC;
	case 0x68: // XRL A, R0
		printf("XRL A, R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, 8 * PSWROM, 0);
		return PC;
	case 0x69: // XRL A, R1
		printf("XRL A, R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, 8 * PSWROM + 1, 0);
		return PC;
	case 0x6A: // XRL A, R2
		printf("XRL A, R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, 8 * PSWROM + 2, 0);
		return PC;
	case 0x6B: // XRL A, R3
		printf("XRL A, R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, 8 * PSWROM + 3, 0);
		return PC;
	case 0x6C: // XRL A, R4
		printf("XRL A, R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, 8 * PSWROM + 4, 0);
		return PC;
	case 0x6D: // XRL A, R5
		printf("XRL A, R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, 8 * PSWROM + 5, 0);
		return PC;
	case 0x6E: // XRL A, R6
		printf("XRL A, R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, 8 * PSWROM + 6, 0);
		return PC;
	case 0x6F: // XRL A, R7
		printf("XRL A, R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		xorOperation(ACC, 8 * PSWROM + 7, 0);
		return PC;

		// 0x70 - 0x7F
	case 0x70: // JNZ
		printf("JNZ %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[ACC])
			return PC + (char)data1;

		return PC;
	case 0x71: // ACALL
		printf("ACALL %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(PC % 0x100, 0);
		stackOperation(PC / 0x100, 0);
		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x72: // ORL C, bit
		printf("ORL C, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(C, data1, 0, 1);
		return PC;
	case 0x73: // JMP @A+DPTR
		printf("JMP @A+DPTR\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return chip.internal_RAM[DPH] * 0x100 + chip.internal_RAM[DPL] + chip.internal_RAM[ACC] - 1;
	case 0x74: // MOV A, data
		printf("MOV A, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, data1, 1);
		return PC;
	case 0x75: // MOV dir, data
		printf("MOV %03XH, #%03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, data2, 1);
		return PC;
	case 0x76: // MOV @R0, dat
		printf("MOV @R0, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(chip.internal_RAM[8 * PSWROM], data1, 1);
		return PC;
	case 0x77: // MOV @R1, dat
		printf("MOV @R1, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(chip.internal_RAM[8 * PSWROM + 1], data1, 1);
		return PC;
	case 0x78: // MOV R0, dat
		printf("MOV R0, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM, data1, 1);
		return PC;
	case 0x79: // MOV R1, dat
		printf("MOV R1, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 1, data1, 1);
		return PC;
	case 0x7A: // MOV R2, dat
		printf("MOV R2, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 2, data1, 1);
		return PC;
	case 0x7B: // MOV R3, dat
		printf("MOV R3, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 3, data1, 1);
		return PC;
	case 0x7C: // MOV R4, dat
		printf("MOV R4, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 4, data1, 1);
		return PC;
	case 0x7D: // MOV R5, dat
		printf("MOV R5, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 5, data1, 1);
		return PC;
	case 0x7E: // MOV R6, dat
		printf("MOV R6, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 6, data1, 1);
		return PC;
	case 0x7F: // MOV R7, dat
		printf("MOV R7, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 7, data1, 1);
		return PC;

		// 0x80-0x8F
	case 0x80: // SJMP
		printf("SJMP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return PC + (char)data1;

	case 0x81: // AJMP
		printf("AJMP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x82: // ANL C, bit
		printf("ANL C, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(C, data1, 0, 1);
		return PC;
	case 0x83: // MOVC A, @A+PC
		printf("MOVC A, @A+PC\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[ACC] = ROM[PC + chip.internal_RAM[ACC]];
		return PC;
	case 0x84: // DIV
		printf("DIV AB\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		mulAndDiv(1);
		return PC;
	case 0x85: // MOV dir, dir
		printf("MOV %03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, data2, 0);
		return PC;
	case 0x86: // MOV dir, @R0
		printf("MOV %03XH, @R0\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, chip.internal_RAM[8 * PSWROM], 0);
		return PC;
	case 0x87: // MOV dir, @R1
		printf("MOV %03XH, @R1\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, chip.internal_RAM[8 * PSWROM + 1], 0);
		return PC;
	case 0x88: // MOV dir, R0
		printf("MOV %03XH, R0\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, 8 * PSWROM, 0);
		return PC;
	case 0x89: // MOV dir, R1
		printf("MOV %03XH, R1\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, 8 * PSWROM + 1, 0);
		return PC;
	case 0x8A: // MOV dir, R2
		printf("MOV %03XH, R2\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, 8 * PSWROM + 2, 0);
		return PC;
	case 0x8B: // MOV dir, R3
		printf("MOV %03XH, R3\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, 8 * PSWROM + 3, 0);
		return PC;
	case 0x8C: // MOV dir, R4
		printf("MOV %03XH, R4\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, 8 * PSWROM + 4, 0);
		return PC;
	case 0x8D: // MOV dir, R5
		printf("MOV %03XH, R5\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, 8 * PSWROM + 5, 0);
		return PC;
	case 0x8E: // MOV dir, R6
		printf("MOV %03XH, R6\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, 8 * PSWROM + 6, 0);
		return PC;
	case 0x8F: // MOV dir, R7
		printf("MOV %03XH, R7\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, 8 * PSWROM + 7, 0);
		return PC;

		// 0x90-0x9F
	case 0x90: // MOV DPTR, #data
		printf("MOV DPTR, #%03XH\n", data1 * 0x100 + data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(DPH, data1, 1);
		movFunc(DPL, data2, 1);
		return PC;
	case 0x91: // ACALL
		printf("ACALL %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(PC % 0x100, 0);
		stackOperation(PC / 0x100, 0);
		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x92: // MOV bit, C
		printf("MOV %03XH, C\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (getBitAddr(C))
		{
			setBitAddr(data1);
		}
		else {
			clearBitAddr(data1);
		}
		return PC;
	case 0x93: // MOVC A, @A + DPTR
		printf("MOVC A, @A + DPTR\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[ACC] = ROM[chip.internal_RAM[DPH] * 0x100 + chip.internal_RAM[DPL] + chip.internal_RAM[ACC]];
		return PC;
	case 0x94: // SUBB A, data
		printf("SUBB A, #%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(data1, 1);
		return PC;
	case 0x95: // SUBB DIR
		printf("SUBB A, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(data1, 0);
		return PC;
	case 0x96: // SUBB @R0
		printf("SUBB A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(chip.internal_RAM[8 * PSWROM], 0);
		return PC;
	case 0x97: // SUBB @R1
		printf("SUBB A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(chip.internal_RAM[8 * PSWROM + 1], 0);
		return PC;
	case 0x98: // SUBB R0
		printf("SUBB A, R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(8 * PSWROM, 0);
		return PC;
	case 0x99: // SUBB R1
		printf("SUBB A, R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(8 * PSWROM + 1, 0);
		return PC;
	case 0x9A: // SUBB R2
		printf("SUBB A, R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(8 * PSWROM + 2, 0);
		return PC;
	case 0x9B: // SUBB R3
		printf("SUBB A, R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(8 * PSWROM + 3, 0);
		return PC;
	case 0x9C: // SUBB R4
		printf("SUBB A, R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(8 * PSWROM + 4, 0);
		return PC;
	case 0x9D: // SUBB R5
		printf("SUBB A, R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(8 * PSWROM + 5, 0);
		return PC;
	case 0x9E: // SUBB R6
		printf("SUBB A, R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(8 * PSWROM + 6, 0);
		return PC;
	case 0x9F: // SUBB R7
		printf("SUBB A, R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		subbFunc(8 * PSWROM + 7, 0);
		return PC;

		// 0xA0-0xAF
	case 0xA0: // ORL C, ~bit
		printf("ORL C, /%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(C, data1, 0, -1);
		return PC;

	case 0xA1: // AJMP
		printf("AJMP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0xA2: // MOV C, bit
		printf("MOV C, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (getBitAddr(data1))
		{
			setBitAddr(C);
		}
		else {
			clearBitAddr(C);
		}
		return PC;
	case 0xA3: // INC DPTR
		printf("INC DPTR\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		// DPL 1 증가
		chip.internal_RAM[DPL]++;
		if (chip.internal_RAM[DPL] == 0) // DPL Overflow시, DPH 1 증가
			chip.internal_RAM[DPH]++;

		return PC;
	case 0xA4: // MUL
		printf("MUL AB\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		mulAndDiv(0);
		return PC;
	case 0xA5: // Unused
		printf("DB 0A5H\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return PC;
	case 0xA6: // MOV @R0, dir
		printf("MOV @R0, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(chip.internal_RAM[8 * PSWROM], data1, 0);
		return PC;
	case 0xA7: // MOV @R1, dir
		printf("MOV @R1, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(chip.internal_RAM[8 * PSWROM + 1], data1, 0);
		return PC;
	case 0xA8: // MOV R0, dir
		printf("MOV R0, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM, data1, 0);
		return PC;
	case 0xA9: // MOV R1, dir
		printf("MOV R1, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 1, data1, 0);
		return PC;
	case 0xAA: // MOV R2, dir
		printf("MOV R2, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 2, data1, 0);
		return PC;
	case 0xAB: // MOV R3, dir
		printf("MOV R3, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 3, data1, 0);
		return PC;
	case 0xAC: // MOV R4, dir
		printf("MOV R4, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 4, data1, 0);
		return PC;
	case 0xAD: // MOV R5, dir
		printf("MOV R5, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 5, data1, 0);
		return PC;
	case 0xAE: // MOV R6, dir
		printf("MOV R6, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 6, data1, 0);
		return PC;
	case 0xAF: // MOV R7, dir
		printf("MOV R7, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 7, data1, 0);
		return PC;

		//0xB0-0xBF
	case 0xB0: // ANL C, ~bit
		printf("ANL C, /%03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		andOperation(C, data1, 0, -1);
		return PC;
	case 0xB1: // ACALL
		printf("ACALL %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(PC % 0x100, 0);
		stackOperation(PC / 0x100, 0);
		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0xB2: // CPL bit
		printf("CPL %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();


		// 명령 위치가 Pin인 경우를 구분
		if ((data1 >= 0x80 && data1 <= 0x87) || (data1 >= 0x90 && data1 <= 0x97) || (data1 >= 0xA0 && data1 <= 0xA7) || (data1 >= 0xB0 && data1 <= 0xB7))
		{
			// ReadLatch 실행
			if (chip.latch[(data1 - 0x80) / 16] & (unsigned char)pow(2, data1 % 16))
			{
				clearBitAddr(data1);
			}else{
				setBitAddr(data1);
			}

		}else{
			if (getBitAddr(data1))
			{
				clearBitAddr(data1);
			}else{
				setBitAddr(data1);
			}
		}
		return PC;
	case 0xB3: // CPL C
		printf("CPL C\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (getBitAddr(C))
		{
			clearBitAddr(C);
		}else{
			setBitAddr(C);
		}
		return PC;
	case 0xB4: // CJNE A, DATA, label
		printf("CJNE A, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[ACC] != data1)
			return PC + (char)data2;

		return PC;
	case 0xB5: // CJNE A, dir, label
		printf("CJNE A, %03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[ACC] != chip.internal_RAM[data1])
			return PC + (char)data2;

		return PC;
	case 0xB6: // CJNE @R0, DATA, label
		printf("CJNE @R0, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[chip.internal_RAM[8 * PSWROM]] != data1)
			return PC + (char)data2;

		return PC;
	case 0xB7: // CJNE @R1, DATA, label
		printf("CJNE @R1, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[chip.internal_RAM[8 * PSWROM + 1]] != data1)
			return PC + (char)data2;

		return PC;
	case 0xB8: // CJNE R0, DATA, label
		printf("CJNE R0, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM] != data1)
			return PC + (char)data2;

		return PC;
	case 0xB9: // CJNE R1, DATA, label
		printf("CJNE R1, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 1] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBA: // CJNE R2, DATA, label
		printf("CJNE R2, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 2] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBB: // CJNE R3, DATA, label
		printf("CJNE R3, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 3] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBC: // CJNE R4, DATA, label
		printf("CJNE R4, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 4] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBD: // CJNE R5, DATA, label
		printf("CJNE R5, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 5] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBE: // CJNE R6, DATA, label
		printf("CJNE R6, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 6] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBF: // CJNE R7, DATA, label
		printf("CJNE R7, #%03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 7] != data1)
			return PC + (char)data2;

		return PC;

		//0xC0-0xCF
	case 0xC0: // PUSH
		printf("PUSH %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(data1, 0);
		return PC;
	case 0xC1: // AJMP
		printf("AJMP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0xC2: // CLR bit
		printf("CLR %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		clearBitAddr(data1);
		return PC;
	case 0xC3: // CLR C
		printf("CLR C\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		clearBitAddr(C);
		return PC;
	case 0xC4: // SWAP A
		printf("SWAP A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[ACC] = (chip.internal_RAM[ACC] % 16) * 16 + chip.internal_RAM[ACC] / 16;
		return PC;
	case 0xC5: // XCH dir
		printf("XCH %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, data1);
		return PC;
	case 0xC6: // XCH @R0
		printf("XCH @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, chip.internal_RAM[8 * PSWROM]);
		return PC;
	case 0xC7: // XCH @R1
		printf("XCH @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, chip.internal_RAM[8 * PSWROM + 1]);
		return PC;
	case 0xC8: // XCH R0
		printf("XCH R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, 8 * PSWROM);
		return PC;
	case 0xC9: // XCH R1
		printf("XCH R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, 8 * PSWROM + 1);
		return PC;
	case 0xCA: // XCH R2
		printf("XCH R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, 8 * PSWROM + 2);
		return PC;
	case 0xCB: // XCH R3
		printf("XCH R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, 8 * PSWROM + 3);
		return PC;
	case 0xCC: // XCH R4
		printf("XCH R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, 8 * PSWROM + 4);
		return PC;
	case 0xCD: // XCH R5
		printf("XCH R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, 8 * PSWROM + 5);
		return PC;
	case 0xCE: // XCH R6
		printf("XCH R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, 8 * PSWROM + 6);
		return PC;
	case 0xCF: // XCH R7
		printf("XCH R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(ACC, 8 * PSWROM + 7);
		return PC;

		//0xD0-0xDF
	case 0xD0: // POP
		printf("POP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(data1, 1);
		return PC;
	case 0xD1: // ACALL
		printf("ACALL %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(PC % 0x100, 0);
		stackOperation(PC / 0x100, 0);
		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0xD2: // SETB bit
		printf("SETB %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		setBitAddr(data1);
		return PC;
	case 0xD3: // SETB C
		printf("SETB C\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		setBitAddr(C);
		return PC;
	case 0xD4: // DA A
		printf("DA A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		DAOperation();
		return PC;
	case 0xD5: // DJNZ dir, label
		printf("DJNZ %03XH, %03XH\n", data1, data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[data1]--;
		if (chip.internal_RAM[data1] != 0)
		{
			return PC + (char)data2;
		}

		return PC;
	case 0xD6: // XCHD A, @R0
		printf("XCHD A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		tmpValue = chip.internal_RAM[chip.internal_RAM[8 * PSWROM]] & 0x0F;
		chip.internal_RAM[chip.internal_RAM[8 * PSWROM]] = (chip.internal_RAM[chip.internal_RAM[8 * PSWROM]] & 0xF0) + (chip.internal_RAM[ACC] & 0x0F);
		chip.internal_RAM[ACC] = (chip.internal_RAM[ACC] & 0xF0) + tmpValue;
		return PC;
	case 0xD7: // XCHD A, @R1
		printf("XCHD A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		tmpValue = chip.internal_RAM[chip.internal_RAM[8 * PSWROM + 1]] & 0x0F;
		chip.internal_RAM[chip.internal_RAM[8 * PSWROM + 1]] = (chip.internal_RAM[chip.internal_RAM[8 * PSWROM + 1]] & 0xF0) + (chip.internal_RAM[ACC] & 0x0F);
		chip.internal_RAM[ACC] = (chip.internal_RAM[ACC] & 0xF0) + tmpValue;
		return PC;
	case 0xD8: // DJNZ R0, label
		printf("DJNZ R0, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM]--;
		if (chip.internal_RAM[8 * PSWROM] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xD9: // DJNZ R1, label
		printf("DJNZ R1, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 1]--;
		if (chip.internal_RAM[8 * PSWROM + 1] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDA: // DJNZ R2, label
		printf("DJNZ R2, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 2]--;
		if (chip.internal_RAM[8 * PSWROM + 2] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDB: // DJNZ R3, label
		printf("DJNZ R3, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 3]--;
		if (chip.internal_RAM[8 * PSWROM + 3] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDC: // DJNZ R4, label
		printf("DJNZ R4, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 4]--;
		if (chip.internal_RAM[8 * PSWROM + 4] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDD: // DJNZ R5, label
		printf("DJNZ R5, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 5]--;
		if (chip.internal_RAM[8 * PSWROM + 5] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDE:// DJNZ R6, label
		printf("DJNZ R6, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 6]--;
		if (chip.internal_RAM[8 * PSWROM + 6] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDF: // DJNZ R7, label
		printf("DJNZ R7, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[8 * PSWROM + 7]--;
		if (chip.internal_RAM[8 * PSWROM + 7] != 0)
		{
			return PC + (char)data1;
		}

		return PC;

		// 0xE0-0xEF
	case 0xE0: // MOVX A, @A+DPTR
		printf("MOVX A, @A+DPTR\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		printf("No External RAM!\n");
		return PC;

	case 0xE1: // AJMP
		printf("AJMP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0xE2: // MOVX A, @R0
		printf("MOVX A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		printf("No External RAM!\n");
		return PC;
	case 0xE3: // MOVX A, @R1
		printf("MOVX A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		printf("No External RAM!\n");
		return PC;
	case 0xE4: // CLR A
		printf("CLR A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, 0, 1);
		return PC;
	case 0xE5: // MOV A, dir
		printf("MOV A, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, data1, 0);
		return PC;
	case 0xE6: // MOV A, @R0
		printf("MOV A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, chip.internal_RAM[8 * PSWROM], 0);
		return PC;
	case 0xE7: // MOV A, @R1
		printf("MOV A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, chip.internal_RAM[8 * PSWROM + 1], 0);
		return PC;
	case 0xE8: // MOV A, R0
		printf("MOV A, R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, 8 * PSWROM, 0);
		return PC;
	case 0xE9: // MOV A, R1
		printf("MOV A, R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, 8 * PSWROM + 1, 0);
		return PC;
	case 0xEA: // MOV A, R2
		printf("MOV A, R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, 8 * PSWROM + 2, 0);
		return PC;
	case 0xEB: // MOV A, R3
		printf("MOV A, R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, 8 * PSWROM + 3, 0);
		return PC;
	case 0xEC: // MOV A, R4
		printf("MOV A, R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, 8 * PSWROM + 4, 0);
		return PC;
	case 0xED: // MOV A, R5
		printf("MOV A, R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, 8 * PSWROM + 5, 0);
		return PC;
	case 0xEE: // MOV A, R6
		printf("MOV A, R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, 8 * PSWROM + 6, 0);
		return PC;
	case 0xEF: // MOV A, R7
		printf("MOV A, R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(ACC, 8 * PSWROM + 7, 0);
		return PC;

		// 0xF0-OxFF
	case 0xF0: // MOVX A, @A+PC
		printf("MOVX A, @A+PC\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		printf("No External RAM!\n");
		return PC;

	case 0xF1: // ACALL
		printf("ACALL %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(PC % 0x100, 0);
		stackOperation(PC / 0x100, 0);
		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0xF2: // MOVX @R0, A
		printf("MOVX @R0, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		printf("No External RAM!\n");
		return PC;
	case 0xF3: // MOVX @R1, A
		printf("MOVX @R1, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		printf("No External RAM!\n");
		return PC;
	case 0xF4: // CPL A
		printf("CPL A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		chip.internal_RAM[ACC] = 0xFF - chip.internal_RAM[ACC];
		return PC;
	case 0xF5: // MOV dir, A
		printf("MOV %03XH, A\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data1, ACC, 0);
		return PC;
	case 0xF6: // MOV @R0, A
		printf("MOV @R0, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(chip.internal_RAM[8 * PSWROM], ACC, 0);
		return PC;
	case 0xF7: // MOV @R1, A
		printf("MOV @R1, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(chip.internal_RAM[8 * PSWROM + 1], ACC, 0);
		return PC;
	case 0xF8: // MOV R0, A
		printf("MOV R0, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM, ACC, 0);
		return PC;
	case 0xF9: // MOV R1, A
		printf("MOV R1, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 1, ACC, 0);
		return PC;
	case 0xFA: // MOV R2, A
		printf("MOV R2, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 2, ACC, 0);
		return PC;
	case 0xFB: // MOV R3, A
		printf("MOV R3, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 3, ACC, 0);
		return PC;
	case 0xFC: // MOV R4, A
		printf("MOV R4, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 4, ACC, 0);
		return PC;
	case 0xFD: // MOV R5, A
		printf("MOV R5, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 5, ACC, 0);
		return PC;
	case 0xFE: // MOV R6, A
		printf("MOV R6, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 6, ACC, 0);
		return PC;
	case 0xFF: // MOV R7, A
		printf("MOV R7, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(8 * PSWROM + 7, ACC, 0);
		return PC;
	}

	// 명령어 데이터가 없는 경우, 아직 입력된 명령어가 아니라고 알려준다
	printf("UnImplemented!\n");
	if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
		inputDat();

	return PC;
}


/* RunProgram() 함수
*
* 기능 : 프로그램을 실행한다.
* 입력 변수 : mode(자동 실행시 1, 수동 디버그시 0), end_PC(.hex 파일에서 마지막 Program Counter의 위치)
* 출력 변수 없음
*/
void RunProgram(unsigned char mode, int end_PC)
{
	// 명령어 Cycle과 Program Counter
	unsigned long long int cycle = 0;
	unsigned long long int prevCycle = 0;
	unsigned short PC = 0; // 0xFFFF 이후 오버플로우 발생하도록 unsigned short로 만듬
	unsigned short prev_PC = 0;

	// 이번에 실행하는 명령어
	unsigned char tmp_Code;

	// 추가 데이터와 명령어 길이
	unsigned char dat1=0, dat2=0, bytes=1;

	// 종료 여부 확인 변수
	int isEnd = 0;

	// 파일 종료까지 반복
	while (!isEnd)
	{

		// 디버그 모드인 경우 출력
		if (!mode)
		{
			system("cls"); //cls
			printChip(cycle, PC);
		}

		// 기존값 저장
		prevCycle = cycle;
		prev_PC = PC;

		// Interrupt 계산
		PC = interruptControl(PC);

		// 실행할 명령 코드를 가져온 후, PC값 1 증가
		tmp_Code = ROM[PC];
		PC++;

		// 자동 실행 모드에서, Program Counter가 Overflow된 경우, 프로그램 종료
		if (PC == 0 && mode) { isEnd = 1; }


		// 명령어 길이 계산
		bytes = 1;
		for (int i = 0; 0xF5 != TWO_BYTES[i]; i++) // 2byte 명령어인지 확인
		{
			if (TWO_BYTES[i] == tmp_Code)
			{
				bytes = 2;
				dat1 = ROM[PC];
				PC++;
				break;
			}
		}

		// 자동 실행 모드에서, Program Counter가 Overflow된 경우, 프로그램 종료
		if (PC == 0 && mode) { isEnd = 1; }


		for (int i = 0; 0xD5 != THREE_BYTES[i]; i++) // 3byte 명령어인지 확인
		{
			if (THREE_BYTES[i] == tmp_Code)
			{
				bytes = 3;
				dat1 = ROM[PC];
				PC++;
				dat2 = ROM[PC];
				PC++;
				break;
			}
		}

		// 자동 실행 모드에서, Program Counter가 Overflow된 경우, 프로그램 종료
		if (PC == 0 && mode) { isEnd = 1; }


		// Cycle 계산
		cycle++;

		for (int i = 0; 0xF3 != TWO_CYCLE[i]; i++) // 2cycle 명령어인지 확인
		{
			if (TWO_CYCLE[i] == tmp_Code)
			{
				cycle++;
				break;
			}
		}

		for (int i = 0; i < 2; i++) // 4cycle 명령어인지 확인
		{
			if (FOUR_CYCLE[i] == tmp_Code)
			{
				cycle += 3;
				i = 3;
			}
		}

		// 자동 실행 모드에서, Program Counter가 파일 끝을 넘어간 경우, 실행 종료
		if (PC > end_PC && mode){ isEnd = 1; }

		// Timer 계산
		timerControl(cycle - prevCycle);

		// 프로그램 실행
		PC = programRunner(tmp_Code, dat1, dat2, PC, mode); // 해당 명령어 실행
		putParity(); // Parity 계산


		// 자동 실행 모드에서, Program Counter가 파일 끝을 넘어간 경우, 실행 종료
		if (PC > end_PC && mode) { isEnd = 1; }

		// 자동 실행 모드에서, Program Counter가 Overflow시, 실행 종료 (다만, 실행 시간이 매우 오래 걸림)
		if (mode && (prev_PC > PC)) { break; }
	}

	if (mode) // 자동 실행 모드에서, 결괏값 출력
	{
		system("cls"); //cls
		printChip(cycle, PC);
		printf("Auto-Run Finished\n");
		system("PAUSE"); // PAUSE
	}

	return;
}


int main(int argc, char* argv[])
{
	// 순서대로 mode와 마지막 Program Counter를 저장하는 변수와, 읽어들일 파일명을 저장하는 변수 선언
	int mode = 0, end_PC = 0;
	char fileName[256] = "";

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
		exit(1);
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