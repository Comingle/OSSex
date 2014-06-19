/* Execute a function on button press
 * Example by Craig Durkin/Comingle, June 19, 2014
 * {♥} COMINGLE
*/

#include <Comingle.h>

Comingle dev(0);

void led() {
  if (digitalRead(2) == LOW) {
    dev.setLED(0, 250);
  } else {
    dev.setLED(0,0);
  }
}

void setup() {
  dev.setButton(led);
}
 
void loop() {
}
