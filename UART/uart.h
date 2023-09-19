#ifndef UART_H
#define UART_H
#include <stdint.h> 
#include <stddef.h>
#include "timer.h" 
#include "circular_buf.h" 

#define F_CPU 16000000

struct circular_buffer usartRxBuffer;
struct circular_buffer usartTxBuffer;

void UART_init(void);
void UART_send(uint8_t* data, size_t length);
int UART_receive(uint8_t* data, size_t length);


#endif /* UART_H */