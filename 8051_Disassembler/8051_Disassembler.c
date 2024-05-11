#include<stdio.h>
#include<stdlib.h>
#include<string.h>


/*
* 8051 Disassembler
* 
* 기능 : .hex 파일의 주소를 받아 Disassemble한 결과를 .a51 파일을 만들어 출력한다.
* 
* 작성자 : YHC03
*/

// ROM 설정
unsigned char ROM[65535];


// 특수 명령어 Bytes
const unsigned char TWO_BYTES[91] = {0x01, 0x05, 0x11, 0x15, 0x21, 0x24, 0x25, 0x31, 0x34,
0x35, 0x40, 0x41, 0x42, 0x44, 0x45, 0x50, 0x51, 0x52, 0x54, 0x55, 0x60, 0x61, 0x62, 0x64,
0x65, 0x70, 0x71, 0x72, 0x74, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
0x80, 0x81, 0x82, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x91, 0x92,
0x94, 0x95, 0xA0, 0XA1, 0xA2, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
0xB0, 0xB1, 0xB2, 0xC1, 0xC2, 0xC3, 0xC5, 0xD0, 0xD1, 0xD2, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC,
0xDD, 0xDE, 0xDF, 0xE1, 0xE5, 0xF1, 0xF5};
const unsigned char THREE_BYTES[24] = { 0x02, 0x10, 0x12, 0x21, 0x30, 0x43, 0x53, 0x63, 0x75,
0x85, 0x90, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xD5 };


// 함수 시작

// 파일 입력
int fileReader(char* fileName);
unsigned char asciiToHEX(unsigned char orig); // HEX 변환

// 프로그램 구동
void RunProgram(char* fileName, int end_PC);
void programRunner(char* fileName, unsigned char code, unsigned char data1, unsigned char data2, unsigned short PC);


// 함수 끝


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
		printf("File Not Found at %s\n", fileName);
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

		// 이전값이 최댓값을 넘어간 경우, 그 값을 저장 후
		if (prev_PC < tmp_PC)
		{
			prev_PC = tmp_PC;
			prev_REM = tmp_REM;
		}

		// 해당 줄의 명령어 개수를 읽고
		tmp_REM = asciiToHEX(tmp_Data[1]) * 16 + asciiToHEX(tmp_Data[2]); //CC
		parity += tmp_REM;

		// 해당 줄의 Program Counter 위치를 읽고
		tmp_PC = asciiToHEX(tmp_Data[3]) * 0x1000 + asciiToHEX(tmp_Data[4]) * 0x100 + asciiToHEX(tmp_Data[5]) * 0x10 + asciiToHEX(tmp_Data[6]); //AAAA
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

	return prev_PC + prev_REM;
}


/* programRunner() 함수
* 
* 기능 : 주어진 code의 명령을 실행한다.
* 입력 변수 : fileName(출력 파일의 위치와 이름), code(명령 코드), data1(추가데이터1), data2(추가데이터2), PC(다음에 읽어들일 Program Counter의 값)
* 출력 변수 없음
*/
void programRunner(char* fileName, unsigned char code, unsigned char data1, unsigned char data2, unsigned short PC)
{
	FILE* targetFile = fopen(fileName, "a");

	if (targetFile == NULL)
	{
		printf("Output Error on Writing File!\n");
		exit(1);
	}

	// 명령어별로 실행
	switch (code)
	{
	case 0x00: // NOP
		fprintf(targetFile, "NOP\n");

		break;
	case 0x01: // AJMP
		fprintf(targetFile, "AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x02: // LJMP
		fprintf(targetFile, "LJMP %05XH\n", data1 * 0x100 + data2);

		break;
	case 0x03: // RR A
		fprintf(targetFile, "RR A\n");

		break;
	case 0x04: // INC A
		fprintf(targetFile, "INC A\n");

		break;
	case 0x05: // INC DIR
		fprintf(targetFile, "INC %03XH\n", data1);

		break;
	case 0x06: // INC @R0
		fprintf(targetFile, "INC @R0\n");

		break;
	case 0x07: // INC @R1
		fprintf(targetFile, "INC @R1\n");

		break;
	case 0x08: // INC R0
		fprintf(targetFile, "INC R0\n");

		break;
	case 0x09: // INC R1
		fprintf(targetFile, "INC R1\n");

		break;
	case 0x0A: // INC R2
		fprintf(targetFile, "INC R2\n");

		break;
	case 0x0B: // INC R3
		fprintf(targetFile, "INC R3\n");

		break;
	case 0x0C: // INC R4
		fprintf(targetFile, "INC R4\n");

		break;
	case 0x0D: // INC R5
		fprintf(targetFile, "INC R5\n");

		break;
	case 0x0E: // INC R6
		fprintf(targetFile, "INC R6\n");

		break;
	case 0x0F: // INC R7
		fprintf(targetFile, "INC R7\n");

		break;

		// 0x10-Ox1F

	case 0x10: // JBC
		fprintf(targetFile, "JBC %03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0x11: // ACALL
		fprintf(targetFile, "ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x12: // LCALL
		fprintf(targetFile, "LCALL %05XH\n", data1 * 0x100 + data2);

		break;
	case 0x13: // RRC A
		fprintf(targetFile, "RRC A\n");

		break;
	case 0x14: // DEC A
		fprintf(targetFile, "DEC A\n");

		break;
	case 0x15: // DEC DIR
		fprintf(targetFile, "DEC %03XH\n", data1);

		break;
	case 0x16: // DEC @R0
		fprintf(targetFile, "DEC @R0\n");

		break;
	case 0x17: // DEC @R1
		fprintf(targetFile, "DEC @R1\n");

		break;
	case 0x18: // DEC R0
		fprintf(targetFile, "DEC R0\n");

		break;
	case 0x19: // DEC R1
		fprintf(targetFile, "DEC R1\n");

		break;
	case 0x1A: // DEC R2
		fprintf(targetFile, "DEC R2\n");

		break;
	case 0x1B: // DEC R3
		fprintf(targetFile, "DEC R3\n");

		break;
	case 0x1C: // DEC R4
		fprintf(targetFile, "DEC R4\n");

		break;
	case 0x1D: // DEC R5
		fprintf(targetFile, "DEC R5\n");

		break;
	case 0x1E: // DEC R6
		fprintf(targetFile, "DEC R6\n");

		break;
	case 0x1F: // DEC R7
		fprintf(targetFile, "DEC R7\n");

		break;

		// 0x20-0x2F
	case 0x20: // JB
		fprintf(targetFile, "JB %03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0x21: // AJMP
		fprintf(targetFile, "AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x22: // RET
		fprintf(targetFile, "RET\n");

		break;
	case 0x23: // RL A
		fprintf(targetFile, "RL A\n");

		break;
	case 0x24: // ADD A, data
		fprintf(targetFile, "ADD A, #%03XH\n", data1);

		break;
	case 0x25: // ADD DIR
		fprintf(targetFile, "ADD A, %03XH\n", data1);

		break;
	case 0x26: // ADD @R0
		fprintf(targetFile, "ADD A, @R0\n");

		break;
	case 0x27: // ADD @R1
		fprintf(targetFile, "ADD A, @R1\n");

		break;
	case 0x28: // ADD R0
		fprintf(targetFile, "ADD A, R0\n");

		break;
	case 0x29: // ADD R1
		fprintf(targetFile, "ADD A, R1\n");

		break;
	case 0x2A: // ADD R2
		fprintf(targetFile, "ADD A, R2\n");

		break;
	case 0x2B: // ADD R3
		fprintf(targetFile, "ADD A, R3\n");

		break;
	case 0x2C: // ADD R4
		fprintf(targetFile, "ADD A, R4\n");

		break;
	case 0x2D: // ADD R5
		fprintf(targetFile, "ADD A, R5\n");

		break;
	case 0x2E: // ADD R6
		fprintf(targetFile, "ADD A, R6\n");

		break;
	case 0x2F: // ADD R7
		fprintf(targetFile, "ADD A, R7\n");

		break;

		// 0x30-Ox3F
	case 0x30: // JNB
		fprintf(targetFile, "JNB %03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0x31: // ACALL
		fprintf(targetFile, "ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x32: // RETI
		fprintf(targetFile, "RETI\n");

		break;
	case 0x33: // RLC A
		fprintf(targetFile, "RLC A\n");

		break;
	case 0x34: // ADDC A, data
		fprintf(targetFile, "ADDC A, #%03XH\n", data1);

		break;
	case 0x35: // ADDC DIR
		fprintf(targetFile, "ADDC A, %03XH\n", data1);

		break;
	case 0x36: // ADDC @R0
		fprintf(targetFile, "ADDC A, @R0\n");

		break;
	case 0x37: // ADDC @R1
		fprintf(targetFile, "ADDC A, @R1\n");

		break;
	case 0x38: // ADDC R0
		fprintf(targetFile, "ADDC A, R0\n");

		break;
	case 0x39: // ADDC R1
		fprintf(targetFile, "ADDC A, R1\n");

		break;
	case 0x3A: // ADDC R2
		fprintf(targetFile, "ADDC A, R2\n");

		break;
	case 0x3B: // ADDC R3
		fprintf(targetFile, "ADDC A, R3\n");

		break;
	case 0x3C: // ADDC R4
		fprintf(targetFile, "ADDC A, R4\n");

		break;
	case 0x3D: // ADDC R5
		fprintf(targetFile, "ADDC A, R5\n");

		break;
	case 0x3E: // ADDC R6
		fprintf(targetFile, "ADDC A, R6\n");

		break;
	case 0x3F: // ADDC R7
		fprintf(targetFile, "ADDC A, R7\n");

		break;

		// 0x40-0x4F
	case 0x40: // JC
		fprintf(targetFile, "JC %05XH\n", PC + (char)data1);

		break;
	case 0x41: // AJMP
		fprintf(targetFile, "AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x42: // ORL dir A
		fprintf(targetFile, "ORL %03XH, A\n", data1);

		break;
	case 0x43: // ORL dir data
		fprintf(targetFile, "ORL %03XH, #%03XH\n", data1, data2);

		break;
	case 0x44: // ORL A, data
		fprintf(targetFile, "ORL A, #%03XH\n", data1);

		break;
	case 0x45: // ORL A, dir
		fprintf(targetFile, "ORL A, %03XH\n", data1);

		break;
	case 0x46: // ORL A, @R0
		fprintf(targetFile, "ORL A, @R0\n");

		break;
	case 0x47: // ORL A, @R1
		fprintf(targetFile, "ORL A, @R1\n");

		break;
	case 0x48: // ORL A, R0
		fprintf(targetFile, "ORL A, R0\n");

		break;
	case 0x49: // ORL A, R1
		fprintf(targetFile, "ORL A, R1\n");

		break;
	case 0x4A: // ORL A, R2
		fprintf(targetFile, "ORL A, R2\n");

		break;
	case 0x4B: // ORL A, R3
		fprintf(targetFile, "ORL A, R3\n");

		break;
	case 0x4C: // ORL A, R4
		fprintf(targetFile, "ORL A, R4\n");

		break;
	case 0x4D: // ORL A, R5
		fprintf(targetFile, "ORL A, R5\n");

		break;
	case 0x4E: // ORL A, R6
		fprintf(targetFile, "ORL A, R6\n");

		break;
	case 0x4F: // ORL A, R7
		fprintf(targetFile, "ORL A, R7\n");

		break;

		// 0x50-0x5F
	case 0x50: // JNC
		fprintf(targetFile, "JNC %05XH\n", PC + (char)data1);

		break;
	case 0x51: // ACALL
		fprintf(targetFile, "ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x52: // ANL dir A
		fprintf(targetFile, "ANL %03XH, A\n", data1);

		break;
	case 0x53: // ANL dir data
		fprintf(targetFile, "ANL %03XH, #%03XH\n", data1, data2);

		break;
	case 0x54: // ANL A, data
		fprintf(targetFile, "ANL A, #%03XH\n", data1);

		break;
	case 0x55: // ANL A, dir
		fprintf(targetFile, "ANL A, %03XH\n", data1);

		break;
	case 0x56: // ANL A, @R0
		fprintf(targetFile, "ANL A, @R0\n");

		break;
	case 0x57: // ANL A, @R1
		fprintf(targetFile, "ANL A, @R1\n");

		break;
	case 0x58: // ANL A, R0
		fprintf(targetFile, "ANL A, R0\n");

		break;
	case 0x59: // ANL A, R1
		fprintf(targetFile, "ANL A, R1\n");

		break;
	case 0x5A: // ANL A, R2
		fprintf(targetFile, "ANL A, R2\n");

		break;
	case 0x5B: // ANL A, R3
		fprintf(targetFile, "ANL A, R3\n");

		break;
	case 0x5C: // ANL A, R4
		fprintf(targetFile, "ANL A, R4\n");

		break;
	case 0x5D: // ANL A, R5
		fprintf(targetFile, "ANL A, R5\n");

		break;
	case 0x5E: // ANL A, R6
		fprintf(targetFile, "ANL A, R6\n");

		break;
	case 0x5F: // ANL A, R7
		fprintf(targetFile, "ANL A, R7\n");

		break;

		// 0x60-0x6F
	case 0x60: // JZ
		fprintf(targetFile, "JZ %05XH\n", PC + (char)data1);

		break;
	case 0x61: // AJMP
		fprintf(targetFile, "AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x62: // XRL dir A
		fprintf(targetFile, "XRL %03XH, A\n", data1);

		break;
	case 0x63: // XRL dir data
		fprintf(targetFile, "XRL %03XH, #%03XH\n", data1, data2);

		break;
	case 0x64: // XRL A, data
		fprintf(targetFile, "XRL A, #%03XH\n", data1);

		break;
	case 0x65: // XRL A, dir
		fprintf(targetFile, "XRL A, %03XH\n", data1);

		break;
	case 0x66: // XRL A, @R0
		fprintf(targetFile, "XRL A, @R0\n");

		break;
	case 0x67: // XRL A, @R1
		fprintf(targetFile, "XRL A, @R1\n");

		break;
	case 0x68: // XRL A, R0
		fprintf(targetFile, "XRL A, R0\n");

		break;
	case 0x69: // XRL A, R1
		fprintf(targetFile, "XRL A, R1\n");

		break;
	case 0x6A: // XRL A, R2
		fprintf(targetFile, "XRL A, R2\n");

		break;
	case 0x6B: // XRL A, R3
		fprintf(targetFile, "XRL A, R3\n");

		break;
	case 0x6C: // XRL A, R4
		fprintf(targetFile, "XRL A, R4\n");

		break;
	case 0x6D: // XRL A, R5
		fprintf(targetFile, "XRL A, R5\n");

		break;
	case 0x6E: // XRL A, R6
		fprintf(targetFile, "XRL A, R6\n");

		break;
	case 0x6F: // XRL A, R7
		fprintf(targetFile, "XRL A, R7\n");

		break;

		// 0x70 - 0x7F
	case 0x70: // JNZ
		fprintf(targetFile, "JNZ %05XH\n", PC + (char)data1);

		break;
	case 0x71: // ACALL
		fprintf(targetFile, "ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x72: // ORL C, bit
		fprintf(targetFile, "ORL C, %03XH\n", data1);

		break;
	case 0x73: // JMP @A+DPTR
		fprintf(targetFile, "JMP @A+DPTR\n");

	case 0x74: // MOV A, data
		fprintf(targetFile, "MOV A, #%03XH\n", data1);

		break;
	case 0x75: // MOV dir, data
		fprintf(targetFile, "MOV %03XH, #%03XH\n", data1, data2);

		break;
	case 0x76: // MOV @R0, dat
		fprintf(targetFile, "MOV @R0, #%03XH\n", data1);

		break;
	case 0x77: // MOV @R1, dat
		fprintf(targetFile, "MOV @R1, #%03XH\n", data1);

		break;
	case 0x78: // MOV R0, dat
		fprintf(targetFile, "MOV R0, #%03XH\n", data1);

		break;
	case 0x79: // MOV R1, dat
		fprintf(targetFile, "MOV R1, #%03XH\n", data1);

		break;
	case 0x7A: // MOV R2, dat
		fprintf(targetFile, "MOV R2, #%03XH\n", data1);

		break;
	case 0x7B: // MOV R3, dat
		fprintf(targetFile, "MOV R3, #%03XH\n", data1);

		break;
	case 0x7C: // MOV R4, dat
		fprintf(targetFile, "MOV R4, #%03XH\n", data1);

		break;
	case 0x7D: // MOV R5, dat
		fprintf(targetFile, "MOV R5, #%03XH\n", data1);

		break;
	case 0x7E: // MOV R6, dat
		fprintf(targetFile, "MOV R6, #%03XH\n", data1);

		break;
	case 0x7F: // MOV R7, dat
		fprintf(targetFile, "MOV R7, #%03XH\n", data1);

		break;

		// 0x80-0x8F
	case 0x80: // SJMP
		fprintf(targetFile, "SJMP %05XH\n", PC + (char)data1);

		break;

	case 0x81: // AJMP
		fprintf(targetFile, "AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x82: // ANL C, bit
		fprintf(targetFile, "ANL C, %03XH\n", data1);

		break;
	case 0x83: // MOVC A, @A+PC
		fprintf(targetFile, "MOVC A, @A+PC\n");

		break;
	case 0x84: // DIV
		fprintf(targetFile, "DIV AB\n");

		break;
	case 0x85: // MOV dir, dir
		fprintf(targetFile, "MOV %03XH, %03XH\n", data1, data2);

		break;
	case 0x86: // MOV dir, @R0
		fprintf(targetFile, "MOV %03XH, @R0\n", data1);

		break;
	case 0x87: // MOV dir, @R1
		fprintf(targetFile, "MOV %03XH, @R1\n", data1);

		break;
	case 0x88: // MOV dir, R0
		fprintf(targetFile, "MOV %03XH, R0\n", data1);

		break;
	case 0x89: // MOV dir, R1
		fprintf(targetFile, "MOV %03XH, R1\n", data1);

		break;
	case 0x8A: // MOV dir, R2
		fprintf(targetFile, "MOV %03XH, R2\n", data1);

		break;
	case 0x8B: // MOV dir, R3
		fprintf(targetFile, "MOV %03XH, R3\n", data1);

		break;
	case 0x8C: // MOV dir, R4
		fprintf(targetFile, "MOV %03XH, R4\n", data1);

		break;
	case 0x8D: // MOV dir, R5
		fprintf(targetFile, "MOV %03XH, R5\n", data1);

		break;
	case 0x8E: // MOV dir, R6
		fprintf(targetFile, "MOV %03XH, R6\n", data1);

		break;
	case 0x8F: // MOV dir, R7
		fprintf(targetFile, "MOV %03XH, R7\n", data1);

		break;

		// 0x90-0x9F
	case 0x90: // MOV DPTR, #data
		fprintf(targetFile, "MOV DPTR, #%05XH\n", data1 * 0x100 + data2);

		break;
	case 0x91: // ACALL
		fprintf(targetFile, "ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0x92: // MOV bit, C
		fprintf(targetFile, "MOV %03XH, C\n", data1);

		break;
	case 0x93: // MOVC A, @A+DPTR
		fprintf(targetFile, "MOVC A, @A+DPTR\n");

		break;
	case 0x94: // SUBB A, data
		fprintf(targetFile, "SUBB A, #%03XH\n", data1);

		break;
	case 0x95: // SUBB DIR
		fprintf(targetFile, "SUBB A, %03XH\n", data1);

		break;
	case 0x96: // SUBB @R0
		fprintf(targetFile, "SUBB A, @R0\n");

		break;
	case 0x97: // SUBB @R1
		fprintf(targetFile, "SUBB A, @R1\n");

		break;
	case 0x98: // SUBB R0
		fprintf(targetFile, "SUBB A, R0\n");

		break;
	case 0x99: // SUBB R1
		fprintf(targetFile, "SUBB A, R1\n");

		break;
	case 0x9A: // SUBB R2
		fprintf(targetFile, "SUBB A, R2\n");

		break;
	case 0x9B: // SUBB R3
		fprintf(targetFile, "SUBB A, R3\n");

		break;
	case 0x9C: // SUBB R4
		fprintf(targetFile, "SUBB A, R4\n");

		break;
	case 0x9D: // SUBB R5
		fprintf(targetFile, "SUBB A, R5\n");

		break;
	case 0x9E: // SUBB R6
		fprintf(targetFile, "SUBB A, R6\n");

		break;
	case 0x9F: // SUBB R7
		fprintf(targetFile, "SUBB A, R7\n");

		break;

		// 0xA0-0xAF
	case 0xA0: // ORL C, ~bit
		fprintf(targetFile, "ORL C, /%03XH\n", data1);

		break;
	case 0xA1: // AJMP
		fprintf(targetFile, "AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0xA2: // MOV C, bit
		fprintf(targetFile, "MOV C, %03XH\n", data1);

		break;
	case 0xA3: // INC DPTR
		fprintf(targetFile, "INC DPTR\n");

		break;
	case 0xA4: // MUL
		fprintf(targetFile, "MUL AB\n");

		break;
	case 0xA5: // Unused
		fprintf(targetFile, "DB 0A5H\n");

		break;
	case 0xA6: // MOV @R0, dir
		fprintf(targetFile, "MOV @R0, %03XH\n", data1);

		break;
	case 0xA7: // MOV @R1, dir
		fprintf(targetFile, "MOV @R1, %03XH\n", data1);

		break;
	case 0xA8: // MOV R0, dir
		fprintf(targetFile, "MOV R0, %03XH\n", data1);

		break;
	case 0xA9: // MOV R1, dir
		fprintf(targetFile, "MOV R1, %03XH\n", data1);

		break;
	case 0xAA: // MOV R2, dir
		fprintf(targetFile, "MOV R2, %03XH\n", data1);

		break;
	case 0xAB: // MOV R3, dir
		fprintf(targetFile, "MOV R3, %03XH\n", data1);

		break;
	case 0xAC: // MOV R4, dir
		fprintf(targetFile, "MOV R4, %03XH\n", data1);

		break;
	case 0xAD: // MOV R5, dir
		fprintf(targetFile, "MOV R5, %03XH\n", data1);

		break;
	case 0xAE: // MOV R6, dir
		fprintf(targetFile, "MOV R6, %03XH\n", data1);

		break;
	case 0xAF: // MOV R7, dir
		fprintf(targetFile, "MOV R7, %03XH\n", data1);

		break;

		// 0xB0-0xBF
	case 0xB0: // ANL C, ~bit
		fprintf(targetFile, "ANL C, /%03XH\n", data1);

		break;
	case 0xB1: // ACALL
		fprintf(targetFile, "ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0xB2: // CPL bit
		fprintf(targetFile, "CPL %03XH\n", data1);

		break;
	case 0xB3: // CPL C
		fprintf(targetFile, "CPL C\n");

		break;
	case 0xB4: // CJNE A, DATA, label
		fprintf(targetFile, "CJNE A, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xB5: // CJNE A, dir, label
		fprintf(targetFile, "CJNE A, %03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xB6: // CJNE @R0, DATA, label
		fprintf(targetFile, "CJNE @R0, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xB7: // CJNE @R1, DATA, label
		fprintf(targetFile, "CJNE @R1, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xB8: // CJNE R0, DATA, label
		fprintf(targetFile, "CJNE R0, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xB9: // CJNE R1, DATA, label
		fprintf(targetFile, "CJNE R1, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xBA: // CJNE R2, DATA, label
		fprintf(targetFile, "CJNE R2, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xBB: // CJNE R3, DATA, label
		fprintf(targetFile, "CJNE R3, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xBC: // CJNE R4, DATA, label
		fprintf(targetFile, "CJNE R4, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xBD: // CJNE R5, DATA, label
		fprintf(targetFile, "CJNE R5, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xBE: // CJNE R6, DATA, label
		fprintf(targetFile, "CJNE R6, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xBF: // CJNE R7, DATA, label
		fprintf(targetFile, "CJNE R7, #%03XH, %05XH\n", data1, PC + (char)data2);

		break;

		// 0xC0-0xCF
	case 0xC0: // PUSH
		fprintf(targetFile, "PUSH %03XH\n", data1);

		break;
	case 0xC1: // AJMP
		fprintf(targetFile, "AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0xC2: // CLR bit
		fprintf(targetFile, "CLR %03XH\n", data1);

		break;
	case 0xC3: // CLR C
		fprintf(targetFile, "CLR C\n");

		break;
	case 0xC4: // SWAP A
		fprintf(targetFile, "SWAP A\n");

		break;
	case 0xC5: // XCH dir
		fprintf(targetFile, "XCH A, %03XH\n", data1);

		break;
	case 0xC6: // XCH @R0
		fprintf(targetFile, "XCH A, @R0\n");

		break;
	case 0xC7: // XCH @R1
		fprintf(targetFile, "XCH A, @R1\n");

		break;
	case 0xC8: // XCH R0
		fprintf(targetFile, "XCH A, R0\n");

		break;
	case 0xC9: // XCH R1
		fprintf(targetFile, "XCH A, R1\n");

		break;
	case 0xCA: // XCH R2
		fprintf(targetFile, "XCH A, R2\n");

		break;
	case 0xCB: // XCH R3
		fprintf(targetFile, "XCH A, R3\n");

		break;
	case 0xCC: // XCH R4
		fprintf(targetFile, "XCH A, R4\n");

		break;
	case 0xCD: // XCH R5
		fprintf(targetFile, "XCH A, R5\n");

		break;
	case 0xCE: // XCH R6
		fprintf(targetFile, "XCH A, R6\n");

		break;
	case 0xCF: // XCH R7
		fprintf(targetFile, "XCH A, R7\n");

		break;

		// 0xD0-0xDF
	case 0xD0: // POP
		fprintf(targetFile, "POP %03XH\n", data1);

		break;
	case 0xD1: // ACALL
		fprintf(targetFile, "ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0xD2: // SETB bit
		fprintf(targetFile, "SETB %03XH\n", data1);

		break;
	case 0xD3: // SETB C
		fprintf(targetFile, "SETB C\n");

		break;
	case 0xD4: // DA A
		fprintf(targetFile, "DA A\n");

		break;
	case 0xD5: // DJNZ dir, label
		fprintf(targetFile, "DJNZ %03XH, %05XH\n", data1, PC + (char)data2);

		break;
	case 0xD6: // XCHD A, @R0
		fprintf(targetFile, "XCHD A, @R0\n");

		break;
	case 0xD7: // XCHD A, @R1
		fprintf(targetFile, "XCHD A, @R1\n");

		break;
	case 0xD8: // DJNZ R0, label
		fprintf(targetFile, "DJNZ R0, %05XH\n", PC + (char)data1);

		break;
	case 0xD9: // DJNZ R1, label
		fprintf(targetFile, "DJNZ R1, %05XH\n", PC + (char)data1);

		break;
	case 0xDA: // DJNZ R2, label
		fprintf(targetFile, "DJNZ R2, %05XH\n", PC + (char)data1);

		break;
	case 0xDB: // DJNZ R3, label
		if (data1 > 0x80)
		fprintf(targetFile, "DJNZ R3, %05XH\n", PC + (char)data1);

		break;
	case 0xDC: // DJNZ R4, label
		fprintf(targetFile, "DJNZ R4, %05XH\n", PC + (char)data1);

		break;
	case 0xDD: // DJNZ R5, label
		fprintf(targetFile, "DJNZ R5, %05XH\n", PC + (char)data1);

		break;
	case 0xDE:// DJNZ R6, label
		fprintf(targetFile, "DJNZ R6, %05XH\n", PC + (char)data1);

		break;
	case 0xDF: // DJNZ R7, label
		fprintf(targetFile, "DJNZ R7, %05XH\n", PC + (char)data1);

		break;

		// 0xE0-0xEF
	case 0xE0: // MOVX A, @DPTR
		fprintf(targetFile, "MOVX A, @DPTR\n");

		break;
	case 0xE1: // AJMP
		fprintf(targetFile, "AJMP %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0xE2: // MOVX A, @R0
		fprintf(targetFile, "MOVX A, @R0\n");

		break;
	case 0xE3: // MOVX A, @R1
		fprintf(targetFile, "MOVX A, @R1\n");

		break;
	case 0xE4: // CLR A
		fprintf(targetFile, "CLR A\n");

		break;
	case 0xE5: // MOV A, dir
		fprintf(targetFile, "MOV A, %03XH\n", data1);

		break;
	case 0xE6: // MOV A, @R0
		fprintf(targetFile, "MOV A, @R0\n");

		break;
	case 0xE7: // MOV A, @R1
		fprintf(targetFile, "MOV A, @R1\n");

		break;
	case 0xE8: // MOV A, R0
		fprintf(targetFile, "MOV A, R0\n");

		break;
	case 0xE9: // MOV A, R1
		fprintf(targetFile, "MOV A, R1\n");

		break;
	case 0xEA: // MOV A, R2
		fprintf(targetFile, "MOV A, R2\n");

		break;
	case 0xEB: // MOV A, R3
		fprintf(targetFile, "MOV A, R3\n");

		break;
	case 0xEC: // MOV A, R4
		fprintf(targetFile, "MOV A, R4\n");

		break;
	case 0xED: // MOV A, R5
		fprintf(targetFile, "MOV A, R5\n");

		break;
	case 0xEE: // MOV A, R6
		fprintf(targetFile, "MOV A, R6\n");

		break;
	case 0xEF: // MOV A, R7
		fprintf(targetFile, "MOV A, R7\n");

		break;

		// 0xF0-OxFF
	case 0xF0: // MOVX @DPTR, A
		fprintf(targetFile, "MOVX @DPTR, A\n");

		break;
	case 0xF1: // ACALL
		fprintf(targetFile, "ACALL %05XH\n", (PC / 0x800) * 0x800 + 0x100 * (code / 0x20) + data1);

		break;
	case 0xF2: // MOVX @R0, A
		fprintf(targetFile, "MOVX @R0, A\n");

		break;
	case 0xF3: // MOVX @R1, A
		fprintf(targetFile, "MOVX @R1, A\n");

		break;
	case 0xF4: // CPL A
		fprintf(targetFile, "CPL A\n");

		break;
	case 0xF5: // MOV dir, A
		fprintf(targetFile, "MOV %03XH, A\n", data1);

		break;
	case 0xF6: // MOV @R0, A
		fprintf(targetFile, "MOV @R0, A\n");

		break;
	case 0xF7: // MOV @R1, A
		fprintf(targetFile, "MOV @R1, A\n");

		break;
	case 0xF8: // MOV R0, A
		fprintf(targetFile, "MOV R0, A\n");

		break;
	case 0xF9: // MOV R1, A
		fprintf(targetFile, "MOV R1, A\n");

		break;
	case 0xFA: // MOV R2, A
		fprintf(targetFile, "MOV R2, A\n");

		break;
	case 0xFB: // MOV R3, A
		fprintf(targetFile, "MOV R3, A\n");

		break;
	case 0xFC: // MOV R4, A
		fprintf(targetFile, "MOV R4, A\n");

		break;
	case 0xFD: // MOV R5, A
		fprintf(targetFile, "MOV R5, A\n");

		break;
	case 0xFE: // MOV R6, A
		fprintf(targetFile, "MOV R6, A\n");

		break;
	case 0xFF: // MOV R7, A
		fprintf(targetFile, "MOV R7, A\n");

		break;
	}

	fclose(targetFile);
	return;
}


/* RunProgram() 함수
*
* 기능 : 프로그램을 실행한다.
* 입력 변수 : fileName(출력 파일의 위치와 이름), end_PC(.hex 파일에서 마지막 Program Counter의 위치)
* 출력 변수 없음
*/
void RunProgram(char* fileName, int end_PC)
{
	// Program Counter
	unsigned short PC = 0; // 0xFFFF 이후 오버플로우 발생하도록 unsigned short로 만듬
	unsigned short prev_PC = 0;

	// 이번에 실행하는 명령어
	unsigned char tmp_Code;

	// 추가 데이터와 명령어 길이
	unsigned char dat1=0, dat2=0, bytes=1;

	// 종료 여부 확인 변수
	int isEnd = 0;

	// ORG 출력여부 변수
	int isOrg = 1;

	FILE* target;

	// 파일 초기화
	target = fopen(fileName, "w");
	if (target == NULL)
	{
		printf("Output Error at Clearing Target File!\n");
		exit(1);
	}
	fclose(target);

	while (!isEnd)
	{
		// 실행할 명령 코드를 가져온 후, PC값 1 증가
		tmp_Code = ROM[PC];
		PC++;

		if (PC == 0) { isEnd = 1; }

		// ORG 출력 관련, 기존 Nemonic이 NOP이고, 신규 Nemonic이 NOP가 아닌 경우 출력
		if (tmp_Code == 0x00)
		{
			isOrg = 1;
			// 다음 입력으로 넘어가기

		}else{
			if (isOrg)
			{
				target = fopen(fileName, "a");
				if (target == NULL)
				{
					printf("Output Error on Writing File!\n");
					exit(1);
				}
				fprintf(target, "ORG %05XH\n", PC - 1);
				fclose(target);
			}
			isOrg = 0;


			// 명령어 길이 계산
			bytes = 1;
			for(int i = 0; i < 91; i++) // 2byte 명령어인지 확인
			{
				if (TWO_BYTES[i] == tmp_Code)
				{
					bytes = 2;
					dat1 = ROM[PC];
					PC++;
					break;
				}
			}

			if (PC == 0) { isEnd = 1; }

			for(int i = 0; i < 24; i++) // 3byte 명령어인지 확인
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

			if (PC == 0) { isEnd = 1; }

			// 프로그램 실행
			programRunner(fileName, tmp_Code, dat1, dat2, PC); // 해당 명령어 실행

		}

		if (PC > end_PC)
		{
			isEnd = 1;
		}

	}


	// END 출력
	target = fopen(fileName, "a");
	if (target == NULL)
	{
		printf("Output Error at Finish Writing File!\n");
		exit(1);
	}
	fprintf(target, "END\n");
	fclose(target);

	printf("Output Completed at %s\n", fileName);
	return;
}


int main(int argc, char* argv[])
{
	// 순서대로 eof를 저장하는 변수와, 읽어들일 파일명을 저장하는 변수 선언
	int eof = 0, isFirst0 = 1;
	char fileName[256] = "", targetFileName[256] = "";

	// 안내문구 출력
	printf("8051 Disassembler by YHC03\n\n");

	// 파일명에 공백이 있는 경우를 처리한다.
	for (int i = 1; i < argc; i++)
	{
		strcat(fileName, argv[i]);
		strcat(fileName, " ");
	}
	
	isFirst0 = 1;
	for (int i = strlen(fileName); i >= 0; i--)
	{
		targetFileName[i] = fileName[i];

		// Target 파일은 .a51로 변경
		if (fileName[i] == '.' && isFirst0)
		{
			targetFileName[i + 1] = 'a';
			targetFileName[i + 2] = '5';
			targetFileName[i + 3] = '1';
			targetFileName[i + 4] = '\0';
			isFirst0 = 0;
		}
	}

	// 파일을 추가하지 않은 경우
	if (argc == 1)
	{
		printf("No File Included!\n");
		exit(1);
	}


	// 파일 읽기
	eof = fileReader(fileName);
	
	// 프로그램 실행
	RunProgram(targetFileName, eof);

	return 0;
}