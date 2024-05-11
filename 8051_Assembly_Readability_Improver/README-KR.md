# 8051 Assembly Code Readability Improver

[English](https://github.com/YHC03/8051_Tools/blob/main/8051_Assembly_Readability_Improver/README.md)  

Freeware  

#### 이용 언어
C

###### 이용 라이브러리(헤더 파일)
stdio.h: stdin, stdout 이용과 File 입출력을 위함  
string.h: 읽어들일 File 주소와 이름에 공백이 있는 경우를 처리하기 위함  

---
## 작동 방법
.exe 파일 생성 후, "(파일명).exe" "(Assembly 파일 위치).a51" 명령어로 프로그램을 실행한다.  
파일이 정상적으로 Load되지 않는 경우, 오류가 나오며 프로그램이 종료된다.  
이 외의 정상적인 경우에는, 입력 파일 위치에 출력이 입력 파일명에 _Processd가 붙은 파일로 생성된다.  

## 기능
1. 공백(탭)이 있으면 공백 하나로 바꾸기(다음 단어 발견 시 공백도 함께 작성)  
- 가장 처음부터 공백인 경우, 공백 삭제  
- :(Colon)이 가장 처음 나온 경우, Colon 앞 공백 삭제  
  
2. 소문자 알파벳을 대문자로 바꾸기 (주석 제외)  
3. , 뒤에 공백 없는 경우 공백 생성  
4. Assembly에서 주석 표기를 하는 ;(Semicolon) 뒤에 공백이 최소 1개 이상 있도록 변경  
- 주석 뒤에 공백 유무와 관계없이, 주석의 내용 그대로 출력되는 코드를 해당 소스 코드 위에 주석으로 남겨두었음  

5. 공백만 있는 줄의 공백 제거(해당 줄을 완전히 삭제하지는 않음)  

---
작성자 : YHC03  
최종 작성일 : 2024/05/11  