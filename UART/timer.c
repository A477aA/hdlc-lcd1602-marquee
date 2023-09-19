#include <avr/io.h>
#include <avr/interrupt.h>

static volatile unsigned long timerMillis = 0;

void timerInit() {
	TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
	OCR1A = 250;
	TIMSK1 |= (1 << OCIE1A);
	sei();
}

unsigned long millis() {
	unsigned long ms;
	ms = timerMillis;
	return ms;
}

ISR(TIMER1_COMPA_vect) {
	timerMillis++;
}