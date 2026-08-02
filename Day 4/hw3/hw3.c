/*

Day 4 - 과제 3

[메모]

PSD : PF1 (ADC1)
PC  : UART1 (TXD1 = PD3), 9600

Day 3 과제 3 의 4핀 케이블 그대로. 보내기만 하니 PD2 는 안 쓴다.
PD2 커패시터 문제와 무관해서 9600 그대로.

PORTF 할당 : PF0 가변저항, PF1 PSD, PF2 ~ PF7 IR 센서(추후)

측정 주기는 Timer0 CTC. 분주 256, OCR0 = 124 -> 2ms (오차 0). 50번 세면 100ms.

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#define PSD_CH      1                       // ADC 채널 = PF1
#define PERIOD_MS   100
#define TICKS_PER_PERIOD (PERIOD_MS / 2)    // 2ms 단위

#define USE_LED_BAR 0                       // USB 전원(100mA) 이면 반드시 0

/* 측정 상태 */
#define PSD_OK        0
#define PSD_TOO_CLOSE 1                     // 10cm 보다 가까움. 값이 모호한 구간
#define PSD_TOO_FAR   2                     // 80cm 초과 또는 미검출

typedef struct
{
    uint16_t adc;
    uint16_t mm;                            // mm 로 둬야 소수점 한 자리가 나온다
} psdPoint;

/*

Sharp GP2Y0A21YK0F 데이터시트 곡선에서 뽑은 점.
10bit ADC, 기준전압 5V 환산값.

센서 개체차가 있으니 정확도가 필요하면 자로 재면서 이 표를 고친다.
다른 모델(GP2Y0A02 등)도 표만 바꾸면 그대로 동작.

*/
const psdPoint psdTable[] =
{
    { 471, 100 },                           // 2.30V, 10.0cm
    { 338, 150 },
    { 266, 200 },
    { 194, 300 },
    { 147, 400 },
    { 121, 500 },
    { 102, 600 },
    {  90, 700 },
    {  82, 800 }                            // 0.40V, 80.0cm
};

#define PSD_POINTS   (sizeof(psdTable) / sizeof(psdTable[0]))
#define PSD_ADC_NEAR (psdTable[0].adc)
#define PSD_ADC_FAR  (psdTable[PSD_POINTS - 1].adc)

volatile uint8_t measureFlag = 0;
volatile uint8_t tickCount = 0;

void INIT_timer0(void);
void INIT_pcUart(void);
void SEND_pcChar(char c);
void SEND_pcString(char* str);
uint16_t READ_adc(uint8_t ch);
uint8_t CONVERT_psdToMm(uint16_t adc, uint16_t* mm);

int main(void)
{
    uint16_t adc;
    uint16_t mm;
    uint8_t status;
    char buf[48];
#if USE_LED_BAR
    uint8_t bar;
    uint8_t level;
#endif

    DDRF &= ~(1 << PF1);                    // PSD 를 입력용(0)으로 설정
    PORTF &= ~(1 << PF1);                   // ADC 핀은 내부 풀업을 꺼야 한다

    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // 분주 128 -> 125kHz. ADC 는 50 ~ 200kHz 에서만 10비트 정확도가 나온다

    DDRA = 0xFF;
    PORTA = 0xFF;                           // Active Low 라 전부 소등

    INIT_pcUart();
    INIT_timer0();
    sei();

    SEND_pcString("PSD distance meter\r\n");
    sprintf(buf, "period %d ms, range %d-%d cm\r\n",
            PERIOD_MS, psdTable[0].mm / 10, psdTable[PSD_POINTS - 1].mm / 10);
    SEND_pcString(buf);

    while (1)
    {
        if (!measureFlag)                   // 타이머가 깃발을 세울 때까지 대기
            continue;

        measureFlag = 0;

        adc = READ_adc(PSD_CH);
        status = CONVERT_psdToMm(adc, &mm);

        switch (status)
        {
        case PSD_OK:
            sprintf(buf, "ADC:%4d  DIST:%3d.%d cm\r\n", adc, mm / 10, mm % 10);
            break;

        case PSD_TOO_CLOSE:
            sprintf(buf, "ADC:%4d  WARN: TOO CLOSE (<%d cm)\r\n",
                    adc, psdTable[0].mm / 10);
            break;

        default:
            sprintf(buf, "ADC:%4d  WARN: OUT OF RANGE (>%d cm)\r\n",
                    adc, psdTable[PSD_POINTS - 1].mm / 10);
            break;
        }

        SEND_pcString(buf);

#if USE_LED_BAR
        if (status == PSD_TOO_CLOSE)
            bar = 0xFF;
        else if (status == PSD_TOO_FAR)
            bar = 0x00;
        else
        {
            level = (uint8_t)(8 - (mm - 100) / 88);      // 100 ~ 800mm -> 8 ~ 1

            if (level > 8)
                level = 8;

            bar = (uint8_t)((1 << level) - 1);
        }

        PORTA = (uint8_t)~bar;
#endif
    }
}

/* ---- 측정 주기 ---- */
/*

ISR 은 깃발만 세운다. ADC 변환과 UART 출력은 시간이 걸려서
ISR 안에서 하면 다른 인터럽트가 밀린다.

*/
ISR(TIMER0_COMP_vect)
{
    if (++tickCount >= TICKS_PER_PERIOD)
    {
        tickCount = 0;
        measureFlag = 1;
    }
}

void INIT_timer0(void)
{
    TCCR0 = (1 << WGM01) | (1 << CS02) | (1 << CS01);   // CTC, 분주 256
    OCR0 = 124;                                          // 2ms
    TCNT0 = 0;
    TIMSK |= (1 << OCIE0);
}

/* ---- UART1 : PC ---- */
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
    while (!(UCSR1A & (1 << UDRE1)));

    UDR1 = (uint8_t)c;
}

void SEND_pcString(char* str)
{
    while (*str)
        SEND_pcChar(*str++);
}

/* ---- ADC ---- */
uint16_t READ_adc(uint8_t ch)
{
    ADMUX = (1 << REFS0) | (ch & 0x1F);     // 기준전압 AVCC(5V)
    _delay_us(10);                          // 채널 전환 후 입력이 안정될 시간

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    return ADC;
}

/* ---- ADC 값을 거리(mm) 로 ---- */
/*

PSD 출력은 거리에 반비례하는 곡선이라 비례식으로는 안 된다.
거듭제곱 근사식은 pow() 가 필요해 AVR 에서 느리고 무겁다.
그래서 표의 점 사이를 직선으로 잇는다(선형보간).

가장 조심할 것은 10cm 안쪽에서 출력이 다시 낮아진다는 점.
5cm 와 50cm 가 비슷한 값을 내서 값만으로는 구분이 안 된다.
이 구간은 거리를 계산하지 않고 경고로 넘긴다.

*/
uint8_t CONVERT_psdToMm(uint16_t adc, uint16_t* mm)
{
    uint8_t i;
    uint16_t adcSpan;
    uint16_t mmSpan;

    if (adc > PSD_ADC_NEAR)
    {
        *mm = 0;
        return PSD_TOO_CLOSE;
    }

    if (adc < PSD_ADC_FAR)
    {
        *mm = 0;
        return PSD_TOO_FAR;
    }

    for (i = 0; i < PSD_POINTS - 1; i++)    // 표는 adc 내림차순
    {
        if (adc <= psdTable[i].adc && adc >= psdTable[i + 1].adc)
        {
            adcSpan = psdTable[i].adc - psdTable[i + 1].adc;
            mmSpan = psdTable[i + 1].mm - psdTable[i].mm;

            if (adcSpan == 0)               // 0 으로 나누기 방어
            {
                *mm = psdTable[i].mm;
                return PSD_OK;
            }

            *mm = psdTable[i].mm
                + (uint16_t)(((uint32_t)(psdTable[i].adc - adc) * mmSpan) / adcSpan);

            return PSD_OK;
        }
    }

    *mm = 0;
    return PSD_TOO_FAR;
}
