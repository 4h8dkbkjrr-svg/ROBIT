/*

Day 4 - 과제 2

[메모]

가변저항 : PF0 (ADC0)
SW1     : PC0 (항목 확정 -> 다음)
SW2     : PC1 (시간 시작 / 정지)
LCD     : I2C, PD0 = SCL, PD1 = SDA

시간 기준은 Timer0 CTC. 분주 256, OCR0 = 124 -> 2ms (오차 0)
2ms 를 5번 세면 10ms = 1/100초. 화면 소수점 두 자리 단위가 여기서 나온다.

예시 "190722 10:50:48.34" 는 18글자라 16x2 한 줄에 안 들어간다.
1행 날짜, 2행 시간으로 나눠서 표시.

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "LCD_I2C.h"

#define INITIAL "LDY"

#define TICKS_PER_CS 5                      // 2ms * 5 = 10ms

#define SW1 0x01
#define SW2 0x02

/* 세팅 진행 상태 */
#define ST_YEAR  0
#define ST_MON   1
#define ST_DAY   2
#define ST_HOUR  3
#define ST_MIN   4
#define ST_SEC   5
#define ST_READY 6
#define ST_RUN   7

volatile uint8_t tCs = 0;                   // 1/100초
volatile uint8_t tSec = 0;
volatile uint8_t tMin = 0;
volatile uint8_t tHour = 0;
volatile uint8_t tDay = 1;
volatile uint8_t tMon = 1;
volatile uint8_t tYear = 25;                // 두 자리. 25 = 2025년
volatile uint8_t running = 0;
volatile uint8_t tickCount = 0;

uint8_t isLEAP_year(uint8_t y);
uint8_t GET_daysInMonth(uint8_t y, uint8_t m);
void INIT_timer0(void);
uint8_t READ_switch(void);
uint16_t READ_adc(void);
uint8_t MAP_adcToRange(uint16_t adc, uint8_t lo, uint8_t hi);
void PRINT_lcdLine(uint8_t line, char* str);
uint8_t isSAME_string(char* a, char* b);
void COPY_string(char* dst, char* src);

int main(void)
{
    uint8_t state = ST_YEAR;
    uint8_t now;
    uint8_t prev = 0;
    uint8_t pressed;
    uint16_t adc;
    uint8_t cs, sec, mi, hh, dd, mm, yy;    // 화면에 찍을 값의 복사본
    char buf[24];
    char line0[24];
    char prevLine0[24] = "";

    DDRC &= ~((1 << PC0) | (1 << PC1));     // 스위치를 입력용(0)으로 설정
    PORTC |= (1 << PC0) | (1 << PC1);       // 내부 풀업

    DDRF &= ~(1 << PF0);                    // 가변저항을 입력용(0)으로 설정
    PORTF &= ~(1 << PF0);                   // ADC 핀은 내부 풀업을 꺼야 한다

    ADMUX = (1 << REFS0);                   // 기준전압 AVCC(5V), 채널 ADC0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // 분주 128 -> 125kHz. ADC 는 50 ~ 200kHz 에서만 10비트 정확도가 나온다

    lcdInit();
    INIT_timer0();
    sei();

    while (1)
    {
        now = READ_switch();
        pressed = now & ~prev;              // 새로 눌린 것만
        prev = now;

        if (pressed & SW2)                  // 언제든 시작, 정지
        {
            running = !running;
            state = running ? ST_RUN : ST_YEAR;
        }

        if (state != ST_RUN)                // 세팅 중이면 가변저항으로 값을 정한다
        {
            adc = READ_adc();

            switch (state)
            {
            case ST_YEAR:
                tYear = MAP_adcToRange(adc, 0, 99);
                break;

            case ST_MON:
                tMon = MAP_adcToRange(adc, 1, 12);
                break;

            case ST_DAY:
                tDay = MAP_adcToRange(adc, 1, GET_daysInMonth(tYear, tMon));
                break;

            case ST_HOUR:
                tHour = MAP_adcToRange(adc, 0, 23);
                break;

            case ST_MIN:
                tMin = MAP_adcToRange(adc, 0, 59);
                break;

            case ST_SEC:
                tSec = MAP_adcToRange(adc, 0, 59);
                tCs = 0;
                break;

            default:
                break;
            }

            if (pressed & SW1)              // 확정하고 다음 항목으로
            {
                if (state < ST_READY)
                    state++;

                // 1월 31일로 정해둔 뒤 2월로 바꾸면 없는 날이 된다
                if (tDay > GET_daysInMonth(tYear, tMon))
                    tDay = GET_daysInMonth(tYear, tMon);
            }
        }
        else if (pressed & SW1)             // 동작 중 SW1 은 정지 후 세팅 모드
        {
            running = 0;
            state = ST_YEAR;
        }

        /*

        시, 분, 초가 별개 변수라 읽는 도중 인터럽트가 끼면
        10:59:59 를 읽다가 초만 0 으로 바뀌어 10:59:00 이 찍힌다.

        */
        cli();
        cs = tCs; sec = tSec; mi = tMin; hh = tHour;
        dd = tDay; mm = tMon; yy = tYear;
        sei();

        switch (state)
        {
        case ST_YEAR:  sprintf(line0, "SET YEAR   %02d", yy); break;
        case ST_MON:   sprintf(line0, "SET MONTH  %02d", mm); break;
        case ST_DAY:   sprintf(line0, "SET DAY    %02d", dd); break;
        case ST_HOUR:  sprintf(line0, "SET HOUR   %02d", hh); break;
        case ST_MIN:   sprintf(line0, "SET MIN    %02d", mi); break;
        case ST_SEC:   sprintf(line0, "SET SEC    %02d", sec); break;
        case ST_READY: sprintf(line0, "SW2 TO START"); break;
        default:       sprintf(line0, "%02d%02d%02d   %s", yy, mm, dd, INITIAL); break;
        }

        // I2C 가 느려서 1행까지 매번 쓰면 시계가 뚝뚝 끊긴다
        if (!isSAME_string(line0, prevLine0))
        {
            PRINT_lcdLine(0, line0);
            COPY_string(prevLine0, line0);
        }

        sprintf(buf, "%02d:%02d:%02d.%02d", hh, mi, sec, cs);
        PRINT_lcdLine(1, buf);
    }
}

/* ---- 시간 진행 (2ms 마다) ---- */
/*

자리올림만 한다. 넘칠 때만 다음 단계로 가도록 일찍 빠져나간다.
1/100초는 100번에 한 번만 초를 건드린다.

*/
ISR(TIMER0_COMP_vect)
{
    if (!running)
        return;

    if (++tickCount < TICKS_PER_CS)
        return;

    tickCount = 0;

    if (++tCs < 100)
        return;

    tCs = 0;

    if (++tSec < 60)
        return;

    tSec = 0;

    if (++tMin < 60)
        return;

    tMin = 0;

    if (++tHour < 24)
        return;

    tHour = 0;

    if (++tDay <= GET_daysInMonth(tYear, tMon))
        return;

    tDay = 1;

    if (++tMon <= 12)
        return;

    tMon = 1;

    if (++tYear <= 99)
        return;

    tYear = 0;                              // 99년 다음은 00년
}

/* ---- Timer0 CTC ---- */
void INIT_timer0(void)
{
    TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS01);   // CTC, 분주 256
    OCR0 = 124;                                          // 125 * 16us = 2ms
    TCNT0 = 0;
    TIMSK |= (1 << OCIE0);
}

/* ---- 윤년, 그 달의 마지막 날 ---- */
uint8_t isLEAP_year(uint8_t y)
{
    uint16_t year = 2000 + y;

    return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 1 : 0;
}

uint8_t GET_daysInMonth(uint8_t y, uint8_t m)
{
    const uint8_t table[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (m < 1 || m > 12)
        return 31;                          // 범위 밖이면 안전하게

    if (m == 2 && isLEAP_year(y))
        return 29;

    return table[m - 1];
}

/* ---- 스위치 ---- */
uint8_t READ_switch(void)
{
    uint8_t sw = 0;

    if (!(PINC & (1 << PINC0))) sw |= SW1;  // 외부 풀업 + Active Low
    if (!(PINC & (1 << PINC1))) sw |= SW2;

    return sw;
}

/* ---- 가변저항 ---- */
uint16_t READ_adc(void)
{
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    return ADC;                             // 0 ~ 1023
}

/* ---- 가변저항 값을 lo ~ hi 로 눌러 담기 ---- */
uint8_t MAP_adcToRange(uint16_t adc, uint8_t lo, uint8_t hi)
{
    uint16_t span = (uint16_t)(hi - lo) + 1;
    uint16_t v = (uint16_t)(((uint32_t)adc * span) / 1024);

    if (v >= span)                          // adc 가 1023 일 때 방어
        v = span - 1;

    return (uint8_t)(lo + v);
}

/* ---- LCD 한 줄 출력 (16칸을 공백으로 메워 잔상 제거) ---- */
void PRINT_lcdLine(uint8_t line, char* str)
{
    char buf[17];
    uint8_t i = 0;

    while (i < 16 && str[i])
    {
        buf[i] = str[i];
        i++;
    }

    while (i < 16)
    {
        buf[i] = ' ';
        i++;
    }

    buf[16] = '\0';

    lcdString(line, 0, buf);
}

/* ---- 문자열 비교, 복사 ---- */
uint8_t isSAME_string(char* a, char* b)
{
    while (*a && *a == *b)
    {
        a++;
        b++;
    }

    return (*a == *b) ? 1 : 0;
}

void COPY_string(char* dst, char* src)
{
    while (*src)
        *dst++ = *src++;

    *dst = '\0';
}
