#include"FileInput.h"


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
	unsigned char tmp_Data[300], parity = 0;
	int curr_PC = 0, tmp_PC = 0, prev_PC = 0, tmp_REM = 0, prev_REM = 0, isEnd = 0, lineNo = 0;
	while (!isEnd) // 종료 표시가 있을때까지 반복
	{
		// Parity를 초기화하고
		parity = 0;

		// 데이터 한줄을 읽어들인다.
		fscanf(hexFile, "%s", tmp_Data);
		
		// 이전값이 최댓값을 넘어간 경우, 그 값을 저장 후
		if (prev_PC < tmp_PC)
		{
			prev_PC = tmp_PC;
			prev_REM = tmp_REM;
		}

		// 해당 줄의 시작이 :가 아닌 경우를 확인하고
		if (tmp_Data[0] != ':')
		{
			// 줄의 시작이 :가 아니므로 파일 형식 오류를 출력한다.
			printf("File Type Error at line #%d!\n", lineNo + 1);
			exit(1);
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
			// Parity 오류 발견 시, Parity 오류를 발견한 줄의 순서를 출력한다.
			printf("Parity Error at Line #%d!\n", lineNo + 1);
			exit(1);
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