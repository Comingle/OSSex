/* OSSex.cpp v0.3.1 - Library for controlling Arduino-based sex-toys
 * Written by Craig Durkin/Comingle
 * {♥} COMINGLE
*/


#include <Arduino.h>
#include <OSSex.h>
#include <avr/interrupt.h> 
#include <avr/io.h>
#include "OneButton.h"

// Set up the interrupt to trigger the update() function.
#if defined(__AVR_ATmega32U4__) // Lilypad USB
ISR(TIMER4_OVF_vect) {
	Toy.update();
};
#endif

// Pre-instantiate with empty constructor. Pre-instantiation is necessary for the timer2/timer4 interrupt to work. Empty constructor because
// the user defines which device they're using elsewhere (in setID());
OSSex Toy = OSSex();
OSSex::OSSex() {}

// the real constructor. give it a device ID and it will set up your device's pins and timers.
void OSSex::setID(int deviceId) {
#if defined(__AVR_ATmega32U4__)
	_timer_start_mask = &TCCR4B;
	_timer_count = &TCNT4;
    _timer_interrupt_flag = &TIFR4;
    _timer_interrupt_mask_b = &TIMSK4;
    _timer_init = TIMER4_INIT;
#endif

	if (deviceId == 1) {
		// Beta Model
		device.outCount = 3;
		device.outPins[0] = 3; 
		device.outPins[1] = 5; 
		device.outPins[2] = 10;
		
		device.deviceId = 1;
		
		device.ledCount = 1;
		device.ledPins[0] = 13;

		// Technically 4, but 2 inputs remain unconnected in most models
		device.inCount = 2;
		device.inPins[0] = A8; // D+
		device.inPins[1] = A7; // D-

		device.buttons[0].button.setPin(2);
		device.buttons[0].button.setActiveLow(true);	
		device.buttons[0].pin = 2;	

	} else {
		// Lilypad USB  / Alpha model
		device.outCount = 3;
		device.outPins[0] = 3; 
		device.outPins[1] = 9; 
		device.outPins[2] = 10;
		
		device.deviceId = 0;
		
		device.ledCount = 1;
		device.ledPins[0] = 13;

		device.buttons[0].button.setPin(2);
		device.buttons[0].button.setActiveLow(true);	
		device.buttons[0].pin = 2;	

		device.inCount = 2;
		device.inPins[0] = A2; // D+
		device.inPins[1] = A3; // D-
		//New Version of Input Structures
		device.inputs[0].pin = A2; // D+
		device.inputs[1].pin = A3; //D-
		
		
			device.bothWays = false;
				device.isLedMultiColor = false;
				
		//Setup Inputs //The defacto buffer should have only 1 point (same as reading raw data)
		const int numReadings = 1;
int inputBuffer1[numReadings];      // the readings from the analog input 1
int inputBuffer2[numReadings];
int input1
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

int inputPin = device.inPins[inNumber];

//Input Structure//Calibration Values
//Setup input 1
		device.inputs[0].min = 0;					//the input's min from the calibration
		device.inputs[0].max = 1023;					//the input's max from the calibration
		device.inputs[0].avg = 512;                // the computed from calibration average
		device.inputs[0].STDEV = -1; 				//Computed Standard deviation from the calibration
		device.inputs[0].rawValue = -1; 				//gets the most recent reading from the ADC
		device.inputs[0].scaledValue= -1; 			//gets the most recent reading scaled between the min and max, and mapped between 0 and 255
		device.inputs[0].lastCal = -1; 				//time in milliseconds of the last calibration (to find time since previous calibration do millis()-lastCal
		
		//Buffer Values
		device.inputs[0].buffersize = -1;				 //New buffersize to set things to
		device.inputs[0].buffer[_defaultInputBufferSize];      // the past n readings from the analog input
		device.inputs[0].bufferMin = -1;  			//value of the current lowest value in the buffer
		device.inputs[0].bufferMax = -1;
		device.inputs[0].bufferAVG = -1;				//Current avg of the whole buffer
		device.inputs[0].index = 0;					// location in array of newest value
		//Useful Values for Quick Access (These might be better as functions, but this can be changed)
		device.inputs[0].customThreshold = 512;        //A user defined threshold
		device.inputs[0].diffAVG = -1; 			// The present value's current deviation from the computed average
		device.inputs[0].diffThresh = -1;  		// abs(present value - custom Threshold);
//Setup Input 2
		device.inputs[1].min = 0;					//the input's min from the calibration
		device.inputs[1].max = 1023;					//the input's max from the calibration
		device.inputs[1].avg = 512;                // the computed from calibration average
		device.inputs[1].STDEV = -1; 				//Computed Standard deviation from the calibration
		device.inputs[1].rawValue = -1; 				//gets the most recent reading from the ADC
		device.inputs[1].scaledValue= -1; 			//gets the most recent reading scaled between the min and max, and mapped between 0 and 255
		device.inputs[1].lastCal = -1; 				//time in milliseconds of the last calibration (to find time since previous calibration do millis()-lastCal
		
		//Buffer Values
		device.inputs[1].buffersize = -1;				 //New buffersize to set things to
		device.inputs[1].buffer[_defaultInputBufferSize];      // the past n readings from the analog input
		device.inputs[1].bufferMin = -1;  			//value of the current lowest value in the buffer
		device.inputs[1].bufferMax = -1;
		device.inputs[1].bufferAVG = -1;				//Current avg of the whole buffer
		device.inputs[1].index = 0;					// location in array of newest value
		//Useful Values for Quick Access (These might be better as functions, but this can be changed)
		device.inputs[1].customThreshold = 512;        //A user defined threshold
		device.inputs[1].diffAVG = -1; 			// The present value's current deviation from the computed average
		device.inputs[1].diffThresh = -1;  		// abs(present value - custom Threshold);
	}
	device.bothWays = false;

	device.isLedMultiColor = false;
    
	for (int i = 0; i < device.outCount; i++) {
		pinMode(device.outPins[i], OUTPUT);
		if (device.bothWays) {
			pinMode(device.tuoPins[i], OUTPUT);
		}
	}
	for (int i = 0; i < device.inCount; i++) {
		pinMode(device.inPins[i], INPUT);
	}
	for (int i = 0; i < device.ledCount; i++) {
		pinMode(device.ledPins[i], OUTPUT);
	}


	// Start the interrupt timer (timer2/timer4)
	// Thanks for Noah at arduinomega.blogspot.com for clarifying this
	
	*_timer_interrupt_mask_b = 0x04;    // Timer INT Reg: Timer Overflow Interrupt Enable: 00000100   
  	_tickCount = 0;
 	*_timer_count = _timer_init;			// Reset Timer Count
 	*_timer_interrupt_flag = 0x00;			// Timer INT Flag Reg: Clear Timer Overflow Flag
 	*_timer_start_mask = 0x05;				// Timer PWM disable, prescale / 16: 00000101

 	_powerScale = 1.0;
 	_powerScaleStep = 0.1;
 	_timeScale = 1.0;
 	_timeScaleStep = 0.1;

}

// Called by the timer interrupt to check if a change needs to be made to the pattern or update the button status.
// If a pattern is running, it will set the _running flag
void OSSex::update() {
	device.buttons[0].button.tick(); 
	if (_running) {
		_tickCount++;
		if (_tickCount > (_currentStep->duration * _timeScale)) {
	  		if (_currentStep->nextStep == NULL) { 
	  			// stop the pattern if at last step
	    		_running = false;
	  		} else {
	  			// run the next step
	  			_currentStep = _currentStep->nextStep;

	  			// if we're running a large pre-set pattern, we're supplied all the steps at once so we can't store
	  			// all our allocated memory in _memQueue (since it only holds 2 elements). this !_patternCallback 
	  			// check ensures that memory still gets freed eventually in those situations.
	  			if (!_patternCallback) {
	  				_memQueue[1] = _currentStep;
	  			}
	  			setOutput(_currentStep->outNumber, _currentStep->powerLevel);
	  		}
	  		free((void*)_memQueue[0]);
	  		_memQueue[0] = _memQueue[1];
	  		_memQueue[1] = NULL;
	  		_tickCount = 0; 
		} else if (_currentStep->nextStep == NULL && _patternCallback) {
			// if it's not time for the next step, go ahead and queue it up
			int *callbackStep = _patternCallback(_seq);
			if (callbackStep != NULL) {
				_seq++;
				_currentStep->nextStep = new struct pattern;
				_memQueue[1] = _currentStep->nextStep;
				_currentStep->nextStep->outNumber =  *(callbackStep++);
				_currentStep->nextStep->powerLevel = *(callbackStep++);
				_currentStep->nextStep->duration = *(callbackStep++);
				_currentStep->nextStep->nextStep = NULL;
			} else {
				_running = false;
			}
		}
	} 

	*_timer_count = _timer_init;		//Reset timer after interrupt triggered
  	*_timer_interrupt_flag = 0x00;		//Clear timer overflow flag
}


// Set an output to a particular power level. If outNumber is -1, set all outputs to powerLevel.
// outNumber of any other negative number or a number greater than or equal to the number of available outputs will be rolled over.
// Ex: in a 4 output device, you can access outputs 0, 1, 2, and 3.
// Specifying outNumber of -3 will map to output 3. Specifying an outNumber of 5 will map to output 1.
// powerLevel can be from 0..255 in devices that aren't bidirectional, and -255..255 in birdirectional devices. 
// Negative powerLevel values are coerced to 0 in devices that aren't bidirectional.
// powerLevel of 0 turns the output off. Values greater than +/-255 get coerced to +/-255.
// XXX Add serial (Stream object) feedback from function for diagnostics
int OSSex::setOutput(int outNumber, int powerLevel) {
	int iterations = 1, constrainedPower;
	// set all outputs, starting at 0.
	if (outNumber == -1) {
		iterations = device.outCount;
		outNumber = 0;
	} else {
		outNumber = abs(outNumber) % device.outCount;
	}

	if (device.bothWays) {
		constrainedPower = constrain(powerLevel, -255, 255);
	} else {
		constrainedPower = constrain(powerLevel, 0, 255);
	}

	if (_powerScale * constrainedPower > 255) {
		_powerScale = 255/constrainedPower;
	}
	
	for (int i = 0; i < iterations; i++) {
		if (constrainedPower == 0) {
			analogWrite(device.outPins[outNumber], 0);
			if (device.bothWays) {
				analogWrite(device.tuoPins[outNumber], 0);
			}
		} else if (constrainedPower > 0) {
			analogWrite(device.outPins[outNumber], constrainedPower * _powerScale);
		} else {
			analogWrite(device.tuoPins[outNumber], constrainedPower * _powerScale);
		}
		outNumber = i+1;
	}

	return 1;

}


// Turn an LED on or off. lightLevel can be a value from 0-255. 0 turns the LED off.
// Accept html color codes (both "#50a6c2" and "midnight blue"?)
// Add serial (Stream object) feedback from function for diagnostics
//void OSSex::setLED(unsigned int lightLevel, ledNumber, colorCode) {}
int OSSex::setLED(int ledNumber, int powerLevel) {
	int constrainedPower;
	if (!device.ledCount) {
		return -1;
	}
	// sanitize ledNumber XXX -1 logic
	ledNumber %= device.ledCount;
	constrainedPower = constrain(powerLevel, 0, 255);
	analogWrite(device.ledPins[ledNumber], constrainedPower);

	return 1;

}

// Run preset pattern from an array of {outputNumber, powerLevel, duration} steps
// This function will not return until the pattern is finished running.
int OSSex::runShortPattern(int* patSteps, size_t patternLength) {
	stop();
	
	if (patternLength) {
		_singlePattern = new struct pattern;
		if (!_singlePattern) {
			return -1;
		}
		_memQueue[0] = _singlePattern;

		_singlePattern->nextStep = NULL;
		pattern* patIndex = _singlePattern;

		for (int i = 0; i < patternLength; i++) {
			patIndex->outNumber = *(patSteps++);
			patIndex->powerLevel = *(patSteps++);
			patIndex->duration = *(patSteps++);
			if (i < patternLength-1) {
				patIndex->nextStep = new struct pattern;
				if (!patIndex->nextStep) {
					return -1;
				}
				patIndex = patIndex->nextStep;
			} else {
				patIndex->nextStep = NULL;
			}
		}

		// position _currentStep at start of pattern, start the first step, and set things in motion
		_currentStep = _singlePattern;
		setOutput(_currentStep->outNumber, _currentStep->powerLevel);
		_running = true;

		// Wait until pattern is finished to return
		while (_running) {}		
		return 1;
	} else {
		return 0;
	}

}


// Run a pattern from a callback function. The callback should return a pointer to a 3-item array: [outputNumber, powerLevel, duration]
// This function will return before the pattern is finished running since many functions will run indefinitely and block all other processing.
int OSSex::runPattern(int* (*callback)(int)) {
	stop();
	
	// get the first two steps of the sequence. 
	// if we don't, some patterns with short first steps won't run well
	// since the next step is queued while the current one is running
	_patternCallback = callback;
	int *callbackStep = _patternCallback(_seq);
	if (callbackStep != NULL) {
		_seq++;
		_singlePattern = new struct pattern;
		if (!_singlePattern) {
			return -1;
		}
		_memQueue[0] = _singlePattern;

		_singlePattern->outNumber = *(callbackStep++);
		_singlePattern->powerLevel = *(callbackStep++);
		_singlePattern->duration = *(callbackStep++);
		
		callbackStep = _patternCallback(_seq);
		if (callbackStep != NULL) {
			_seq++;
			_singlePattern->nextStep = new struct pattern;
			if (!_singlePattern->nextStep) {
				return -1;
			}
			_memQueue[1] = _singlePattern->nextStep;

			_singlePattern->nextStep->outNumber = *(callbackStep++);
			_singlePattern->nextStep->powerLevel = *(callbackStep++);
			_singlePattern->nextStep->duration = *(callbackStep++);
			_singlePattern->nextStep->nextStep = NULL;
		} else {
			return 0;
		}
		_currentStep = _singlePattern;
		setOutput(_currentStep->outNumber, _currentStep->powerLevel);
		_running = true;
		return 1;
	} else {
		return 0;
	}

}

// run a specific pattern from the queue
int OSSex::runPattern(unsigned int pos) {
	if (_currentPattern) {
		_currentPattern = _first;
		for (int i = 0; i < pos; i++) {
			_currentPattern = _currentPattern->nextPattern;
			if (_currentPattern == NULL) {
				return -2;
			}
		}

		return runPattern(_currentPattern->patternFunc);
	} else {
		return -1;
	}
}

int OSSex::getPattern() {
	if (_currentPattern) {
		int pos = 0;
		for (volatile patternList *stepper = _first; stepper != _currentPattern; stepper = stepper->nextPattern) {
			if (stepper == NULL) {
				return -2;
			}
			pos++;
		}
		return pos;
	} else {
		return -1;
	}
}

void OSSex::setPowerScale(float step) {
	_powerScaleStep = step;
}

void OSSex::setScale(float step) {
	setPowerScale(step);
}

float OSSex::increasePower() {
	_powerScale *= (1.0 + _powerScaleStep);
	return _powerScale;
}

float OSSex::decreasePower() {
	_powerScale *= (1.0 - _powerScaleStep);
	return _powerScale;
}

void OSSex::setTimeScale(float step) {
	_timeScaleStep = step;
}

float OSSex::increaseTime() {
	_timeScale *= (1.0 + _timeScaleStep);
	return _timeScale;
}

float OSSex::decreaseTime() {
	_timeScale *= (1.0 - _timeScaleStep);
	return _timeScale;
}

int OSSex::cyclePattern() {
    if (_currentPattern) {
		if (_currentPattern->nextPattern == NULL) {
			_currentPattern = _first;
		} else {
			_currentPattern = _currentPattern->nextPattern;
		}
		runPattern(_currentPattern->patternFunc);
		return 1;
	} else {
		return -1;
	}
}

int OSSex::addPattern(int* (*patternFunc)(int)) {
	if (_first == NULL) {
		_first = new struct patternList;
		if (!_first) {
			return -1;
		}
		_first->patternFunc = patternFunc;
		_first->nextPattern = NULL;
		_currentPattern = _first;
		return 1;
	} else {
		volatile patternList *iterator = _first;
		while (iterator->nextPattern != NULL) {
			iterator = iterator->nextPattern;
		}
		iterator->nextPattern = new struct patternList;
		if (!iterator->nextPattern) {
			return -1;
		}
		iterator = iterator->nextPattern;
		iterator->patternFunc = patternFunc;
		iterator->nextPattern = NULL;
		_currentPattern = iterator;
		return 1;
	}
}

// stop all the motors and patterns, reset to beginning. this could be better written.
void OSSex::stop() {
	_running = false;
	_powerScale = 1.0;
	_timeScale = 1.0;
	_seq = 0;
	setOutput(-1, 0);
	_patternCallback = NULL;
	volatile pattern* current = _memQueue[0];
	pattern* future = current->nextStep;
	while (current != NULL) {
		free((void *)current);
		current = future;
		future = future->nextStep;
	}
	_memQueue[0] = _memQueue[1] = NULL;
}

/***
*INPUT SECTION
*The part of the library dealing with accessing sensory inputs for the Device
***/

/*** SetupInput ***/
//Setting up the input basically establishes a new clean buffer
void OSSex::setupInput(int inNumber, int sampleSize) {

//setting a constant int lets us specify the array size
const int numReadings = sampleSize;
int readings[numReadings];      // the readings from the analog input

int inputPin = device.inPins[inNumber];
//Initialize the whole Array to zero
for (int thisReading = 0; thisReading < numReadings; thisReading++){
    readings[thisReading] = 0;    }
	
	//Set the Buffer on the input to the new fresh one we just created
		device.inputs[inNumber].buffer = readings;
	return;
}

/*** Updates Input ***/
// Read and update specified input channel, 
//add values to that input's buffer
//Returns raw value from 0-1023 //Potentially rename this to update input
int OSSex::getInput(int inNumber) {
	inNumber = abs(inNumber) % device.inCount;
	return analogRead(device.inPins[inNumber]);
	
}

//New version with Rolling input (will be folded into basic input in the future)
//Adapted from Arduino smoothing example (mellis igoe)
int OSSex::updateInput(int inNumber) {

int index = device.inputs[inNumber].index;                  // the index of the current reading
int total = device.inputs[inNumber].total;                  // the running total
int average = device.inputs[inNumber].avg;                // the average
int readings[] = device.inputs[inNumber].buffer[] 			//Copy the buffer over //Might be better to just have pointers
	

// subtract the last reading:
  total = total - readings[index];         
  // read from the sensor:  
  readings[index] = analogRead(inputPin); 
  // store this value as the current RAw value
   device.inputs[inNumber].rawValue = readings[index];
   //Store the scaled version of this value to the scaledValue var
    device.inputs[inNumber].scaledValue = map(device.inputs[inNumber].rawValue, 
		device.inputs[inNumber].min,device.inputs[inNumber].max,0,255);
  
  // add the reading to the total:
  total = total + readings[index];       
  // advance to the next position in the array:  
  index = index + 1;                    

  // if we're at the end of the array...
  if (index >= numReadings)              
    // ...wrap around to the beginning: 
    index = 0;                           

  // calculate the average:
  average = total / numReadings;  

//Calculate if we need to update the bufferMax and bufferMin

   if (readings[index] > device.inputs[inNumber].bufferMax) {
      device.inputs[inNumber].bufferMax = readings[index];
    }

    // record the minimum sensor value
    if (readings[index] < device.inputs[inNumber].bufferMin) {
      device.inputs[inNumber].bufferMin = readings[index];
    }
  
  
  
  // update the Structure
	
	device.inputs[inNumber].index=index;                  // the index of the current reading
device.inputs[inNumber].total =total;                  // the running total
device.inputs[inNumber].avg=average;                // the average
device.inputs[inNumber].buffer[]=readings[]; 			//Copy the buffer over //Might be better to just have pointers
	
	//Add other handy values
device.inputs[inNumber].diffAVG = abs(average-readings[index]);
device.inputs[inNumber].diffThresh = abs( device.inputs[inNumber].customThreshold-readings[index] );
 
	return analogRead(device.inPins[inNumber]);
}

/*** Calibration ***/
//Adapted from Arduino example (Mellis, Igoe)
//specify sensor number and duration of calibration
void OSSex::calibrateInput(int inNumber, int duration) {
const int sensorPin = device.inPins[inNumber];    // pin that the sensor is attached to

	int sensorValue = 0;         // the sensor value
int sensorMin = 1023;        // minimum sensor value
int sensorMax = 0;           // maximum sensor value
int total = 0;
int loopcounter=0;
	
	// calibrate during n milliseconds 
  while (millis() < duration) {
    sensorValue = analogRead(sensorPin);

	
	   // record the maximum sensor value
    if (sensorValue > sensorMax) {
      sensorMax = sensorValue;
    }

    // record the minimum sensor value
    if (sensorValue < sensorMin) {
      sensorMin = sensorValue;
    }
  
	
	
    // record the maximum sensor value
   if (readings[index] > device.inputs[inNumber].bufferMax) {
      device.inputs[inNumber].bufferMax = readings[index];
    }

    // record the minimum sensor value
    if (readings[index] < device.inputs[inNumber].bufferMin) {
      device.inputs[inNumber].bufferMin = readings[index];
    }
	total=total+sensorValue;
	loopcounter++;
  }
  
  //asssign values back to the STRUCT
  
  device.inputs[inNumber].min = sensorMin;
  device.inputs[inNumber].max = sensorMax;
  device.inputs[inNumber].avg = total/loopcounter;
  
  
  
  //Just for the hell of it, return the currently read value mapped to the calibration
     sensorValue = analogRead(sensorPin);
   sensorValue = map(sensorValue, sensorMin, sensorMax, 0, 255);
    // in case the sensor value is outside the range seen during calibration
  sensorValue = constrain(sensorValue, 0, 255);
  
  //Record a timestamp for when this calibration took place
    device.inputs[inNumber].lastCal = millis();

  
	return sensorValue ;
}

void ResizeArray(int **orig, int size) {
    int *resized = new int[size * 2]; 
    for (int i = 0; i < size; i ++)
        resized[i] = (*orig)[i];
    delete [] *orig;
    *orig = resized;
}

/***
*Built in Buttons and Click Handlers
*
***/
void OSSex::attachClick(void (*callback)()) {
	device.buttons[0].button.attachClick(callback);
}

void OSSex::attachDoubleClick(void (*callback)()) {
	device.buttons[0].button.attachDoubleClick(callback);
}

void OSSex::attachLongPressStart(void (*callback)()) {
	device.buttons[0].button.attachLongPressStart(callback);
}

void OSSex::attachLongPressStop(void (*callback)()) {
	device.buttons[0].button.attachLongPressStop(callback);
}

void OSSex::attachDuringLongPress(void (*callback)()) {
	device.buttons[0].button.attachDuringLongPress(callback);
}



