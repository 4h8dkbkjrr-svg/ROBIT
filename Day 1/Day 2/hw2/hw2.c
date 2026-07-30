/*
avr-gcc -mmcu=atmega128 -Os -Wall -std=gnu99 -o firmware.elf hw2.c
avr-objcopy -O ihex -R .eeprom firmware.elf firmware.hex
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "LCD_I2C.h"

#define INITIAL "LDY"

uint16_t adcRead(void)
{
	ADCSRA |= (1 << ADSC);                  // 변환
	while (ADCSRA & (1 << ADSC));           // 변환이 끝나면 ADSC 가 저절로 0 이 된다

	return ADC;                             // ADCL, ADCH 를 합친 10비트 값 (0 ~ 1023)
}

int main(void)
{
	uint16_t adc;                           // 전압
	uint8_t pos;                            // LED 위치 0 ~ 7
	char buf[17];

	PORTA = 0xFF;                           // LED PA0 ~ PA7 을 소등
	DDRA = 0xFF;                            // LED. PA0 ~ PA7 을 출력용(1)으로 설정한다.

	DDRF &= ~(1 << PF0);                    // 가변저항. PF0 을 입력용(0)으로 설정
	PORTF &= ~(1 << PF0);                   // 내부 풀업을 끄기

	ADMUX = (1 << REFS0);                   // 기준전압 AVCC(5V), 채널 ADC0
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	/*

	ADEN  : ADC 전원 킴
	ADPS2:0 = 111 -> 분주비 128 -> 16메가헤르츠 , 128 = 125키로헤르트

	REFS1:REFS0 = 0:1 -> 기준전압 AVCC(5V)

	*/

	lcdInit();
	lcdString(0, 0, "ADC:");
	lcdString(0, 13, INITIAL);              // 이니셜은 1번째 줄 오른쪽 끝
	lcdString(1, 0, "VOL:");

	while (1)
	{
		adc = adcRead();

		// 1. 0 ~ 1023 을 8칸으로 나눠 LED 1개를 이동
		pos = adc >> 7;                     // 1023 >> 7 = 7 이므로 0 ~ 7 로 떨어짐
		PORTA = ~(1 << pos);

		// 2. ADC 값 표시한다.
		sprintf(buf, "%4d", adc);
		lcdString(0, 4, buf);

		// 3. 전압 계산 후 표시
		mv = (uint32_t)adc * 5000 / 1023;   // 0 ~ 1023 을 0 ~ 5000mV 로 환산
		sprintf(buf, "%d.%02dV", mv / 1000, (mv % 1000) / 10);
		lcdString(1, 4, buf);

		_delay_ms(100);
	}
}
