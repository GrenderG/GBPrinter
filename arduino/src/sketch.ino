#include <Arduino.h>
#include "gbprinter.h"


void setup()
{
	pinMode(GBP_OUT, OUTPUT);
	pinMode(GBP_CLOCK, OUTPUT);
	pinMode(GBP_IN, INPUT); // set pin to input
	digitalWrite(GBP_IN, HIGH);  // turn on pullup resistors
	digitalWrite(GBP_OUT, HIGH);  // turn on pullup resistors
	CBInit();
	ArduinoStateInit();
	GBPStateInit();
	Serial.begin(9600);
}

void loop()
{
	if (Serial.available() > 0)
		ARDUINO_STATE.current = (ptrfuncptr) ARDUINO_STATE.current();
}
