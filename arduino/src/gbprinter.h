#pragma once

#include <Arduino.h>

// Game Boy Game Link Arduino Pinout
#define GBP_CLOCK 8
#define GBP_IN 9
#define GBP_OUT 10
#define DELAY_MS 20

/*
 * Circular buffer implementation. Will serve as buffer to store image data received from Arduino
 * serial connection to computer
 *
 * It is very barebones, but leannes is key here
 */

#define BUFFER_SIZE 640u

typedef struct CBuffer{
	uint8_t buffer[BUFFER_SIZE];
	uint16_t start;
	uint16_t end;
} CBuffer;

void CBInit();

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
#define GBC_TRANSFER 0x04
#define GBC_REPORT 0x0F

/*
 * State Machine functions
 * Use as ref:
 * - http://c-faq.com/decl/recurfuncp.html
 * - http://codeandlife.com/2013/10/06/tutorial-state-machines-with-c-callbacks/
 * The library will have 2 state machines, one to govern the control flow between the PC and the
 * Arduino and another to handle the control flow between the Arduino and the GBPrinter.
 */
typedef void (*funcptr)();   /* generic function pointer */
typedef funcptr (*ptrfuncptr)();  /* ptr to fcn returning g.f.p. */

typedef struct ArduinoState{
	ptrfuncptr current;
	uint32_t total;
	uint32_t printed;
} ArduinoState;

typedef struct GBPState{
	ptrfuncptr current;
} GBPState;

extern ArduinoState ARDUINO_STATE;
extern GBPState GBP_STATE;
#define MSG_SIZE 4
extern uint8_t MSG_BUFFER[MSG_SIZE];

// Functions to reset state machine
void ArduinoStateInit();
void GBPStateInit();

// Arduino States
funcptr ArduinoIdle();
funcptr ArduinoSetup();
funcptr ArduinoPrint();

// GBP States
funcptr GBPInitialize();
funcptr GBPTransfer();
funcptr GBPPrint();
