# 8051 Simulator

Freeware  

#### 이용 언어
C

###### 이용 라이브러리(헤더 파일)
stdio.h : stdin, stdout 이용과 File 입출력을 위함  
stdlib.h : exit() 함수를 사용하기 위함  
string.h : 읽어들일 File 주소와 이름에 공백이 있는 경우를 처리하기 위함  
math.h : pow() 함수를 사용하기 위함  
windows.h : system("PAUSE"), system("cls") 함수를 사용하기 위함  

---
## 작동 방법
.exe 파일 생성 후, "(파일명).exe" "(HEX 파일 위치).hex" 명령어로 프로그램을 실행한다.  
실행할 HEX 파일의 주소와 이름을 char* argv[]로 받는다.  
파일이 정상적으로 Load되지 않는 경우, 오류가 나오며 프로그램이 종료된다.  
HEX 파일의 Parity에서 오류가 발생하는 경우, Parity Error의 위치를 알려주며 프로그램이 종료한다.  

## 작동 설명

### 1. 초기 화면
0 선택 시, 단계별 실행(디버그)  
1 선택 시, Program Counter가 .hex 파일의 종료 지점의 Program Counter의 이전 까지(Interrupt Enable 여부와는 무관하다), 혹은 Program Counter Overflow 발생시까지 계속 실행 후, 결과 출력, 이후 프로그램 종료  

---
### 2. 디버그 화면
Enter 입력시마다, 자동으로 한 문장씩 실행  
다음으로 실행될 문장과, 8051의 메모리 정보(PSW, TCON은 bit 단위로도), P0-P3 Port의 Latch 정보를 출력함  

---
## 구현한 기능
HEX File의 Parity를 확인하는 기능  
MOVX를 제외한 모든 8051의 명령어  
Timer/Counter, Interrupt 기능  
P0-P3 Port의 Latch에 저장된 데이터 조회 기능  

##### 이용 불가 기능
Port 입력 기능 비활성화됨(inputDat() 함수에서 getPinValue() 함수 호출하는 방법 이용 가능)  
Serial 관련 기능 이용 불가(Interrupt Routine또한 구현되지 않음)  
외부 메모리 이용 불가  

##### 주의사항
inputDat() 함수에서 getPinValue() 함수 호출 시, P0 Port에는 외부에 Pull-Up 저항이 연결되어 있다고 가정한다.  

---
## 기획 의도
대학교 3학년 1학기 마이크로프로세서및HDL 과목에서 배우는 8051 칩의 Simulator를 구현해보고자 하였다.  

---
## 제작 과정에서 참고한 자료
8051 Instruction Set  

---
작성자 : YHC03  
최종 작성일 : 2024/05/06  