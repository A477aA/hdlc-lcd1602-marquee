# Architecture

This document describes the modules of **hdlc-lcd1602-marquee**, the framing and
application protocols, and the message exchange between the board and the host.

## Layered view

```
        +-------------------------------------------------+
        |  Application    protocol_encode / protocol_decode|
        |                 struct message (text + 3 floats) |
        +-------------------------------------------------+
        |  Client         connect / send / decode / retry  |
        |                 HDLC client state machine        |
        +-------------------------------------------------+
        |  HDLC           hdlc_frame_data / hdlc_get_data  |
        |                 framing, byte-stuffing, FCS      |
        +-------------------------------------------------+
        |  UART           interrupt-driven TX/RX + ring    |
        |                 Timer1 millis()                  |
        +-------------------------------------------------+
        |  Hardware       ATmega328P USART0 @ 9600 8N1     |
        +-------------------------------------------------+

        LCD output path:  struct message -> lcd_print -> MyLCD (I²C) -> HD44780
```

---

## UART layer (`UART/`)

### `circular_buf.[ch]`
A fixed-size ring buffer (`BUFFER_SIZE = 32`) with `buf_head` / `buf_tail`
indices. One slot is kept free to distinguish the full and empty states, so the
usable capacity is `BUFFER_SIZE - 1` bytes.

- `initialize_buffer` — reset head and tail to 0.
- `buffer_empty` / `buffer_full` — state queries.
- `write_buffer` — push one byte (ignored when full).
- `read_buffer` — pop one byte (returns `-1` when empty).

### `timer.[ch]`
A 1 kHz time base built on Timer1 in CTC mode (`OCR1A = 250`, prescaler 64 at
16 MHz → 1000 interrupts/s). `ISR(TIMER1_COMPA_vect)` increments a millisecond
counter, exposed through `millis()`. `timerInit()` also enables global
interrupts (`sei()`).

### `uart.[ch]`
USART0 driver.

- `UART_init` — enable RX/TX, RX-complete and data-register-empty interrupts,
  configure 8N1, set 9600 baud (`UBRR0L = 103`).
- `UART_send` — enqueue bytes into the TX ring and enable the data-register-empty
  interrupt so the bytes are shifted out.
- `UART_receive` — drain the RX ring into a caller buffer, finishing after an
  inter-byte gap of ~4 ms; returns the number of received bytes.
- `ISR(USART_RX_vect)` — push each received byte into the RX ring.

Two shared ring buffers, `usartRxBuffer` and `usartTxBuffer`, connect the ISRs
and the API.

---

## HDLC layer (`hdlc/`)

### `fcs.[ch]`
Frame-check sequence: **CRC-16/CCITT** with polynomial `0x1021` and a reflected
lookup table. `calc_fcs` folds one byte at a time; `FCS_INIT_VALUE = 0xFFFF`,
and a correct frame yields the magic residue `FCS_GOOD_VALUE = 0xF0B8`.

### `hdlc.[ch]`
HDLC framing, modelled on the *yahdlc* library.

- Frame types: `I_FRAME` (information), `S_FRAME` (receive-ready / ACK),
  `S_FRAME_NACK` (reject).
- `hdlc_control_t` holds the frame type plus a **3-bit sequence number**.
- `hdlc_frame_data` builds a frame: start flag `0x7E`, all-station address
  `0xFF`, control byte, optional payload (I-frames only), inverted FCS, end flag.
  Any `0x7E`/`0x7D` byte in the body is escaped with `0x7D` and XOR `0x20`.
- `hdlc_get_data` / `hdlc_get_data_with_state` parse frames incrementally from
  one or more input chunks (as delivered by the UART): they locate the flags,
  reverse the byte-stuffing, verify the FCS, and return the payload and control
  field. A `hdlc_state_t` keeps the running parse state between chunks.
- `hdlc_get_control_type` / `hdlc_frame_control_type` translate between the raw
  control byte and the structured `hdlc_control_t`.

### `client.[ch]`
A client state machine on top of the framing layer.

- `struct Client` holds the current `HDLCState`, the current sequence index, the
  S/I/reject control frames, the pending I-frame payload, and the connect
  timeout.
- States: `UNINITIALIZED`, `IDLE`, `READY`, `CONNECTING`, `DISCONNECTING`,
  `RECIVING`.
- `init_hdlc_client` — initialise the client and store the connect timeout.
- `hdlc_connect` — enter `CONNECTING` and prepare an S-frame (seq 0).
- `hdlc_send_data` — prepare an I-frame with the caller's payload.
- `hdlc_get_raw_frame` — serialise the prepared frame into a byte buffer via
  `hdlc_frame_data` (ready to hand to `UART_send`).
- `hdlc_decode_recived_raw_data` — parse an incoming buffer with
  `hdlc_get_data`, check the sequence number, advance the state, and on a
  mismatch prepare a reject frame.
- `hdlc_timeout_handler` — decrement the connect timeout and report a timeout so
  the caller can retransmit.

### `main.c`
A **host-side (PC) test harness** that drives the client through a connect and a
data exchange and prints the result. It is not compiled into the firmware.

---

## Application protocol (`protocol.*`)

The firmware serialises a message with `protocol_encode` and reconstructs it with
`protocol_decode`. The message is:

```c
struct message {
    float  numbers[/* N */];  // numeric values (three are used)
    size_t len_numbers;       // number of valid entries
    char  *str;               // text line
    size_t len_str;           // length of the text line
};

void protocol_encode(struct message msg, uint8_t *out, size_t *out_len);
void protocol_decode(const uint8_t *in, size_t in_len, struct message *out);
```

---

## LCD output (`Lcd_print/` and `MyLCD/`)

### `MyLCD/`
An HD44780 character-LCD driver (Peter Fleury style) talking to the display
through a PCF8574 I²C expander:

- `i2cmaster.h` / `twimaster.cpp` — I²C (TWI) master.
- `pcf8574.[ch]` — PCF8574 port-expander access.
- `lcdpcf8574.[ch]` / `MyLCD.h` — LCD primitives: `lcd_init`, `lcd_home`,
  `lcd_clrscr`, `lcd_gotoxy(x, y)`, `lcd_led(on/off)`, `lcd_puts`.

### `lcd_print.[ch]`
Turns a decoded `struct message` into what appears on the 16×2 display.

- `Lcd_inciliation` — initialise the LCD, home the cursor, set the backlight, and
  record the `millis()` start time used for scrolling.
- `printLcd(const struct message *msg)` — every 500 ms it advances a scroll
  offset; the **top row** shows a 16-character window of `msg->str` (wrapping
  around), and the **bottom row** shows the three numbers formatted as
  `v1.v2.v3`.

---

## Message sequence

```
Board                                   Host (PC)
  |                                         |
  |  init LCD / UART / HDLC client          |
  |                                         |
  |------ S-frame (connect, seq 0) -------->|
  |                                         |
  |<----- S-frame (ACK) --------------------|   (retransmit on timeout)
  |  state -> READY                         |
  |                                         |
  |  protocol_encode(message)               |
  |------ I-frame (encoded message) ------->|
  |                                         |
  |<----- I-frame (reply) ------------------|
  |  FCS ok + expected seq -> decode        |
  |                                         |
  |  protocol_decode -> printLcd            |
  |  (text scrolls, numbers on row 2)       |
  v                                         v
```

On a sequence-number mismatch or a rejected frame the client prepares an S-NACK
and moves to `DISCONNECTING`.
