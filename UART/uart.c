#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <string.h>
#include "timer.h"
#include "circular_buf.h"
#include "uart.h"

void UART_init(void) {
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1 << UDRIE0); // прерывание по приему и опустошению буфера передачи
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	UBRR0H = 0;
	UBRR0L = 103;
}

void UART_send(uint8_t* data, size_t length) {
	for (size_t i = 0; i < length; i++) {
		if (!buffer_full(&usartTxBuffer)) {
			write_buffer(&usartTxBuffer, data[i]);
			} else {
			break; // если буфер передачи заполнен, то отправка прерывается
		}
	}
	UCSR0B |= (1 << UDRIE0); // Включаем прерывание по опустошению буфера
}

// Получение данных из буфера
int UART_receive(uint8_t* data, size_t length) {
	char overflow=0; // Флаг переполнения, который устанавливается, если превышен размер массива 
	uint32_t byteCount=0; // Счетчик байтов, принятых из буфера приема
	uint32_t timeout_ms = 4; // Таймаут в миллисекундах для общего времени приема данных
	uint32_t start_time = millis();
	//Цикл приема данных с таймаутом
	while(1)
	{
		// Пока буфер приема не пуст и не истек таймаут общего времени,  
		// функция продолжает читать байты из буфера и сохранять их в массив data.
		while (!buffer_empty(&usartRxBuffer)) {
			int  reader = read_buffer(&usartRxBuffer);//прием и запись символа в переменную
			if(byteCount<=length){
				data[byteCount] = reader; // запись в массив с индексом byteCount
			}
			else{
				overflow=1;
			}
			byteCount++;

			start_time = millis();
		}
		if ((millis() - start_time) > timeout_ms) { // если превышение времени в 4 ms
			break;
		}
		
	}
	return overflow?-1:byteCount; // возвращает количество успешно принятых байт или -1
}

// прерывание по завершению приема
ISR(USART_RX_vect) {
	uint8_t data = UDR0; // читаем из регистра UDR0
	if (!buffer_full(&usartRxBuffer)) {
		write_buffer(&usartRxBuffer, data);// записываем символ в буфер приема
	}
}
