# 3일차 과제 2 : UART 통신 기반 LED 제어

> **광운대학교 로봇학부**  
> **작성자:** 이동엽
> **제출일:** 2026년 8월 2일

---

## 1. 개요 (Overview)
본 과제는 PC 시리얼 터미널과 ATmega128 사이에 UART 통신 경로를 구성하고, PC에서 보낸 문자로 LED를 제어하는 것을 목표로 함. 보드에서 PC 방향의 송신과 PC에서 보드 방향의 수신을 모두 다루며, 스위치 입력에 대한 응답까지 포함함.

### 핵심 목표
* USART 레지스터 설정을 통한 9600bps 8N1 통신로 확보
* 수신 문자에 따른 LED 점등 및 좌우 이동 처리
* 스위치 엣지 검출을 통한 상태 초기화 및 "RESET" 응답
* 정의되지 않은 입력에 대한 예외처리

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Visual Studio Code / AVR-GCC 14.3.0 (macOS) |
| **Flasher Tool** | NEWTC AD-USBISP V7.0 (STK500v2) / avrdude |
| **Terminal** | Terminal v1.9b (Windows) |
| **언어** | C Language |
| **주요 부품** | ROBIT 실습보드, LED 8개, 택트 스위치 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                 [Target Component]
 PA0 ~ PA7           ----->   LED 8개 (Active Low)
 PC0                 <-----   SW1 (상태 초기화)
 PE0 (RXD0)          <-----   USB-Serial 어댑터 TX
 PE1 (TXD0)          ----->   USB-Serial 어댑터 RX
 PE2                 ----->   MAX485 RE, DE
```

어댑터의 VCC는 연결하지 않음. 보드에 이미 전원이 인가되어 있어 전원끼리 충돌함.

### UART0과 UART1 중 UART0을 선택한 근거

ATmega128은 UART가 2개이므로 어느 쪽을 PC에 연결할지 선택해야 함. 양쪽 모두 다른 기능과 핀을 공유하고 있어 장단점이 갈림.

| 항목 | UART0 (PE0, PE1) | UART1 (PD2, PD3) |
| :--- | :--- | :--- |
| 공유 대상 | ISP (PDI, PDO), MAX485 | SW3, SW4 (10kohm 풀업 + 104 커패시터) |
| 9600bps 수신 | 정상 | **문자 깨짐** |
| 플래시 중 충돌 | 있음 (TX선 분리 필요) | 없음 |

UART1은 플래시할 때 선을 뺄 필요가 없어 작업 편의성이 높으나, 실제 시험에서 9600bps 수신이 불안정함. 0~9를 차례로 입력했을 때 0, 2, 8만 정상 인식되고 나머지는 전부 예외 처리로 빠짐. 하위 4비트 중 b0 또는 b2가 1인 문자만 실패하는 규칙성이 나타났으며, 이는 PD2에 연결된 104(0.1uF) 커패시터가 수신 파형의 상승 에지를 둔화시켜 발생한 것으로 판단함. 보레이트를 4800으로 낮추면 정상 동작하나 요구 사양인 9600을 만족하지 못함.

UART0은 9600bps에서 0~9 전 구간이 정상 인식됨. 본 과제는 Dynamixel을 사용하지 않아 MAX485가 UART0을 점유할 일이 없으므로, **UART0을 사용하고 플래시 시 어댑터 TX선을 분리하는 방식**을 채택함.

### MAX485 처리
MAX485가 PE0, PE1에 상시 연결되어 있어 RO 출력이 PE0을 함께 구동함. 어댑터 TX와 충돌하므로 PE2를 High로 설정하여 RE를 비활성화함. MAX485의 RO는 3상태 출력이므로 RE가 High이면 하이임피던스가 되어 PE0을 놓아줌. 이 처리로 485 모듈을 분리하지 않고 그대로 둔 채 통신이 가능함.

---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c), 선언부(.h)만 구조에 표기함.
```text
├── Day 3/hw2/
│   ├── hw2.c       # UART 설정, 수신 처리, LED 제어
│   └── REPORT.md
```

Makefile 이나 프로젝트 파일 없이 아래 명령만으로 빌드됨.

```text
avr-gcc -mmcu=atmega128 -Os -Wall -std=gnu99 -o firmware.elf hw2.c
avr-objcopy -O ihex -R .eeprom firmware.elf firmware.hex
```

-Os 는 필수. 최적화를 끄면 _delay 계열 함수가 설계대로 동작하지 않음.

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### UART0 초기화 (hw2.c)
```c
void INIT_uart(void)
{
    UCSR0A = (1 << U2X0);
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
}
```

| 레지스터 | 설정 | 의미 |
| :--- | :--- | :--- |
| UCSR0A | U2X0 = 1 | 2배속 모드. 분주비 16에서 8로 낮춰 보레이트 오차를 줄임 |
| UBRR0 | 207 | 9600bps. 16MHz 기준 오차 +0.16% |
| UCSR0C | UCSZ01, UCSZ00 = 1 | 데이터 8비트 |
| UCSR0B | TXEN0, RXEN0 = 1 | 송수신 활성화. 해당 핀이 I/O가 아닌 통신 핀으로 동작 |

UBRR 값은 U2X 설정에 따라 달라짐. U2X = 1일 때 `UBRR = F_CPU / (8 x baud) - 1`이므로 `16000000 / (8 x 9600) - 1 = 207.3`에서 207을 사용함. U2X = 0이면 같은 9600bps에 103을 써야 함.

### 수신 처리
```c
uint8_t isRECEIVED_char(char* out)
{
    if (!(UCSR0A & (1 << RXC0)))
        return 0;                           // 없으면 기다리지 않는다

    *out = (char)UDR0;

    return 1;
}
```
RXC0가 1이면 수신 버퍼에 읽지 않은 데이터가 있음을 의미함. 데이터가 없을 때 대기하는 방식으로 구현하면 그동안 스위치 입력을 읽지 못하므로, 즉시 반환하는 비대기 방식으로 작성하여 메인 루프가 수신과 스위치를 번갈아 확인할 수 있도록 함.

### LED 좌우 이동
```c
uint8_t ROTATE_left(uint8_t value)
{
    return (uint8_t)((value << 1) | (value >> 7));
}
```
단순 시프트를 사용하면 끝에 도달했을 때 LED가 모두 꺼짐. 밀려난 비트를 반대편으로 되돌리는 회전 연산으로 처리하여 PA7에서 PA0으로 순환하도록 함.

### 예외처리

| 입력 | 처리 |
| :--- | :--- |
| 0 ~ 7 | 해당 번호 LED 점등 후 "n LED on" 전송 |
| 8, 9 | 좌우 회전 후 "LEFT", "RIGHT" 전송 |
| CR, LF | 터미널의 엔터 입력이므로 무시. 응답하지 않음 |
| 그 외 문자 | "ERROR: x" 전송. 제어문자는 터미널 출력이 깨지므로 물음표로 치환 |

### 스위치 엣지 검출
```c
swNow = (PINC & (1 << SW_RESET_PIN)) ? 0 : 1;

if (swNow && !swPrev)                       // 하강 엣지에서만
{
    RESET_state();
    _delay_ms(20);                          // 접점 떨림 무시
}

swPrev = swNow;
```
현재 상태만 확인하면 스위치를 누르고 있는 동안 루프마다 "RESET"이 반복 전송됨. 직전 상태와 비교하여 눌리는 순간에만 1회 반응하도록 처리함.

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오
1. 전원 인가 시 PE2를 High로 설정해 MAX485를 PE0에서 분리함
2. LED 8개가 전체로 3회 점멸하여 업로드 완료를 표시함
3. UART0을 9600bps로 열고 "READY"를 전송함
4. PC에서 0~7 입력 시 해당 LED 점등 및 "n LED on" 응답
5. 8, 9 입력 시 LED가 좌우로 1칸 이동하고 "LEFT", "RIGHT" 응답
6. 정의되지 않은 문자 입력 시 "ERROR: x" 응답
7. SW1 입력 시 LED 전체 소등 및 "RESET" 응답

```text
READY
3 LED on
LEFT
RIGHT
ERROR: a
RESET
```

### 검증 과정에서 확인한 사항

**터미널 보레이트 불일치**
초기 시험에서 "READY"가 "r" 한 글자로만 표시되고 이후 입력한 문자가 알파벳에 기호가 붙은 형태로 깨져 출력됨. 보드 측 송신을 확인하기 위해 TX 핀에 0x00을 연속 송신하는 코드를 작성하고 멀티미터로 전압을 측정함. 유휴 상태의 5V가 아닌 330mV가 측정되어 송신 자체는 정상임을 확인함. 원인은 터미널 프로그램의 보레이트가 4800으로 설정되어 있었던 것이며, 9600으로 변경 후 정상 동작함.

**AD-USBISP의 UART 기능 제약**
사용 중인 프로그래머는 ISP와 USB-UART 기능을 겸하나, 데이터시트에 "USB-UART로 사용하실 경우 ISP 커넥터에는 커넥터를 연결하시면 안됩니다. 데이터 수신부가 합선되어 문제가 생길 수 있습니다"라는 제약이 명시되어 있음. 또한 macOS에서는 UART 모드가 동작하지 않아 Windows 환경에서 시연을 진행함.

**UART1 수신 불량**
3절에 기술한 대로 UART1의 9600bps 수신이 불안정하여 UART0으로 최종 결정함.

### 동작 사진 / 영상

| 시연 영상 |
| :---: |
| [3일차 과제 2 시연 영상](https://drive.google.com/file/d/1qMKi_23b2t1Endd5pBOP_HigIrn0Js8X/view?usp=sharing) |

---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **Claude** | 디버깅 & 문제 진단 | - 문자 깨짐 현상의 원인 범위 좁히기 (보레이트, 커패시터 영향)<br>- 송신 정상 여부를 확인할 검증 방법 검토 |
| **Claude** | 구조 검토 | - UART0과 UART1 중 어느 쪽을 쓸지에 대한 판단 근거 검토 |

### AI 활용 및 검증 원칙
1. **코드 검증:** 작성된 코드는 -Wall -Wextra 옵션으로 경고 없이 컴파일되는지 확인하고, 실제 보드에서 0부터 9까지 전 구간과 예외 입력을 하나씩 대조하여 검증하였습니다.
2. **문제 진단:** 문자 깨짐 현상은 AI의 추측에 의존하지 않고, TX 핀 전압 측정이라는 물리적 근거로 송신 정상 여부를 먼저 확정한 뒤 원인을 좁혀 나갔습니다.
3. **학습 주도성:** 코드 작성과 보고서 작성은 직접 수행하였으며, AI는 원인 파악이 막혔을 때 가능성을 좁히고 설계 판단의 근거를 함께 검토하는 용도로만 활용하였습니다. UART0과 UART1 중 어느 쪽을 사용할지에 대한 최종 결정은 실제 측정 결과를 근거로 직접 내렸습니다.
