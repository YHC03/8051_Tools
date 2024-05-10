#include"Timer_and_Interrupt.h"


/* timerControl() 함수
*
* 기능 : timer 기능 작동
* 입력 변수 : 이번 명령어 실행에서 지나간 Cycle(현재 Cycle - 이전 Cycle 의 값)
* 출력 변수 없음
*
* 버그 : C/T 활성화 시, clock 감지 불가 오류 있음
*/
void timerControl(int cycle)
{
	// 외부 Clock (Rising Edge) 관련 변수 선언
	static char T0 = 0;
	static char T1 = 0;

	// TRO 활성화시
	if (getBitAddr(TR0))
	{
		// GATE=0 혹은 GATE=1이면서 INT0=1
		if (!(chip.internal_RAM[TMOD] & 0x08) || getBitAddr(0xB2)/*P3.3*/)
		{
			// C/T=1이면서 T0=(1->0) 혹은 C/T=0
			if ((!getBitAddr(0xA4/*P3.4*/) && T0) || !(chip.internal_RAM[TMOD] & 0x04))
			{
				// T0값 저장
				T0 = getBitAddr(0xA4/*P3.4*/);

				// Timer Mode 확인 후 값 증가
				switch (chip.internal_RAM[TMOD] & 0x03)
				{
				case 0: // 2^13
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					// Timer Mode 0은 TL0가 0x20 이상이면 TH0의 값을 1 증가시키며, TH0 Overflow 발생 시 TF0을 1로 바꾼다.
					if (chip.internal_RAM[TL0] + (!(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1) >= 0x20)
					{
						// TH0 1 증가
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
						chip.internal_RAM[TL0] -= 0x20;
						chip.internal_RAM[TH0]++;

						// 2^13(TL0: 2^5 -> TH0: 2^8) 초과 시
						if (chip.internal_RAM[TH0] == 0)
						{
							// Timer0 초기화 후, TF0를 1로 설정
							chip.internal_RAM[TH0] = 0;
							setBitAddr(TF0);
						}
					}
					else {
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
					}
					break;

				case 1: // 2^16
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					if (chip.internal_RAM[TL0] + (!(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1) >= 0x100)
					{
						// TH0 1 증가 및 TL0 초기화(기존값에 Cycle만큼 더한 값에서 0x20 빼기)
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
						chip.internal_RAM[TH0]++;

						// 2^16 초과 시
						if (chip.internal_RAM[TH0] == 0)
						{
							// Timer0 초기화 후, TF0를 1로 설정
							setBitAddr(TF0);
						}
					}
					else {
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
					}
					break;

				case 2: // 2^8 setup
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					if (chip.internal_RAM[TL0] + (!(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1) >= 0x100)
					{
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;

						// (Overflow가 발생한) TL0(Timer변수)에 TH0(기존값 저장한 변수) 더하기
						chip.internal_RAM[TL0] += chip.internal_RAM[TH0];

						// Timer0 초기화 후, TF0를 1로 설정
						setBitAddr(TF0);
					}
					else {
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
					}
					break;

				case 3: // 2 * 2^8
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter). TH0(무조건 Timer)에 대해서는 뒤에서 처리
					if (chip.internal_RAM[TL0] + (!(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1) >= 0x100)
					{
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;

						// Timer0(TL0) 초기화 후, TF0를 1로 설정
						setBitAddr(TF0);
					}
					else {
						chip.internal_RAM[TL0] += !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1;
					}
					break;
				}
			}
			// T0값 저장
			T0 = getBitAddr(0xA4/*P3.4*/);
		}
		if ((chip.internal_RAM[TMOD] & 0x03) == 0x03) // TMOD=0x03에서의 2번째 Timer는 무조건 시간에 의해서만 작동함
		{
			// Timer3 TH0 Overflow 발생 시
			if (chip.internal_RAM[TH0] + cycle >= 0x100)
			{
				// Timer0(TH0) 초기화 후, TF1를 1로 설정
				chip.internal_RAM[TH0] += cycle;
				setBitAddr(TF1);
			}
			else {
				chip.internal_RAM[TH0] += cycle;
			}
		}
	}

	// TR1 활성화시
	if (getBitAddr(TR1))
	{
		// GATE=0 혹은 GATE=1이면서 INT1=1
		if (!(chip.internal_RAM[TMOD] & 0x80) || getBitAddr(0xB2)/*P3.3*/)
		{
			// C/T=1이면서 T1=(1->0) 혹은 C/T=0
			if ((!getBitAddr(0xA5/*P3.5*/) && T1) || !(chip.internal_RAM[TMOD] & 0x40))
			{
				// T1값 저장
				T1 = getBitAddr(0xA5/*P3.5*/);

				// Timer Mode 확인 후 값 증가
				switch (chip.internal_RAM[TMOD] & 0x30)
				{
				case 0x00: // 2^13
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					// Timer Mode 0은 TL1가 0x20 이상이면 TH1의 값을 1 증가시키며, TH1 Overflow 발생 시 TF1을 1로 바꾼다.
					if (chip.internal_RAM[TL1] + (!(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1) >= 0x20)
					{
						// TH1 1 증가 및 TL1 초기화(기존값에 Cycle만큼 더한 값에서 0x20 빼기)
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
						chip.internal_RAM[TL1] -= 0x20;
						chip.internal_RAM[TH1]++;

						// 2^13(TL1: 2^5 -> TH1: 2^8) 초과 시
						if (chip.internal_RAM[TH1] == 0)
						{
							// Timer1 초기화 후, TF1를 1로 설정
							chip.internal_RAM[TH1] = 0;
							setBitAddr(TF1);
						}
					}
					else {
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
					}
					break;

				case 0x10: // 2^16
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)					
					if (chip.internal_RAM[TL1] + (!(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1) >= 0x100)
					{
						// TH1 1 증가
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
						chip.internal_RAM[TH1]++;

						// 2^16 초과 시
						if (chip.internal_RAM[TH1] == 0)
						{
							// Timer1 초기화 후, TF1를 1로 설정
							setBitAddr(TF1);
						}
					}
					else {
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
					}
					break;

				case 0x20: // 2^8 setup
					// !(chip.internal_RAM[TMOD] & 0x04) ? cycle : 1 는 Timer/Counter 선택이다. cycle은 Timer, 1은 외부입력(Counter)
					if (chip.internal_RAM[TL1] + (!(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1) >= 0x100)
					{
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;

						// (Overflow가 발생한) TL0(Timer변수)에 TH0(기존값 저장한 변수) 더하기
						chip.internal_RAM[TL1] += chip.internal_RAM[TH1];

						// Timer1 초기화 후, TF1를 1로 설정
						setBitAddr(TF1);
					}
					else {
						chip.internal_RAM[TL1] += !(chip.internal_RAM[TMOD] & 0x40) ? cycle : 1;
					}
					break;

				case 0x30: // STOP
					// Timer1 Mode 3. 해당 경우에는, Timer1의 작동을 중지함.
					clearBitAddr(TR1);
					break;
				}
			}
			// T1값 저장
			T1 = getBitAddr(0xA5/*P3.4*/);
		}
	}

	return;
}


/* getInterruptPriorityRun() 함수
*
* 기능 : 우선순위 순서로 활성화 할 Interrupt의 숫자를 불러온다
* 입력 변수 없음
* 출력 변수 : 활성화 할 Interrupt의 숫자
*/
char getInterruptPriorityRun()
{
	char interruptPrior = chip.internal_RAM[IP] & 0x0F;

	// Priority 설정 -> 일반 순서로 가되
	// 이미 상위 순서가 실행중인 경우 거부

	// 0, 1, 2, 3 순서로 Priority & 실행 중이 아님을 확인(실행 중인 경우 -1)
	if ((interruptPrior & 0x01) && intData[0] == 2) // Priority 설정한 Interrupt 0 실행 대기
	{
		return 0;
	}
	else if ((interruptPrior & 0x01) && intData[0] == 1) { // Priority 설정한 Interrupt 0 실행중
		return -1;

	}
	else if ((interruptPrior & 0x02) && intData[1] == 2) { // Priority 설정한 Interrupt 1 실행 대기
		clearBitAddr(TF0);
		return 1;
	}
	else if ((interruptPrior & 0x02) && intData[1] == 1) { // Priority 설정한 Interrupt 1 실행중
		return -1;

	}
	else if ((interruptPrior & 0x04) && intData[2] == 2) { // Priority 설정한 Interrupt 2 실행 대기
		return 2;
	}
	else if ((interruptPrior & 0x04) && intData[2] == 1) { // Priority 설정한 Interrupt 2 실행중
		return -1;

	}
	else if ((interruptPrior & 0x08) && intData[3] == 2) { // Priority 설정한 Interrupt 3 실행 대기
		clearBitAddr(TF1);
		return 3;
	}
	else if ((interruptPrior & 0x08) && intData[3] == 1) { // Priority 설정한 Interrupt 3 실행중
		return -1;
	}

	// 0, 1, 2, 3 순서로 ~Priority & 실행 중이 아님을 확인(실행 중인 경우 -1)
	if (!(interruptPrior & 0x01) && intData[0] == 2) // Priority 설정하지 않은 Interrupt 0 실행 대기
	{
		return 0;
	}
	else if (!(interruptPrior & 0x01) && intData[0] == 1) { // Priority 설정하지 않은 Interrupt 0 실행중
		return -1;

	}
	else if (!(interruptPrior & 0x02) && intData[1] == 2) { // Priority 설정하지 않은 Interrupt 1 실행 대기
		clearBitAddr(TF0);
		return 1;
	}
	else if (!(interruptPrior & 0x02) && intData[1] == 1) { // Priority 설정하지 않은 Interrupt 1 실행중
		return -1;

	}
	else if (!(interruptPrior & 0x04) && intData[2] == 2) { // Priority 설정하지 않은 Interrupt 2 실행 대기
		return 2;
	}
	else if (!(interruptPrior & 0x04) && intData[2] == 1) { // Priority 설정하지 않은 Interrupt 2 실행중
		return -1;

	}
	else if (!(interruptPrior & 0x08) && intData[3] == 2) { // Priority 설정하지 않은 Interrupt 3 실행 대기
		clearBitAddr(TF1);
		return 3;
	}
	else if (!(interruptPrior & 0x08) && intData[3] == 1) { // Priority 설정하지 않은 Interrupt 3 실행중
		return -1;
	}

	return -1; //No Interrupt
}


/* clearInterrupt() 함수
*
* 기능 : 우선순위 순서로 완료한 Interrput를 비활성화한다
* 입출력 변수 없음
*/
void clearInterrupt()
{
	char interruptPrior = chip.internal_RAM[IP] & 0x0F;

	// 0, 1, 2, 3 순서로 Priority & 실행 중 확인
	if ((interruptPrior & 0x01) && intData[0] == 1) // Priority 설정한 Interrupt 0 실행 종료
	{
		intData[0] = 0;
		clearBitAddr(IE0);
		return;
	}
	else if ((interruptPrior & 0x02) && intData[1] == 1) { // Priority 설정한 Interrupt 1 실행 종료
		intData[1] = 0;
		return;
	}
	else if ((interruptPrior & 0x04) && intData[2] == 1) { // Priority 설정한 Interrupt 2 실행 종료
		intData[2] = 0;
		clearBitAddr(IE1);
		return;
	}
	else if ((interruptPrior & 0x08) && intData[3] == 1) { // Priority 설정한 Interrupt 3 실행 종료
		intData[3] = 0;
		return;
	}

	// 0, 1, 2, 3 순서로 ~Priority & 실행 중 확인
	if (intData[0] == 1) // Priority 설정 안 한 Interrupt 0 실행 종료 (Priority가 설정된 경우, 위에서 처리하여 Return됨)
	{
		intData[0] = 0;
		clearBitAddr(IE0);
		return;
	}
	else if (intData[1] == 1) { // Priority 설정 안 한 Interrupt 1 실행 종료 (Priority가 설정된 경우, 위에서 처리하여 Return됨)
		intData[1] = 0;
		return;
	}
	else if (intData[2] == 1) { // Priority 설정 안 한 Interrupt 2 실행 종료 (Priority가 설정된 경우, 위에서 처리하여 Return됨)
		intData[2] = 0;
		clearBitAddr(IE1);
		return;
	}
	else if (intData[3] == 1) { // Priority 설정 안 한 Interrupt 3 실행 종료 (Priority가 설정된 경우, 위에서 처리하여 Return됨)
		intData[3] = 0;
		return;
	}

	// 그냥 RETI가 실행된 경우 ( = RET)
	return;
}


/* interruptControl() 함수
*
* 기능 : Interrupt 기능을 작동시킨다.
* 입력 변수 : 현재 Program Counter값
* 출력 변수 : 다음 Program Counter값
*/
int interruptControl(int PC)
{
	// 외부 Interrupt (Falling Edge) 관련 변수 선언
	static char INT0 = 1;
	static char INT1 = 1;

	const unsigned char INTERRUPT_PC[4] = { 0x03, 0x0B, 0x13, 0x1B }; // Serial은 지원 안함.

	// 이동할 인터럽트의 위치를 임시로 저장
	char res;

	// 전체 인터럽트 비활성화 시, 인터럽트 작동 종료
	if (!getBitAddr(EA))
	{
		// 실행 대기중(intData = 2)인 인터럽트 초기화
		// 실행중(intData = 1)인 인터럽트는 초기화하지 않음
		for (char i = 0; i < 4; i++)
		{
			if (intData[i] == 2)
			{
				intData[i] = 0;
			}
		}

		return PC;
	}


	// 외부 인터럽트 관련 설정
	if ((!getBitAddr(0xB2/*P3.2*/) && INT0) || getBitAddr(IE0)) // P3.2가 Falling Edge거나 IE0 = 1(현상 유지)일 때
	{
		setBitAddr(IE0);
	}
	else {
		clearBitAddr(IE0);
	}
	if ((!getBitAddr(0xB3/*P3.3*/) && INT1) || getBitAddr(IE1)) // P3.3이 Falling Edge거나 IE1 = 1(현상 유지)일 때
	{
		setBitAddr(IE1);
	}
	else {
		clearBitAddr(IE1);
	}


	// 각 인터럽트별로 활성화 확인
	if (getBitAddr(EX0)) // 외부 인터럽트 0
	{
		// IE0이 활성화되었으며 IT0 = 1이고 해당 인터럽트가 실행 대기나 실행 중이 아니거나, 혹은 IT0 = 0이며 P3.2가 0일 때
		if ((getBitAddr(IE0) && getBitAddr(IT0) && !intData[0]) || (!getBitAddr(0xB2/*P3.2*/) && !getBitAddr(IT0)))
		{
			intData[0] = 2;
		}

	}
	INT0 = getBitAddr(0xB2/*P3.2*/);

	if (getBitAddr(ET0)) // 타이머 인터럽트 0
	{
		if (getBitAddr(TF0))
		{
			intData[1] = 2;
		}
	}

	if (getBitAddr(EX1)) // 외부 인터럽트 1
	{
		// IE1이 활성화되었으며 IT1 = 1이고 해당 인터럽트가 실행 대기나 실행 중이 아니거나, 혹은 IT1 = 0이며 P3.3이 0일 때
		if ((getBitAddr(IE1) && getBitAddr(IT1) && !intData[2]) || (!getBitAddr(0xB3/*P3.3*/) && !getBitAddr(IT1)))
		{
			intData[2] = 2;
		}

	}
	INT1 = getBitAddr(0xB3/*P3.3*/);

	if (getBitAddr(ET1)) // 타이머 인터럽트 1
	{
		if (getBitAddr(TF1))
		{
			intData[3] = 2;
		}
	}

	// 활성화할 Interrupt가 있는지 확인
	res = getInterruptPriorityRun();


	if (res != -1) // 작동할 Interrput가 있는 경우
	{
		// Stack에 현재 PC값 추가 후
		stackOperationPC(PC % 0x100, 0);
		stackOperationPC(PC / 0x100, 0);

		// 해당 Interrupt 실행 중 표기한다.
		intData[res] = 1;

		// Interrupt 위치로 PC 이동
		return INTERRUPT_PC[res];
	}
	else { // 작동할 Interrput가 없는 경우
		// 다음 PC값 실행
		return PC;
	}
}