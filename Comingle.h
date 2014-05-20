/* Comingle.h v0.1 - Library for controlling Arduino-based sex-toys
 * Written by Craig Durkin/Comingle, May 9, 2014
 * 
*/

#ifndef Comingle_h
#define Comingle_h

#include <Arduino.h>
#include <avr/interrupt.h> 
#include <avr/io.h>

#define DEVICE_TONGA (0);

#define MAX_OUTPUTS (8);
#define MAX_LEDS (8);
#define MAX_INPUTS (4);
#define MAX_PATTERN_STEPS (16);

// 25 cycles out of 255 total (timer 4 is 8-bit). This, combined with TCCR4B and the clock speed, sets the interrupt time to ~1 ms
#define TIMER4_INIT 25;

class Comingle {
  public:
    Comingle();
    int setOutput(int, int);
    int setLED(int, int);
    int runPattern(int*, unsigned int);
    void definePattern(unsigned int, int*);
    void readInput();
    void deviceInfo();
    void fade();
    void oscillate();
    static const int _max_pattern_steps = 16;
    void checkPattern();
  private:

    static const int _device_tonga = 0;
    static const int _max_outputs = 8;
    static const int _max_leds = 8;
    static const int _max_inputs = 4;
    int _tickCount;
    int _singlePattern[_max_pattern_steps][3]; // {motornumber, powerlevel, time (millis)}
    size_t _singlePatternLength; 
    int _i;
    struct device {
      bool bothWays;                    // can outputs go both forward and backward?
      uint8_t outCount;                 // number of outputs (electrodes, motors)
      uint8_t outPins[_max_outputs];     // array mapping to output pins
      uint8_t tuoPins[_max_outputs];     // array mapping to reverse output pins;
      bool isLedMultiColor;             // do we have multicolored LEDs?
      uint8_t ledCount;                 // number of LEDs
      uint8_t ledPins[_max_leds];        // array mapping to LED output pins
      uint8_t inputCount;               // number of input pins
      uint8_t inputPins[_max_inputs];    // array mapping to input pins
    } _device;
    
    unsigned int _threshold;
    int _deviceId;
    unsigned int _patterns[_max_pattern_steps][4];
};

#endif



