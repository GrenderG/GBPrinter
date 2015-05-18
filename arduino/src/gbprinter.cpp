#include "gbprinter.h"

// Initialize global circular buffer
CBuffer CBUFFER = { .start = 0, .end= 0 };

byte CBRead(CBuffer* cb) {
	byte b = cb->buffer[cb->start];
	cb->start = ++cb->start % BUFFER_SIZE;
	return b;
}

void CBWrite(CBuffer* cb, byte b) {
	cb->buffer[cb->end] = b;
	cb->end = ++cb->end % BUFFER_SIZE;
}

uint8_t GBSendByte(uint8_t b) {
	b = 0x00;
	return b;
}

uint16_t GBSendPacket(uint8_t command, uint16_t size) {
	uint16_t status, checksum = 0x0000;
	// Send magic bytes
	GBSendByte(0x88);
	GBSendByte(0x33);
	// Send command
	GBSendByte(command);
	checksum += command;
	// Send compression
	GBSendByte(0x00);
	// Send size
	GBSendByte(lowByte(size));
	GBSendByte(highByte(size));
	checksum += lowByte(size);
	checksum += highByte(size);
	// Send data
	uint8_t b;
	for (uint16_t i = 0; i < size; ++i) {
		b = CBRead(&CBUFFER);
		checksum += b;
		GBSendByte(b);
	}
	// Send checksum
	GBSendByte(lowByte(checksum));
	GBSendByte(highByte(checksum));
	// Read status
	status = GBSendByte(0x00);
	status = status << 8;
	status |= GBSendByte(0x00);
	return status;
}