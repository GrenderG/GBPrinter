#pragma once

#include <Arduino.h>

#define BUFFER_SIZE 640

typedef struct CBuffer{
	byte buffer[BUFFER_SIZE];
	unsigned int start;
	unsigned int end;
} CBuffer;

byte CBRead(CBuffer* cb);
void CBWrite(CBuffer* cb, byte b);
