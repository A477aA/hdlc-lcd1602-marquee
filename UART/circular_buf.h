#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#define BUFFER_SIZE 32

struct circular_buffer{
	unsigned char buffer[BUFFER_SIZE];
	unsigned char buf_head;
	unsigned char buf_tail;
};

void initialize_buffer(struct circular_buffer* cb);
int buffer_empty(const struct circular_buffer* cb);
int buffer_full(const struct circular_buffer* cb);
void write_buffer(struct circular_buffer* cb, int value);
int read_buffer(struct circular_buffer* cb);

#endif /* CIRCULAR_BUFFER_H */
