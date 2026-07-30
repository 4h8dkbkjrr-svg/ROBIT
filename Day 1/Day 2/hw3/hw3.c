/*
avr-gcc -mmcu=atmega128 -Os -Wall -std=gnu99 -o firmware.elf hw3.c
avr-objcopy -O ihex -R .eeprom firmware.elf firmware.hex
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "LCD_I2C.h"

#define INITIAL "LDY"

#define SW1 0x01                            // PC0 : A 1 증가
#define SW2 0x02                            // PC1 : 연산자 변경
#define SW3 0x04                            // PD2 : B 1 증가
#define SW4 0x08                            // PD3 : 계산 실행

const char opTable[4] = {'+', '-', '*', '/'};

uint8_t swRead(void)
{
	uint8_t sw = 0;

	if (!(PINC & (1 << PINC0))) sw |= SW1;
	if (!(PINC & (1 << PINC1))) sw |= SW2;
	if (!(PIND & (1 << PIND2))) sw |= SW3;
	if (!(PIND & (1 << PIND3))) sw |= SW4;

	return sw;
}

// 16칸을 공백으로 채워서 한 줄을 통째로 덮어씀(리셋? 개념)
void lcdLine(uint8_t line, char *str)
{
	char buf[17];
	uint8_t i = 0;

	while (i < 16 && str[i])
	{
		buf[i] = str[i];
		i = i + 1;
	}

	while (i < 16)
	{
		buf[i] = ' ';
		i = i + 1;
	}

	buf[16] = '\0';

	lcdString(line, 0, buf);
}

int main(void)
{
	uint8_t now;
	uint8_t prev = 0;
	uint8_t pressed;

	uint8_t a = 1;                          // 초기값 A = 1
	uint8_t b = 1;                          // 초기값 B = 1
	uint8_t opIdx = 0;                      // 0 = '+', 1 = '-', 2 = '*', 3 = '/'
	int16_t c;

	char buf[24];

	DDRC &= ~((1 << PC0) | (1 << PC1));     // SW1, SW2 를 입력용(0)으로 세팅
	PORTC |= (1 << PC0) | (1 << PC1);       // 내부 풀업

	DDRD &= ~((1 << PD2) | (1 << PD3));     // SW3, SW4 를 입력용(0)으로 세팅
	PORTD |= (1 << PD2) | (1 << PD3);       // 내부 풀업

	lcdInit();

	sprintf(buf, "%d %c %d = ?", a, opTable[opIdx], b);
	lcdLine(0, buf);
	lcdLine(1, INITIAL);

	while (1)
	{
		now = swRead();
		pressed = now & ~prev;              // 새로 눌린 것만 남긴다
		prev = now;

		if (pressed & SW1)
		{
			a = a + 1;
			if (a > 99) a = 1;              // 예외처리 : 두 자리를 넘으면 1 로 돌아감
		}

		if (pressed & SW2)
		{
			opIdx = (opIdx + 1) & 0x03;     // 0 -> 1 -> 2 -> 3 -> 0 순환
		}

		if (pressed & SW3)
		{
			b = b + 1;
			if (b > 99) b = 1;
		}

		if (pressed & SW4)                  // 계산 실행
		{
			switch (opIdx)
			{
			case 0:  c = (int16_t)a + b; break;
			case 1:  c = (int16_t)a - b; break;
			case 2:  c = (int16_t)a * b; break;
			default: c = (b != 0) ? (int16_t)(a / b) : 0; break;
			}
			/*

			예외처리

			*/

			sprintf(buf, "%d %c %d = %d", a, opTable[opIdx], b, c);
			lcdLine(0, buf);
		}

		else if (pressed & (SW1 | SW2 | SW3))
		{
			sprintf(buf, "%d %c %d = ?", a, opTable[opIdx], b);
			lcdLine(0, buf);                // 아직 계산 전이므로 결과는 ? 로 둔다
		}

		_delay_ms(20);
	}
}

