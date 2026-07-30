#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <avr/io.h>
#include <util/delay.h>

//==============================================================================
// I2C LCD (PCF8574 백팩 + HD44780) 드라이버
//   PD0 = SCL, PD1 = SDA
//   P0 = RS, P1 = RW, P2 = E, P3 = 백라이트, P4 ~ P7 = D4 ~ D7
//==============================================================================

#define LCD_RS 0x01
#define LCD_EN 0x04
#define LCD_BL 0x08                         // 백라이트
#define TWI_TIMEOUT 20000                   // 약 5ms. 응답이 없으면 실패로 본다

static uint8_t lcdAddr = 0;                 // 자동 탐색으로 찾은 주소 (0 = 못 찾음)

static uint8_t twiWait(void)
{
	uint16_t timeout = TWI_TIMEOUT;

	while (!(TWCR & (1 << TWINT)))
	{
		if (--timeout == 0)
			return 1;                       // 무한 대기하지 않고 빠져나온다
	}

	return 0;
}

static uint8_t twiStart(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

	return twiWait();
}

static uint8_t twiWrite(uint8_t byte)
{
	TWDR = byte;
	TWCR = (1 << TWINT) | (1 << TWEN);

	return twiWait();
}

static void twiStop(void)
{
	uint16_t timeout = TWI_TIMEOUT;

	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

	while (TWCR & (1 << TWSTO))
	{
		if (--timeout == 0)
			return;
	}
}

// 해당 주소에 장치가 있는지 확인한다 (ACK 가 오면 1)
static uint8_t twiProbe(uint8_t addr)
{
	uint8_t ack = 0;

	if (twiStart() == 0)
	{
		if (twiWrite(addr << 1) == 0)
			ack = ((TWSR & 0xF8) == 0x18);  // 0x18 = 주소 전송 후 ACK 받음
	}

	twiStop();

	return ack;
}

// PCF8574 에 1바이트를 그대로 밀어 넣는다 (= P0 ~ P7 핀 상태)
static void pcfWrite(uint8_t byte)
{
	if (lcdAddr == 0)
		return;

	if (twiStart() == 0)
	{
		if (twiWrite(lcdAddr << 1) == 0)
			twiWrite(byte | LCD_BL);
	}

	twiStop();
}

static void lcdNibble(uint8_t nibble, uint8_t rs)
{
	uint8_t data = (nibble & 0xF0) | rs;

	pcfWrite(data);                         // E = 0 상태로 데이터와 RS 를 먼저 확정시킨다
	pcfWrite(data | LCD_EN);                // E = 1
	_delay_us(1);
	pcfWrite(data);                         // E = 0 -> 이 순간 LCD 가 4비트를 읽어간다
	_delay_us(50);
	/*

	PCF8574 는 8핀이 동시에 바뀐다. 데이터와 E 를 한 번에 보내면
	E 가 올라가는 시점에 데이터가 아직 안정되지 않아 LCD 가 못 읽는다.
	그래서 E = 0 인 상태로 한 번 먼저 보내 데이터를 세워둔다.

	*/
}

static void lcdByte(uint8_t byte, uint8_t rs)
{
	lcdNibble(byte & 0xF0, rs);             // 상위 4비트 먼저
	lcdNibble(byte << 4, rs);               // 하위 4비트
}

static void lcdCommand(uint8_t byte)
{
	lcdByte(byte, 0);                       // RS = 0 -> 명령
}

static void lcdClear(void)
{
	lcdCommand(0x01);
	_delay_ms(2);                           // Clear 는 1.5ms 정도 걸린다
}

static void lcdInit(void)
{
	uint8_t addr;

	DDRD &= ~((1 << PD0) | (1 << PD1));     // PD0 = SCL, PD1 = SDA
	PORTD |= (1 << PD0) | (1 << PD1);       // 내부 풀업 (I2C 는 풀업이 필수)

	TWSR = 0x00;                            // 분주비 1
	TWBR = 72;                              // SCL = 16MHz / (16 + 2*72) = 100kHz

	_delay_ms(50);                          // LCD 전원이 안정될 때까지 기다린다

	// 응답하는 주소를 직접 찾는다 (PCF8574 = 0x20~0x27, PCF8574A = 0x38~0x3F)
	lcdAddr = 0;

	for (addr = 0x20; addr <= 0x27; addr++)
	{
		if (twiProbe(addr)) { lcdAddr = addr; break; }
	}

	if (lcdAddr == 0)
	{
		for (addr = 0x38; addr <= 0x3F; addr++)
		{
			if (twiProbe(addr)) { lcdAddr = addr; break; }
		}
	}

	if (lcdAddr == 0)                       // 못 찾아도 프로그램은 계속 돈다
		return;
	/*

	HD44780 은 전원을 넣으면 8비트 모드로 시작한다.
	0x30 을 세 번 보내 "확실히 8비트 모드" 로 만든 뒤 0x20 으로 4비트 전환한다.

	*/
	lcdNibble(0x30, 0);
	_delay_ms(5);
	lcdNibble(0x30, 0);
	_delay_us(150);
	lcdNibble(0x30, 0);
	_delay_us(150);
	lcdNibble(0x20, 0);                     // 여기부터 4비트 모드
	_delay_us(150);

	lcdCommand(0x28);                       // 4비트, 2줄, 5x8 폰트
	lcdCommand(0x0C);                       // 화면 ON, 커서 OFF
	lcdCommand(0x06);                       // 문자 쓰면 커서 우측 이동
	lcdClear();
}

static void lcdString(uint8_t line, uint8_t col, char *str)
{
	if (lcdAddr == 0)
		return;

	if (line == 0)
		lcdCommand(0x80 + col);             // 1번째 줄 시작 주소 0x80
	else
		lcdCommand(0xC0 + col);             // 2번째 줄 시작 주소 0xC0

	while (*str)
		lcdByte(*str++, LCD_RS);            // RS = 1 -> 문자 데이터
}

#endif
