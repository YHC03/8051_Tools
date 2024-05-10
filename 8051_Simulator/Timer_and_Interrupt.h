#pragma once

// Part of 8051 Simulator

/* Timer_and_Interrupt.h
* 8051 시뮬레이터의 Timer, Interrupt 연산을 관할한다.
*/

#ifndef _TIMER_AND_INTERRUPT_H_
#define _TIMER_AND_INTERRUPT_H_

#include"8051_Variables.h"
#include"RunSpecific.h"

// Timer/Counter
void timerControl(int cycle);

// Interrupt Control
int interruptControl(int PC);
char getInterruptPriorityRun();
void clearInterrupt();


#endif