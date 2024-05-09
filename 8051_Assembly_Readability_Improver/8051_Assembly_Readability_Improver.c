#include<stdio.h>
#include<string.h>


// 문자열 초기화 함수(targ 문자열을 len 길이만큼 \0으로 초기화한다
void initChar(char* targ, int len)
{
	for (int i = 0; i < len; i++)
	{
		targ[i] = '\0';
	}

	return;
}


/* process() 함수
* 
* 기능 : 주어진 어셈블리어 파일의 다음과 같은 조정을 하고, (해당 파일 주소와 이름)_Processd.(해당 파일의 확장자)에 저장한다.
* 1) 공백(탭)이 있으면 공백 하나로 바꾸기(다음 단어 발견 시 공백도 함께 작성)
* 1-1) 가장 처음부터 공백인 경우, 공백 삭제
* 1-2) :(Colon)이 가장 처음 나온 경우, 공백 삭제(두번 이상 나온 경우(잘못된 코드), 공백 추가)
* 2) 소문자 알파벳을 대문자로 바꾸기 (주석 제외)
* 3) , 뒤에 공백 없는 경우 공백 생성
* 4) Assembly에서 주석 표기를 하는 ;(Semicolon) 뒤에 공백이 최소 1개 이상 있도록 변경
* 4-1) 주석 뒤에 공백 유무와 관계없이, 주석의 내용 그대로 출력되는 코드를 해당 소스 코드 위에 주석으로 남겨두었음
* 5) 공백만 있는 줄의 공백 제거
* 
* 입력 변수 : 입력할 파일의 이름
* 출력 변수 : 입출력 오류 여부 (1이면 오류, 0이면 정상)
*/
int process(char* fileName)
{
	FILE *fileRead, *fileWrite; // 각각 입력 파일과 출력 파일
	char lineTmp[4096] = "", lineWrite[255] = ""; // 각각 읽을 문장과 저장할 문장

	/* 변수 설명
	* readCurr : 읽는 위치
	* writeCurr : 쓰는 위치
	* isBlank : 해당 줄이 공백인지 확인
	* isPrevBlank : 이전에 공백이 나타났는지 확인
	* isColonAppeared : 해당 줄에 :이 나타났는지 확인
	* operandNo : 나타난 명령어의 개수
	* prevLetter : 이전에 나타난 공백이 아닌 문자
	*/
	int readCurr, writeCurr, isBlank, isPrevBlank, isColonAppeared, operandNo;
	char prevLetter = ' ';

	fileRead = fopen(fileName, "r");
	if (fileRead == NULL)
	{
		printf("File Not Found!\n");
		return 1;
	}

	// 출력 파일명은 입력 파일명 뒤에 _Processed 붙이기
	char writeName[1024] = "";
	strncpy(writeName, fileName, strlen(fileName) - 5);
	strcat(writeName, "_Processed");
	strcat(writeName, fileName + (strlen(fileName) - 5));

	fileWrite = fopen(writeName, "w");
	if (fileWrite == NULL)
	{
		printf("File Output Error!\n");
		return 1;
	}

	while (!feof(fileRead))
	{
		// 읽기 초기화 후, 문장 읽어들이기
		initChar(lineTmp, 4096);
		fgets(lineTmp, 4096, fileRead);

		if (feof(fileRead)) { break; } // 끝에 공백 한줄 추가되는 것을 방지함

		// 쓰기 초기화
		initChar(lineWrite, 255);

		// 변수 초기화
		isBlank = 1; isPrevBlank = 0; isColonAppeared = 0; operandNo = 0;
		writeCurr = 0;

		// 해당 문장 끝까지 반복
		for (readCurr = 0; readCurr < strlen(lineTmp); readCurr++)
		{
			if (lineTmp[readCurr] == ';') // 주석 발견 시
			{
				if(operandNo != 0) // 앞에 명령어가 있는 경우
					strcat(lineWrite, " "); // 공백 추가


				/* 아래 경우는, 단순히 주석 이후의 문자열을 그대로 복사하는 경우이다. */
				/*
				strncat(lineWrite, lineTmp + readCurr, strlen(lineTmp) - readCurr - 1); // 주석은 개행문자 빼고 그대로 출력
				*/

				/* 아래 경우는, Assembly에서 주석 표기를 하는 ;(Semicolon) 뒤에 공백이 하나 이상 있도록 설정하는 경우이다. */
				strcat(lineWrite, ";"); // 주석 표시 추가
				if (lineTmp[readCurr + 1] != ' ' && lineTmp[readCurr + 1] != '\r' && lineTmp[readCurr + 1] != '\n') // 주석 뒤에 공백이나 탭이 없거나, 해당 줄이 주석 시작으로 종료되지 않은 경우
				{
					strcat(lineWrite, " "); // 공백 추가
				}
				strncat(lineWrite, lineTmp + readCurr + 1, strlen(lineTmp) - readCurr - 2); // 주석은 개행문자 빼고 그대로 출력
				/**/

				readCurr = strlen(lineTmp);
				isBlank = 0;

			}else if (lineTmp[readCurr] >= 'a' && lineTmp[readCurr] <= 'z'){ // 소문자 발견 시

				if (isPrevBlank && prevLetter != ',')
				{
					if (operandNo) // Label이나 Nemonic인 경우 공백 1개만 추가
						lineWrite[writeCurr++] = ' ';

					operandNo++;
				}

				isBlank = 0;
				isPrevBlank = 0;

				if (!readCurr) { // 첫번째 글자인 경우
					operandNo++; // 명령어/Label 존재 확인
				}
				if (prevLetter == ',' && lineTmp[readCurr] != ' ') { lineWrite[writeCurr++] = ' '; }

				lineWrite[writeCurr++] = lineTmp[readCurr] - ('a' - 'A'); // 대문자로 변환해 저장
				prevLetter = lineTmp[readCurr] - ('a' - 'A');

			}else{ // 그 외
				if(lineTmp[readCurr] == ' ' || lineTmp[readCurr] == '\t' || lineTmp[readCurr] == '\n') // 공백이 나타난 경우
				{
					isPrevBlank = 1;
				}else{ // 공백이 나타나지 않은 경우
					
					if (isPrevBlank) // 이전에 공백이 있었던 경우
					{
						if (lineTmp[readCurr] == ':') // Colon이 나타난 경우
						{
							if (isColonAppeared) // Colon이 한번 이상 나타난 경우(입력된 코드의 오류)
							{
								lineWrite[writeCurr++] = ' ';
							} // Colon이 한번 이상 나타나지 않은 경우에는 공백을 출력하지 않음

							isColonAppeared = 1;

						}else if(operandNo){ // Colon이 한번도 나타나지 않은 경우면서, 첫 문장이 아닌 경우
							if (prevLetter != '#' && prevLetter != '@')
							{
								if(prevLetter != ',') // ,의 경우 나중에 처리
									lineWrite[writeCurr++] = ' ';

							}
						}

						operandNo++; // 명령어/Label 존재 확인

					}else if (!readCurr){ // 첫번째 글자인 경우

						operandNo++; // 명령어/Label 존재 확인
					}

					// 공백 아님 처리
					isBlank = 0;
					isPrevBlank = 0;
					if (lineTmp[readCurr] == ':'){ isColonAppeared = 1; } // Colon이 나타났다고 표기

					if (prevLetter == ',' && lineTmp[readCurr] != ' '){ lineWrite[writeCurr++] = ' '; }

					lineWrite[writeCurr++] = lineTmp[readCurr]; // 그대로 작성
					prevLetter = lineTmp[readCurr];
				}
			}

		}


		if (strcmp(lineWrite, "") && !isBlank) // 빈 줄이 아닌 경우
		{
			fputs(lineWrite, fileWrite);
			fputs("\n", fileWrite);
		}else{
			fputs("\n", fileWrite);
		}
	}


	fclose(fileRead);
	fclose(fileWrite);

	return 0;
}


int main(int argc, char* argv[])
{
	char fileName[1024] = "";

	// 안내문구 출력
	printf("8051 Assembly Code Readability Improver by YHC03\n\n");

	if (argc == 1)
	{
		printf("No File Included!\n");
		return 0;
	}

	for (int i = 1; i < argc; i++)
	{
		strcat(fileName, argv[i]);
		strcat(fileName, " ");
	}
	if(!process(fileName))
		printf("Process Finished\n");

	return 0;
}