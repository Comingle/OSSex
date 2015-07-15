/* Runs a simple pattern when the button is clicked.
 * Example by Craig Durkin/Comingle, May 9, 2014
 * {♥} COMINGLE
*/

#include <OSSex.h>
#include <Wire.h>

int pattern[][4] = {
    {200, 200, 200, 500},
    {0, 0, 0, 500},
};
unsigned int patternSize = sizeof(pattern) / sizeof(int) / 4;

bool clicked = false;

void setup() {
  Toy.setID(MOD);
  
  Toy.attachClick(click);
}

void loop() {
  if (clicked) {
    Toy.runShortPattern(*pattern, patternSize);
  } else {
    Toy.setOutput(-1, 0);
  }
}

void click() {
  clicked = !clicked;
}
