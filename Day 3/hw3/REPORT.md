# 3일차 과제 3 : RS-485 기반 Dynamixel 위치 및 속도 제어

> **광운대학교 로봇학부**  
> **작성자:** 이동엽
> **제출일:** 2026년 8월 2일

---

## 1. 개요 (Overview)
본 과제는 MAX485 트랜시버를 통해 Dynamixel MX-64와 RS-485로 통신하며, 가변저항으로 목표 위치를, PC 입력으로 목표 속도를 제어하는 것을 목표로 함. 두 개의 UART를 동시에 운용하고, 반이중 통신의 방향 전환과 프로토콜 패킷 구성을 직접 다룸.

### 핵심 목표
* MAX485를 이용한 반이중 RS-485 통신 구현
* Dynamixel Protocol 2.0 패킷 생성 및 CRC 검증
* UART 2채널 동시 운용 (UART0 = 서보, UART1 = PC)
* ADC 값을 목표 위치로, PC 입력을 목표 속도로 변환
* 하드웨어 에러 검출 및 자동 복구

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Visual Studio Code / AVR-GCC 14.3.0 (macOS) |
| **Flasher Tool** | NEWTC AD-USBISP V7.0 (STK500v2) / avrdude |
| **Terminal** | Terminal v1.9b (Windows) |
| **언어** | C Language |
| **주요 부품** | ROBIT 실습보드, MAX485, Dynamixel MX-64, I2C 텍스트 LCD (16x2), 가변저항 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                 [Target Component]
 PF0 (ADC0)          <-----   가변저항
 PD0 (SCL)           <---->   I2C Text LCD 16x2
 PD1 (SDA)           <---->   I2C Text LCD 16x2
 PE0 (RXD0)          <-----   MAX485 RO
 PE1 (TXD0)          ----->   MAX485 DI
 PE2                 ----->   MAX485 RE + DE (묶여 있음)
 PD2 (RXD1)          <-----   USB-Serial 어댑터 TX
 PD3 (TXD1)          ----->   USB-Serial 어댑터 RX
 PA0 ~ PA7           ----->   LED 8개 (Active Low, 위치 표시용)

[MAX485]                    [Dynamixel MX-64]
 D+ (A)              <---->   D+
 D- (B)              <---->   D-
                              12V, GND는 파워서플라이에서 별도 공급
```

### UART 포트 배분
MAX485가 PE0, PE1에 하드웨어로 고정되어 있어 Dynamixel은 UART0을 사용할 수밖에 없음. 따라서 PC는 남은 UART1을 사용함. 과제 2에서는 PC를 UART0에 연결했으나, 본 과제는 두 대상과 동시에 통신해야 하므로 케이블을 UART1로 이설함.

UART1의 PD2, PD3는 SW3, SW4와 핀을 공유함. 본 과제는 스위치를 사용하지 않으므로 기능상 충돌은 없으나, 통신 중 해당 스위치를 누르면 안 됨. 특히 PD3는 TXD1 출력이므로 스위치를 누르면 출력이 GND로 단락됨.

### PD2 커패시터로 인한 보레이트 제약
PD2에는 스위치 채터링 방지용 104(0.1uF) 커패시터가 연결되어 있음. 이 커패시터가 수신 파형의 에지를 둔화시켜 9600bps에서 문자가 깨짐. 과제 2 진행 중 0~9 입력 시 0, 2, 8만 정상 인식되는 현상으로 확인함. 본 과제에서는 PC 통신 보레이트를 4800으로 낮춰 대응함. 커패시터를 제거하면 9600 사용이 가능함.

### 반이중 방향 제어
MAX485의 RE와 DE가 PE2 하나에 묶여 있어 송신과 수신을 동시에 할 수 없음.

| PE2 | DE | RE | 동작 |
| :--- | :--- | :--- | :--- |
| High | 1 (활성) | 1 (비활성) | 송신. 수신부 차단 |
| Low | 0 (비활성) | 0 (활성) | 수신. 송신부 차단 |

리셋 직후 PE2는 입력 상태로 부유하므로 DE가 임의로 활성화되어 버스를 점유할 수 있음. 이를 막기 위해 main 진입 직후 가장 먼저 PE2를 출력 Low로 고정함.

---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c), 선언부(.h)만 구조에 표기함.
```text
├── Day 3/hw3/
│   ├── hw3.c       # UART 2채널, Dynamixel 프로토콜, ADC, LCD 출력
│   ├── LCD_I2C.h    # PCF8574 기반 I2C LCD 드라이버 (2일차 과제와 동일)
│   └── REPORT.md
```

Makefile 이나 프로젝트 파일 없이 아래 명령만으로 빌드됨.

```text
avr-gcc -mmcu=atmega128 -Os -Wall -std=gnu99 -o firmware.elf hw3.c
avr-objcopy -O ihex -R .eeprom firmware.elf firmware.hex
```

-Os 는 필수. 최적화를 끄면 _delay 계열 함수가 설계대로 동작하지 않음.

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### 반이중 송신 (hw3.c)
```c
void SEND_rs485(uint8_t* buf, uint8_t len)
{
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
    FLUSH_dxlRx();
}
```

| 플래그 | 의미 | 사용 이유 |
| :--- | :--- | :--- |
| UDRE0 | 송신 버퍼가 비어 다음 데이터를 받을 수 있음 | 버퍼에 다음 바이트를 넣어도 되는 시점 판단 |
| TXC0 | 시프트 레지스터의 데이터가 모두 전송 완료됨 | DE를 내려도 되는 시점 판단 |

두 플래그의 차이가 이 함수의 핵심임. UDRE0은 버퍼가 비었을 뿐 마지막 바이트가 아직 선로에 나가는 중일 수 있음. UDRE0만 보고 DE를 내리면 마지막 바이트가 전송 도중 잘림. TXC0은 시프트 레지스터까지 비워진 시점을 알려주므로 이때 방향을 전환해야 함. TXC0은 1을 write하여 지우는 방식이므로 송신 시작 전에 미리 클리어함.

방향 전환 시 선로 상태 변화로 수신 버퍼에 잡음이 들어오므로, 수신 모드 전환 직후 버퍼를 비움.

### Protocol 2.0 패킷 구성
```c
void SEND_dxlPacket(uint8_t id, uint8_t* body, uint8_t bodyLen)
{
    uint16_t len = (uint16_t)bodyLen + 2;

    p[n++] = 0xFF; p[n++] = 0xFF; p[n++] = 0xFD; p[n++] = 0x00;
    p[n++] = id;
    p[n++] = (uint8_t)len;
    p[n++] = (uint8_t)(len >> 8);
    ...
}
```

```text
Header1 Header2 Header3 Reserved  ID  Len_L Len_H  Instruction  Param...  CRC_L CRC_H
 0xFF    0xFF    0xFD     0x00
```

LENGTH 필드는 해당 필드 뒤에 오는 바이트 수, 즉 `INSTRUCTION + PARAM + CRC 2바이트`임. 구현 초기에 `body 길이 + 3`으로 계산하여 값이 1 크게 나갔고, 서보가 응답 자체를 하지 않음. 브로드캐스트 PING 패킷을 문서의 예시값 `FF FF FD 00 FE 03 00 01 31 42`와 바이트 단위로 대조하여 발견함.

CRC는 다항식 0x8005를 사용하는 CRC-16이며, 헤더부터 마지막 파라미터까지 전체를 대상으로 계산함.

### Status Packet 수신
```c
while (match < 4)                           // 헤더 탐색. 0xFF 만나면 거기서 다시 센다
{
    c = READ_dxlByte(2000);                 // 20ms

    if (c < 0)
        return 0;

    if ((uint8_t)c == hdr[match])
        match++;
    else
        match = ((uint8_t)c == 0xFF) ? 1 : 0;
}
```
헤더 탐색 중 불일치가 발생했을 때 무조건 0으로 되돌리면, `FF FF FF FD 00`처럼 0xFF가 하나 더 붙은 입력에서 헤더를 놓침. 불일치한 바이트가 0xFF라면 그 바이트를 새 헤더의 시작으로 간주하여 match를 1로 설정함.

### 목표 속도 레지스터 선택

| 레지스터 | 주소 | 적용 모드 |
| :--- | :--- | :--- |
| Goal Velocity | 104 | 속도 제어 모드 전용 |
| **Profile Velocity** | **112** | **위치 제어 모드의 이동 속도** |

과제 명세의 "목표 속도"는 프로토콜 1.0의 Moving Speed(32)에 해당하는 개념임. 프로토콜 2.0 위치 제어 모드에서 이에 대응하는 것은 Profile Velocity(112)이며, Goal Velocity(104)는 속도 제어 모드에서만 유효하므로 위치 제어 중에는 아무 효과가 없음. 따라서 112를 사용함.

### 예외처리

| 항목 | 처리 내용 |
| :--- | :--- |
| 속도 0 입력 | Profile Velocity는 0이 "속도 제한 없음"을 의미하여 서보가 전속력으로 동작함. 하한을 1로 제한하여 방지 |
| 급가속으로 인한 과부하 | Profile Acceleration 기본값 0은 가속도 무제한을 의미함. 20으로 제한하여 순간 전류 상승 억제 |
| 하드웨어 에러 래치 | 에러 발생 시 서보가 토크를 차단하고 잠김. Torque Enable에 1을 써도 0으로 복귀함. 2초 주기로 감시하여 발견 시 REBOOT 후 재설정 |
| ADC 잡음 | 정지 상태에서도 1~2씩 변동함. 4 이상 변화한 경우에만 패킷 전송하여 버스 점유 감소 |
| 숫자 외 입력 | "ERROR" 응답. CR, LF는 무시 |
| 서보 미검출 | PA5를 점멸하며 PING 반복. 전원 인가 순서와 무관하게 동작 |

### 목표 위치 범위
명세는 목표 위치를 0~1023으로 지정하고 있어 ADC 값을 그대로 사용함. 다만 MX-64의 프로토콜 2.0 위치 범위는 0~4095이므로 이 설정에서는 전체 회전의 약 1/4만 사용하게 됨. `POS_SCALE`을 4로 변경하면 전체 범위를 사용할 수 있도록 구성함.

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오
1. 전원 인가 시 PE2를 Low로 고정하여 485 버스 점유를 방지함
2. UART0(57600), UART1(4800), LCD, ADC를 초기화함
3. 브로드캐스트 PING으로 서보를 탐색하고 응답한 ID를 PC로 전송함
4. 하드웨어 에러가 남아 있으면 REBOOT 후 재탐색함
5. Profile Acceleration, Profile Velocity 설정 후 토크를 인가함
6. 가변저항 값에 따라 Goal Position을 갱신함
7. PC에서 0~9 입력 시 Profile Velocity를 0~300 범위로 설정함
8. LCD 1행에 목표 속도, 2행에 목표 위치를 표시함
9. 2초 주기로 서보 상태를 확인하고 에러 발생 시 자동 복구함

```text
+----------------+
|SPEED:266   LDY |
|POS  : 512      |
+----------------+
```

```text
READY
FOUND ID 1
MODE 3 TQ 1 ERR 0
SPEED 266
G 512 P 498 V 266 TQ 1 E 0
```

### 검증 과정에서 확인한 사항

**LENGTH 필드 오류**
초기 구현에서 서보가 PING에 전혀 응답하지 않음. 패킷을 직접 덤프하여 문서의 예시값과 대조한 결과 LENGTH가 1 크게 계산되고 있었음. `body 길이 + 3`을 `+ 2`로 수정하여 해결함.

**모터 미동작**
통신은 성립하나 서보가 회전하지 않음. 상태 레지스터를 읽어 PC로 출력하는 진단 코드를 추가한 결과 `MODE 3 TQ 0 ERR 32`가 확인됨. ERR 32는 과부하(Overload)를 의미하며, 이 상태에서는 토크가 유지되지 않음. 부하를 제거하고 REBOOT하여 정상 동작을 확인함.

이후 반복적인 과부하의 원인이 Profile Acceleration 미설정에 있음을 파악함. 기본값 0은 가속도 무제한을 의미하여 목표 위치가 변경될 때마다 서보가 급출발하고, 그 순간 전류가 급상승하여 과부하 판정을 유발함. 가속도를 20으로 제한한 뒤 과부하 발생이 크게 감소함.

**초기 속도값**
Profile Velocity의 단위는 0.229 rev/min이므로 초기값 1은 실질적으로 정지 상태에 가까움. 1/4 회전에 약 1분이 소요됨. 시연 시에는 PC에서 8 이상을 입력하여 속도를 확보한 후 가변저항을 조작해야 함.

### 동작 사진 / 영상

| 시연 영상 |
| :---: |
| [3일차 과제 3 시연 영상](https://drive.google.com/file/d/1_V5D67VQumfcVqhT0md_6pp3KKxoGDNz/view?usp=sharing) |

---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **Claude** | 개념 이해 | - Dynamixel Protocol 2.0 패킷 구조 및 CRC 방식 확인<br>- UDRE와 TXC 플래그의 차이 확인 |
| **Claude** | 디버깅 & 문제 진단 | - 서보 무응답의 원인 범위 좁히기 (LENGTH 필드 오류 추적)<br>- 모터 미동작 시 확인할 상태 레지스터 선정<br>- 반복적인 과부하의 원인 분석 |
| **Claude** | 구조 검토 | - Goal Velocity와 Profile Velocity 중 선택 근거 검토<br>- UART 2채널 배분 방식 검토 |

### AI 활용 및 검증 원칙
1. **코드 검증:** 작성된 코드는 -Wall -Wextra 옵션으로 경고 없이 컴파일되는지 확인하고, 실제 서보를 연결하여 위치 추종과 속도 변경을 육안으로 검증하였습니다.
2. **프로토콜 대조:** 패킷 구성은 AI의 설명에 의존하지 않고 ROBOTIS 공식 문서의 예시 바이트열과 직접 대조하여 확인하였습니다.
3. **문제 진단:** 모터 미동작 원인은 추측 대신 서보의 상태 레지스터를 직접 읽어 확인하는 방식으로 특정하였습니다.
4. **학습 주도성:** 코드 작성과 보고서 작성은 직접 수행하였으며, AI는 원인 파악이 막혔을 때 가능성을 좁히고 설계 판단의 근거를 함께 검토하는 용도로만 활용하였습니다. UART 포트 배분과 목표 속도 레지스터 선택 등의 최종 결정은 직접 내렸습니다.
