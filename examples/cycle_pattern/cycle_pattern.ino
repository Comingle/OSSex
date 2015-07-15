/* Runs a simple Pattern
 * Example by Craig Durkin/Comingle, May 9, 2014
 * {♥} COMINGLE
*/

#include <OSSex.h>
#include <Wire.h>

void setup() {
  Toy.setID(MOD);

  Toy.addPattern(fade_cos);
  Toy.addPattern(pulse);

  Toy.attachClick(click);  
}

void loop() {
}

void click() {
  Toy.cyclePattern();
}

// fade all of the motors from low to high over and over
int fade_cos(int seq) {
  Toy.step[0] = Toy.step[1] = Toy.step[2] = round(127 * cos((seq / (8*PI))-PI) + 127);
  Toy.step[3] = 50;
  return 1;
}

// Randomly blip an output on for a short burst.
int pulse(int seq) {
  if (seq % 2) {
    Toy.step[0] = Toy.step[1] = Toy.step[2] = 0;
  } else {
    Toy.step[random(0,3)] = 144;
  }

  Toy.step[3] = 70;
  return 1;
}
