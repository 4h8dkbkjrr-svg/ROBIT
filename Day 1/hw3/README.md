# 1일차 과제 3 : 외부 인터럽트 4채널 및 2진 카운터

> **광운대학교 로봇학부**  
> **작성자:** 이동엽
> **제출일:** 2026년 7월 30일

---

## 1. 개요 (Overview)
본 과제는 ATmega128의 외부 인터럽트 INT0 부터 INT3 까지를 동시에 사용하여, 주기적으로 갱신되는 2진 카운터에 네 가지 서로 다른 동작을 적용하는 것을 목표로 함. 메인 루프와 ISR이 하나의 변수를 공유하는 구조에서 발생하는 문제를 함께 확인함.

### 핵심 목표
* EICRA 한 레지스터로 INT0 부터 INT3 까지 네 채널을 동시에 falling edge 설정
* ISR과 메인 루프가 공유하는 변수의 volatile 처리
* 시프트 대신 회전 연산을 사용하여 비트 손실 없이 LED 이동 구현
* 인터럽트 플래그를 이용한 중복 요청 예외처리

---

## 2. 개발 환경 (Environment)

| 항목 | 내용 |
| :--- | :--- |
| **MCU** | ATmega128A (16MHz External Crystal) |
| **IDE / Compiler** | Visual Studio Code / AVR-GCC 14.3.0 (macOS) |
| **Flasher Tool** | NEWTC USB_ISP (STK500v2) / avrdude |
| **언어** | C Language |
| **주요 부품** | ROBIT 실습보드, 8-Bit LED, 택트 스위치 4개, 점퍼선 2가닥 |

---

## 3. 하드웨어 구성 및 핀 맵 (Hardware Structure)

### Pin Configuration

```text
[ATmega128]                 [Target Component]
 PORTA (PA0 ~ PA7)   ----->   8-Bit LED (Active Low)
 PD0 (INT0)          <-----   SW1  (PC0 에서 점퍼 연결)
 PD1 (INT1)          <-----   SW2  (PC1 에서 점퍼 연결)
 PD2 (INT2)          <-----   SW3
 PD3 (INT3)          <-----   SW4
```

### 주요 회로 특징
* **인터럽트 핀 고정:** INT0 부터 INT3 까지는 PD0 부터 PD3 에 하드웨어로 고정되어 있어 다른 핀으로 대체할 수 없음. ATmega128에는 핀 체인지 인터럽트가 없으므로 PC 포트로는 인터럽트 생성이 불가능함
* **점퍼 배선:** 07/29 수정 회로도에서 PD0, PD1은 LCD의 I2C 통신선(SCL, SDA)으로 배정되어 실제 스위치는 PC0, PC1에 연결되어 있음. 본 과제 수행을 위해 PD0과 PC0, PD1과 PC1을 점퍼선으로 연결함
* **기존 배선 유지:** 배선을 옮기지 않고 추가하는 방식을 택함. 과제 2의 폴링 코드가 수정 없이 동작하고, 2일차 LCD 실습 시 추가한 선만 제거하면 원상복구되기 때문임. 과제 3 완료 후 점퍼선은 제거함
* **스위치:** 외부 10kohm 풀업 + 104(0.1uF) 커패시터 구조. 눌림 = LOW = falling edge

---

## 4. 프로젝트 구조 (Directory Structure)
> 구현부(.c), 선언부(.h)만 구조에 표기함.
```text
├── Day 1/hw3/
│   ├── hw3.c      # 2진 카운터 및 INT0~INT3 ISR 전체
│   └── README.md
```
단일 소스로 구성되어 Makefile 없이 아래 명령만으로 빌드됨.

```text
avr-gcc -mmcu=atmega128 -Os -Wall -std=gnu99 -o firmware.elf hw3.c
avr-objcopy -O ihex -R .eeprom firmware.elf firmware.hex
```

최적화 옵션 -Os 를 생략하면 _delay_ms() 가 설계된 지연 시간대로 동작하지 않으므로 반드시 지정해야 함.

---

## 5. 핵심 코드 및 레지스터 설정 (Key Implementation)

### 외부 인터럽트 4채널 초기화 (hw3.c)
```c
DDRD &= ~0x0F;                       // PD0 ~ PD3 입력 설정
PORTD |= 0x0F;                       // 내부 풀업

EICRA = (1 << ISC31) | (1 << ISC21) | (1 << ISC11) | (1 << ISC01);
EIMSK = (1 << INT3) | (1 << INT2) | (1 << INT1) | (1 << INT0);

sei();
```
INT0 부터 INT3 까지는 모두 EICRA가 담당하므로 한 줄로 네 채널을 설정할 수 있음. INT4 이상은 EICRB를 사용해야 함.

### 공유 변수의 volatile 처리
```c
volatile uint8_t count = 0;
```
ISR이 값을 변경하고 메인 루프가 읽는 변수임. volatile 키워드가 없으면 컴파일러가 while 루프 내에서 값이 변경되지 않는다고 판단하여 레지스터에 캐싱하며, 이 경우 INT3으로 카운터를 초기화해도 화면에 반영되지 않음.

### 회전 이동 구현
```c
ISR(INT0_vect)                       // 3개씩 우측 이동 X 2
{
    uint8_t i;
    for (i = 0; i < 2; i++) {
        count = (count << 3) | (count >> 5);
        PORTA = ~count;
        _delay_ms(200);
    }
    EIFR = 0x0F;                     // 중복 요청 폐기
}
```
왼쪽으로 3비트만 시프트하면 밀려난 상위 3비트가 소실되어 몇 번의 입력만으로 카운터가 0이 됨. 반대쪽에서 되돌아오도록 오른쪽 5비트 시프트 결과를 OR로 결합하면 점등된 LED 개수가 유지됨. 이동량과 나머지 비트 수의 합이 8이 되어야 함(3과 5, 1과 7).

| 인터럽트 | 동작 | 연산 |
| :--- | :--- | :--- |
| INT0 | 3개씩 우측 이동 X 2 | (count << 3) \| (count >> 5) |
| INT1 | 3개씩 좌측 이동 X 2 | (count >> 3) \| (count << 5) |
| INT2 | 1개 좌측 이동 후 우측 이동 | 1비트 우측 시프트 후 1비트 좌측 시프트 |
| INT3 | 2진 카운터 초기화 | count = 0 |

---

## 6. 동작 설명 및 결과 (Results)

### 동작 시나리오
1. 전원 인가 시 PORTA를 출력으로 설정하고 전체 소등 상태로 초기화함
2. 메인 루프에서 0.1초마다 count 값을 1씩 증가시키며 PORTA에 반전 출력함
3. count 는 uint8_t 이므로 0xFF 다음 자동으로 0으로 순환함
4. SW1(INT0) 입력 시 현재 값이 3칸씩 우측으로 두 번 이동함
5. SW2(INT1) 입력 시 3칸씩 좌측으로 두 번 이동함
6. SW3(INT2) 입력 시 1칸 좌측 이동 후 다시 우측으로 복귀함
7. SW4(INT3) 입력 시 카운터가 0으로 초기화되고 처음부터 다시 증가함

### 예외처리
이동 애니메이션 실행 중 추가 입력이 발생하면 인터럽트 플래그에 누적되어 ISR 종료 직후 재실행됨. 이를 방지하기 위해 각 ISR 종료 시 EIFR 에 0x0F 를 써서 하위 4비트 플래그를 일괄 클리어함.

카운터가 0인 상태에서는 회전 연산의 결과도 0이므로 화면 변화가 없음. 점등된 LED가 없어 이동 대상이 존재하지 않는 상황이며, 회전 연산이 자체적으로 처리하므로 별도 조건문은 두지 않음.

### 검증 과정에서 확인한 사항
코드 정리 중 main 함수의 닫는 중괄호가 누락되어 ISR 네 개가 main 내부에 중첩된 상태가 됨. 컴파일 시 "static declaration of __vector_1 follows non-static declaration" 오류가 발생하였으며, 원인 파악에 시간이 소요됨. 들여쓰기를 일관되게 유지했다면 조기에 발견 가능한 문제였음.

### 동작 사진 / 영상

| 시연 영상 |
| :---: |
| [1일차 과제 3 시연 영상](https://drive.google.com/file/d/1D7nY_bcUUSahqv-MnoHVlhigaPA1VGS5/view?usp=sharing) |

---

## 7. AI 툴 활용 명시 (AI Tools Declaration)
본 과제 작성 및 구현 과정에서 활용한 AI 도구(Generative AI)의 사용 현황 및 목적은 다음과 같음.

| 도구명 (Tool) | 활용 영역 | 세부 사용 목적 및 내용 |
| :--- | :--- | :--- |
| **Claude** | 코드 작성 & 디버깅 | - 중괄호 누락으로 인한 벡터 관련 컴파일 오류 원인 분석<br>- 회전 연산 구현 방식 검토<br>- volatile 누락 시 발생 가능한 문제 확인 |
| **Claude** | 회로 검토 & 문서화 | - PD0, PD1의 I2C 배정과 점퍼 배선 방식 검토<br>- 보고서 구조 및 서술 정리 |

### AI 활용 및 검증 원칙
1. **코드 검증:** 레지스터 설정은 데이터시트의 External Interrupts 항목과 대조하였으며, 컴파일 결과를 -Wall -Wextra 옵션으로 확인한 뒤 실제 보드에서 동작을 검증하였습니다.
2. **하드웨어 판단:** 점퍼선을 옮기지 않고 추가하는 방식은 2일차 LCD 실습과의 충돌을 고려하여 직접 결정하였습니다.
3. **학습 주도성:** 인터럽트별 동작 로직과 예외처리 방식은 직접 설계하였으며, AI는 오류 원인 분석과 문서화를 돕는 보조 도구로 활용하였습니다.
