#pragma once

// Part of 8051 Simulator

/* RunFunction.h
* 8051 시뮬레이터의 Main 실행을 관할한다.(대부분의 값 수정은 RunSpecific.h에서 진행한다.)
*/

#ifndef _RUN_FUNCTION_H_
#define _RUN_FUNCTION_H_

#include<stdio.h>
#include<math.h>
#include"8051_Variables.h" // 8051의 각종 상수와, RAM, ROM 변수를 저장함
#include"RunSpecific.h" // 8051 Simulator의 특정 연산 명령을 수행함
#include"Timer_and_Interrupt.h" // 8051의 Timer와 Interrupt 연산을 수행함


// 프로그램 구동
void RunProgram(unsigned char mode, int end_PC);
int programRunner(unsigned char code, unsigned char data1, unsigned char data2, unsigned short PC, char isDebugMode);

// 출력
void printChip(unsigned long long int cycle, int programCounter);

// 입력 보조
void inputDat();

// Port Input 가져오기
void getPortValue();


#endif