#pragma once

#include <Arduino.h>


/*
 * Circular buffer implementation. Will serve as buffer to store image data received from Arduino
 * serial connection to computer
 *
 * It is very barebones, but leannes is key here
 */

#define BUFFER_SIZE 640

typedef struct CBuffer{
	byte buffer[BUFFER_SIZE];
	unsigned int start;
	unsigned int end;
} CBuffer;

byte CBRead(CBuffer* cb);

void CBWrite(CBuffer* cb, byte b);
