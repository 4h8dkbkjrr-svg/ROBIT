/*

Day 3 - 과제 2

[메모]

LED : PA0 ~ PA7 (Active Low)
SW  : PC0, PC1, PD2, PD3 (눌림 = LOW)
PC  : UART0 (PE0 = RXD0, PE1 = TXD0)

어댑터 TX -> PE0
어댑터 RX -> PE1
어댑터 GND -> 보드 GND
어댑터 VCC 는 연결하지 않음(충돌 현상)

9600

UART0 은 ISP 와 핀을 공유(PDI = PE0, PDO = PE1).
플래시할 때는 어댑터 TX 선을 뺄 것.

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define SW_RESET_PIN   PC0                  // 보드 표기 SW1
#define UBRR_VALUE     207                  // 9600 baud, U2X = 1

uint8_t ledPattern;                         // 1 인 비트가 켜진 LED

void WRITE_led(uint8_t pattern);
uint8_t ROTATE_left(uint8_t value);
uint8_t ROTATE_right(uint8_t value);
void INIT_uart(void);
void SEND_char(char c);
void SEND_string(char* str);
uint8_t isRECEIVED_char(char* out);
void SEND_ledOn(char digit);
void SEND_error(char c);
void HANDLE_char(char c);
void RESET_state(void);

int main(void)
{
    uint8_t swNow;
    uint8_t swPrev = 0;
    uint8_t i;
    char c;

    DDRE |= (1 << PE2);                     // MAX485 RE, DE
    PORTE |= (1 << PE2);                    // High -> RO 가 하이임피던스. PE0 을 놓아준다

    DDRA = 0xFF;
    WRITE_led(0x00);

    DDRC &= (uint8_t)~(1 << SW_RESET_PIN);
    PORTC |= (1 << SW_RESET_PIN);           // 내부 풀업

    for (i = 0; i < 3; i++)                 // 부팅 표시
    {
        WRITE_led(0xFF);
        _delay_ms(120);
        WRITE_led(0x00);
        _delay_ms(120);
    }

    INIT_uart();
    SEND_string("READY\r\n");

    while (1)
    {
        swNow = (PINC & (1 << SW_RESET_PIN)) ? 0 : 1;

        if (swNow && !swPrev)               // 하강 엣지에서만
        {
            RESET_state();
            _delay_ms(20);                  // 접점 떨림 무시
        }

        swPrev = swNow;

        if (isRECEIVED_char(&c))
            HANDLE_char(c);
    }
}

/* ---- LED 출력 ---- */
void WRITE_led(uint8_t pattern)
{
    ledPattern = pattern;
    PORTA = (uint8_t)~pattern;              // Active Low
}

/* ---- 좌우 회전 ---- */
uint8_t ROTATE_left(uint8_t value)
{
    return (uint8_t)((value << 1) | (value >> 7));
}

uint8_t ROTATE_right(uint8_t value)
{
    return (uint8_t)((value >> 1) | (value << 7));
}

/* ---- UART0 ---- */
void INIT_uart(void)
{
    UCSR0A = (1 << U2X0);
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
}

void SEND_char(char c)
{
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = (uint8_t)c;
}

void SEND_string(char* str)
{
    while (*str)
        SEND_char(*str++);
}

uint8_t isRECEIVED_char(char* out)
{
    if (!(UCSR0A & (1 << RXC0)))
        return 0;                           // 없으면 기다리지 않는다

    *out = (char)UDR0;

    return 1;
}

/* ---- 응답 문자열 ---- */
void SEND_ledOn(char digit)
{
    char msg[] = "0 LED on\r\n";

    msg[0] = digit;

    SEND_string(msg);
}

void SEND_error(char c)
{
    char msg[] = "ERROR: x\r\n";

    msg[7] = (c >= 0x20 && c <= 0x7E) ? c : '?';    // 제어문자는 화면이 깨진다

    SEND_string(msg);
}

/* ---- 수신 문자 처리 ---- */
void HANDLE_char(char c)
{
    if (c >= '0' && c <= '7')
    {
        WRITE_led((uint8_t)(1 << (c - '0')));
        SEND_ledOn(c);
    }
    else if (c == '8')
    {
        WRITE_led(ROTATE_left(ledPattern));
        SEND_string("LEFT\r\n");
    }
    else if (c == '9')
    {
        WRITE_led(ROTATE_right(ledPattern));
        SEND_string("RIGHT\r\n");
    }
    else if (c == '\r' || c == '\n')
    {
        // 엔터는 무시
    }
    else
    {
        SEND_error(c);                      // 예외 처리
    }
}

/* ---- 상태 초기화 ---- */
void RESET_state(void)
{
    WRITE_led(0x00);

    SEND_string("RESET\r\n");
}
