/* Nunchuck basic click example -- sets the C button to immediately stop
 * whatever pattern is playing, and the Z button to increase power
 * Craig Durkin / Comingle - Jul 14, 2015
 */

#include <OSSex.h>
#include <Wire.h>
#include <WiiChuck.h>

bool c_update();
bool z_update();

// Create the nunchuck object
// c_update and z_update are the button update functions. OSSex/OneButton normally works
// only with physical buttons (something that is measurably high or low on a digital
// input), but can also work with any function that returns 0/1, high/low, true/false.
WiiChuck nunchuck = WiiChuck(c_update, z_update);

bool c_update() {
  return nunchuck.cPressed();
}

bool z_update() {
  return nunchuck.zPressed();
}

// c_click is the function that gets called when a click is detected
// on the C button. So OSSex/OneButton continously monitors c_update(), 
// and when it gets a high/true value, it runs c_click()
void c_click() {
  Toy.stop();
}

void c_doubleclick() {
  Toy.cyclePattern();
}

// Likewise for z_update, z_click.
void z_click() {
  Toy.increasePower();
}

void setup() {
  // The nunchuck communicates over the I2C protocol. This line sets the Mod/Dilduino's
  // Hacker Port to I2C mode.
  Toy.setHackerPort(HACKER_PORT_I2C);

  // Open the I2C connection
  nunchuck.begin();

  // Attach our click handlers
  nunchuck.attachCClick(c_click);
  nunchuck.attachCDoubleClick(c_doubleclick);
  nunchuck.attachZClick(z_click);

  // Queue up our vibration patterns
  Toy.addPattern(sharp_ramp);
  Toy.addPattern(weird2);
  Toy.addPattern(weird3);
  Toy.addPattern(flicker);
  Toy.addPattern(second);
}

void loop() {
  // Poll the I2C connection for updates (button clicks, but also 
  // acceleration changes and joystick movements)
  nunchuck.update();
}

// Some patterns to control
int sharp_ramp(int seq) {
  // neat exponential sequence inspired by github/jgeisler0303
  const uint8_t fadeTable[32] = {0, 1, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 9, 10, 12, 15, 17, 21, 25, 30, 36, 43, 51, 61, 73, 87, 104, 125, 149, 178, 213, 255};
  seq %= 32;
  Toy.step[0] = Toy.step[1] = Toy.step[2] = fadeTable[seq];
  Toy.step[3] = 12 ;
  return 1;
}
int weird2(int seq) {
  Toy.step[2] = round(127*cos(tan(tan(seq/(8*PI)))-PI/2)+127);
  Toy.step[3] = 30;
  return 1;
}
int weird3(int seq) {
  Toy.step[2] = round(50*(cos(seq/(8*PI)+PI/2) + sin(seq/2))+100);
  Toy.step[3] = 30;
  return 1;
}
// Turn on all outputs slightly offset from each other.
int flicker(int seq) {
  // set all motors initially to -1, ie "leave it alone"
  Toy.step[0] = Toy.step[1] = Toy.step[2] = -1;

  if (seq > 2) {
    Toy.step[3] = 200;
  } else {
    Toy.step[3] = 20;
  }

  seq %= 3;
  Toy.step[seq] = 80;

  return 1;
}
// Second motor only
int second(int seq) {
  Toy.step[0] = 0;
  Toy.step[1] = 100;
  Toy.step[2] = 0;
  Toy.step[3] = 50;
  return 1;
}
