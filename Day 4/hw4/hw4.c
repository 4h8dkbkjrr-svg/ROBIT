/*

Day 4 - 과제 4 (심화)

[메모]

배선은 과제 3 과 동일. PSD : PF1 (ADC1), PC : UART1 (TXD1 = PD3), 9600

raw 와 필터 적용 값을 함께 출력한다. 거리는 필터를 거친 값으로 계산.
  RAW: 412 | FILTERED: 405 | DISTANCE: 15.2cm

FILTER_TYPE 을 바꿔가며 같은 조건에서 로그를 뜨면 필터 비교 자료가 된다.

읽기 40ms / 출력 200ms 로 나눈 이유
  PSD 출력은 38.3ms +- 9.6ms 주기로 갱신. 그보다 자주 읽어도 같은 값만 나온다.
  100ms 마다 한 번씩만 읽으면 8개 모으는 데 0.8초라 반응이 굼뜨다.

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#define PSD_CH      1                       // ADC 채널 = PF1

#define SAMPLE_MS   40                      // 센서 갱신 주기에 맞춤
#define PRINT_MS   200
#define SAMPLE_TICKS (SAMPLE_MS / 2)        // 2ms 단위
#define PRINT_TICKS  (PRINT_MS / 2)

#define USE_LED_BAR 0                       // USB 전원(100mA) 이면 반드시 0

/* 필터 종류 */
#define FILTER_NONE 0
#define FILTER_MA   1                       // 이동 평균
#define FILTER_EMA  2                       // 지수 이동 평균
#define FILTER_MED  3                       // 중앙값

#define FILTER_TYPE FILTER_MA               // 여기를 바꿔가며 비교

#define MA_SIZE     8
#define EMA_K       4                       // 클수록 부드러움
#define MED_SIZE    5                       // 홀수

/* 측정 상태 */
#define PSD_OK        0
#define PSD_TOO_CLOSE 1
#define PSD_TOO_FAR   2

typedef struct
{
    uint16_t adc;
    uint16_t mm;
} psdPoint;

/* Sharp GP2Y0A21YK0F 데이터시트 곡선. 10bit ADC, 5V 기준 환산 */
const psdPoint psdTable[] =
{
    { 471, 100 },                           // 10.0cm
    { 338, 150 },
    { 266, 200 },
    { 194, 300 },
    { 147, 400 },
    { 121, 500 },
    { 102, 600 },
    {  90, 700 },
    {  82, 800 }                            // 80.0cm
};

#define PSD_POINTS   (sizeof(psdTable) / sizeof(psdTable[0]))
#define PSD_ADC_NEAR (psdTable[0].adc)
#define PSD_ADC_FAR  (psdTable[PSD_POINTS - 1].adc)

volatile uint8_t sampleFlag = 0;
volatile uint8_t printFlag = 0;
volatile uint8_t sampleTick = 0;
volatile uint8_t printTick = 0;

uint16_t maBuf[MA_SIZE];
uint8_t maIdx = 0;
uint8_t maCount = 0;
uint32_t maSum = 0;

uint16_t emaVal = 0;
uint8_t emaReady = 0;

uint16_t medBuf[MED_SIZE];
uint8_t medCount = 0;

void INIT_timer0(void);
void INIT_pcUart(void);
void SEND_pcChar(char c);
void SEND_pcString(char* str);
uint16_t READ_adc(uint8_t ch);
uint16_t FILTER_movingAverage(uint16_t x);
uint16_t FILTER_exponential(uint16_t x);
uint16_t FILTER_median(uint16_t x);
uint16_t APPLY_filter(uint16_t x);
uint8_t CONVERT_psdToMm(uint16_t adc, uint16_t* mm);

int main(void)
{
    uint16_t raw = 0;
    uint16_t filtered = 0;
    uint16_t mm;
    uint8_t status;
    char buf[64];
#if USE_LED_BAR
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

#if FILTER_TYPE == FILTER_MA
    sprintf(buf, "filter: Moving Average (N=%d)\r\n", MA_SIZE);
#elif FILTER_TYPE == FILTER_EMA
    sprintf(buf, "filter: Exponential MA (K=%d)\r\n", EMA_K);
#elif FILTER_TYPE == FILTER_MED
    sprintf(buf, "filter: Median (N=%d)\r\n", MED_SIZE);
#else
    sprintf(buf, "filter: none\r\n");
#endif
    SEND_pcString(buf);

    sprintf(buf, "sample %d ms, print %d ms\r\n", SAMPLE_MS, PRINT_MS);
    SEND_pcString(buf);

    while (1)
    {
        if (sampleFlag)
        {
            sampleFlag = 0;

            raw = READ_adc(PSD_CH);
            filtered = APPLY_filter(raw);
        }

        if (printFlag)
        {
            printFlag = 0;

            status = CONVERT_psdToMm(filtered, &mm);    // 거리는 필터 값 기준

            switch (status)
            {
            case PSD_OK:
                sprintf(buf, "RAW: %4d | FILTERED: %4d | DISTANCE: %d.%dcm\r\n",
                        raw, filtered, mm / 10, mm % 10);
                break;

            case PSD_TOO_CLOSE:
                sprintf(buf, "RAW: %4d | FILTERED: %4d | DISTANCE: TOO CLOSE\r\n",
                        raw, filtered);
                break;

            default:
                sprintf(buf, "RAW: %4d | FILTERED: %4d | DISTANCE: OUT OF RANGE\r\n",
                        raw, filtered);
                break;
            }

            SEND_pcString(buf);

#if USE_LED_BAR
            if (status == PSD_OK)
            {
                level = (uint8_t)(8 - (mm - 100) / 88);

                if (level > 8)
                    level = 8;

                PORTA = (uint8_t)~((1 << level) - 1);
            }
            else
            {
                PORTA = (status == PSD_TOO_CLOSE) ? 0x00 : 0xFF;
            }
#endif
        }
    }
}

/* ---- 측정, 출력 주기 ---- */
/* ISR 은 깃발만. ADC 와 UART 는 시간이 걸려 ISR 안에서 하면 안 된다 */
ISR(TIMER0_COMP_vect)
{
    if (++sampleTick >= SAMPLE_TICKS)
    {
        sampleTick = 0;
        sampleFlag = 1;
    }

    if (++printTick >= PRINT_TICKS)
    {
        printTick = 0;
        printFlag = 1;
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
    _delay_us(10);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    return ADC;
}

/* ---- 이동 평균 ---- */
/* 버퍼가 차기 전에는 채워진 개수로만 나눈다. 안 그러면 0 이 섞여 낮게 나온다 */
uint16_t FILTER_movingAverage(uint16_t x)
{
    maSum -= maBuf[maIdx];                  // 가장 오래된 값을 빼고
    maBuf[maIdx] = x;                       // 새 값으로 덮은 뒤
    maSum += x;

    maIdx = (uint8_t)((maIdx + 1) % MA_SIZE);

    if (maCount < MA_SIZE)
        maCount++;

    return (uint16_t)(maSum / maCount);
}

/* ---- 지수 이동 평균 ---- */
/* 정수 나눗셈이라 (x - y) 가 K 보다 작으면 더는 안 움직인다 */
uint16_t FILTER_exponential(uint16_t x)
{
    if (!emaReady)                          // 첫 값은 그대로
    {
        emaVal = x;
        emaReady = 1;

        return x;
    }

    emaVal = (uint16_t)((int16_t)emaVal + ((int16_t)x - (int16_t)emaVal) / EMA_K);

    return emaVal;
}

/* ---- 중앙값 ---- */
/* 평균과 달리 스파이크를 아예 무시한다 */
uint16_t FILTER_median(uint16_t x)
{
    uint16_t tmp[MED_SIZE];
    uint8_t i, j;
    uint16_t key;

    for (i = MED_SIZE - 1; i > 0; i--)      // 한 칸씩 밀고
        medBuf[i] = medBuf[i - 1];

    medBuf[0] = x;

    if (medCount < MED_SIZE)
        medCount++;

    for (i = 0; i < medCount; i++)
        tmp[i] = medBuf[i];

    for (i = 1; i < medCount; i++)          // 삽입 정렬. 개수가 적어 충분하다
    {
        key = tmp[i];

        for (j = i; j > 0 && tmp[j - 1] > key; j--)
            tmp[j] = tmp[j - 1];

        tmp[j] = key;
    }

    return tmp[medCount / 2];
}

uint16_t APPLY_filter(uint16_t x)
{
#if FILTER_TYPE == FILTER_MA
    return FILTER_movingAverage(x);
#elif FILTER_TYPE == FILTER_EMA
    return FILTER_exponential(x);
#elif FILTER_TYPE == FILTER_MED
    return FILTER_median(x);
#else
    return x;
#endif
}

/* ---- ADC 값을 거리(mm) 로 ---- */
/* 10cm 안쪽은 출력이 다시 낮아져 5cm 와 50cm 가 구분이 안 된다. 경고로 넘긴다 */
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
