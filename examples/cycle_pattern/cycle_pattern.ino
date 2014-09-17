/* Runs a simple Pattern
 * Example by Craig Durkin/Comingle, May 9, 2014
 * {♥} COMINGLE
*/

#include <OSSex.h>

void setup() {
  Toy.setID(0);

  Toy.addPattern(fadeCos);
  Toy.addPattern(pulse);

  Toy.attachClick(click);  
}

void loop() {
}

void click() {
  Toy.cyclePattern();
}

int step[3];
// fade all of the motors from low to high over and over
int *fadeCos(int seq) {
  step[0] = -1;
  step[2] = 50;
  step[1] = round(127 * cos((seq / (8*PI))-PI) + 127);
  return step;
}

// Randomly blip an output on for a short burst.
int* pulse(int seq) {
  if (seq % 2) {
    step[0] = -1;
    step[1] = 0;
  } else {
    step[0] = random(0,3);
    step[1] = 144;
  }
  step[2] = 70;
  return step;
}
