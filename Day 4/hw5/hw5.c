/*

Day 4 - 과제 5

[메모]

서보 신호 : PB7 (OC1C)
서보 커넥터 : 1 = GND(갈색), 2 = 5V(적색), 3 = SIGNAL(노란색)
PC : UART1 (RXD1 = PD2, TXD1 = PD3), 4800

서보 전원 주의
  SG90 은 동작 중 100 ~ 250mA, 스톨 시 최대 700mA.
  AD-USBISP 의 USB 전원은 권장 한계가 100mA 라 절대 못 돌린다.
  12V 서플라이 -> 레귤레이터 5V 로 급전하고 보드와 GND 를 공통으로.

PD2(RXD1) 커패시터 때문에 9600 수신은 깨진다. 이 과제는 값을 받아야 해서 4800.
커패시터를 떼면 PC_UBRR 을 207 로.

Timer1 Fast PWM 모드 14. 분주 8 -> 1틱 0.5us, ICR1 = 39999 -> 20.000ms (오차 0)

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define HOME_ANGLE  90                      // 리셋 시 복귀할 원점

/*

SG90 펄스 폭. 흔히 1.0 ~ 2.0ms 로 알려져 있으나 실제로는 0.5 ~ 2.4ms 를
줘야 0 ~ 180도 전체를 쓴다. 1.0 ~ 2.0ms 만 주면 90 ~ 120도 정도만 돈다.

개체차가 있다. 180 에서 "지지지" 소리가 나면 기계적 한계에 부딪힌 것이니
PULSE_MAX_US 를 줄인다. 그대로 두면 전류를 계속 먹고 서보가 상한다.

*/
#define PULSE_MIN_US 600                    // 0도
#define PULSE_MAX_US 2400                   // 180도

#define ANGLE_MIN 0
#define ANGLE_MAX 180

#define PC_UBRR 415                         // 4800. PD2 커패시터 때문에 9600 은 깨짐

void INIT_servo(void);
uint16_t CONVERT_angleToUs(uint8_t angle);
uint16_t CONVERT_angleToOcr(uint8_t angle);
void WRITE_servo(uint8_t angle);
void INIT_pcUart(void);
void SEND_pcChar(char c);
void SEND_pcString(char* str);
uint8_t isRECEIVED_pcChar(char* out);

int main(void)
{
    uint16_t acc = 0;                       // 입력 중인 숫자
    uint8_t hasDigit = 0;
    uint8_t badChar = 0;
    uint8_t negative = 0;
    uint8_t angle = HOME_ANGLE;
    char c;
    char buf[64];

    INIT_pcUart();
    INIT_servo();

    /*

    원점 복귀.
    전원이 들어오자마자 현재 위치에서 원점까지 한 번에 움직이므로
    그 동안 전류가 크게 흐른다. 도착할 시간을 주고 입력을 받는다.

    */
    WRITE_servo(HOME_ANGLE);
    _delay_ms(800);

    SEND_pcString("\r\nSG90 servo control\r\n");
    sprintf(buf, "HOME: %d deg (pulse %dus)\r\n", HOME_ANGLE, CONVERT_angleToUs(HOME_ANGLE));
    SEND_pcString(buf);
    sprintf(buf, "input %d-%d then Enter\r\n", ANGLE_MIN, ANGLE_MAX);
    SEND_pcString(buf);

    while (1)
    {
        if (!isRECEIVED_pcChar(&c))
            continue;

        if (c >= '0' && c <= '9')
        {
            hasDigit = 1;

            if (acc < 10000)                // 넘침 방지
                acc = (uint16_t)(acc * 10 + (c - '0'));
            else
                badChar = 1;
        }
        else if (c == '-')
        {
            negative = 1;                   // 0도 미만 입력 시도
        }
        else if (c == '\r' || c == '\n')
        {
            if (!hasDigit && !negative && !badChar)
            {
                // 엔터만 친 경우는 그냥 넘어간다
            }
            else if (negative)
            {
                SEND_pcString("ERROR: angle < 0 (allowed 0-180)\r\n");
            }
            else if (badChar)
            {
                SEND_pcString("ERROR: digits only\r\n");
            }
            else if (acc > ANGLE_MAX)
            {
                sprintf(buf, "ERROR: %d is out of range (0-180)\r\n", acc);
                SEND_pcString(buf);
            }
            else
            {
                angle = (uint8_t)acc;       // 유효할 때만 움직인다
                WRITE_servo(angle);

                sprintf(buf, "OK: %d deg (pulse %dus)\r\n", angle, CONVERT_angleToUs(angle));
                SEND_pcString(buf);
            }

            acc = 0;
            hasDigit = 0;
            badChar = 0;
            negative = 0;
        }
        else if (c == ' ' || c == '\t')
        {
            // 공백은 무시
        }
        else
        {
            badChar = 1;
        }
    }
}

/* ---- Timer1 Fast PWM ---- */
/*

8비트 타이머로 20ms 주기를 만들려면 분주를 크게 잡아야 하고, 그러면
1틱이 굵어져 각도 분해능이 무너진다. (분주 1024 면 1틱 64us -> 180도를 약 28단계)

Timer1 은 16비트라 분주 8 로 1틱 0.5us 까지 쪼갠다.
펄스 0.6 ~ 2.4ms = 1200 ~ 4800틱 -> 3600단계.

PB7 은 OC2 겸 OC1C 인데 Timer1 의 C 채널로 쓴다.
OC1C 는 ATmega103 호환 모드에서 못 쓰지만, 이 보드는 UART1 이 동작하므로
호환 모드가 꺼져 있는 것이 이미 확인되었다.

*/
void INIT_servo(void)
{
    DDRB |= (1 << PB7);                     // OC1C 출력

    ICR1 = 39999;                           // TOP. 40000 * 0.5us = 20ms
    TCCR1A = (1 << COM1C1) | (1 << WGM11);  // 비반전, Fast PWM 하위비트
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);   // 모드 14, 분주 8
}

/* ---- 각도 변환 ---- */
/*

(PULSE_MAX_US - PULSE_MIN_US) * angle 은 최대 1800 * 180 = 324000.
AVR 의 int 는 16비트라 uint32_t 로 올리지 않으면 값이 접힌다.

*/
uint16_t CONVERT_angleToUs(uint8_t angle)
{
    return (uint16_t)(PULSE_MIN_US
           + ((uint32_t)(PULSE_MAX_US - PULSE_MIN_US) * angle) / ANGLE_MAX);
}

uint16_t CONVERT_angleToOcr(uint8_t angle)
{
    return (uint16_t)(CONVERT_angleToUs(angle) * 2);    // 1틱 = 0.5us
}

void WRITE_servo(uint8_t angle)
{
    OCR1C = CONVERT_angleToOcr(angle);
}

/* ---- UART1 : PC ---- */
void INIT_pcUart(void)
{
    UCSR1A = (1 << U2X1);
    UBRR1H = (uint8_t)(PC_UBRR >> 8);
    UBRR1L = (uint8_t)PC_UBRR;
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10); // 8N1
    UCSR1B = (1 << TXEN1) | (1 << RXEN1);
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

uint8_t isRECEIVED_pcChar(char* out)
{
    if (!(UCSR1A & (1 << RXC1)))
        return 0;

    *out = (char)UDR1;

    return 1;
}
