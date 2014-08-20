/* Comingle.cpp v0.3 - Library for controlling Arduino-based sex-toys
 * Written by Craig Durkin/Comingle, May 9, 2014
 * {♥} COMINGLE
*/


#include <Arduino.h>
#include <Comingle.h>
#include <avr/interrupt.h> 
#include <avr/io.h>
#include "OneButton.h"

// Set up the interrupt to trigger the update() function.
#if defined(__AVR_ATmega328P__) // Uno
ISR(TIMER2_OVF_vect) {
	Device.update();
};
#elif defined(__AVR_ATmega32U4__) // Lilypad USB
ISR(TIMER4_OVF_vect) {
	Device.update();
};
#endif

// Pre-instantiate with empty constructor. Pre-instantiation is necessary for the timer2/timer4 interrupt to work. Empty constructor because
// the user defines which device they're using elsewhere (in setID());
Comingle Device = Comingle();
Comingle::Comingle() {
}

// the real constructor. give it a device ID and it will set up your device's pins and timers.
void Comingle::setID(int deviceId) {
#if defined(__AVR_ATmega328P__)
	_timer_start_mask = &TCCR2B;
	_timer_count = &TCNT2;
    _timer_interrupt_flag = &TIFR2;
    _timer_interrupt_mask_b = &TIMSK2;
    _timer_interrupt_mask_a = &TCCR2A;
    _timer_init = TIMER2_INIT;
#elif defined(__AVR_ATmega32U4__)
	_timer_start_mask = &TCCR4B;
	_timer_count = &TCNT4;
    _timer_interrupt_flag = &TIFR4;
    _timer_interrupt_mask_b = &TIMSK4;
    _timer_init = TIMER4_INIT;
#endif

	if (deviceId == 1) {
		// Arduino UNO
		device.outCount = 6;
		device.outPins[0] = 3; 
		device.outPins[1] = 5; 
		device.outPins[2] = 6;
		device.outPins[3] = 9;
		device.outPins[4] = 10;
		device.outPins[5] = 11;
		
		device.deviceId = 1;
		
		device.ledCount = 6;
		device.ledPins[0] = 2;
		device.ledPins[1] = 4;
		device.ledPins[2] = 7;
		device.ledPins[3] = 8;
		device.ledPins[4] = 12;
		device.ledPins[5] = 13;
	} else {
		// Lilypad USB  
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
		pinMode(2, INPUT_PULLUP);
	}
	device.bothWays = false;

	device.inCount = 3;
	device.inPins[0] = A2;
	device.inPins[1] = A3;
	device.inPins[2] = A4;

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
	if (device.deviceId == 1) {
		*_timer_interrupt_mask_b = 0x01;
		*_timer_interrupt_mask_a = 0x00;
 	} else {
		*_timer_interrupt_mask_b = 0x04;    // Timer INT Reg: Timer Overflow Interrupt Enable: 00000100   
 	}
  	_tickCount = 0;
 	*_timer_count = _timer_init;			// Reset Timer Count
 	*_timer_interrupt_flag = 0x00;			// Timer INT Flag Reg: Clear Timer Overflow Flag
 	*_timer_start_mask = 0x05;				// Timer PWM disable, prescale / 16: 00000101

 	_scale = 1.0;
 	_scaleStep = 0.1;

}


// Called by the timer interrupt to check if a change needs to be made to the pattern or update the button status;
void Comingle::update() {
	device.buttons[0].button.tick();

	if (_running) {
		_tickCount++;
		if (_tickCount > _currentStep->duration) {
	  		if (_currentStep->nextStep == NULL) { 
	  			// stop the pattern if at last step
	    		_running = false;
	  		} else {
	  			// run the next step
	  			_currentStep = _currentStep->nextStep;
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
int Comingle::setOutput(int outNumber, int powerLevel) {
	int iterations = 1, scaledPower;
	// set all outputs, starting at 0.
	if (outNumber == -1) {
		iterations = device.outCount;
		outNumber = 0;
	} else {
		outNumber = abs(outNumber) % device.outCount;
	}

	if (device.bothWays) {
		scaledPower = constrain(powerLevel, -255, 255);
	} else {
		scaledPower = constrain(powerLevel, 0, 255);
	}

	if (_scale * scaledPower > 255) {
		_scale = 255/scaledPower;
	}
	
	for (int i = 0; i < iterations; i++) {
		if (scaledPower == 0) {
			analogWrite(device.outPins[outNumber], 0);
			if (device.bothWays) {
				analogWrite(device.tuoPins[outNumber], 0);
			}
		} else if (scaledPower > 0) {
			analogWrite(device.outPins[outNumber], scaledPower * _scale);
		} else {
			analogWrite(device.tuoPins[outNumber], scaledPower * _scale);
		}
		outNumber = i+1;
	}

	return 1;

}


// Turn an LED on or off. lightLevel can be a value from 0-255. 0 turns the LED off.
// Accept html color codes (both "#50a6c2" and "midnight blue"?)
// Add serial (Stream object) feedback from function for diagnostics
//void Comingle::setLED(unsigned int lightLevel, ledNumber, colorCode) {}
int Comingle::setLED(int ledNumber, int powerLevel) {
	int scaledPower;
	if (!device.ledCount) {
		return -1;
	}
	// sanitize ledNumber XXX -1 logic
	ledNumber %= device.ledCount;
	scaledPower = constrain(powerLevel, 0, 255);
	analogWrite(device.ledPins[ledNumber], scaledPower);

	return 1;

}

// Run preset pattern from an array of {outputNumber, powerLevel, duration} steps
// This function will not return until the pattern is finished running.
int Comingle::runPattern(int* patSteps, size_t patternLength) {
	_singlePattern = new struct pattern;
	_singlePattern->nextStep = NULL;
    pattern* patIndex = _singlePattern;
    pattern* nextStep;

    if (patternLength) {
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
int Comingle::runPattern(int* (*callback)(int)) {
	setOutput(-1,0);
	_seq = 0;

	// get the first two steps of the sequence. 
	// some patterns with short first steps won't run otherwise as there isn't time to queue the 2nd step
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
			return -1;
		}
		_currentStep = _singlePattern;
		setOutput(_currentStep->outNumber, _currentStep->powerLevel);
		_running = true;
		return 1;
	} else {
		return -1;
	}

}

void Comingle::setScale(float step) {
	_scaleStep = step;
}

void Comingle::increasePower() {
	_scale *= (1.0 + _scaleStep);
}

void Comingle::decreasePower() {
	_scale *= (1.0 - _scaleStep);
}

int Comingle::cyclePattern() {
	_running = false;
	_scale = 1.0;

	if (_currentPattern->nextPattern == NULL) {
		_currentPattern = _first;
	} else {
		_currentPattern = _currentPattern->nextPattern;
	}
	free((void*)_memQueue[0]);
	free((void*)_memQueue[1]);
	_memQueue[0] = _memQueue[1] = NULL;
	runPattern(_currentPattern->patternFunc);
	return 1;
}

int Comingle::addPattern(int* (*patternFunc)(int)) {
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

// Read input channel
int Comingle::getInput(int inNumber) {
	inNumber = abs(inNumber) % device.inCount;
	return analogRead(device.inPins[inNumber]);
}

void Comingle::attachClick(void (*callback)()) {
	device.buttons[0].button.attachClick(callback);
}

void Comingle::attachDoubleClick(void (*callback)()) {
	device.buttons[0].button.attachDoubleClick(callback);
}

void Comingle::attachLongPressStart(void (*callback)()) {
	device.buttons[0].button.attachLongPressStart(callback);
}

void Comingle::attachLongPressStop(void (*callback)()) {
	device.buttons[0].button.attachLongPressStop(callback);
}

void Comingle::attachDuringLongPress(void (*callback)()) {
	device.buttons[0].button.attachDuringLongPress(callback);
}



