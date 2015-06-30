#include "gbprinter.h"


void setup()
{
	pinMode(GBP_OUT, OUTPUT);
	pinMode(GBP_CLOCK, OUTPUT);
	pinMode(GBP_IN, INPUT); // set pin to input
	digitalWrite(GBP_IN, HIGH);  // turn on pullup resistors
	digitalWrite(GBP_OUT, HIGH);  // turn on pullup resistors
    // Init circle buffer and state machine
    CBInit();
	ArduinoStateInit();
    // Start serial port
	Serial.begin(9600);
	Serial.setTimeout(20000);
}

void loop()
{
    // The loop is remarkably simple. The protocol defined by the state machine basically
    // requires for the Desktop client to initiate the connection, which means that we will
    // wait on serial data to be available. Once that is happened, we can execute the current
    // state, which will return whichever kind of response is appropriate to the client AND
    // advance the machine to the next state.
    // Rinse and repeat, and we should be good to go
	if (Serial.available() > 0)
		ARDUINO_STATE.current = (ptrfuncptr) ARDUINO_STATE.current();
}
