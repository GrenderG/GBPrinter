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
		CBUFFER.buffer[i] = 0u;
}

byte CBRead() {
	byte b = CBUFFER.buffer[CBUFFER.start];
	CBUFFER.start = (CBUFFER.start + 1) % BUFFER_SIZE;
	return b;
}

void CBWrite(byte b) {
	CBUFFER.buffer[CBUFFER.end] = b;
	CBUFFER.end = (CBUFFER.end + 1) % BUFFER_SIZE;
}

uint8_t GBSendByte(uint8_t b) {
// This will allow us to test GBSendPacket on test builds
#ifdef TESTBUILD
	Serial.write(b);
	return Serial.read();
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
		if (command == GBC_TRANSFER) {
			b = 0xff;
		} else {
			b = CBRead();
		}
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

funcptr ArduinoIdle() {
	// Reset status
	ARDUINO_STATE.total = 0;
	ARDUINO_STATE.printed = 0;
	// Read buffer
    uint8_t readBytes;
    while(Serial.available() < 4) ; // Make sure bytes are available
	readBytes = Serial.readBytes(MSG_BUFFER, MSG_SIZE);
    if (readBytes != MSG_SIZE) {
    	// If read times out, just return to idle state again
        return (funcptr) ArduinoIdle;
    }
	for (int i = MSG_SIZE; i > 0; --i) {
		ARDUINO_STATE.total |= MSG_BUFFER[i-1] << 8* (MSG_SIZE - i);
	}
	// Write max packet size (= BUFFER_SIZE)
	uint32_t bufferSize = BUFFER_SIZE;
	for (int i = 1; i <= MSG_SIZE; ++i) {
		Serial.write((uint8_t) (BUFFER_SIZE >> 8 * (MSG_SIZE - i)));
	}

	// Return next state
	return (funcptr) ArduinoSetup;
}

funcptr ArduinoSetup() {
	// Read buffer
    uint8_t readBytes;
    while(Serial.available() < 2) ; // Make sure bytes are available
	readBytes = Serial.readBytes(MSG_BUFFER, 2);
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
    for (uint16_t i = 0; i < toRead; ++i) {
        while(!Serial.available()) ; // Make sure bytes are available
		CBWrite(Serial.read());
    }
	// Now start print

	// Run the GBPrinter inner state machine
	GBPStateInit();
	do {
		GBP_STATE.current = (ptrfuncptr) GBP_STATE.current();
	} while(GBP_STATE.current != GBPInitialize);
	// If everything has gone well
	ARDUINO_STATE.printed += toRead;
	uint16_t status = GBP_STATE.status;
	Serial.write("OK");
	// Write last game boy status also
	Serial.write(lowByte(status));
	Serial.write(highByte(status));
	if (ARDUINO_STATE.printed >= ARDUINO_STATE.total)
		// That would mean we have printed everything!
		return (funcptr) ArduinoIdle;
	else
		// Read more data from serial, print it
		return (funcptr) ArduinoPrint;
}


void GBPStateInit() {
	GBP_STATE.current = GBPInitialize;
	GBP_STATE.status = 0x8100;
	GBP_STATE.txBytes = 0;
}

funcptr GBPInitialize() {
	// txBytes = 0. Query printer status. If all OK, proceed to transfer buffer to gbprinter
	GBP_STATE.txBytes = 0;
	GBP_STATE.status = GBSendPacket(GBC_INITIALIZE, 0);
	uint16_t status = GBP_STATE.status;
	if (GBP_STATE.status == 0x8100 || GBP_STATE.status == 0x8000) {
		// All OK, advance status
		return (funcptr) GBPTransfer;
	}
	else {
		return (funcptr) GBPInitialize;
	}
}

funcptr GBPTransfer() {
	uint16_t txSize = (uint16_t) min(ARDUINO_STATE.total - ARDUINO_STATE.printed - GBP_STATE.txBytes, PACKET_SIZE);
	GBP_STATE.txBytes += txSize;
	GBP_STATE.status = GBSendPacket(GBC_TRANSFER, txSize);
	uint16_t status = GBP_STATE.status;
	if ((GBP_STATE.status == 0x8100 || GBP_STATE.status == 0x8108) && GBP_STATE.txBytes < BUFFER_SIZE) {
		// All OK, advance status
		return (funcptr) GBPTransfer;
	}
	else if ((GBP_STATE.status == 0x8100 || GBP_STATE.status == 0x8108) && GBP_STATE.txBytes >= BUFFER_SIZE) {
        // GBP Memory chip can hold 8KB of image data, which, in packages of 640bytes, it's 12
        // of them (7680 bytes). The Arduino Nano I am using has an Atmel 328P, with 2048 bytes of RAM,
        // so I use the CBUFFER size as a limiter here, instead of GBP_MEMORY
        //
        // Also, before starting the print, send one last empty transfer packet (purpose unknown, but
        // it is specified in the protocol)
		GBP_STATE.status = GBSendPacket(GBC_TRANSFER, 0);
		return (funcptr) GBPPrint;
	}
	else {
		return (funcptr) GBPInitialize;
	}
}

funcptr GBPPrint() {
	uint8_t topMargin, bottomMargin;
	topMargin = bottomMargin = 0x00;
	// Check if this is the first print or the last, set margins accordingly
	// ard
	// Write print status stuff
	if (ARDUINO_STATE.total == 0) {
		topMargin = 0x04;
	}
	if (ARDUINO_STATE.printed + GBP_STATE.status >= ARDUINO_STATE.total) {
		bottomMargin = 0x04;
	}
	CBWrite(topMargin);
	CBWrite(bottomMargin);
	// Palette
	CBWrite(0xFF);
	// Exposure
	CBWrite(0xFF);
	GBP_STATE.status = GBSendPacket(GBC_PRINT, 0x04);
	// I guess now I should check whether the command has gone OK or what, and then poll the GBPrinter
	// with inquiry, and only return whenever everything goes well
	while (true) {
		GBP_STATE.status = GBSendPacket(GBC_REPORT, 0);
		if (lowByte(GBP_STATE.status & 0x02))
			break;
		delay(100);
	}
	while (true) {
		GBP_STATE.status = GBSendPacket(GBC_REPORT, 0);
		if (!lowByte(GBP_STATE.status & 0x02))
			break;
		delay(100);
	}
    // Once we print, we will have read all our cached buffer, so we move to initialize
	return (funcptr) GBPInitialize;
}
