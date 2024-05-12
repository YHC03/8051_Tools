#pragma once

// Part of 8051 Simulator

/* 8051_Variables.h
* 8051의 상수와 전역변수 RAM, ROM을 저장한다
*/

#ifndef _8051_VARIABLES_H_
#define _8051_VARIABLES_H_

#include<windows.h> // 다른 위치에서 include시 오류가 발생하는 경우가 있음

typedef struct
{
	unsigned char internal_RAM[256]; // Special Function Register, SBUF의 수신(8051 기준 Read) Register 포함
	unsigned char latch[4];
	unsigned char SBUF_send; // SBUF의 송신(8051 기준 Write) Register
}Chip;

// RAM, ROM 선언
extern Chip chip;
extern unsigned char ROM[65535];

// Interrupt 관련 변수 선언
extern char intData[4];

// 특수 명령어 Bytes
extern const unsigned char TWO_BYTES[91];
extern const unsigned char THREE_BYTES[24];

// 특수 명령어 Cycle
extern const unsigned char TWO_CYCLE[90];
extern const unsigned char FOUR_CYCLE[2]; 

// Special Function Register
#define ACC 0xE0
#define PSW 0xD0
#define SP 0x81
#define DPH 0x83
#define DPL 0x82
#define TCON 0x88
#define TMOD 0x89
#define TL0 0x8A
#define TL1 0x8B
#define TH0 0x8C
#define TH1 0x8D
#define SBUF 0x99
#define IE 0xA8
#define IP 0xB8

// Special Bit Address
#define C 0xD7 // Carry Bit
#define AC 0xD6
#define PSW1 0xD4
#define PSW0 0xD3
#define OV 0xD2
#define P 0xD0
#define IT0 0x88
#define IE0 0x89
#define IT1 0x8A
#define IE1 0x8B
#define TR0 0x8C
#define TR1 0x8E
#define TF0 0x8D
#define TF1 0x8F
#define EA 0xAF
#define ES 0xAC
#define ET1 0xAB
#define EX1 0xAA
#define ET0 0xA9
#define EX0 0xA8
#define PS 0xBC
#define PT1 0xBB
#define PX1 0xBA
#define PT0 0xB9
#define PX0 0xB8

#endif