/*
avr-gcc -mmcu=atmega128 -Os -Wall -std=gnu99 -o firmware.elf hw2.c
avr-objcopy -O ihex -R .eeprom firmware.elf firmware.hex
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int main(void)
{
	int count = 0;

	PORTA = 0xFF;                           // LED. PA0 - PA7 을 소등

    /*
    
    5V - 저항 - LED - PAx 순. PAx에 5V를 주면 전압차 없어짐
     
    */

	DDRA = 0xFF;  // LED. PA0 - PA7 을 출력용(1)으로 설정

	DDRC &= ~((1 << PC0) | (1 << PC1));     // PC0(스위치 1), PC1(스위치 2)을 입력용(0)으로 설정
	PORTC |= (1 << PC0) | (1 << PC1);       // 내부 풀업

	DDRD &= ~((1 << PD3) | (1 << PD2));     // PD3(스위치 4) = INT3, PD2(스위치 3) = INT2
	PORTD |= (1 << PD3) | (1 << PD2);       // 내부 풀업  

	// 스위치는 눌리면 LOW 가 되므로 falling edge
	EICRA = (1 << ISC31) | (1 << ISC21);    // INT3, INT2 (ISCn1 = 1)
    /*
    
    ISCn0 의 초기값은 0이므로 따로 건들지 않는다.
    falling edge -> ISCn1 = 1,ISCn0 = 0
    
    */
	EIMSK = (1 << INT3) | (1 << INT2);      // INT3, INT2 허용

	sei();

	while (1) 
    {
		if ((!(PINC & (1 << PINC0))) && (!(PINC & (1 << PINC1)))) 
        {
			PORTA = 0x00;                   // 둘 다 눌림 -> 8개 전부 온
		}

		else if (!(PINC & (1 << PINC0))) 
        {
			PORTA = 0x0F;                   // SW1 -> LED 4 - 7만 온
		}

		else if (!(PINC & (1 << PINC1))) 
        {
			PORTA = 0xF0;                   // SW2 -> LED 0 - 3만 온
		}

		else 
        {
			if (count == 0)                 // 아무것도 안 눌림 -> 0.5초 마다 전체 깜빡인다
            {
				count = 1;
				PORTA = 0x00;               // 전부 켜기
			}

			else 
            {
				count = 0;
				PORTA = 0xFF;               // 전부 끄기
			}
		}

		_delay_ms(500);
	}
}

ISR(INT3_vect)        // 좌측 이동
{
	uint8_t temp;

	for (temp = 0b10000000; temp != 0b00000001; temp = temp >> 1) 
    {
		PORTA = ~temp;
		_delay_ms(200);
	}
	PORTA = ~temp;    // 마지막 칸은 반복문을 탈출 이후 온
	_delay_ms(200);

    // 예외처리
	EIFR = (1 << INTF3) | (1 << INTF2);
}

ISR(INT2_vect)        // 우측 이동
{
	uint8_t temp;

	for (temp = 0b00000001; temp != 0b10000000; temp = temp << 1) {
		PORTA = ~temp;
		_delay_ms(200);
	}
	PORTA = ~temp;    // 마지막 칸(LED7)
	_delay_ms(200);

	EIFR = (1 << INTF3) | (1 << INTF2);
}
