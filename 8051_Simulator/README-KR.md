# 8051 CLI Simulator

[English](https://github.com/YHC03/8051_Tools/blob/main/8051_Simulator/README.md)  

Freeware  

#### 이용 언어
C

###### 이용 라이브러리(헤더 파일)
stdio.h: stdin, stdout 이용과 File 입출력을 위함  
stdlib.h: exit() 함수를 사용하기 위함  
string.h: 읽어들일 File 주소와 이름에 공백이 있는 경우를 처리하기 위함  
math.h: pow() 함수를 사용하기 위함  
windows.h: system("PAUSE"), system("cls") 함수를 사용하기 위함  

---
## 세부 파일 소개
8051_Variables: 8051 Chip 관련 상수와, Chip의 RAM, ROM 전역 변수를 저장함   
RunFunction: 8051 Chip의 명령어를 읽어들어 실행함  
RunSpecific: RunFunction에서 요청한 명령들의 함수를 실제로 실행함  
Timer_and_Interrupt: 8051 Timer와 Interrupt를 실행함  
FileInput: 실행할 .hex 파일을 읽어들임  
8051_Simulator: 전체 프로그램을 관리하고 실행함  

---
## 작동 방법
모든 Source 및 Header 파일을 Build해 .exe 파일 생성 후, "(파일명).exe" "(HEX 파일 위치).hex" 명령어로 프로그램을 실행한다.  
파일이 정상적으로 Load되지 않는 경우, 오류가 나오며 프로그램이 종료된다.  
HEX 파일의 Parity에서 오류가 발생하는 경우, Parity Error의 위치를 알려주며 프로그램이 종료한다.  

---
## 작동 설명

### 1. 초기 화면
0 선택 시, 단계별 실행(디버그)  
1 선택 시, Program Counter가 .hex 파일의 종료 지점의 Program Counter의 이전까지(Interrupt Enable 여부와는 무관하다), 혹은 Program Counter Overflow 발생시까지 계속 실행 후, 결과 출력, 이후 프로그램 종료  

---
### 2. 디버그 화면
Enter 입력시마다, 자동으로 한 문장씩 실행함  
다음으로 실행될 문장과, 8051의 메모리 정보(PSW, TCON은 bit 단위로도), P0-P3 Port의 Latch 정보를 출력함  

---
## 구현한 기능
- HEX File의 Parity를 확인하는 기능  
- MOVX를 제외한 모든 8051의 명령어  
- Timer/Counter, Interrupt 기능  
- P0-P3 Port의 Latch에 저장된 데이터 조회 기능  
- Port 입력 기능 (특정 Port만을 수정할 수는 없음. 다만, 기존값을 그대로 입력하는 것은 가능함.)  

##### 이용 불가 기능
- Serial 관련 기능 이용 불가(Interrupt Routine또한 구현되지 않음)  
- 외부 메모리 이용 불가  

##### 주의사항
inputDat() 함수에서 getPortValue() 함수 호출 시, P0 Port에는 외부에 Pull-Up 저항이 연결되어 있다고 가정한다.  
SBUF 레지스터는 입력용 레지스터와 출력용 레지스터가 구분되어 있는데, 출력되는 Memory Map에서의 I/O는 (8051으로의 입력, 8051에서의 출력)을 뜻한다.  

---
## 기획 의도
대학교 3학년 1학기 마이크로프로세서및HDL 과목에서 배우는 8051 칩의 Simulator를 구현해보고자 하였다.  

---
## 제작 과정에서 참고한 자료
8051 Instruction Set  
홍익대학교 마이크로프로세서및HDL 과목(2024학년도 1학기) 강의록  

---
작성자 : YHC03  
최종 작성일 : 2024/05/11  