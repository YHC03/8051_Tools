#pragma once

// Part of 8051 Simulator

/* RunSpecific.h
* 8051 시뮬레이터의 RunFunction.h에 정의된 함수에서 연산을 하는 세부 함수를 호출해 대부분의 값 수정을 관할한다.
*/

#ifndef _RUN_SPECIFIC_H_
#define _RUN_SPECIFIC_H_

#include<stdio.h>
#include<math.h>
#include"8051_Variables.h" // 8051의 각종 상수와, RAM, ROM 변수를 저장함

// 8051 내부 함수 구현


// Bit 연산
char getBitAddr(unsigned char location); // Bit값 가져오기
void setBitAddr(unsigned char location); // SETB
void clearBitAddr(unsigned char location); // CLR

// 명령 이후의 처리
void putParity(); // ACC의 Parity
void syncLatch(char port); // Latch 연동

// 가.감 연산
void incFunc(short dest); // INC (DPTR 제외)
void decFunc(short dest); // DEC
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
void swapOperation(unsigned char src); // XCH
void halfSwapOperation(unsigned char src); // XCHD

// DA 함수
void DAOperation();


#endif