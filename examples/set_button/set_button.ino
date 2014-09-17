/* Execute a function on button press
 * Example by Craig Durkin/Comingle, June 19, 2014
 * {♥} COMINGLE
*/

#include <OSSex.h>

void setup() {
  Toy.setID(0);
  Toy.attachClick(led);
}

void loop() {
}

byte state = 255;
void led() {
  Toy.setLED(0, state);
  state = abs(state - 255);
}
