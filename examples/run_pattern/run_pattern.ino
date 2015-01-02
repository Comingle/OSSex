/* Runs a simple pattern when the button is clicked.
 * Example by Craig Durkin/Comingle, May 9, 2014
 * {♥} COMINGLE
*/

#include <OSSex.h>

int pattern[][3] = {
    {-1, 200, 1000},
    {-1, 0, 1000},
};
unsigned int patternSize = sizeof(pattern) / sizeof(int) / 3;

bool clicked = false;

void setup() {
  Toy.setID(0);
  
  Toy.attachClick(click);
}

void loop() {
  if (clicked) {
    Toy.runShortPattern(*pattern, patternSize);
  }
}

void click() {
  clicked = true;
}
