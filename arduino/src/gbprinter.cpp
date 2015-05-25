#include "gbprinter.h"
#include <Serial.h>

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
uint8_t MSG_BUFFER[MSG_SIZE];

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
	Serial.readBytes(MSG_BUFFER, MSG_SIZE);
	for (int i = MSG_SIZE; i > 0; --i) {
		ARDUINO_STATE.total |= MSG_BUFFER[i-1] << 8* (MSG_SIZE - i);
	}
	// Write max packet size (= BUFFER_SIZE)
	uint32_t bufferSize = BUFFER_SIZE;
	for (int i = 1; i <= MSG_SIZE; ++i) {
		Serial.write((uint8_t) (bufferSize >> 8 * (MSG_SIZE - i)));
	}

	// Return next state
	return (funcptr) ArduinoSetup;
}

funcptr ArduinoSetup() {
	// Read buffer
	Serial.readBytes(MSG_BUFFER, 2);
	if (strncmp((char *) MSG_BUFFER, "OK", 2) == 0) {
		Serial.write("OK");
		return (funcptr) ArduinoPrint;
	}
	else {
		Serial.write("KO");
		return (funcptr) ArduinoIdle;	
	}
}

funcptr ArduinoPrint() {
	uint16_t toRead = min(ARDUINO_STATE.total - ARDUINO_STATE.printed, BUFFER_SIZE);
	for (uint16_t i = 0; i < toRead; ++i)
		CBWrite(Serial.read());
	// Now start print

	// Do here all the Arduino - gbprinter shit

	// If everything has gone well
	ARDUINO_STATE.printed += toRead;
	Serial.write("OK");
	// Write last game boy status also
	Serial.write(0x81);
	Serial.write(0x22);
	if (ARDUINO_STATE.printed == ARDUINO_STATE.total)
		return (funcptr) ArduinoIdle;
	else
		return (funcptr) ArduinoPrint;
}