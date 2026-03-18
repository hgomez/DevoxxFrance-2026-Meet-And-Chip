#include <bluefruit.h>

/***
 * Turn all leds OFF
 */
void all_leds_off() {

  //  Output is LOW, no current flow from GPIO
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
}

/***
 * Turn all leds ON
 */
void all_leds_on() {

  //  Output is HIGH, current flow from GPIO to LED and ground
  digitalWrite(D3, HIGH);
  digitalWrite(D4, HIGH);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  digitalWrite(D7, HIGH);
  digitalWrite(D8, HIGH);
}


void circle_one_led(int time) {

  all_leds_off();

  digitalWrite(D3, HIGH);
  delay(time);

  digitalWrite(D3, LOW);
  digitalWrite(D4, HIGH);
  delay(time);

  digitalWrite(D4, LOW);
  digitalWrite(D5, HIGH);
  delay(time);

  digitalWrite(D5, LOW);
  digitalWrite(D6, HIGH);
  delay(time);

  digitalWrite(D6, LOW);
  digitalWrite(D7, HIGH);
  delay(time);

  digitalWrite(D7, LOW);
  digitalWrite(D8, HIGH);
  delay(time);

  digitalWrite(D8, LOW);
}

void setup() {
  // initialize digital pin D3-D8 and the built-in LED as an output.
  pinMode(D3,OUTPUT);
  pinMode(D4,OUTPUT);
  pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // Init serial communication for debugging
  Serial.begin(115200);
  long start = millis();
  while ( !Serial && (millis() - start < 2000)) delay(10);
}


void loop() {

  // Manage bootloader switch for over_the_cable fw update (type ! on serial console)
  while ( Serial && Serial.available() ) if (Serial.read()=='!') enterSerialDfu();

  Serial.println("Leds off");
  all_leds_off();
  delay(1000);

  Serial.println("Leds on");
  all_leds_on();
  delay(1000);

  Serial.println("Leds circle");
  circle_one_led(100);
}
