#include "gbprinter.h"
#ifdef TESTBUILD
	#include <Serial.h>
#endif

// Initialize global circular buffer
CBuffer CBUFFER;

void CBInit() {
	CBUFFER.start = 0;
	CBUFFER.end = 0;
	for (unsigned int i = 0; i < BUFFER_SIZE; ++i)
	{
		CBUFFER.buffer[i] = 0u;
	}
}

byte CBRead() {
	byte b = CBUFFER.buffer[CBUFFER.start];
	CBUFFER.start = ++CBUFFER.start % BUFFER_SIZE;
	return b;
}

void CBWrite(byte b) {
	CBUFFER.buffer[CBUFFER.end] = b;
	CBUFFER.end = ++CBUFFER.end % BUFFER_SIZE;
}

uint8_t GBSendByte(uint8_t b) {
	// This will allow us to test GBSendPacket on test builds
#ifdef TESTBUILD
	Serial.write(b);
	return 0x00;
#else
	uint8_t reply = 0;
	for (uint8_t bit_pos = 0; bit_pos < 8; ++bit_pos) {
		reply <<= 1;
		digitalWrite(GBP_CLOCK, 0); // Send clock signal

		if ((b << bit_pos) & 0x80) {
			digitalWrite(GBP_OUT, 1); // Write out to printer
		}
		else { 
			digitalWrite(GBP_OUT, 0);
		}
		delayMicroseconds(DELAY_MS);
		digitalWrite(GBP_CLOCK, 1);
		
		if (digitalRead(GBP_IN)){
		  reply |= 1;    // Fetch printer reply
		}
		
		delayMicroseconds(DELAY_MS);
	}

	delayMicroseconds(DELAY_MS);

	return reply;
#endif
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
		b = CBRead();
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

// Initialize state
ArduinoState ARDUINO_STATE;
GBPState GBP_STATE;

void ArduinoStateInit() {
	ARDUINO_STATE.current = ArduinoIdle;
	ARDUINO_STATE.total = 0;
	ARDUINO_STATE.printed = 0;
}
//void GBPStateInit();

funcptr ArduinoIdle() {
	// Reset status
	ARDUINO_STATE.total = 0;
	ARDUINO_STATE.printed = 0;
	// Read buffer
	Serial.readBytesUntil('\n', SERIAL_B, 5);
	// read buffer. If it contains a valid buffer size
	// write total number of batches, set state
	return (funcptr) ArduinoSetup;
}

funcptr ArduinoSetup() {
	return (funcptr) ArduinoIdle;
}