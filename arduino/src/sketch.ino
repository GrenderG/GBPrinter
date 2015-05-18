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
}

void loop()
{
}
