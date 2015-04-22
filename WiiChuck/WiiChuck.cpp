#include <WiiChuck.h>

WiiChuck::WiiChuck() {}

void WiiChuck::begin()
{

    //send initialization handshake
    Wire.begin();

    //averageCounter = 0;
    // instead of the common 0x40 -> 0x00 initialization, we
    // use 0xF0 -> 0x55 followed by 0xFB -> 0x00.
    // this lets us use 3rd party nunchucks (like cheap $4 ebay ones)
    // while still letting us use official oness.
    // only side effect is that we no longer need to decode bytes in _nunchuk_decode_byte
    // see http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1264805255
    //
    Wire.beginTransmission(0x52);       // device address
    Wire.write(0xF0);
    Wire.write(0x55);
    Wire.endTransmission();
    delay(1);
    Wire.beginTransmission(0x52);
    Wire.write(0xFB);
    Wire.write((uint8_t)0x00);

    Wire.endTransmission();
    update();
    for (int i = 0; i < 3;i++) {
        angles[i] = 0;
    }
    zeroJoyX = DEFAULT_ZERO_JOY_X;
    zeroJoyY = DEFAULT_ZERO_JOY_Y;

    buttonZ.button.setActiveLow(false);
    buttonC.button.setActiveLow(false);
}

void WiiChuck::calibrateJoy() {
    zeroJoyX = joyX;
    zeroJoyY = joyY;
}

void WiiChuck::update() {
  uint8_t cnt = 0;

  Wire.requestFrom (0x52, 6);

  while (Wire.available()) {
    // receive byte as an integer
    status[cnt] = Wire.read();
    cnt++;
  }
  if (cnt > 5) {
    lastJoyX = readJoyX();
    lastJoyY = readJoyY();
    //averageCounter ++;
    //if (averageCounter >= AVERAGE_N)
    //    averageCounter = 0;

    joyX = status[0];
    joyY = status[1];
    for (int i = 0; i < 3; i++) {
      //accelArray[i][averageCounter] = ((int)status[i+2] << 2) + ((status[5] & (B00000011 << ((i+1)*2) ) >> ((i+1)*2)));
      angles[i] = (status[i+2] << 2) + ((status[5] & (B00000011 << ((i+1)*2) ) >> ((i+1)*2)));
    }
    //accelYArray[averageCounter] = ((int)status[3] << 2) + ((status[5] & B00110000) >> 4);
    //accelZArray[averageCounter] = ((int)status[4] << 2) + ((status[5] & B11000000) >> 6);

    buttonZ.value = !(status[5] & B00000001);
    buttonC.value = !((status[5] & B00000010) >> 1);
    buttonZ.button.tick();
    buttonC.button.tick();
    _send_zero(); // send the request for next bytes
    cnt = 0;
  }
}

byte * WiiChuck::getStatus() {
  return status;
}

float WiiChuck::readAccelX() {
  // total = 0; // accelArray[xyz][averageCounter] * FAST_WEIGHT;
  return (float)angles[0] - ZEROX;
}
float WiiChuck::readAccelY() {
    // total = 0; // accelArray[xyz][averageCounter] * FAST_WEIGHT;
    return (float)angles[1] - ZEROY;
}
float WiiChuck::readAccelZ() {
    // total = 0; // accelArray[xyz][averageCounter] * FAST_WEIGHT;
    return (float)angles[2] - ZEROZ;
}

bool WiiChuck::zPressed() {
    return buttonZ.value;
}
bool WiiChuck::cPressed() {
    return buttonC.value;
}

// COMBINE right/leftJoy?
// for using the joystick like a directional button
bool WiiChuck::rightJoy(int thresh=60) {
    return (readJoyX() > thresh and lastJoyX <= thresh);
}

// for using the joystick like a directional button
bool WiiChuck::leftJoy(int thresh=60) {
    return (readJoyX() < -thresh and lastJoyX >= -thresh);
}


uint8_t WiiChuck::readJoyX() {
    return joyX; // - zeroJoyX;
}

uint8_t WiiChuck::readJoyY() {
    return joyY; // - zeroJoyY;
}


//R, the radius, generally hovers around 210 (at least it does with mine)
int WiiChuck::R() {
    return sqrt(readAccelX() * readAccelX() +readAccelY() * readAccelY() + readAccelZ() * readAccelZ());
}


// returns roll degrees
int WiiChuck::readRoll() {
    return (int)(atan2(readAccelX(),readAccelZ())/ M_PI * 180.0);
}

// returns pitch in degrees
int WiiChuck::readPitch() {
    return (int) (acos(readAccelY()/R())/ M_PI * 180.0);  // optionally swap 'RADIUS' for 'R()'
}

void WiiChuck::_send_zero()
{
    Wire.beginTransmission (0x52);
    Wire.write ((uint8_t)0x00);
    Wire.endTransmission ();
}

void WiiChuck::attachZClick(void (*callback)(void)) {
  buttonZ.button.attachClick(callback);
}

void WiiChuck::attachZDoubleClick(void (*callback)(void)) {
  buttonZ.button.attachDoubleClick(callback);
}

void WiiChuck::attachZUpdate(bool (*callback)(void)) {
  buttonZ.button.setPseudo(callback);
}

void WiiChuck::attachCClick(void (*callback)(void)) {
  buttonC.button.attachClick(callback);
}

void WiiChuck::attachCDoubleClick(void (*callback)(void)) {
  buttonC.button.attachDoubleClick(callback);
}

void WiiChuck::attachCUpdate(bool (*callback)(void)) {
  buttonC.button.setPseudo(callback);
}
