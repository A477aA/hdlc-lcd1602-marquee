#include <stdio.h>
#include <stdlib.h>
#include "circular_buf.h"


void initialize_buffer(struct circular_buffer* cb) {
	cb->buf_head = 0;
	cb->buf_tail = 0;
}
// Проверка, пустой ли буфер
int buffer_empty(const struct circular_buffer* cb) {
	return cb->buf_head == cb->buf_tail;
}
// Проверка, заполнен ли буфер
int buffer_full(const struct circular_buffer* cb) {
	return (cb->buf_tail + 1) % BUFFER_SIZE == cb->buf_head; //проверяем следующее число, если оно будет совпадать с индексом головы то будет false, при совпадении вывод true
}
// Запись в буфер
void write_buffer(struct circular_buffer* cb, int value) {
	if (buffer_full(cb)) { // проверяем, заполнен ли буфер
		return;
	}
	cb->buffer[cb->buf_tail] = value;// записываем значение в элемент массива в хвост
	cb->buf_tail = (cb->buf_tail + 1) % BUFFER_SIZE;// присваивается cb->buf_tail, обновляется его значение на следующий индекс в буфере
}
// Чтение элемента
int read_buffer(struct circular_buffer* cb) {
	if (buffer_empty(cb)) { // проверка на пустоту
		return -1;// -1  как индикатор в случае ошибки
	}
	int value = cb->buffer[cb->buf_head]; // чтение по индексу головы
	cb->buf_head = (cb->buf_head + 1) % BUFFER_SIZE; // увеличиваем индекс на 1
	return value;
}


