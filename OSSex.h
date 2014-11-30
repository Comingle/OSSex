/* OSSex.h v0.3.1 - Library for controlling Arduino-based sex-toys
 * Written by Craig Durkin/Comingle
 * {♥} COMINGLE
*/


#ifndef OSSex_h
#define OSSex_h

#include <Arduino.h>
#include <avr/interrupt.h> 
#include <avr/io.h>
#include "OneButton.h"

// Timer4 is for ATmega32U4/Lilypad usb and should interrupt every ~1ms. Timer2 is for ATmega328P/Arduino Uno and should interrupt ~1ms.
#define TIMER4_INIT 16;
#define TIMER2_INIT 131;

// model names
 #define ALPHA 0
 #define BETA 1

class OSSex {
  public:
    OSSex();
    void setID(int);
    int setOutput(int, int);
    int setLED(int, int);
    int runShortPattern(int*, size_t);
    int runPattern(unsigned int);
    int runPattern(int* (*callback)(int));
    int cyclePattern();
    int addPattern(int* (*callback)(int));
	
	//Inputs
    int getInput(int);
    void calibrateInput(int,unsigned int);
	int updateInput(int);
	void setupInput(int, int);
	
	//Buttons and Patterns
	int getPattern();
    void update();
    void attachClick(void (*callback)());
    void attachDoubleClick(void (*callback)());
    void attachLongPressStart(void (*callback)());
    void attachLongPressStop(void (*callback)());
    void attachDuringLongPress(void (*callback)());
    void setScale(float);
    void setPowerScale(float);
    float increasePower();
    float decreasePower();
    void setTimeScale(float);
    float increaseTime();
    float decreaseTime();
    void stop();

	//Max number of inputs and outputs to set for the arrays for our current Lilypad USB based boards
    static const int _max_outputs = 8;
    static const int _max_leds = 8;
    static const int _max_inputs = 4;

    struct {
      int deviceId;                     // device identifier number
      bool bothWays;                    // can outputs go both forward and backward?
      uint8_t outCount;                 // number of outputs (electrodes, motors)
      uint8_t outPins[_max_outputs];    // array mapping to output pins
      uint8_t tuoPins[_max_outputs];    // array mapping to reverse output pins; //ahaha this is cute
      bool isLedMultiColor;             // do we have multicolored LEDs?
      uint8_t ledCount;                 // number of LEDs
      uint8_t ledPins[_max_leds];       // array mapping to LED output pins
	  
	  //Inputs section
	  uint8_t inCount;                  // number of input pins
      uint8_t inPins[_max_inputs];      // array mapping to input pins
	
	//Input Structure
		static const int _defaultInputBufferSize = 100;

	  struct {
        unsigned int pin;               // onboard pin
		
		//Calibration Values
		int min;					//the input's min from the calibration
		int max;					//the input's max from the calibration
		int avg;                // the computed from calibration average
		int STDEV; 				//Computed Standard deviation from the calibration
		int rawValue; 				//gets the most recent reading from the ADC
		int scaledValue; 			//gets the most recent reading scaled between the min and max, and mapped between 0 and 255
		int lastCal; 				//time in milliseconds of the last calibration (to find time since previous calibration do millis()-lastCal

		
		//Buffer Values
		int buffersize;				 //New buffersize to set things to
		int buffer[_defaultInputBufferSize];      // the past n readings from the analog input
		int bufferMin;  			//value of the current lowest value in the buffer
		int bufferMax;
		int bufferAVG;				//Current avg of the whole buffer
		int index;					// location in array of newest value
		double buffertotal; 					//the total value of the buffer at any given time
		//Useful Values for Quick Access (These might be better as functions, but this can be changed)
		int customThreshold;        //A user defined threshold
		int diffAVG; 			// The present value's current deviation from the computed average
		int diffThresh;  		// abs(present value - custom Threshold);

      } inputs[_max_inputs];
  
      struct {
        OneButton button;               // button object
        unsigned int pin;               // onboard pin
        unsigned int memAddress;        // EEPROM address for storing button state
      } buttons[1]; //TODO in the future, this may become variable like the max_outputs
	  

	  
    } device;
   
  private:
    struct pattern {
      int outNumber;
      uint8_t powerLevel;
      unsigned int duration;
      pattern *nextStep; 
    };
    pattern *_singlePattern;
    volatile pattern *_currentStep;

    struct patternList {
      int* (*patternFunc)(int);
      patternList *nextPattern;
    };
    volatile patternList *_currentPattern;
    volatile patternList *_first;
    int* (*_patternCallback)(int);

    float _powerScale;
    float _powerScaleStep;
    float _timeScale;
    float _timeScaleStep;
    volatile unsigned int _tickCount;
    volatile unsigned char *_timer_start_mask;
    volatile uint16_t *_timer_count;
    volatile unsigned char *_timer_interrupt_flag;
    volatile unsigned char *_timer_interrupt_mask_b;
    volatile unsigned char *_timer_interrupt_mask_a;
    unsigned int _timer_init;
    volatile bool _running;
    volatile pattern* _memQueue[2];
    volatile unsigned int _seq;

};

extern OSSex Toy;

#endif

