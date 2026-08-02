/*

Day 3 - 과제 3

[메모]

가변저항    : PF0 (ADC0)
LCD       : I2C, PD0 = SCL, PD1 = SDA
Dynamixel : UART0 -> MAX485 (RO = PE0, DI = PE1, RE/DE = PE2), 57600
PC        : UART1 (RXD1 = PD2, TXD1 = PD3)

UART1 로 통신. UART0으로 하면 에러

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "LCD_I2C.h"

#define INITIAL "LDY"

/*

Dynamixel Protocol 2.0 Control Table

*/
#define ADDR_OPERATING_MODE  11             // 3 이면 위치 제어 모드
#define ADDR_TORQUE_ENABLE   64
#define ADDR_HW_ERROR        70             // 0 이면 정상
#define ADDR_PROFILE_ACC    108             // 가속도. 0 이면 무제한이라 급출발
#define ADDR_PROFILE_VEL    112             // 위치 제어 모드에서의 이동 속도
#define ADDR_GOAL_POSITION  116
#define ADDR_PRESENT_POS    132

#define DIAG 1                              // 1 이면 서보 상태를 PC 로 주기 보고

#define PROFILE_ACC 20                      // 0 이면 급출발 -> Overload(ERR 32)

#define VEL_MAX 300                         // PC 입력 9 일 때
#define VEL_MIN 1                           // 0 은 "속도 무제한" 이라 하한으로 막음

#define POS_SCALE 1                         // 명세가 0 ~ 1023. 전체 범위를 쓰려면 4

#define RS485_DE PE2                        // MAX485 RE, DE (1 = 송신, 0 = 수신)

#define PC_UBRR 415                         // 4800. PD2 커패시터 때문에 9600 은 깨짐

uint8_t dxlId = 0;                          // PING 으로 찾아낸 ID
uint8_t dxlParam[4];                        // 마지막 Status Packet 의 PARAM

void INIT_dxlUart(void);
void FLUSH_dxlRx(void);
void SEND_rs485(uint8_t* buf, uint8_t len);
int16_t READ_dxlByte(uint16_t timeout);
void INIT_pcUart(void);
void SEND_pcChar(char c);
void SEND_pcString(char* str);
uint8_t isRECEIVED_pcChar(char* out);
uint16_t CALC_dxlCrc(uint8_t* data, uint8_t len);
void SEND_dxlPacket(uint8_t id, uint8_t* body, uint8_t bodyLen);
uint8_t isRECEIVED_dxlStatus(void);
uint8_t READ_dxlRegister(uint8_t id, uint16_t addr, uint8_t len);
uint8_t isFOUND_dxl(void);
uint8_t REBOOT_dxl(uint8_t id);
uint8_t WRITE_dxlByte(uint8_t id, uint16_t addr, uint8_t value);
uint8_t WRITE_dxlLong(uint8_t id, uint16_t addr, uint32_t value);
uint16_t READ_adc(void);
void PRINT_lcdLine(uint8_t line, char* str);
void WAIT_dxl(void);

int main(void)
{
    uint16_t adc;
    uint16_t goalPos = 0;
    uint16_t prevPos = 0xFFFF;              // 처음 한 번은 반드시 보내게
    uint16_t goalVel = VEL_MIN;
    uint16_t present;
    uint8_t mode;
    uint8_t torque;
    uint8_t hwerr;
    uint8_t diagTick = 0;
    uint8_t i;
    char c;
    char buf[32];                           // 가장 긴 게 상태 보고 26바이트

    // 리셋 직후 PE2 가 떠 있으면 DE 가 올라가 버스를 점유한다. 제일 먼저 고정
    DDRE |= (1 << RS485_DE);
    PORTE &= ~(1 << RS485_DE);

    DDRA = 0xFF;                            // LED 를 출력용(1)으로 설정
    PORTA = 0xFF;                           // Active Low 라 전부 소등

    DDRF &= ~(1 << PF0);                    // 가변저항을 입력용(0)으로 설정
    PORTF &= ~(1 << PF0);                   // ADC 핀은 내부 풀업을 꺼야 한다

    ADMUX = (1 << REFS0);                   // 기준전압 AVCC(5V), 채널 ADC0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // 분주 128 -> 125kHz. ADC 는 50 ~ 200kHz 에서만 10비트 정확도가 나온다

    INIT_dxlUart();
    INIT_pcUart();
    lcdInit();

    PRINT_lcdLine(0, "Dynamixel...");
    PRINT_lcdLine(1, "searching");
    SEND_pcString("READY\r\n");

    WAIT_dxl();

    sprintf(buf, "FOUND ID %d", dxlId);
    SEND_pcString(buf);
    SEND_pcString("\r\n");

    // 에러가 남아 있으면 토크가 안 걸린다. 리부트 말곤 지울 방법이 없다
    if (READ_dxlRegister(dxlId, ADDR_HW_ERROR, 1) && dxlParam[0] != 0)
    {
        sprintf(buf, "HW ERROR %d -> REBOOT\r\n", dxlParam[0]);
        SEND_pcString(buf);
        PRINT_lcdLine(0, "HW ERROR");
        PRINT_lcdLine(1, "rebooting...");

        REBOOT_dxl(dxlId);
        _delay_ms(2000);                    // 서보가 다시 올라올 때까지

        WAIT_dxl();
    }

    WRITE_dxlByte(dxlId, ADDR_TORQUE_ENABLE, 0);        // 설정 전 토크 해제
    WRITE_dxlLong(dxlId, ADDR_PROFILE_ACC, PROFILE_ACC);
    WRITE_dxlLong(dxlId, ADDR_PROFILE_VEL, goalVel);
    WRITE_dxlByte(dxlId, ADDR_TORQUE_ENABLE, 1);

    // MODE 3 = 위치 제어, TQ 1 = 토크 인가됨, ERR 0 = 정상
    mode = 0;
    torque = 0;
    hwerr = 0;

    if (READ_dxlRegister(dxlId, ADDR_OPERATING_MODE, 1))
        mode = dxlParam[0];

    if (READ_dxlRegister(dxlId, ADDR_TORQUE_ENABLE, 1))
        torque = dxlParam[0];

    if (READ_dxlRegister(dxlId, ADDR_HW_ERROR, 1))
        hwerr = dxlParam[0];

    sprintf(buf, "MODE %d TQ %d ERR %d\r\n", mode, torque, hwerr);
    SEND_pcString(buf);

    lcdClear();

    while (1)
    {
        // PC 에서 0 ~ 9 -> 목표 속도 0 ~ 300
        if (isRECEIVED_pcChar(&c))
        {
            if (c >= '0' && c <= '9')
            {
                goalVel = (uint16_t)((uint32_t)(c - '0') * VEL_MAX / 9);

                if (goalVel < VEL_MIN)
                    goalVel = VEL_MIN;

                WRITE_dxlLong(dxlId, ADDR_PROFILE_VEL, goalVel);

                sprintf(buf, "SPEED %d\r\n", goalVel);
                SEND_pcString(buf);
            }
            else if (c != '\r' && c != '\n')
            {
                SEND_pcString("ERROR\r\n");  // 예외 처리
            }
        }

        adc = READ_adc();
        goalPos = adc * POS_SCALE;

        // ADC 는 가만히 둬도 1 ~ 2 씩 떨린다. 4 이상 변했을 때만 보낸다
        if (goalPos > prevPos + 3 || goalPos + 3 < prevPos)
        {
            WRITE_dxlLong(dxlId, ADDR_GOAL_POSITION, goalPos);
            prevPos = goalPos;
        }

        sprintf(buf, "SPEED:%3d   %s", goalVel, INITIAL);
        PRINT_lcdLine(0, buf);

        sprintf(buf, "POS  :%4d", goalPos);
        PRINT_lcdLine(1, buf);

        i = (uint8_t)(adc >> 7);            // 0 ~ 1023 을 0 ~ 7 로
        PORTA = (uint8_t)~(1 << i);         // 돌리는 게 눈에 보이도록 LED 막대

        // 2초마다 상태 확인. 과부하로 잠기면 여기서 되살린다
        if (++diagTick >= 40)               // 50ms * 40 = 2초
        {
            diagTick = 0;
            present = 0;
            torque = 0;
            hwerr = 0;

            if (READ_dxlRegister(dxlId, ADDR_PRESENT_POS, 4))
                present = (uint16_t)dxlParam[0] | ((uint16_t)dxlParam[1] << 8);

            if (READ_dxlRegister(dxlId, ADDR_TORQUE_ENABLE, 1))
                torque = dxlParam[0];

            if (READ_dxlRegister(dxlId, ADDR_HW_ERROR, 1))
                hwerr = dxlParam[0];

#if DIAG
            sprintf(buf, "G %d P %d V %d TQ %d E %d\r\n",
                    goalPos, present, goalVel, torque, hwerr);
            SEND_pcString(buf);
#endif

            if (hwerr != 0)
            {
                sprintf(buf, "E %d -> REBOOT\r\n", hwerr);
                SEND_pcString(buf);

                REBOOT_dxl(dxlId);
                _delay_ms(2000);

                WRITE_dxlByte(dxlId, ADDR_TORQUE_ENABLE, 0);
                WRITE_dxlLong(dxlId, ADDR_PROFILE_ACC, PROFILE_ACC);
                WRITE_dxlLong(dxlId, ADDR_PROFILE_VEL, goalVel);
                WRITE_dxlByte(dxlId, ADDR_TORQUE_ENABLE, 1);

                prevPos = 0xFFFF;           // 목표 위치를 다시 보내게
            }
            else if (torque == 0)           // 에러는 없는데 토크만 꺼진 경우
            {
                WRITE_dxlByte(dxlId, ADDR_TORQUE_ENABLE, 1);
            }
        }

        _delay_ms(50);
    }
}

/* ---- UART0 : Dynamixel ---- */
void INIT_dxlUart(void)
{
    UCSR0A = (1 << U2X0);
    UBRR0H = 0;
    UBRR0L = 34;                            // 57600 baud (오차 -0.79%)
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
}

void FLUSH_dxlRx(void)
{
    while (UCSR0A & (1 << RXC0))
        (void)UDR0;
}

/* ---- 485 송신 ---- */
void SEND_rs485(uint8_t* buf, uint8_t len)
{
    uint8_t i;

    PORTE |= (1 << RS485_DE);               // 송신 모드
    _delay_us(2);
    UCSR0A |= (1 << TXC0);                  // TXC0 는 1 을 써서 지운다

    for (i = 0; i < len; i++)
    {
        while (!(UCSR0A & (1 << UDRE0)));

        UDR0 = buf[i];
    }

    while (!(UCSR0A & (1 << TXC0)));        // UDRE 로 내리면 마지막 바이트가 잘린다

    PORTE &= ~(1 << RS485_DE);              // 수신 모드
    FLUSH_dxlRx();                          // 전환할 때 생기는 쓰레기 버리기
}

/* ---- 485 수신 (timeout 은 10us 단위) ---- */
int16_t READ_dxlByte(uint16_t timeout)
{
    while (timeout--)
    {
        if (UCSR0A & (1 << RXC0))
            return (int16_t)UDR0;

        _delay_us(10);
    }

    return -1;                              // 못 받음
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
        return 0;                           // 없으면 기다리지 않는다

    *out = (char)UDR1;

    return 1;
}

/* ---- CRC-16 (다항식 0x8005) ---- */
uint16_t CALC_dxlCrc(uint8_t* data, uint8_t len)
{
    uint16_t crc = 0;
    uint8_t i;
    uint8_t bit;

    for (i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i] << 8;

        for (bit = 0; bit < 8; bit++)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x8005)
                                 : (uint16_t)(crc << 1);
    }

    return crc;
}

/* ---- 패킷 송신 ---- */
/* body 는 INSTRUCTION 부터 마지막 PARAM 까지 */
void SEND_dxlPacket(uint8_t id, uint8_t* body, uint8_t bodyLen)
{
    uint8_t p[20];
    uint8_t n = 0;
    uint8_t i;
    uint16_t len = (uint16_t)bodyLen + 2;
    uint16_t crc;

    p[n++] = 0xFF;
    p[n++] = 0xFF;
    p[n++] = 0xFD;
    p[n++] = 0x00;
    p[n++] = id;
    p[n++] = (uint8_t)len;
    p[n++] = (uint8_t)(len >> 8);

    for (i = 0; i < bodyLen; i++)
        p[n++] = body[i];

    crc = CALC_dxlCrc(p, n);
    p[n++] = (uint8_t)crc;
    p[n++] = (uint8_t)(crc >> 8);

    SEND_rs485(p, n);
}

/* ---- Status Packet 수신 (결과는 dxlId, dxlParam) ---- */
uint8_t isRECEIVED_dxlStatus(void)
{
    uint8_t buf[24];
    uint8_t hdr[4] = { 0xFF, 0xFF, 0xFD, 0x00 };
    uint8_t n = 0;
    uint8_t match = 0;
    uint8_t i;
    int16_t c;
    uint16_t len;
    uint16_t crc;

    while (match < 4)                       // 헤더 탐색. 0xFF 만나면 거기서 다시 센다
    {
        c = READ_dxlByte(2000);             // 20ms

        if (c < 0)
            return 0;

        if ((uint8_t)c == hdr[match])
            match++;
        else
            match = ((uint8_t)c == 0xFF) ? 1 : 0;
    }

    for (i = 0; i < 4; i++)
        buf[n++] = hdr[i];

    for (i = 0; i < 3; i++)                 // ID, LEN_L, LEN_H
    {
        c = READ_dxlByte(500);

        if (c < 0)
            return 0;

        buf[n++] = (uint8_t)c;
    }

    dxlId = buf[4];
    len = (uint16_t)buf[5] | ((uint16_t)buf[6] << 8);

    if (len < 4 || len > (uint16_t)(sizeof(buf) - 7))
        return 0;                           // 길이가 이상하면 버린다

    for (i = 0; i < len; i++)
    {
        c = READ_dxlByte(500);

        if (c < 0)
            return 0;

        buf[n++] = (uint8_t)c;
    }

    if (buf[7] != 0x55)
        return 0;                           // Status Packet 이 아니다

    crc = CALC_dxlCrc(buf, (uint8_t)(n - 2));

    if ((uint8_t)crc != buf[n - 2] || (uint8_t)(crc >> 8) != buf[n - 1])
        return 0;

    for (i = 0; i < 4; i++)                 // PARAM 은 buf[9] 부터. buf[8] 은 ERROR
        dxlParam[i] = (9 + i < n - 2) ? buf[9 + i] : 0;

    return 1;
}

/* ---- 레지스터 읽기 (결과는 dxlParam) ---- */
uint8_t READ_dxlRegister(uint8_t id, uint16_t addr, uint8_t len)
{
    uint8_t body[5];

    body[0] = 0x02;                         // INST_READ
    body[1] = (uint8_t)addr;
    body[2] = (uint8_t)(addr >> 8);
    body[3] = len;
    body[4] = 0;

    SEND_dxlPacket(id, body, 5);

    return isRECEIVED_dxlStatus();
}

/* ---- 브로드캐스트 PING ---- */
uint8_t isFOUND_dxl(void)
{
    uint8_t body[1] = { 0x01 };             // INST_PING

    SEND_dxlPacket(0xFE, body, 1);          // 0xFE = 브로드캐스트

    return isRECEIVED_dxlStatus();
}

/* ---- 서보 재시작 ---- */
uint8_t REBOOT_dxl(uint8_t id)
{
    uint8_t body[1] = { 0x08 };             // INST_REBOOT

    SEND_dxlPacket(id, body, 1);

    return isRECEIVED_dxlStatus();
}

/* ---- 레지스터 쓰기 ---- */
uint8_t WRITE_dxlByte(uint8_t id, uint16_t addr, uint8_t value)
{
    uint8_t body[4];

    body[0] = 0x03;                         // INST_WRITE
    body[1] = (uint8_t)addr;
    body[2] = (uint8_t)(addr >> 8);
    body[3] = value;

    SEND_dxlPacket(id, body, 4);

    return isRECEIVED_dxlStatus();
}

uint8_t WRITE_dxlLong(uint8_t id, uint16_t addr, uint32_t value)
{
    uint8_t body[7];

    body[0] = 0x03;
    body[1] = (uint8_t)addr;
    body[2] = (uint8_t)(addr >> 8);
    body[3] = (uint8_t)value;               // 낮은 바이트부터 (리틀 엔디안)
    body[4] = (uint8_t)(value >> 8);
    body[5] = (uint8_t)(value >> 16);
    body[6] = (uint8_t)(value >> 24);

    SEND_dxlPacket(id, body, 7);

    return isRECEIVED_dxlStatus();
}

/* ---- 서보를 찾을 때까지 PING ---- */
void WAIT_dxl(void)
{
    while (!isFOUND_dxl())
    {
        PORTA = ~0x20;
        _delay_ms(200);
        PORTA = 0xFF;
        _delay_ms(300);
    }
}

/* ---- 가변저항 ---- */
uint16_t READ_adc(void)
{
    ADCSRA |= (1 << ADSC);                  // 변환 시작
    while (ADCSRA & (1 << ADSC));           // 끝나면 ADSC 가 저절로 0

    return ADC;                             // 0 ~ 1023
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
