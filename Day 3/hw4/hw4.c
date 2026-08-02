/*

Day 3 - 과제 4

[메모]

PD3 -> PC (UART1 헤더의 TXD 자리. 과제 3 배선 그대로)

UART 레지스터(UCSRn, UBRRn, UDRn) 사용 금지. PORTD 와 DDRD 만으로 파형을 만든다.
보내기만 하니 PD2 는 안 쓴다. PD2 커패시터 문제와 무관해서 9600 그대로.

터미널 9600.

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TX_PIN   PD3
#define BIT_US   104                        // 1/9600 = 104.17us

#define USE_LED  1                          // 1 이면 보낼 때마다 PA0 토글

void INIT_softTx(void);
void SEND_softByte(uint8_t byte);
void SEND_softString(char* str);

int main(void)
{
#if USE_LED
    DDRA = 0xFF;
    PORTA = 0xFF;                           // Active Low 라 전부 소등
#endif

    INIT_softTx();

    while (1)
    {
        SEND_softString("HelloWorld!\r\n");  // \r\n 은 터미널 줄바꿈용

#if USE_LED
        PORTA ^= (1 << PA0);
#endif

        _delay_ms(1000);
    }
}

/* ---- 송신 핀 초기화 ---- */
void INIT_softTx(void)
{
    PORTD |= (1 << TX_PIN);                 // 순서 주의. DDRD 를 먼저 바꾸면
    DDRD |= (1 << TX_PIN);                  // 핀이 잠깐 Low -> 앞에 쓰레기 문자

    _delay_ms(1);                           // 수신 측이 idle 을 알아볼 시간
}

/* ---- 1바이트 송신 ---- */
/*

Start(0) - D0 ~ D7 (LSB 부터) - Stop(1). 각 구간 104.17us.
깨지면 BIT_US 를 103 이나 105 로 미세 조정.

*/
void SEND_softByte(uint8_t byte)
{
    uint8_t i;
    uint8_t sreg = SREG;

    cli();                                  // 프레임 중 인터럽트가 끼면 비트 폭이 틀어진다

    PORTD &= ~(1 << TX_PIN);                // Start bit
    _delay_us(BIT_US);

    for (i = 0; i < 8; i++)
    {
        if (byte & 0x01)
            PORTD |= (1 << TX_PIN);
        else
            PORTD &= ~(1 << TX_PIN);

        byte >>= 1;
        _delay_us(BIT_US);
    }

    PORTD |= (1 << TX_PIN);                 // Stop bit
    _delay_us(BIT_US);

    SREG = sreg;                            // 인터럽트 상태 복구
}

void SEND_softString(char* str)
{
    while (*str)
        SEND_softByte((uint8_t)*str++);
}
