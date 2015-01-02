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
		device.outPins[0] = 5; 
		device.outPins[1] = 10; 
		device.outPins[2] = 11;
		
		device.deviceId = 1;
		
		device.ledCount = 1;
		device.ledPins[0] = 13;

		// Technically 4, but 2 inputs remain unconnected in most models
		device.inCount = 2;
		device.inPins[0] = A8; // D+
		device.inPins[1] = A7; // D-

		device.muxPins[0] = 8;
		device.muxPins[1] = 12;
		pinMode(device.muxPins[0], OUTPUT);
		pinMode(device.muxPins[1], OUTPUT);

	} else {
		// Lilypad USB  / Alpha model
		device.outCount = 3;
		device.outPins[0] = 3; 
		device.outPins[1] = 9; 
		device.outPins[2] = 10;
		
		device.deviceId = 0;
		
		device.ledCount = 1;
		device.ledPins[0] = 13;

		device.inCount = 2;
		device.inPins[0] = A2; // D+
		device.inPins[1] = A3; // D-
	}
	device.bothWays = false;

	device.isLedMultiColor = false;

	device.buttons[0].pin = 2;
	device.buttons[0].button.setPin(device.buttons[0].pin);
	device.buttons[0].button.setActiveLow(true);	
    
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

#define HACKER_PORT_AIN 0
#define HACKER_PORT_PWM 0
#define HACKER_PORT_I2C 1
#define HACKER_PORT_SERIAL 2

int OSSex::setHackerPort(unsigned int flag) {
	if (device.deviceId < 1) {
		return -1;
	}
	switch (flag) {
		case HACKER_PORT_AIN:
			digitalWrite(device.muxPins[0], LOW);
			digitalWrite(device.muxPins[1], LOW);
			break;
		case HACKER_PORT_I2C:
			digitalWrite(device.muxPins[0], HIGH);
			digitalWrite(device.muxPins[1], LOW);
			break;
		case HACKER_PORT_SERIAL:
			digitalWrite(device.muxPins[0], LOW);
			digitalWrite(device.muxPins[1], HIGH);
			break;
		default:
			return -1;

	}
	return 0;

}

// Read input channel
int OSSex::getInput(int inNumber) {
	inNumber = abs(inNumber) % device.inCount;
	return analogRead(device.inPins[inNumber]);
}

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



