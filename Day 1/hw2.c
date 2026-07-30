/*
 * hw2.c  -  Day 1 과제 2
 *
 *   1. 0.5s 마다 모든 LED 깜빡이기
 *   2. SW1 이 눌리면 4~7 LED 켜기
 *   3. SW2 가 눌리면 0~3 LED 켜기
 *   4. 둘 다 눌리면 모두 켜기          -> 폴링 (No Ext Interrupt)
 *   5. INT3 발생시 LED 좌측 이동
 *   6. INT4 발생시 LED 우측 이동       -> External Interrupt
 *
 * 배선 (07/29 수정 회로도)
 *   LED  : PA0 ~ PA7   Active Low (0 = 점등), PA0 이 왼쪽 끝
 *   SW1  : PC0         LCD 가 I2C 로 바뀌면서 PD0 -> PC0 으로 옮겨졌다.
 *   SW2  : PC1                              PD1 -> PC1
 *   INT3 : PD3 = SW4   좌측 이동
 *   INT2 : PD2 = SW3   우측 이동
 *
 * ※ 과제 문구는 우측 이동을 INT4 로 요구하지만, INT4 는 PE4 에 고정되어 있고
 *   이 보드에는 PE4 에 스위치가 달려 있지 않아 점퍼가 필요하다.
 *   점퍼를 확보하지 못해 INT2(PD2 = SW3)로 대체했다.
 *   INT2 도 EICRA 로 설정하므로 EICRB 는 쓰지 않는다.
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int main(void)
{
	int count = 0;

	PORTA = 0xFF; // LED, PA0 ~ PA7 을 소등한다. (Active Low -> 1)
	DDRA = 0xFF;  // LED, PA0 ~ PA7 을 출력용(1)으로 설정

	DDRC &= ~((1 << PC0) | (1 << PC1)); // PC0(스위치 1), PC1(스위치 2)을 입력용(0)으로 설정
	PORTC |= (1 << PC0) | (1 << PC1);   // 내부 풀업 (외부 10k 가 있어서 보험용)

	DDRD &= ~((1 << PD3) | (1 << PD2)); // PD3(스위치 4) = INT3, PD2(스위치 3) = INT2
	PORTD |= (1 << PD3) | (1 << PD2);   // 내부 풀업 (외부 10k 가 있어서 보험용)

	// 스위치는 눌리면 LOW 가 되므로 falling edge (ISCn1 = 1, ISCn0 = 0)
	EICRA = (1 << ISC31) | (1 << ISC21); // INT3, INT2 falling (INT0~3 은 EICRA)
	EIMSK = (1 << INT3) | (1 << INT2);   // INT3, INT2 허용

	sei();

	while (1) {

		// 스위치는 PORTC 가 아니라 PINC 로 읽어야 한다. (PORTC 는 출력 래치)
		if ((!(PINC & (1 << PINC0))) && (!(PINC & (1 << PINC1)))) {
			PORTA = 0x00; // 둘 다 눌림 -> 8개 전부 켜기
		}
		else if (!(PINC & (1 << PINC0))) {
			PORTA = 0x0F; // SW1 -> 상위 4개(LED4~7)만 켜기
		}
		else if (!(PINC & (1 << PINC1))) {
			PORTA = 0xF0; // SW2 -> 하위 4개(LED0~3)만 켜기
		}
		else {
			if (count == 0) { // 아무것도 안 눌림 -> 0.5s 마다 전체 깜빡임
				count = 1;
				PORTA = 0x00; // 전부 켜기
			}
			else {
				count = 0;
				PORTA = 0xFF; // 전부 끄기
			}
		}

		_delay_ms(500);
	}
}

// 보드는 PA0 가 왼쪽 끝, PA7 이 오른쪽 끝이다.
// 그래서 비트를 >> 로 내려야 화면상 왼쪽으로 흘러간다. (<< 는 오른쪽)
ISR(INT3_vect) // 좌측 이동
{
	uint8_t temp;

	for (temp = 0b10000000; temp != 0b00000001; temp = temp >> 1) {
		PORTA = ~temp;
		_delay_ms(200);
	}
	PORTA = ~temp; // 마지막 칸(LED0)은 for 문을 빠져나온 뒤에 켠다
	_delay_ms(200);

	// 예외처리 : 이동하는 동안 눌린 중복 요청은 버린다. (1 을 써야 지워진다)
	EIFR = (1 << INTF3) | (1 << INTF2);
}

ISR(INT2_vect) // 우측 이동
{
	uint8_t temp;

	for (temp = 0b00000001; temp != 0b10000000; temp = temp << 1) {
		PORTA = ~temp;
		_delay_ms(200);
	}
	PORTA = ~temp; // 마지막 칸(LED7)
	_delay_ms(200);

	EIFR = (1 << INTF3) | (1 << INTF2);
}
