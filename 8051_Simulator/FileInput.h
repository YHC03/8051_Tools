#pragma once

// Part of 8051 Simulator

/* FileInput.h
* 8051 시뮬레이터의 데이터 파일 입력 및 저장을 관할한다.
*/

#ifndef _FILE_INPUT_H_
#define _FILE_INPUT_H_

#include<stdio.h>
#include<stdlib.h>
#include"8051_Variables.h" // 8051의 각종 상수와, RAM, ROM 변수를 저장함

// 파일 입력
int fileReader(char* fileName);
unsigned char asciiToHEX(unsigned char orig); // HEX 변환


#endif