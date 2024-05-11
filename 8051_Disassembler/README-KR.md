# 8051 Disassember

[English](https://github.com/YHC03/8051_Tools/blob/main/8051_Disassembler/README.md)  

Freeware  

#### 이용 언어
C

###### 이용 라이브러리(헤더 파일)
stdio.h: stdin, stdout 이용과 File 입출력을 위함  
stdlib.h: exit() 함수를 사용하기 위함  
string.h: 읽어들일 File 주소와 이름에 공백이 있는 경우를 처리하기 위함  

---
## 작동 방법
.exe 파일 생성 후, "(파일명).exe" "(HEX 파일 위치).hex" 명령어로 프로그램을 실행한다.  
파일이 정상적으로 Load되지 않는 경우, 오류가 나오며 프로그램이 종료된다.  
HEX 파일의 Parity에서 오류가 발생하는 경우, Parity Error의 위치를 알려주며 프로그램이 종료한다.  
이 외의 정상적인 경우에는, (HEX 파일 위치).hex 파일 위치에 결과가 (HEX 파일 위치).a51 파일로 생성된다.  

---
## 제작 과정에서 참고, 이용한 자료
[같은 Repository에 있는 8051 Simulator](https://github.com/YHC03/8051_Tools/tree/main/8051_Simulator)  

---
작성자 : YHC03  
최종 작성일 : 2024/05/11  