# hdlc-lcd1602-marquee

Firmware and host-side tooling for a **remote character-display node** built on an
ATmega328P (Arduino Uno / Nano class board).

The board receives a message from a host PC over a UART serial link, using an
**HDLC** data-link layer for framing, sequencing and error detection. On top of
HDLC, a small application protocol carries a payload consisting of one text
string and three floating-point numbers. The decoded message is rendered on a
**16×2 HD44780 character LCD** driven over I²C through a PCF8574 port expander:
the text line scrolls, and the three numbers are shown on the second line.

> Archived snapshot of a project originally written in 2023. The sources are kept
> as-is; see [Repository notes](#repository-notes) for what is and isn't included.

---

## Features

- Interrupt-driven UART (9600 8N1) with a circular RX/TX buffer.
- `millis()` time base from Timer1 (1 kHz), used for timeouts and LCD scrolling.
- HDLC framing (yahdlc-style): start/end flag `0x7E`, byte stuffing with escape
  `0x7D`, all-station address `0xFF`, CRC-16/CCITT frame-check sequence.
- I-, S- and S-NACK frames with 3-bit sequence numbers.
- A client state machine that performs a connect handshake, sends data, checks
  sequence numbers, and retransmits on timeout.
- 16×2 LCD output over I²C (PCF8574), based on Peter Fleury's HD44780 library.
- Host-side test harness (`hdlc/main.c`) and serial terminal scripts.

---

## Hardware

| Item      | Value                                                        |
|-----------|-------------------------------------------------------------|
| MCU       | ATmega328P @ 16 MHz (`F_CPU = 16000000`)                     |
| Serial    | UART0, 9600 baud, 8 data bits, no parity, 1 stop (8N1)       |
| Display   | HD44780 16×2, driven via a PCF8574 I²C backpack              |
| Host      | Windows PC running a serial terminal (see `Terminal1_9_b/`)  |

The UART baud rate is set by `UBRR0L = 103` at 16 MHz, which yields 9600 baud.

---

## Repository layout

```
hdlc-lcd1602-marquee/
├── sketch_sep19a/        Main firmware entry point (Arduino sketch, own main())
├── hdlc/                 HDLC framing, FCS, client state machine, PC test harness
├── UART/                 Interrupt-driven UART, circular buffer, Timer1 millis()
├── Lcd_print/            Message -> LCD formatting and rendering
├── MyLCD/                HD44780 / PCF8574 I²C LCD driver library (Fleury-based)
├── Terminal1_9_b/        Host-side serial terminal + macro/scripts
└── docs/                 Additional documentation
```

A module-by-module description, the protocol format and the message sequence are
in [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

---

## How it works (short version)

1. On start-up the board initialises the LCD, UART and the HDLC client.
2. **Connect:** the board sends an S-frame and waits for the peer's
   acknowledgement, retransmitting on timeout, until it reaches the `READY` state.
3. **Data exchange:** the board encodes a message with the application protocol,
   wraps it in an HDLC I-frame and sends it, then waits for the reply. A frame
   with a valid FCS and the expected sequence number is decoded.
4. **Display:** the decoded message is shown on the LCD — the string scrolls on
   the top row, the three numbers appear on the bottom row.

---

## Building and flashing

The project targets **ATmega328P at 16 MHz**. The firmware defines its own
`int main()` (in `sketch_sep19a/sketch_sep19a.ino`) instead of relying on the
Arduino core `main()`, so it is intended to be built as a plain AVR project
(the repository contains Visual Studio / Microchip-Studio project metadata from
the original 2023 setup).

To rebuild, compile the sketch together with the module sources and link against
avr-libc, for example:

```sh
avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os \
    -I hdlc -I UART -I Lcd_print -I MyLCD/MyLCD \
    sketch_sep19a/sketch_sep19a.ino \
    hdlc/hdlc.c hdlc/fcs.c hdlc/client.c \
    UART/uart.c UART/timer.c UART/circular_buf.c \
    Lcd_print/lcd_print.cpp \
    MyLCD/MyLCD/lcdpcf8574.cpp MyLCD/MyLCD/pcf8574.cpp MyLCD/MyLCD/twimaster.cpp \
    protocol.c \
    -o build/display.elf

avr-objcopy -O ihex -R .eeprom build/display.elf build/display.hex
avrdude -c arduino -p m328p -P <PORT> -b 115200 -U flash:w:build/display.hex
```

The HDLC layer can also be exercised on a PC without hardware:

```sh
gcc -std=c11 hdlc/main.c hdlc/client.c hdlc/hdlc.c hdlc/fcs.c -o hdlc_test
```

---

## Host side

`Terminal1_9_b/` contains **Bray's Terminal** (`Terminal.exe`) together with
macro and script files (`*.tsc`, `*.tmf`) used to drive the serial exchange with
the board from a Windows PC.

---

## Repository notes

- This is a **2023 snapshot**, committed as-is for archival purposes.
- `hdlc/main.c` is a **host-side (PC) test harness** for the HDLC client, not
  part of the firmware image.
- Visual Studio / Microchip-Studio local caches (`.vs/`, `*.suo`, `*.ipch`,
  `Browse.VC.db`, …) are excluded via `.gitignore`.

---

## License

Released under the [MIT License](LICENSE).

Note that `MyLCD/` is derived from Peter Fleury's HD44780 LCD library, which
carries its own license terms; those apply to that code and are unaffected by the
MIT license of this repository.
