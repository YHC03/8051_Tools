#include"RunFunction.h"


/* getPortValue() 함수
*
* 기능 : Port 값을 읽어온다.
* 입출력 변수 없음
*/
void getPortValue()
{
	// 입력값을 받는 변수
	int tmpData = 0;

	// P0 입력
	printf("P0 : 0x");
	scanf("%X", &tmpData);
	tmpData = (unsigned char)tmpData;
	chip.internal_RAM[0x80] = chip.latch[0] & tmpData; // Latch에 0이 써진 경우, 읽지 않음

	// P1 입력
	printf("P1 : 0x");
	scanf("%X", &tmpData);
	tmpData = (unsigned char)tmpData;
	chip.internal_RAM[0x90] = chip.latch[1] & tmpData; // Latch에 0이 써진 경우, 읽지 않음

	// P2 입력
	printf("P2 : 0x");
	scanf("%X", &tmpData);
	tmpData = (unsigned char)tmpData;
	chip.internal_RAM[0xA0] = chip.latch[2] & tmpData; // Latch에 0이 써진 경우, 읽지 않음

	// P3 입력
	printf("P3 : 0x");
	scanf("%X", &tmpData);
	tmpData = (unsigned char)tmpData;
	chip.internal_RAM[0xB0] = chip.latch[3] & tmpData; // Latch에 0이 써진 경우, 읽지 않음

	// 출력이 아니므로, syncLatch() 함수는 호출하지 않는다.

	return;
}


/* inputDat() 함수
*
* 기능 : Port Data Input 등의 입력 값이 필요한 경우 이를 수행할 수 있도록 한다.
* 입력 변수 없음
* 출력 변수 없음
*/
void inputDat()
{
	int mode = 0;
retry:
	// 모드 입력
	printf("1: Pin Input, Other Number: Run Next. : ");
	scanf("%d", &mode);

	switch (mode)
	{
	case 1: // Pin 입력
		getPortValue();
		goto retry;

	default: // 시작
		return;
	}

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
		if (i == 9) // SBUF 출력을 위함
		{
			printf("9(I/O) ");
		}else{
			printf(" %X ", i);
		}
	}
	printf("\n");

	// 모든 RAM의 값 출력
	for (int i = 0; i < 16; i++)
	{
		// Index 출력
		printf("%2X ", i * 16);

		// RAM의 값 출력
		for (int j = 0; j < 16; j++)
		{
			if (j == 9) {
				if (i == 9) // SBUF의 경우
				{
					printf("%2X, %2X ", chip.internal_RAM[i * 16 + j], chip.SBUF_send);
				}else{
					printf("  %2X   ", chip.internal_RAM[i * 16 + j]);
				}
			}else{
				printf("%2X ", chip.internal_RAM[i * 16 + j]);
			}
		}

		// 개행
		printf("\n");
	}

	// 구분선 출력
	printf("--------------------------------------------------\n");

	// PSW와 TCON의 값 BIT단위로 출력
	printf("PSW : %d %d %d %d %d %d %d %d\n", chip.internal_RAM[PSW] / 0x80 % 0x02, chip.internal_RAM[PSW] / 0x40 % 0x02, chip.internal_RAM[PSW] / 0x20 % 0x02,
		chip.internal_RAM[PSW] / 0x10 % 0x02, chip.internal_RAM[PSW] / 0x08 % 0x02, chip.internal_RAM[PSW] / 0x04 % 0x02, chip.internal_RAM[PSW] / 0x02 % 0x02,
		chip.internal_RAM[PSW] / 0x01 % 0x02);
	printf("TCON: %d %d %d %d %d %d %d %d\n", chip.internal_RAM[TCON] / 0x80 % 0x02, chip.internal_RAM[TCON] / 0x40 % 0x02, chip.internal_RAM[TCON] / 0x20 % 0x02,
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


/* programRunner() 함수
* 
* 기능 : 주어진 code의 명령을 실행한다.
* 입력 변수 : code(명령 코드), data1(추가데이터1), data2(추가데이터2), PC(현재의 Program Counter 위치), isDebugMode(디버그 모드 여부)
* 출력 변수 : 다음에 실행할 Program Counter의 위치
*/
int programRunner(unsigned char code, unsigned char data1, unsigned char data2, unsigned short PC, char isDebugMode)
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
		printf("AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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

		incFunc(ACC);
		return PC;
	case 0x05: // INC DIR
		printf("INC %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(data1);
		return PC;
	case 0x06: // INC @R0
		printf("INC @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(chip.internal_RAM[8 * PSWROM]);
		return PC;
	case 0x07: // INC @R1
		printf("INC @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(chip.internal_RAM[8 * PSWROM + 1]);
		return PC;
	case 0x08: // INC R0
		printf("INC R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(8 * PSWROM);
		return PC;
	case 0x09: // INC R1
		printf("INC R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(8 * PSWROM + 1);
		return PC;
	case 0x0A: // INC R2
		printf("INC R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(8 * PSWROM + 2);
		return PC;
	case 0x0B: // INC R3
		printf("INC R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(8 * PSWROM + 3);
		return PC;
	case 0x0C: // INC R4
		printf("INC R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(8 * PSWROM + 4);
		return PC;
	case 0x0D: // INC R5
		printf("INC R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(8 * PSWROM + 5);
		return PC;
	case 0x0E: // INC R6
		printf("INC R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(8 * PSWROM + 6);
		return PC;
	case 0x0F: // INC R7
		printf("INC R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		incFunc(8 * PSWROM + 7);
		return PC;

		// 0x10-Ox1F
	case 0x10: // JBC
		printf("JBC %03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (getBitAddr(data1))
		{
			clearBitAddr(data1);
			return PC + (char)data2;
		}

		return PC;
	case 0x11: // ACALL
		printf("ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		}else{
			clearBitAddr(C);
		}
		return PC;
	case 0x14: // DEC A
		printf("DEC A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(ACC);
		return PC;
	case 0x15: // DEC DIR
		printf("DEC %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(data1);
		return PC;
	case 0x16: // DEC @R0
		printf("DEC @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(chip.internal_RAM[8 * PSWROM]);
		return PC;
	case 0x17: // DEC @R1
		printf("DEC @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(chip.internal_RAM[8 * PSWROM + 1]);
		return PC;
	case 0x18: // DEC R0
		printf("DEC R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM);
		return PC;
	case 0x19: // DEC R1
		printf("DEC R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 1);
		return PC;
	case 0x1A: // DEC R2
		printf("DEC R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 2);
		return PC;
	case 0x1B: // DEC R3
		printf("DEC R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 3);
		return PC;
	case 0x1C: // DEC R4
		printf("DEC R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 4);
		return PC;
	case 0x1D: // DEC R5
		printf("DEC R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 5);
		return PC;
	case 0x1E: // DEC R6
		printf("DEC R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 6);
		return PC;
	case 0x1F: // DEC R7
		printf("DEC R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 7);
		return PC;

		// 0x20-0x2F

	case 0x20: // JB
		printf("JB %03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (getBitAddr(data1))
			return PC + (char)data2;

		return PC;
	case 0x21: // AJMP
		printf("AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		printf("JNB %03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (!getBitAddr(data1))
			return PC + (char)data2;

		return PC;
	case 0x31: // ACALL
		printf("ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		}else{
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
		printf("JC %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (getBitAddr(C))
			return PC + (char)data1;

		return PC;
	case 0x41: // AJMP
		printf("AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		printf("JNC %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (!getBitAddr(C))
			return PC + (char)data1;

		return PC;
	case 0x51: // ACALL
		printf("ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		printf("JZ %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (!chip.internal_RAM[ACC])
			return PC + (char)data1;

		return PC;
	case 0x61: // AJMP
		printf("AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		printf("JNZ %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[ACC])
			return PC + (char)data1;

		return PC;
	case 0x71: // ACALL
		printf("ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(PC % 0x100, 0);
		stackOperation(PC / 0x100, 0);
		return (PC / 0x800) * 0x800 + 0x000 + data1;
	case 0x72: // ORL C, bit
		printf("ORL C, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		orOperation(C, data1, 0, 1);
		return PC;
	case 0x73: // JMP @A+DPTR
		printf("JMP @A+DPTR\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return chip.internal_RAM[DPH] * 0x100 + chip.internal_RAM[DPL] + chip.internal_RAM[ACC];
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
		printf("SJMP %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		return PC + (char)data1;

	case 0x81: // AJMP
		printf("AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
	case 0x85: // MOV dir, dir (Source, Destination 순서로 .hex 파일이 입력됨)
		printf("MOV %03XH, %03XH\n", data2, data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(data2, data1, 0);
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
	case 0x90: // MOV DPTR
		printf("MOV DPTR, #%03XH\n", data1 * 0x100 + data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		movFunc(DPH, data1, 1);
		movFunc(DPL, data2, 1);
		return PC;
	case 0x91: // ACALL
		printf("ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		}else{
			clearBitAddr(data1);
		}
		return PC;
	case 0x93: // MOVC A, @A+DPTR
		printf("MOVC A, @A+DPTR\n");
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
		printf("AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		}else{
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
		printf("ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		printf("CJNE A, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[ACC] != data1)
			return PC + (char)data2;

		return PC;
	case 0xB5: // CJNE A, dir, label
		printf("CJNE A, %03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[ACC] != chip.internal_RAM[data1])
			return PC + (char)data2;

		return PC;
	case 0xB6: // CJNE @R0, DATA, label
		printf("CJNE @R0, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[chip.internal_RAM[8 * PSWROM]] != data1)
			return PC + (char)data2;

		return PC;
	case 0xB7: // CJNE @R1, DATA, label
		printf("CJNE @R1, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[chip.internal_RAM[8 * PSWROM + 1]] != data1)
			return PC + (char)data2;

		return PC;
	case 0xB8: // CJNE R0, DATA, label
		printf("CJNE R0, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM] != data1)
			return PC + (char)data2;

		return PC;
	case 0xB9: // CJNE R1, DATA, label
		printf("CJNE R1, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 1] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBA: // CJNE R2, DATA, label
		printf("CJNE R2, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 2] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBB: // CJNE R3, DATA, label
		printf("CJNE R3, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 3] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBC: // CJNE R4, DATA, label
		printf("CJNE R4, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 4] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBD: // CJNE R5, DATA, label
		printf("CJNE R5, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 5] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBE: // CJNE R6, DATA, label
		printf("CJNE R6, #%03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		if (chip.internal_RAM[8 * PSWROM + 6] != data1)
			return PC + (char)data2;

		return PC;
	case 0xBF: // CJNE R7, DATA, label
		printf("CJNE R7, #%03XH, %05XH\n", data1, PC + (char)data2);
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
		printf("AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		printf("XCH A, %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(data1);
		return PC;
	case 0xC6: // XCH @R0
		printf("XCH A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(chip.internal_RAM[8 * PSWROM]);
		return PC;
	case 0xC7: // XCH @R1
		printf("XCH A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(chip.internal_RAM[8 * PSWROM + 1]);
		return PC;
	case 0xC8: // XCH R0
		printf("XCH A, R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(8 * PSWROM);
		return PC;
	case 0xC9: // XCH R1
		printf("XCH A, R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(8 * PSWROM + 1);
		return PC;
	case 0xCA: // XCH R2
		printf("XCH A, R2\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(8 * PSWROM + 2);
		return PC;
	case 0xCB: // XCH R3
		printf("XCH A, R3\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(8 * PSWROM + 3);
		return PC;
	case 0xCC: // XCH R4
		printf("XCH A, R4\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(8 * PSWROM + 4);
		return PC;
	case 0xCD: // XCH R5
		printf("XCH A, R5\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(8 * PSWROM + 5);
		return PC;
	case 0xCE: // XCH R6
		printf("XCH A, R6\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(8 * PSWROM + 6);
		return PC;
	case 0xCF: // XCH R7
		printf("XCH A, R7\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		swapOperation(8 * PSWROM + 7);
		return PC;

		//0xD0-0xDF
	case 0xD0: // POP
		printf("POP %03XH\n", data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		stackOperation(data1, 1);
		return PC;
	case 0xD1: // ACALL
		printf("ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		printf("DJNZ %03XH, %05XH\n", data1, PC + (char)data2);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(data1);
		if (chip.internal_RAM[data1] != 0)
		{
			return PC + (char)data2;
		}

		return PC;
	case 0xD6: // XCHD A, @R0
		printf("XCHD A, @R0\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		halfSwapOperation(chip.internal_RAM[8 * PSWROM]);
		return PC;
	case 0xD7: // XCHD A, @R1
		printf("XCHD A, @R1\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		halfSwapOperation(chip.internal_RAM[8 * PSWROM + 1]);
		return PC;
	case 0xD8: // DJNZ R0, label
		printf("DJNZ R0, %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM);
		if (chip.internal_RAM[8 * PSWROM] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xD9: // DJNZ R1, label
		printf("DJNZ R1, %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 1);
		if (chip.internal_RAM[8 * PSWROM + 1] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDA: // DJNZ R2, label
		printf("DJNZ R2, %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 2);
		if (chip.internal_RAM[8 * PSWROM + 2] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDB: // DJNZ R3, label
		printf("DJNZ R3, %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 3);
		if (chip.internal_RAM[8 * PSWROM + 3] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDC: // DJNZ R4, label
		printf("DJNZ R4, %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 4);
		if (chip.internal_RAM[8 * PSWROM + 4] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDD: // DJNZ R5, label
		printf("DJNZ R5, %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 5);
		if (chip.internal_RAM[8 * PSWROM + 5] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDE:// DJNZ R6, label
		printf("DJNZ R6, %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 6);
		if (chip.internal_RAM[8 * PSWROM + 6] != 0)
		{
			return PC + (char)data1;
		}

		return PC;
	case 0xDF: // DJNZ R7, label
		printf("DJNZ R7, %05XH\n", PC + (char)data1);
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		decFunc(8 * PSWROM + 7);
		if (chip.internal_RAM[8 * PSWROM + 7] != 0)
		{
			return PC + (char)data1;
		}

		return PC;

		// 0xE0-0xEF
	case 0xE0: // MOVX A, @DPTR
		printf("MOVX A, @DPTR\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		printf("No External RAM!\n");
		return PC;

	case 0xE1: // AJMP
		printf("AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
	case 0xF0: // MOVX @DPTR, A
		printf("MOVX @DPTR, A\n");
		if (!isDebugMode) // 디버그 모드의 경우, 일시 중지
			inputDat();

		printf("No External RAM!\n");
		return PC;

	case 0xF1: // ACALL
		printf("ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);
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
		// 기존값 저장
		prevCycle = cycle;
		prev_PC = PC;

		// Interrupt 계산
		PC = interruptControl(PC);

		// 디버그 모드인 경우 출력
		if (!mode)
		{
			system("cls"); //cls
			printChip(cycle, PC); // prevCycle, prev_PC 대신 사용 가능. Cycle과 Program Counter값 변경 시 prevCycle, prev_PC로 바꿀 것.
		}

		// 실행할 명령 코드를 가져온 후, PC값 1 증가
		tmp_Code = ROM[PC];
		PC++;

		// 자동 실행 모드에서, Program Counter가 Overflow된 경우, 프로그램 종료
		if (PC == 0 && mode) { isEnd = 1; }


		// 명령어 길이 계산
		bytes = 1;
		for (int i = 0; i < 91; i++) // 2byte 명령어인지 확인
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


		for (int i = 0; i < 24; i++) // 3byte 명령어인지 확인
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

		for (int i = 0; i < 90; i++) // 2cycle 명령어인지 확인
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
		if ((PC > end_PC) && mode) { isEnd = 1; }

		// 자동 실행 모드에서, 현재 Program Counter의 값이 직전값과 동일한 경우, 아무것도 하지 않는 무한 루프로 판단해 실행 종료
		if (mode && (prev_PC == PC)) { break; }
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