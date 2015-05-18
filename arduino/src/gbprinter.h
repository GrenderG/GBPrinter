#pragma once

#include <Arduino.h>
#include <Serial.h>

/*
 * Circular buffer implementation. Will serve as buffer to store image data received from Arduino
 * serial connection to computer
 *
 * It is very barebones, but leannes is key here
 */

#define BUFFER_SIZE 640

typedef struct CBuffer{
	uint8_t buffer[BUFFER_SIZE];
	uint16_t start;
	uint16_t end;
} CBuffer;

uint8_t CBRead();

void CBWrite(uint8_t b);

// Declare CBuffer global variable
extern CBuffer CBUFFER;

/*
 * Low level GBP interface
 * Sends bytes and packages to the GBPrinter
 */
uint8_t GBSendByte(uint8_t b);

uint16_t GBSendPacket(uint8_t command, uint16_t size);

// Game Boy Printer Commands
#define GBC_INITIALIZE 0x01
#define GBC_PRINT 0x02
#define GBC_DATA 0x04
#define GBC_STATUS 0x0F