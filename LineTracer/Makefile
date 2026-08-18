# ============================================================
# ATmega128 공용 Makefile (macOS / Apple Silicon)
#   새 과제 폴더에 이 파일만 복사하고 .c/.h 파일을 만들면 된다.
#   폴더 안의 모든 .c 를 자동으로 컴파일한다. (파일 추가 시 수정 불필요)
#
#   make        빌드          -> firmware.hex 생성
#   make flash  업로드
#   make check  프로그래머/보드 연결 확인 (쓰기 없음)
#   make ports  연결된 시리얼 포트 목록
#   make clean  빌드 결과물 삭제
#
#   ※ 퓨즈는 건드리지 않는다. 잘못 쓰면 보드가 벽돌이 된다.
# ============================================================

TARGET = firmware
MCU    = atmega128
F_CPU  = 16000000UL          # 보드 외부 크리스탈 16MHz

# 폴더 안의 .c 전부 (LCD.c 같은 걸 추가해도 자동 반영)
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

CC      = avr-gcc
OBJCOPY = avr-objcopy
SIZE    = avr-size

CFLAGS  = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=gnu99
LDFLAGS = -mmcu=$(MCU)

# --- 프로그래머 (NEWTC USB_ISP = STK500v2 프로토콜) ---
PROG ?= stk500v2
PORT ?= $(firstword $(wildcard /dev/cu.usbmodem*) usb)
AVRDUDE = avrdude -c $(PROG) -p m128 -P $(PORT)

.PHONY: all flash check ports clean

all: $(TARGET).hex

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
	@echo
	$(SIZE) --format=avr --mcu=$(MCU) $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flash: $(TARGET).hex
	$(AVRDUDE) -U flash:w:$<:i

check:
	$(AVRDUDE) -v

ports:
	@ls /dev/cu.* 2>/dev/null

clean:
	rm -f *.o $(TARGET).elf $(TARGET).hex
