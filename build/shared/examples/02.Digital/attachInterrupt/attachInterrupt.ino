/*

attachInterrupt()

Description
  Specifies a function to call when an external interrupt occurs.
  Replaces any previous function that was attached to the interrupt.
  Customized for UDOO Neo.

by UDOO Team
@ 2015/10

*/

int stOut = 0;
int stLed = 0;
uint16_t cbCounter=0;
uint16_t counter=0;

void pinCB (void) {
	digitalWrite(13, stLed);
	stLed ^= 1;
	cbCounter++;
}

void setup() {
  pinMode(8, INPUT);
  pinMode(4, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(4, LOW);
  digitalWrite(13, LOW);

  attachInterrupt(8, pinCB, RISING);

  for (int i=0; i<10; i++) {
    stOut ^= 1;
    digitalWrite(4, stOut);
    delay(1000);
  }
}

void loop() {
  stOut ^= 1;
  digitalWrite(4, stOut);
  delay(250);
}
