#include <bluefruit.h>


void setup() {
  // Init serial communication for debugging
  Serial.begin(115200);
  long start = millis();
  while ( !Serial && (millis() - start < 2000)) delay(10); 
}


void loop() {

  // Manage bootloader switch for over_the_cable fw update (type ! on serial console)
  while ( Serial && Serial.available() ) if (Serial.read()=='!') enterSerialDfu();

  Serial.println("Hello Devoxx France");
  delay(1000);
}
