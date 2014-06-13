/* Comingle.h v0.1.2 - Library for controlling Arduino-based sex-toys
 * Written by Craig Durkin/Comingle, May 9, 2014
 * 
*/

#ifndef Comingle_h
#define Comingle_h

#include <Arduino.h>
#include <avr/interrupt.h> 
#include <avr/io.h>

#define MAX_OUTPUTS (8);
#define MAX_LEDS (8);
#define MAX_INPUTS (4);
#define MAX_PATTERN_STEPS (16);

// Timer4 is for ATmega32U4/Lilypad usb. Timer2 is for ATmega328P/Arduino Uno. Both values were hand-tuned to arrive at an interrupt
// time of approximately 1ms.
#define TIMER4_INIT 25;
#define TIMER2_INIT 131;

class Comingle {
  public:
    Comingle(int);
    int setOutput(int, int);
    int setLED(int, int);
    int runPattern(int*, unsigned int);
    void setPattern(unsigned int, int*);
    int getInput(int);
    int flicker(int, unsigned int, unsigned int);
    void oscillate();
    void checkPattern();
    static const int _max_pattern_steps = 32;
    static const int _max_outputs = 8;
    static const int _max_leds = 8;
    static const int _max_inputs = 4;
    struct device {
      bool bothWays;                    // can outputs go both forward and backward?
      uint8_t outCount;                 // number of outputs (electrodes, motors)
      uint8_t outPins[_max_outputs];     // array mapping to output pins
      uint8_t tuoPins[_max_outputs];     // array mapping to reverse output pins;
      bool isLedMultiColor;             // do we have multicolored LEDs?
      uint8_t ledCount;                 // number of LEDs
      uint8_t ledPins[_max_leds];        // array mapping to LED output pins
      uint8_t inCount;               // number of input pins
      uint8_t inPins[_max_inputs];    // array mapping to input pins
      uint8_t buttonPins[1];
      int deviceId;
    } _device;
  private:
    int _singlePattern[_max_pattern_steps][3]; // {motornumber, powerlevel, time (millis)}
    size_t _singlePatternLength; 
    unsigned int _patterns[_max_pattern_steps][3];
    volatile int _i;
    volatile int _tickCount;
    volatile unsigned char *_timer_start_mask;
    volatile uint16_t *_timer_count;
    volatile unsigned char *_timer_interrupt_flag;
    volatile unsigned char *_timer_interrupt_mask_b;
    volatile unsigned char *_timer_interrupt_mask_a;
    unsigned int _timer_init;
};

#endif



