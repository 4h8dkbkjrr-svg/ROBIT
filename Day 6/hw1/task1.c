/*

Day 6 - 과제 1

[메모]

IR 수광부 : PF2 ~ PF7 (ADC2 ~ ADC7)
LED      : PORTA (Active Low)
SW1      : PC0 - min/max 재보정
SW2      : PC1 - LCD 표시 채널 넘기기
LCD      : I2C (PD0 = SCL, PD1 = SDA)
PC       : UART1 (TXD1 = PD3), 9600

PF0, PF1 은 PSD 1, PSD 2
센서 번호 i 와 ADC 채널이 2 만큼 어긋나게.

PF4 ~ PF7 은 JTAG 핀이라 JTD 를 안 끄면 센서 6개 중 4개가 죽는다.
main 맨 앞에서 MCUCSR 에 두 번 연속 써서 해제
 
UART 는 한 블록이 380바이트라 9600 에서 400ms 가까이 걸린다.
그냥 보내면 그동안 ADC 와 LED 가 멈추므로 링버퍼와 UDRE 인터럽트로 뺐다.
 
버퍼를 정확히 256 으로 잡으면 uint8_t 인덱스가 저절로 돌아가고,
인덱스가 1바이트라 ISR 과 main 이 같이 만져도 값이 찢어지지 않음.

UART0 은 MAX485 에 물려 있어서 TTL 어댑터로는 못 받는다. 반드시 UART1.
PD3 은 회로상 SW4 자리이기도 해서 이 과제에서는 SW4 를 안 쓴다.

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "LCD_I2C.h"

#define IR_COUNT    6                       // PF2 ~ PF7
#define ADC_OFFSET  2                       // 센서 번호 i -> ADC 채널 i + 2

#define FILT_N      8                       // 이동평균 창. 2의 거듭제곱이어야 한다
#define FILT_SHIFT  3                       // log2(FILT_N). 나눗셈 대신 시프트

#define NORM_MAX    1000                    // 정규화를 0 ~ 1000 정수로 다룬다
#define LED_TH      800                     // 0.8 이상이면 LED 점등

#define TXBUF_SIZE  256                     // 인덱스가 uint8_t 라 자동으로 랩된다

uint16_t rawVal[IR_COUNT];                  // 원본 ADC
uint16_t fltVal[IR_COUNT];                  // 이동평균 결과
uint16_t minVal[IR_COUNT];
uint16_t maxVal[IR_COUNT];
uint16_t normVal[IR_COUNT];                 // 0 ~ 1000

uint16_t fBuf[IR_COUNT][FILT_N];            // 이동평균 링버퍼
uint16_t fSum[IR_COUNT];                    // 창 안의 합. 8 x 1023 = 8184 라 16비트로 충분
uint8_t  fIdx = 0;

volatile uint8_t txBuf[TXBUF_SIZE];
volatile uint8_t txHead = 0;
volatile uint8_t txTail = 0;

volatile uint8_t sampleFlag = 0;            // 10ms  : ADC, 필터, LED
volatile uint8_t uartFlag = 0;              // 100ms : UART 한 줄
volatile uint8_t lcdFlag = 0;               // 200ms : LCD 갱신

void INIT_timer0(void);
void INIT_pcUart(void);
void INIT_adc(void);
void INIT_io(void);
void INIT_filter(void);
void SEND_pcChar(char c);
void SEND_pcString(char* str);
uint16_t READ_adc(uint8_t ch);
void READ_sensors(void);
void RESET_minMax(void);
void UPDATE_leds(void);
void PRINT_lcdLine(uint8_t line, char* str);

int main(void)
{
    char buf[64];
    uint8_t lcdCh = 0;                      // LCD 에 보여줄 채널
    uint8_t outCh;
    uint8_t outStep = 0;                    // 0 = 헤더, 1 ~ N = 각 채널, N + 1 = 빈 줄
    uint8_t sw1;
    uint8_t sw2;
    uint8_t prevSw1 = 1;                    // 안 눌린 상태(HIGH) 로 시작
    uint8_t prevSw2 = 1;

    MCUCSR |= (1 << JTD);                   // PF4 ~ PF7 을 JTAG 에서 떼어낸다
    MCUCSR |= (1 << JTD);                   // 4클럭 이내 연속 2회여야 적용된다

    INIT_io();
    INIT_adc();
    INIT_pcUart();
    lcdInit();                              // PD0, PD1 설정은 이 안에서 한다
    INIT_timer0();
    sei();

    INIT_filter();                          // 첫 측정값으로 링버퍼와 min/max 를 채운다

    PRINT_lcdLine(0, "Day6 Task1");
    PRINT_lcdLine(1, "IR calibrating");
    _delay_ms(800);

    while (1)
    {
        /* ---- 10ms : 측정, 필터, 정규화, LED ---- */
        if (sampleFlag)
        {
            sampleFlag = 0;

            READ_sensors();
            UPDATE_leds();

            sw1 = (PINC & (1 << PINC0)) ? 1 : 0;
            sw2 = (PINC & (1 << PINC1)) ? 1 : 0;

            if (prevSw1 && !sw1)            // 눌리는 순간에만 한 번
                RESET_minMax();

            if (prevSw2 && !sw2)
                lcdCh = (uint8_t)((lcdCh + 1) % IR_COUNT);

            prevSw1 = sw1;
            prevSw2 = sw2;
        }

        /* ---- 100ms : UART 한 줄씩 ---- */
        /*

        헤더 + 6채널 + 빈 줄 = 8단계라 한 블록이 800ms 마다 나온다.
        교육자료 예시 화면의 문자 위치에 맞춰 값이 9, 21, 32, 39, 45열에서 시작한다.
        %-4d 로 좌측 정렬해서 1023 처럼 네 자리가 나와도 열이 안 밀린다.

        한 줄이 51바이트라 링버퍼를 넘지 않는다. 그래서 main 이 여기서 멈추지 않는다.

        */
        if (uartFlag)
        {
            uartFlag = 0;

            if (outStep == 0)
            {
                SEND_pcString("         original / filter(MAF) / min / max / norm\r\n");
            }
            else if (outStep <= IR_COUNT)
            {
                outCh = (uint8_t)(outStep - 1);

                sprintf(buf, "IR %d :   %-4d        %-4d       %-4d   %-4d  %d.%02d\r\n",
                        outCh, rawVal[outCh], fltVal[outCh],
                        minVal[outCh], maxVal[outCh],
                        normVal[outCh] / 1000, (normVal[outCh] % 1000) / 10);

                SEND_pcString(buf);
            }
            else
            {
                SEND_pcString("\r\n");
            }

            outStep = (uint8_t)((outStep + 1) % (IR_COUNT + 2));
        }

        /* ---- 200ms : LCD ---- */
        if (lcdFlag)
        {
            lcdFlag = 0;

            sprintf(buf, "S%d R%4d F%4d", lcdCh, rawVal[lcdCh], fltVal[lcdCh]);
            PRINT_lcdLine(0, buf);

            sprintf(buf, "m%4d M%4d %d.%02d",
                    minVal[lcdCh], maxVal[lcdCh],
                    normVal[lcdCh] / 1000, (normVal[lcdCh] % 1000) / 10);
            PRINT_lcdLine(1, buf);
        }
    }
}

/* ---- 측정 주기 ---- */
/*

ISR 은 깃발만 세운다. ADC 변환과 UART, LCD 출력은 시간이 걸려서
ISR 안에서 하면 다른 인터럽트가 밀린다.

카운터를 셋 다 uint8_t 로 뒀다. 100 까지만 세면 되므로 넘칠 일이 없고,
1바이트라 main 에서 읽을 때 값이 찢어지지 않는다.

*/
ISR(TIMER0_COMP_vect)
{
    static uint8_t c10 = 0;
    static uint8_t c100 = 0;
    static uint8_t c200 = 0;

    if (++c10 >= 5)     { c10 = 0;   sampleFlag = 1; }      // 2ms x 5
    if (++c100 >= 50)   { c100 = 0;  uartFlag = 1; }        // 2ms x 50
    if (++c200 >= 100)  { c200 = 0;  lcdFlag = 1; }         // 2ms x 100
}

void INIT_timer0(void)
{
    TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS01);   // CTC, 분주 256
    OCR0 = 124;                                          // 2ms (오차 0)
    TCNT0 = 0;
    TIMSK |= (1 << OCIE0);
}

/* ---- UART1 : PC ---- */
/*

UDRE 인터럽트가 링버퍼에서 한 바이트씩 꺼내 보낸다.
보낼 것이 없으면 인터럽트를 스스로 꺼서 헛돌지 않게 한다.

*/
ISR(USART1_UDRE_vect)
{
    if (txHead == txTail)
    {
        UCSR1B &= ~(1 << UDRIE1);
    }
    else
    {
        UDR1 = txBuf[txTail];
        txTail = (uint8_t)(txTail + 1);
    }
}

void INIT_pcUart(void)
{
    UCSR1A = (1 << U2X1);
    UBRR1H = 0;
    UBRR1L = 207;                           // 9600 baud
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); // 8N1
    UCSR1B = (1 << TXEN1);                  // 송신만 쓴다
}

void SEND_pcChar(char c)
{
    uint8_t next = (uint8_t)(txHead + 1);

    while (next == txTail);                 // 버퍼가 가득 차면 ISR 이 비울 때까지 기다린다

    txBuf[txHead] = (uint8_t)c;
    txHead = next;

    UCSR1B |= (1 << UDRIE1);                // 보낼 것이 생겼으니 인터럽트를 켠다
}

void SEND_pcString(char* str)
{
    while (*str)
        SEND_pcChar(*str++);
}

/* ---- ADC ---- */
void INIT_adc(void)
{
    DDRF &= ~0xFC;                          // PF2 ~ PF7 을 입력으로
    PORTF &= ~0xFC;                         // ADC 핀은 내부 풀업을 꺼야 한다

    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // 분주 128 -> 125kHz. ADC 는 50 ~ 200kHz 에서만 10비트 정확도가 나온다
}

uint16_t READ_adc(uint8_t ch)
{
    ADMUX = (1 << REFS0) | (ch & 0x1F);     // 기준전압 AVCC(5V)
    _delay_us(10);                          // 채널 전환 후 입력이 안정될 시간

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    return ADC;
}

/* ---- 입출력 ---- */
void INIT_io(void)
{
    DDRA = 0xFF;
    PORTA = 0xFF;                           // Active Low 라 전부 소등

    DDRC &= ~((1 << PC0) | (1 << PC1));     // SW1, SW2 입력
    PORTC |= (1 << PC0) | (1 << PC1);       // 외부 풀업이 있지만 보험으로 켜둔다
}

/* ---- 필터와 정규화 ---- */
/*

첫 측정값으로 링버퍼 여덟 칸을 전부 채우고 min, max 도 그 값으로 맞춘다.
이걸 안 하면 버퍼에 남아 있던 0 때문에 min 이 0 으로 굳어서
정규화 값이 계속 1 근처로만 나온다.

*/
void INIT_filter(void)
{
    uint8_t i;
    uint8_t k;
    uint16_t v;

    for (i = 0; i < IR_COUNT; i++)
    {
        v = READ_adc((uint8_t)(i + ADC_OFFSET));

        for (k = 0; k < FILT_N; k++)
            fBuf[i][k] = v;

        fSum[i] = (uint16_t)(v * FILT_N);
        rawVal[i] = v;
        fltVal[i] = v;
        minVal[i] = v;
        maxVal[i] = v;
        normVal[i] = 0;
    }

    fIdx = 0;
}

void RESET_minMax(void)
{
    uint8_t i;

    for (i = 0; i < IR_COUNT; i++)
    {
        minVal[i] = fltVal[i];
        maxVal[i] = fltVal[i];
    }
}

/*

이동평균은 창 안의 합을 들고 다니면서 나간 값을 빼고 들어온 값을 더한다.
매번 여덟 개를 다시 더하는 것보다 빠르고, 창 크기가 2의 거듭제곱이라
평균을 낼 때 나눗셈 대신 시프트를 쓸 수 있다.

min, max 는 원본이 아니라 필터를 거친 값으로 갱신한다.
원본으로 잡으면 노이즈 한 번에 범위가 벌어져서 정규화가 눌린다.

*/
void READ_sensors(void)
{
    uint8_t i;
    uint16_t v;

    for (i = 0; i < IR_COUNT; i++)
    {
        v = READ_adc((uint8_t)(i + ADC_OFFSET));
        rawVal[i] = v;

        fSum[i] = (uint16_t)(fSum[i] - fBuf[i][fIdx] + v);
        fBuf[i][fIdx] = v;
        fltVal[i] = (uint16_t)(fSum[i] >> FILT_SHIFT);

        if (fltVal[i] < minVal[i])
            minVal[i] = fltVal[i];

        if (fltVal[i] > maxVal[i])
            maxVal[i] = fltVal[i];

        if (maxVal[i] > minVal[i])
        {
            normVal[i] = (uint16_t)(((uint32_t)(fltVal[i] - minVal[i]) * NORM_MAX)
                                    / (uint32_t)(maxVal[i] - minVal[i]));
        }
        else
        {
            normVal[i] = 0;                 // 아직 편차가 없다. 0 으로 나누는 것을 막는다
        }
    }

    fIdx = (uint8_t)((fIdx + 1) & (FILT_N - 1));
}

/*

IR 은 흰색일수록 반사가 많아 값이 크다.
그래서 정규화 0.8 이상은 흰 바닥을 보고 있다는 뜻이고, 그때 LED 를 켠다.

*/
void UPDATE_leds(void)
{
    uint8_t i;
    uint8_t mask = 0;

    for (i = 0; i < IR_COUNT; i++)
    {
        if (normVal[i] >= LED_TH)
            mask |= (uint8_t)(1 << i);
    }

    PORTA = (uint8_t)~mask;                 // Active Low 라 내보낼 때 뒤집는다
}

/* ---- LCD ---- */
/*

열여섯 칸을 공백으로 채워서 내보낸다.
자릿수가 줄었을 때 앞 화면의 글자가 남는 것을 막는다.

*/
void PRINT_lcdLine(uint8_t line, char* str)
{
    char pad[17];
    uint8_t i = 0;

    while (str[i] && i < 16)
    {
        pad[i] = str[i];
        i++;
    }

    while (i < 16)
        pad[i++] = ' ';

    pad[16] = '\0';

    lcdString(line, 0, pad);
}
