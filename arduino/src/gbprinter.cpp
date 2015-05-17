#include "gbprinter.h"

byte CBRead(CBuffer* cb) {
	byte b = cb->buffer[cb->start];
	cb->start = ++cb->start % BUFFER_SIZE;
	return b;
}

void CBWrite(CBuffer* cb, byte b) {
	cb->buffer[cb->end] = b;
	cb->end = ++cb->end % BUFFER_SIZE;
}
