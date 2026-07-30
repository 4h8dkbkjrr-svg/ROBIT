/*
avr-gcc -mmcu=atmega128 -Os -Wall -std=gnu99 -o firmware.elf hw3.c
avr-objcopy -O ihex -R .eeprom firmware.elf firmware.hex
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint8_t count = 0;
int main(void)
{
	PORTA = 0xFF;                           // LED. PA0 ~ PA7 을 소등
	DDRA = 0xFF;                            // LED, PA0 ~ PA7 을 출력용(1)으로 설정

	DDRD &= ~0x0F;                          // PD0 ~ PD3 = INT0 ~ INT3 을 입력용(0)으로 설정한다.
	PORTD |= 0x0F;                          // 내부 풀업 설정

	// 스위치 falling edge
	EICRA = (1 << ISC31) | (1 << ISC21) | (1 << ISC11) | (1 << ISC01);
	EIMSK = (1 << INT3) | (1 << INT2) | (1 << INT1) | (1 << INT0);

	sei();

	while (1)
	{
		PORTA = ~count;                     // 0.1s 마다 2진 카운터
		_delay_ms(100);
		count = count + 1;
	}
}

/* 메모

보드는 PA0 가 왼쪽 끝. PA7 이 오른쪽 끝
<< 가 화면상 우측 이동, >> 가 화면상 좌측 이동 -->-->->->--->--->

우측 3칸 : (count << 3) | (count >> 5)
좌측 3칸 : (count >> 3) | (count << 5)


*/

ISR(INT0_vect)        // 3개씩 우측 이동 * 2
{
	uint8_t i;

	for (i = 0; i < 2; i++)
	{
		count = (count << 3) | (count >> 5);
		PORTA = ~count;
		_delay_ms(200);
	}

	// 예외처리
	EIFR = 0x0F;
}

ISR(INT1_vect)        // 3개씩 좌측 이동 * 2
{
	uint8_t i;

	for (i = 0; i < 2; i++)
	{
		count = (count >> 3) | (count << 5);
		PORTA = ~count;
		_delay_ms(200);
	}

	EIFR = 0x0F;
}

ISR(INT2_vect)        // 1개 좌측 이동 후 우측 이동
{
	count = (count >> 1) | (count << 7);    // 좌측 1칸
	PORTA = ~count;
	_delay_ms(200);

	count = (count << 1) | (count >> 7);    // 우측 1칸 -> 원래 값으로 복귀
	PORTA = ~count;
	_delay_ms(200);

	EIFR = 0x0F;
}

ISR(INT3_vect)        // 2진 카운터 리셋한다.
{
	count = 0;
	PORTA = ~count;
	_delay_ms(200);

	EIFR = 0x0F;
}
