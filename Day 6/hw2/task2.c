/*

Day 6 - 과제 2

[메모]

모터 A : IN1 = PB0(5번), IN2 = PB1(7번), ENA = PB5(6번, OC1A), OUT1(2번) / OUT2(3번)
모터 B : IN3 = PB2(10번), IN4 = PB3(12번), ENB = PB6(11번, OC1B), OUT3(13번) / OUT4(14번)
SW1   : PC0 - 모터 A 정방향
SW2   : PC1 - 모터 A 역방향
SW3   : PD2 - 모터 B 정방향
SW4   : PD3 - 모터 B 역방향
LED   : PORTA (Active Low). LED0 ~ LED3 이 각 버튼에 대응

버튼 하나가 모터 한 개의 한 방향을 맡음 누르고 있는 동안만 돌고 떼면 선다.
A 와 B 가 서로 독립이라 SW1 과 SW3 을 같이 누르면 둘 다 정방향으로 회전
같은 모터의 정과 역을 동시에 누르면 IN 두 개가 같아져 애매해지므로 그냥 정지로 본다.

방향은 IN 두 핀의 조합이 정하고 속도는 EN 핀의 PWM 듀티가 정한다. 둘은 독립이다.
 
IN1 = 1, IN2 = 0 -> 정방향
IN1 = 0, IN2 = 1 -> 역방향
IN1 = IN2        -> 정지

PWM 은 5일차 자료 기준으로 Fast PWM 모드 14(TOP = ICR1), 분주 1, 5kHz 다.
   TOP = F_CPU / (분주 x 주파수) - 1 = 16,000,000 / (1 x 5,000) - 1 = 3199

돌고 있는 모터에 반대 전압을 바로 걸면 역기전력이 더해져 전류가 크게 튄다.
그래서 방향이 뒤집힐 때만 잠깐 끊었다 간다. 손을 떼는 것만으로는 실제로 안 멈추므로
마지막으로 실제 구동한 방향을 따로 기억해 두고 비교한다.

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

#define PWM_TOP     3199                    // 5kHz
#define SPEED       50                      // 구동 속도(%). 30 아래는 정지 마찰을 못 이긴다
#define DEADTIME_MS 150                     // 방향이 뒤집힐 때 끼워 넣는 정지 시간

#define IN1         PB0
#define IN2         PB1
#define ENA         PB5                     // OC1A
#define IN3         PB2
#define IN4         PB3
#define ENB         PB6                     // OC1B

#define IN_MASK     ((1 << IN1) | (1 << IN2) | (1 << IN3) | (1 << IN4))
#define EN_MASK     ((1 << ENA) | (1 << ENB))

#define DIR_STOP    0
#define DIR_FWD     1
#define DIR_REV     2

#define SW1_ON()    (!(PINC & (1 << PINC0)))    // 외부 풀업이라 눌리면 LOW
#define SW2_ON()    (!(PINC & (1 << PINC1)))
#define SW3_ON()    (!(PIND & (1 << PIND2)))
#define SW4_ON()    (!(PIND & (1 << PIND3)))

void INIT_motor(void);
void INIT_io(void);
void SET_motorA(int16_t pct);
void SET_motorB(int16_t pct);
uint16_t CONVERT_pctToDuty(int16_t pct);
uint8_t READ_dir(uint8_t fwd, uint8_t rev);
int16_t CONVERT_dirToPct(uint8_t dir);
void UPDATE_leds(uint8_t dirA, uint8_t dirB);

int main(void)
{
    uint8_t dirA;
    uint8_t dirB;
    uint8_t lastA = DIR_STOP;               // 마지막으로 실제 구동한 방향
    uint8_t lastB = DIR_STOP;

    MCUCSR |= (1 << JTD);                   // PF4 ~ PF7 을 JTAG 에서 떼어낸다
    MCUCSR |= (1 << JTD);                   // 이 과제에는 영향이 없지만 습관으로 둔다

    INIT_motor();                           // 모터를 가장 먼저 정지 상태로 확정한다
    INIT_io();

    while (1)
    {
        dirA = READ_dir(SW1_ON(), SW2_ON());
        dirB = READ_dir(SW3_ON(), SW4_ON());

        /* 모터 A : 돌던 방향과 반대로 가려 할 때만 한 번 끊는다 */
        if (dirA != DIR_STOP && dirA != lastA)
        {
            if (lastA != DIR_STOP)
            {
                SET_motorA(0);
                _delay_ms(DEADTIME_MS);
            }

            lastA = dirA;
        }

        /* 모터 B : 동일 */
        if (dirB != DIR_STOP && dirB != lastB)
        {
            if (lastB != DIR_STOP)
            {
                SET_motorB(0);
                _delay_ms(DEADTIME_MS);
            }

            lastB = dirB;
        }

        SET_motorA(CONVERT_dirToPct(dirA));
        SET_motorB(CONVERT_dirToPct(dirB));
        UPDATE_leds(dirA, dirB);

        _delay_ms(10);                      // 10ms 주기 폴링. 채터링이 자연스럽게 걸러짐
    }
}

/* ---- 모터 초기화 ---- */
/*

PORTB 를 먼저 0 으로 만든 뒤에 DDRB 를 출력으로 바꾼다.
순서가 반대면 핀이 출력으로 바뀌는 순간 이전 래치 값이 그대로 나가서
리셋 직후 모터가 한 번 튈 수 있다.

*/
void INIT_motor(void)
{
    PORTB &= ~(IN_MASK | EN_MASK);
    DDRB |= (IN_MASK | EN_MASK);

    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
    // Fast PWM 모드 14(WGM13:0 = 1110), 비반전 출력, 분주 1

    ICR1 = PWM_TOP;
    OCR1A = 0;                              // 듀티 0 으로 시작
    OCR1B = 0;
}

void INIT_io(void)
{
    DDRA = 0xFF;
    PORTA = 0xFF;                           // Active Low 라 전부 소등

    DDRC &= ~((1 << PC0) | (1 << PC1));     // SW1, SW2 입력
    PORTC |= (1 << PC0) | (1 << PC1);       // 외부 풀업이 있지만 보험으로 켜둔다

    DDRD &= ~((1 << PD2) | (1 << PD3));     // SW3, SW4 입력
    PORTD |= (1 << PD2) | (1 << PD3);
}

/* ---- 모터 제어 ---- */
/*

속도는 -100 ~ +100. 부호가 방향이고 크기가 듀티
이렇게 두면 위쪽 로직이 단순해지고 나중에 라인트레이서에서 그대로 쓸 수 있음

*/
uint16_t CONVERT_pctToDuty(int16_t pct)
{
    if (pct < 0)
        pct = -pct;

    if (pct > 100)
        pct = 100;

    return (uint16_t)(((uint32_t)PWM_TOP * (uint16_t)pct) / 100);
}

void SET_motorA(int16_t pct)
{
    if (pct > 0)
    {
        PORTB |= (1 << IN1);
        PORTB &= ~(1 << IN2);
    }
    else if (pct < 0)
    {
        PORTB &= ~(1 << IN1);
        PORTB |= (1 << IN2);
    }
    else
    {
        PORTB &= ~((1 << IN1) | (1 << IN2));
    }

    OCR1A = CONVERT_pctToDuty(pct);
}

void SET_motorB(int16_t pct)
{
    if (pct > 0)
    {
        PORTB |= (1 << IN3);
        PORTB &= ~(1 << IN4);
    }
    else if (pct < 0)
    {
        PORTB &= ~(1 << IN3);
        PORTB |= (1 << IN4);
    }
    else
    {
        PORTB &= ~((1 << IN3) | (1 << IN4));
    }

    OCR1B = CONVERT_pctToDuty(pct);
}

/* ---- 버튼 해석 ---- */
/*

정방향과 역방향을 동시에 누른 경우는 모순이므로 정지로 해석
IN 두 핀을 같은 값으로 두면 급제동이 걸리는데, 그런 애매한 상태를 만들지 않는다.

*/
uint8_t READ_dir(uint8_t fwd, uint8_t rev)
{
    if (fwd && !rev)
        return DIR_FWD;

    if (rev && !fwd)
        return DIR_REV;

    return DIR_STOP;
}

int16_t CONVERT_dirToPct(uint8_t dir)
{
    if (dir == DIR_FWD)
        return SPEED;

    if (dir == DIR_REV)
        return -SPEED;

    return 0;
}

/* ---- LED ---- */
void UPDATE_leds(uint8_t dirA, uint8_t dirB)
{
    uint8_t mask = 0;

    if (dirA == DIR_FWD)
        mask |= (1 << 0);                   // LED0 : 모터 A 정방향

    if (dirA == DIR_REV)
        mask |= (1 << 1);                   // LED1 : 모터 A 역방향

    if (dirB == DIR_FWD)
        mask |= (1 << 2);                   // LED2 : 모터 B 정방향

    if (dirB == DIR_REV)
        mask |= (1 << 3);                   // LED3 : 모터 B 역방향

    PORTA = (uint8_t)~mask;                 // Active Low 라 내보낼 때 뒤집는다
}
