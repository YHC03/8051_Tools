#include"RunSpecific.h"


/* syncLatch() 함수
*
* 기능 : 8051에서 특정 Port로 값을 출력 시, Latch값도 동일하게 한다.
* 입력 변수 : port(P0, P1, P2, P3)
* 출력 변수 없음
*/
void syncLatch(char port)
{
	// Latch값을 8051 내부의 P0, P1, P2, P3 레지스터의 값으로 설정한다.
	chip.latch[port] = chip.internal_RAM[0x80 + 0x10 * port];

	return;
}


/* putParity() 함수
* 
* 기능 : ACC 레지스터를 기준으로 parity bit를 활성화한다.
* 입출력 변수 없음
*/
void putParity()
{
	// ACC값을 저장하는 변수 선언 및 초기화
	unsigned char accumulator = chip.internal_RAM[ACC];

	// ACC의 2진수값의 1의 갯수를 저장하는 변수 선언 및 초기화
	unsigned char count = 0;

	// unsigned char의 범위 내에서 1의 갯수를 센다.
	for (unsigned char i = 0; i < 8; i++)
	{
		// 해당 위치가 홀수라면, count 변수에 1을 더한다.
		count += accumulator % 2;

		// ACC값을 다음 위치로 설정한다.
		accumulator /= 2;
	}

	// ACC의 2진수값의 1의 갯수가 홀수라면 Parity를 1로, 아니라면 Parity를 0으로 설정한다.
	if (count % 2)
	{
		setBitAddr(P);
	}else{
		clearBitAddr(P);
	}

	return;
}


/* incFunc() 함수
*
* 기능 : INC DPTR를 제외한 각종 INC 함수를 실행한다.
* 입력 변수 : destination
* 출력 변수 없음
*/
void incFunc(unsigned char dest)
{
	// 목적지가 SBUF인 경우, SBUF의 8051기준 수신용 RAM에서 1을 더한 값을 8051의 송신용 RAM에 저장한다
	if (dest == SBUF)
	{
		chip.SBUF_send = chip.internal_RAM[dest] + 1;
	}else{
		chip.internal_RAM[dest]++;
	}
	return;
}


/* decFunc() 함수
*
* 기능 : 각종 DEC 함수를 실행한다.
* 입력 변수 : destination
* 출력 변수 없음
*/
void decFunc(unsigned char dest)
{
	// 목적지가 SBUF인 경우, SBUF의 8051기준 수신용 RAM에서 1을 뺀 값을 8051의 송신용 RAM에 저장한다
	if (dest == SBUF)
	{
		chip.SBUF_send = chip.internal_RAM[dest] - 1;
	}else{
		chip.internal_RAM[dest]--;
	}
	return;
}


/* movFunc() 함수
* 
* 기능 : 각종 MOV 함수를 실행한다.
* 입력 변수 : destination, source, isDat(상수 여부)
* 출력 변수 없음
*/
void movFunc(unsigned char dest, unsigned char src, char isDat)
{
	// isDat = 0(src(주소) -> dest), isDat = 1(src(데이터) -> dest)

	// 상수 입력인지 확인
	if (isDat == 0)
	{
		if (dest == SBUF) // Destination이 SBUF인 경우, 해당 값을 SBUF(송신)에 저장
		{
			chip.SBUF_send = chip.internal_RAM[src];
		}else{
			chip.internal_RAM[dest] = chip.internal_RAM[src];
		}

	}else if (isDat == 1){
		if (dest == SBUF) // Destination이 SBUF인 경우, 해당 값을 SBUF(송신)에 저장
		{
			chip.SBUF_send = src;
		}else{
			chip.internal_RAM[dest] = src;
		}
	}

	// Port에 출력한 경우, 해당 Port과 Latch를 연동한다.
	if (dest == 0x80 || dest == 0x90 || dest == 0xA0 || dest == 0xB0)
	{
		syncLatch((dest - 0x80) / 16);
	}

	return;
}


/* addFunc() 함수
*
* 기능 : 각종 ADD, ADDC 함수를 실행한다.
* 입력 변수 : source, isDat(상수 여부), isCarry(Carry 연산 여부)
* 출력 변수 없음
*/
void addFunc(unsigned char src, char isDat, char isCarry)
{
	// 기존값 저장(PSW 조정을 위함)
	unsigned char prevDat = chip.internal_RAM[0xE0];
	
	// 상수 연산인 경우
	if (isDat)
	{
		// ACC에 source와 Carry를 더하는 경우 Carry를 더한다.
		chip.internal_RAM[0xE0] += src + (isCarry && getBitAddr(C));

		// OV Flag 관련
		if (prevDat <= 0x7F && chip.internal_RAM[0xE0] > 0x7F)
		{
			setBitAddr(OV);
		}

		// AC Flag 관련
		// 두 데이터가 기본적인 차이가 있거나, ADDC이며 Carry가 1이고 기존값과 신규값이 같으며 src가 0이 아닌 경우
		if (((prevDat & 0x0F) > (chip.internal_RAM[0xE0] & 0x0F)) || (isCarry && getBitAddr(C) && src && (prevDat == chip.internal_RAM[0xE0])))
		{
			setBitAddr(AC);
		}

		// CY Flag 관련
		if ((chip.internal_RAM[0xE0] < prevDat && !isCarry) || (chip.internal_RAM[0xE0] < prevDat && isCarry && src))
		{
			setBitAddr(C);
		}
	}else{
		// ACC에 source와 Carry를 더하는 경우 Carry를 더한다.
		chip.internal_RAM[0xE0] += chip.internal_RAM[src] + (isCarry && getBitAddr(C));

		// OV Flag 관련
		if (prevDat <= 0x7F && chip.internal_RAM[0xE0] > 0x7F)
		{
			setBitAddr(OV);
		}

		// AC Flag 관련
		if (((prevDat & 0x0F) > (chip.internal_RAM[0xE0] & 0x0F)) || (isCarry && getBitAddr(C) && src && (prevDat == chip.internal_RAM[0xE0])))
		{
			setBitAddr(AC);
		}

		// CY Flag 관련
		if ((chip.internal_RAM[0xE0] < prevDat && !isCarry) || (chip.internal_RAM[0xE0] < prevDat && isCarry && chip.internal_RAM[src]))
		{
			setBitAddr(C);
		}

	}

	// Parity는 함수 외부에서 작업
	return;
}


/* subbFunc() 함수
*
* 기능 : 각종 SUBB 함수를 실행한다.
* 입력 변수 : source, isDat(상수 여부)
* 출력 변수 없음
*/
void subbFunc(unsigned char src, char isDat)
{
	// 기존값 저장(PSW 조정을 위함)
	unsigned char prevDat = chip.internal_RAM[0xE0];

	if (isDat)
	{
		// ACC에서 source와 Carry의 합을 뺀다.
		chip.internal_RAM[0xE0] -= (src + getBitAddr(C));

		// OV Flag 관련
		if (prevDat > 0x7F && chip.internal_RAM[0xE0] <= 0x7F)
		{
			setBitAddr(OV);
		}

		// AC Flag 관련
		if ((prevDat & 0x0F) < (chip.internal_RAM[0xE0] & 0x0F))
		{
			setBitAddr(AC);
		}

		// CY Flag 관련
		if (chip.internal_RAM[0xE0] >= prevDat && src)
		{
			setBitAddr(C);
		}

	}else{
		// ACC에서 source와 Carry의 합을 뺀다.
		chip.internal_RAM[0xE0] -= (chip.internal_RAM[src] + getBitAddr(C));

		// OV Flag 관련
		if (prevDat > 0x7F && chip.internal_RAM[0xE0] <= 0x7F)
		{
			setBitAddr(OV);
		}

		// AC Flag 관련
		if ((prevDat & 0x0F) < (chip.internal_RAM[0xE0] & 0x0F))
		{
			setBitAddr(AC);
		}

		// CY Flag 관련
		if (chip.internal_RAM[0xE0] >= prevDat && chip.internal_RAM[src])
		{
			setBitAddr(C);
		}
	}

	// Parity는 함수 외부에서 작업
	return;
}


/* mulAndiv() 함수
*
* 기능 : MUL, DIV 함수를 실행한다.
* 입력 변수 : isDiv(나눗셈 여부)
* 출력 변수 없음
*/
void mulAndDiv(char isDiv)
{
	// 각각 A와 B에 출력될 값
	short result, rem;

	if (isDiv)
	{
		if (!chip.internal_RAM[0xF0]) { return; } // DIV by 0

		// 몫과 나머지 계산
		result = chip.internal_RAM[0xE0] / chip.internal_RAM[0xF0];
		rem = chip.internal_RAM[0xE0] % chip.internal_RAM[0xF0];
	}else{
		// 곱하기 계산 후, 출력될 값 분리
		result = chip.internal_RAM[0xE0] / chip.internal_RAM[0xF0];
		rem = result / 256;
		result %= 256;
	}
	// 결과 출력
	chip.internal_RAM[0xE0] = result;
	chip.internal_RAM[0xF0] = rem;

	// DID NOT SET the PSW - 칩 제조사마다 변화가 다름

	return;
}


/* stackOperation() 함수
*
* 기능 : PUSH, POP 함수를 실행한다.
* 입력 변수 : src(사용할 변수의 주소), isPop(POP 명령어 여부)
* 출력 변수 없음
*/
void stackOperation(unsigned char src, int isPop)
{
	if (isPop) // POP이면
	{
		// 주어진 주소에 데이터 출력 후, SP 감소
		chip.internal_RAM[src] = chip.internal_RAM[chip.internal_RAM[SP]];
		chip.internal_RAM[SP]--;

	}else{ // PUSH면
		// SP 증가 후, 주어진 주소의 값을 Stack에 저장
		chip.internal_RAM[SP]++;

		// SP 위치가 SBUF인 경우, 해당 값을 SBUF(송신)에 저장(P0P는 SBUF(수신)에서 읽어옴)
		if (chip.internal_RAM[SP] == SBUF)
		{
			chip.SBUF_send = chip.internal_RAM[src];
		}else{
			chip.internal_RAM[chip.internal_RAM[SP]] = chip.internal_RAM[src];
		}
	}

	return;
}


/* stackOperationPC() 함수
*
* 기능 : Program Counter에서의 PUSH, POP 함수를 실행한다.
* 입력 변수 : src(사용할 변수의 주소), isPop(POP 명령어 여부)
* 출력 변수 : POP인 경우 Stack에 저장된 주소, PUSH인 경우 0(미사용)
*/
unsigned char stackOperationPC(unsigned char src, int isPop)
{
	if (isPop)
	{
		// 주소값 반환 후, SP 감소
		return chip.internal_RAM[chip.internal_RAM[SP]--];

	}else{
		// SP 증가 후, 주어진 주소의 값을 Stack에 저장
		chip.internal_RAM[SP]++;

		// SP 위치가 SBUF인 경우, 해당 값을 SBUF(송신)에 저장(P0P는 SBUF(수신)에서 읽어옴)
		if (chip.internal_RAM[SP] == SBUF)
		{
			chip.SBUF_send = src;
		}else{
			chip.internal_RAM[chip.internal_RAM[SP]] = src;
		}
	}

	return 0;
}


/* setBitAddr() 함수
*
* 기능 : SETB 명령어를 수행한다.
* 입력 변수 : location(수정할 bit주소)
* 출력 변수 없음
*/
void setBitAddr(unsigned char location)
{
	if (location <= 0x7F) // 20~2F(bit addressable ram)의 주소인 경우
	{
		chip.internal_RAM[0x20 + location / 8] = chip.internal_RAM[0x20 + location / 8] | (unsigned char)pow(2, location % 8);

	}else if (location <= 0xBF){ // Port0~3, ... 등의 0x?8~0X?F 주소가 존재하는 경우
		if (location % 16 > 0 && location % 16 < 8) // P0~P3
		{
			chip.internal_RAM[(location / 16) * 16] = chip.internal_RAM[(location / 16) * 16] | (unsigned char)pow(2, location % 8); // Pin에 작성
			chip.latch[(location - 0x80) / 16] = chip.latch[(location - 0x80) / 16] | (unsigned char)pow(2, location % 8); // Latch에 작성

		}else{ // TC0N(0x88), SCON(0x98), IE(0xA8), IP(0xB8)
			chip.internal_RAM[(location / 16) * 16 + 0x08] = chip.internal_RAM[(location / 16) * 16 + 0x08] | (unsigned char)pow(2, location % 8);
		}

	}else if (location <= 0xC7 || (location >= 0xD0 && location <= 0xD7) || (location >= 0xE0 && location <= 0xE7) || (location >= 0xF0 && location <= 0xF7)){ // 나머지(0x?8~0X?F 주소가 존재하지 않는 경우)
		chip.internal_RAM[(location / 16) * 16] = chip.internal_RAM[(location / 16) * 16] | (unsigned char)pow(2, location % 8);
	}

	return;
}


/* clearBitAddr() 함수
*
* 기능 : CLR A를 제외한 CLR 명령어를 수행한다.
* 입력 변수 : location(수정할 bit주소)
* 출력 변수 없음
*/
void clearBitAddr(unsigned char location)
{
	if (location <= 0x7F) // 20~2F(bit addressable ram)의 주소인 경우
	{
		chip.internal_RAM[0x20 + location / 8] = chip.internal_RAM[0x20 + location / 8] & (255 - (unsigned char)pow(2, location % 8));

	}else if (location <= 0xBF){ // Port0~3, ... 등의 0x?8~0X?F 주소가 존재하는 경우
		if (location % 16 > 0 && location % 16 < 8) // P0~P3
		{
			chip.internal_RAM[(location / 16) * 16] = chip.internal_RAM[(location / 16) * 16] & (255 - (unsigned char)pow(2, location % 8)); // Pin에 작성
			chip.latch[(location - 0x80) / 16] = chip.latch[(location - 0x80) / 16] & (0xFF - (unsigned char)pow(2, location % 8));  // Latch에 작성

		}else{ // TC0N(0x88), SCON(0x98), IE(0xA8), IP(0xB8)
			chip.internal_RAM[(location / 16) * 16 + 0x08] = chip.internal_RAM[(location / 16) * 16 + 0x08] & (255 - (unsigned char)pow(2, location % 8));
		}

	}else if (location <= 0xC7 || (location >= 0xD0 && location <= 0xD7) || (location >= 0xE0 && location <= 0xE7) || (location >= 0xF0 && location <= 0xF7)){ // 나머지(0x?8~0X?F 주소가 존재하지 않는 경우)
		chip.internal_RAM[(location / 16) * 16] = chip.internal_RAM[(location / 16) * 16] & (255 - (unsigned char)pow(2, location % 8));
	}
	return;
}


/* getBitAddr() 함수
*
* 기능 : 주어진 Bit Address의 값을 가져온다.
* 입력 변수 : location(읽어들일 bit주소)
* 출력 변수 : 해당 bit주소에 저장된 값
*/
char getBitAddr(unsigned char location)
{
	if (location <= 0x7F) // 20~2F(bit addressable ram)의 주소인 경우
	{
		// 해당 Byte Address의 해당 Bit Address의 위치가 1인 경우, 1을 반환하도록 함.
		return !(!(chip.internal_RAM[0x20 + location / 8] & (unsigned char)pow(2, location % 8)));

	}else if (location <= 0xBF){ // Port0~3, ... 등의 0x?8~0X?F 주소가 존재하는 경우
		if (location % 16 > 0 && location % 16 < 8) // P0~P3
		{
			// 해당 Byte Address의 해당 Bit Address의 위치가 1인 경우, 1을 반환하도록 함.
			return !(!(chip.internal_RAM[(location / 16) * 16] & (unsigned char)pow(2, location % 8)));

		}else{ // TCON(0x88), SCON(0x98), IE(0xA8), IP(0xB8)
			// 해당 Byte Address의 해당 Bit Address의 위치가 1인 경우, 1을 반환하도록 함.
			return !(!(chip.internal_RAM[(location / 16) * 16 + 0x08] & (unsigned char)pow(2, location % 8)));
		}

	}else if (location <= 0xC7 || (location >= 0xD0 && location <= 0xD7) || (location >= 0xE0 && location <= 0xE7) || (location >= 0xF0 && location <= 0xF7)){ // 나머지(0x?8~0X?F 주소가 존재하지 않는 경우)
		// 해당 Byte Address의 해당 Bit Address의 위치가 1인 경우, 1을 반환하도록 함.
		return !(!(chip.internal_RAM[(location / 16) * 16] & (unsigned char)pow(2, location % 8)));
	}

	return 0; // 없는 주소의 경우
}


/* orOperation() 함수
*
* 기능 : 각종 ORL 함수를 실행한다.
* 입력 변수 : destination, source, isDat(상수 여부), isBit(bit 주소 여부, -1인 경우 반대 bit값)
* 출력 변수 없음
*/
void orOperation(unsigned char dest, unsigned char src, char isData, char isBit) // isBit : 0(byte), 1(bit), -1(bit transpose)
{
	if (isBit) // Bit 주소의 경우
	{
		if (getBitAddr(C) || getBitAddr(src) ^ (isBit - 1)) // isBit을 이용해, 기존값 반전이 가능하도록 함.
		{
			setBitAddr(C);
		}else{
			clearBitAddr(C);
		}

	}else{ // Byte 주소의 경우
		if (isData == 1) // 데이터 입력인 경우
		{
			if (dest == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
			{
				chip.SBUF_send = chip.internal_RAM[dest] | src;
			}else{
				chip.internal_RAM[dest] |= src;
			}

		}else{ // 주소 입력인 경우
			if (src == 0x80 || src == 0x90 || src == 0xA0 || src == 0xB0) // Port 주소인 경우
			{
				// 이 경우, Read-Modify-Write이므로, Latch의 값을 반전한다.

				if (dest == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
				{
					chip.SBUF_send = chip.internal_RAM[dest] | chip.latch[(src - 0x80) / 16];
				}else{
					chip.internal_RAM[dest] |= chip.latch[(src - 0x80) / 16];
				}

			}else{ // Port 주소가 아닌 경우
				if (dest == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
				{
					chip.SBUF_send = chip.internal_RAM[dest] | chip.internal_RAM[src];
				}else{
					chip.internal_RAM[dest] |= chip.internal_RAM[src];
				}
			}
		}

		// Port 주소인 경우, 해당 Port의 값을 Latch에 연동한다.
		if (dest == 0x80 || dest == 0x90 || dest == 0xA0 || dest == 0xB0)
		{
			syncLatch((dest - 0x80) / 16);
		}
	}

	return;
}


/* andOperation() 함수
*
* 기능 : 각종 ANL 함수를 실행한다.
* 입력 변수 : destination, source, isDat(상수 여부), isBit(bit 주소 여부, -1인 경우 반대 bit값)
* 출력 변수 없음
*/
void andOperation(unsigned char dest, unsigned char src, char isData, char isBit) // isBit : 0(byte), 1(bit), -1(bit transpose)
{
	if (isBit) // Bit 주소의 경우
	{
		if (getBitAddr(C) && getBitAddr(src) ^ (isBit - 1)) // isBit을 이용해, 기존값 반전이 가능하도록 함.
		{
			setBitAddr(C);
		}else{
			clearBitAddr(C);
		}

	}else{ // Byte 주소의 경우
		if (isData == 1) // 데이터 입력인 경우
		{
			if (dest == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
			{
				chip.SBUF_send = chip.internal_RAM[dest] & src;
			}else{
				chip.internal_RAM[dest] &= src;
			}

		}else{ // 주소 입력인 경우
			if (src == 0x80 || src == 0x90 || src == 0xA0 || src == 0xB0) // Port 주소인 경우
			{
				// 이 경우, Read-Modify-Write이므로, Latch의 값을 반전한다.

				if (dest == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
				{
					chip.SBUF_send = chip.internal_RAM[dest] & chip.latch[(src - 0x80) / 16];
				}else{
					chip.internal_RAM[dest] &= chip.latch[(src - 0x80) / 16];
				}

			}else{ // Port 주소가 아닌 경우
				if (dest == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
				{
					chip.SBUF_send = chip.internal_RAM[dest] & chip.internal_RAM[src];
				}else{
					chip.internal_RAM[dest] &= chip.internal_RAM[src];
				}
			}
		}

		// Port 주소인 경우, 해당 Port의 값을 Latch에 연동한다.
		if (dest == 0x80 || dest == 0x90 || dest == 0xA0 || dest == 0xB0)
		{
			syncLatch((dest - 0x80) / 16);
		}
	}

	return;
}


/* xorOperation() 함수
*
* 기능 : 각종 XRL 함수를 실행한다.
* 입력 변수 : destination, source, isDat(상수 여부)
* 출력 변수 없음
*/
void xorOperation(unsigned char dest, unsigned char src, char isData)
{
	if (isData == 1) // 데이터 입력인 경우
	{
		if (dest == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
		{
			chip.SBUF_send = chip.internal_RAM[dest] ^ src;
		}else{
			chip.internal_RAM[dest] ^= src;
		}

	}else{ // 주소 입력인 경우
		if (src == 0x80 || src == 0x90 || src == 0xA0 || src == 0xB0) // Port 주소인 경우
		{
			// 이 경우, Read-Modify-Write이므로, Latch의 값을 반전한다.

			if (dest == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
			{
				chip.SBUF_send = chip.internal_RAM[dest] ^ chip.latch[(src - 0x80) / 16];
			}else{
				chip.internal_RAM[dest] ^= chip.latch[(src - 0x80) / 16];
			}

		}else{ // Port 주소가 아닌 경우

			if (dest == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
			{
				chip.SBUF_send = chip.internal_RAM[dest] ^ chip.internal_RAM[src];
			}else{
				chip.internal_RAM[dest] ^= chip.internal_RAM[src];
			}
		}
	}

	// Port 주소인 경우, 해당 Port의 값을 Latch에 연동한다.
	if (dest == 0x80 || dest == 0x90 || dest == 0xA0 || dest == 0xB0)
	{
		syncLatch((dest - 0x80) / 16);
	}

	return;
}


/* swapOperation() 함수
*
* 기능 : 각종 XCH 함수를 실행한다.
* 입력 변수 : source (destination은 무조건 ACC)
* 출력 변수 없음
*/
void swapOperation(unsigned char src)
{
	unsigned char tmp = chip.internal_RAM[ACC];
	chip.internal_RAM[ACC] = chip.internal_RAM[src];
	if (src == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
	{
		chip.SBUF_send = tmp;
	}else{
		chip.internal_RAM[src] = tmp;
	}

	// P0-3 Port가 Source인 경우, WriteLatch를 진행한다.
	if (src == 0x80 || src == 0x90 || src == 0xA0 || src == 0xB0)
	{
		syncLatch((src - 0x80) / 0x10);
	}

	return;
}


/* halfSwapOperation() 함수
*
* 기능 : 각종 XCHD 함수를 실행한다.
* 입력 변수 : source (destination은 무조건 ACC)
* 출력 변수 없음
*/
void halfSwapOperation(unsigned char src)
{
	unsigned char tmpValue = chip.internal_RAM[src] & 0x0F;
	if (src == SBUF) // SBUF 출력의 경우, 출력을 SBUF(송신)에 한다.
	{
		chip.SBUF_send = (chip.internal_RAM[src] & 0xF0) + (chip.internal_RAM[ACC] & 0x0F);
	}else{
		chip.internal_RAM[src] = (chip.internal_RAM[src] & 0xF0) + (chip.internal_RAM[ACC] & 0x0F);
	}
	chip.internal_RAM[ACC] = (chip.internal_RAM[ACC] & 0xF0) + tmpValue;

	// P0-3 Port가 Source인 경우, 아래 4bit에 대해서만 WriteLatch를 진행한다.
	if (chip.internal_RAM[src] == 0x80 || chip.internal_RAM[src] == 0x90 || chip.internal_RAM[src] == 0xA0 || chip.internal_RAM[src] == 0xB0)
	{
		// 위 4bit는 기존값, 아래 4bit는 새로운 값.
		chip.latch[(src - 0x80) / 0x10] = (chip.latch[(src - 0x80) / 0x10] & 0xF0) + (chip.internal_RAM[src] & 0x0F);
	}

	return;
}


/* DAOperation() 함수
*
* 기능 : DA 함수를 실행한다.
* 입출력 변수 없음
*/
void DAOperation()
{
	unsigned char upper, under; // 각각 ACC의 상위 4bit과 하위 4bit
	upper = chip.internal_RAM[ACC] & 0xF0;
	upper /= 16;
	under = chip.internal_RAM[ACC] & 0x0F;

	// 하위 4bit값이 10 이상이거나, AC값이 1인 경우
	if (under >= 10 || getBitAddr(AC))
	{
		// 상위 4bit값에 1을 더하며, 하위 4bit값에 6을 더한다.
		upper++;
		under += 6;
	}

	// 상위 4bit값이 10 이상이거나, Carry값이 1인 경우
	if (upper >= 10 || getBitAddr(C))
	{
		// Carry값을 1로 설정하며, 상위 4bit값에 6을 더한다.
		setBitAddr(C);
		upper += 6;
	}

	// 상, 하위 4bit값을 16진수로 1의 자릿수만 남긴다.
	upper %= 16;
	under %= 16;

	// 결과를 ACC에 저장한다.
	chip.internal_RAM[ACC] = upper * 16 + under;

	return;
}