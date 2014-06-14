/* Comingle.cpp v0.1.2 - Library for controlling Arduino-based sex-toys
 * Written by Craig Durkin/Comingle, May 9, 2014
 * {♥} COMINGLE
*/


#include <Arduino.h>
#include <Comingle.h>
#include <avr/interrupt.h> 
#include <avr/io.h>

#if defined(__AVR_ATmega328P__) // Uno
Comingle ComingleDevice(1);
ISR(TIMER2_OVF_vect) {
	ComingleDevice.checkPattern();
};
#elif defined(__AVR_ATmega32U4__) // Lilypad USB
Comingle ComingleDevice(0);
ISR(TIMER4_OVF_vect) {
	ComingleDevice.checkPattern();
};
#endif

Comingle::Comingle(int deviceId) {
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
		_device.outCount = 6;
		_device.outPins[0] = 3; 
		_device.outPins[1] = 5; 
		_device.outPins[2] = 6;
		_device.outPins[3] = 9;
		_device.outPins[4] = 10;
		_device.outPins[5] = 11;
		
		_device.deviceId = 1;
		
		_device.ledCount = 6;
		_device.ledPins[0] = 2;
		_device.ledPins[1] = 4;
		_device.ledPins[2] = 7;
		_device.ledPins[3] = 8;
		_device.ledPins[4] = 12;
		_device.ledPins[5] = 13;
	} else {
		// Lilypad USB  
		_device.outCount = 4;
		_device.outPins[0] = 3; 
		_device.outPins[1] = 9; 
		_device.outPins[2] = 10;
		_device.outPins[3] = 11;
		
		_device.deviceId = 0;
		
		_device.ledCount = 1;
		_device.ledPins[0] = 13;

		_device.buttonPins[0] = 2;
		pinMode(_device.buttonPins[0], INPUT_PULLUP);
	}
	_device.bothWays = false;

	_device.inCount = 4;
	_device.inPins[0] = A2;
	_device.inPins[1] = A3;
	_device.inPins[2] = A4;
	_device.inPins[3] = A5;

	_device.isLedMultiColor = false;
	
	*_timer_start_mask = 0x00;
  	_tickCount = 0;
    
	for (int i = 0; i < _device.outCount; i++) {
		pinMode(_device.outPins[i], OUTPUT);
		if (_device.bothWays) {
			pinMode(_device.tuoPins[i], OUTPUT);
		}
	}
	for (int i = 0; i < _device.inCount; i++) {
		pinMode(_device.inPins[i], INPUT);
	}
	for (int i = 0; i < _device.ledCount; i++) {
		pinMode(_device.ledPins[i], OUTPUT);
	}

}


// Called by the timer interrupt to check if a change needs to be made to the pattern.
void Comingle::checkPattern() {
	_tickCount++;
	if (_tickCount > _singlePattern[_i][2]) {
  		if (_i == _singlePatternLength) { 
  			// stop the pattern
    		_i = 0;
    		*_timer_start_mask = 0x00;
  		} else {
  			// run the next step
  			_i++;
  			setOutput(_singlePattern[_i][0], _singlePattern[_i][1]);
    		
  		}
  		_tickCount = 0;            //Resets the interrupt counter
	}
		         
  *_timer_count = _timer_init;		//Reset Timer after interrupt triggered
  *_timer_interrupt_flag = 0x00;	//Timer INT Flag Reg: Clear Timer Overflow Flag
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
		iterations = _device.outCount;
		outNumber = 0;
	} else {
		outNumber = abs(outNumber) % _device.outCount;
	}

	if (_device.bothWays) {
		scaledPower = constrain(powerLevel, -255, 255);
	} else {
		scaledPower = constrain(powerLevel, 0, 255);
	}
	
	for (int i = 0; i < iterations; i++) {
		if (scaledPower == 0) {
			analogWrite(_device.outPins[outNumber], 0);
			if (_device.bothWays) {
				analogWrite(_device.tuoPins[outNumber], 0);
			}
		} else if (scaledPower > 0) {
			analogWrite(_device.outPins[outNumber], scaledPower);
		} else {
			analogWrite(_device.tuoPins[outNumber], scaledPower);
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

	if (!_device.ledCount) {
		return -1;
	}
	// sanitize ledNumber XXX -1 logic
	ledNumber %= _device.ledCount;

	scaledPower = constrain(powerLevel, 0, 255);

	analogWrite(_device.ledPins[ledNumber], scaledPower);

	return 1;

}

// Run preset pattern
// XXX Add serial (Stream object) feedback from function for diagnostics
int Comingle::runPattern(int* pattern, unsigned int patternLength) {
	patternLength = constrain(patternLength, 0, (_max_pattern_steps-1));
	for (int i = 0; i < patternLength; i++) {
		for (int j = 0; j < 3; j++) {
			ComingleDevice._singlePattern[i][j] = *(pattern++);
		}
	}
	ComingleDevice._singlePatternLength = patternLength;
	ComingleDevice._device = _device;
	
	_i = 0;
	// Thanks for Noah at arduinomega.blogspot.com for clarifying this
	if (_device.deviceId == 1) {
		*_timer_interrupt_mask_b = 0x01;
		*_timer_interrupt_mask_a = 0x00;
 	} else {
		*_timer_interrupt_mask_b = 0x04;    // Timer INT Reg: Timer2 Overflow Interrupt Enable: 00000100   
 	}
 	*_timer_count = _timer_init;			// Reset Timer Count
 	*_timer_interrupt_flag = 0x00;			// Timer INT Flag Reg: Clear Timer Overflow Flag
 	*_timer_start_mask = 0x05;				// Timer PWM disable, prescale / 16: 00000101
 	
	setOutput(ComingleDevice._singlePattern[_i][0], ComingleDevice._singlePattern[_i][1]); // Run the first step

 	while (*_timer_start_mask) {			// Wait until pattern is finished to return
	}
	
	return 1;
}

// Define motor/led pattern
// ? Possible implementation: 
// Function takes 4 arguments: first is a pattern number to assign. can be null for one-off patterns.
// second is an MxN matrix of motor steps, where M is number of motors, N is time steps, and value is motor power level
// third is a pattern time: 2000 milliseconds and 5 columns in the matrix means each step runs for 0.4 seconds. 1 2 3 4 5 
// fourth is loop boolean -- run pattern once or run it looping
//void Comingle::setPattern(int patternNumber, int motorSeq[][], unsigned int seqTime, bool loopSeq) {}
// can't really alter the timing of each step
// 
void Comingle::setPattern(unsigned int patternNumber, int* pattern) {}


// Read input channel
int Comingle::getInput(int inNumber) {
	inNumber = abs(inNumber) % _device.inCount;
	return analogRead(_device.inPins[inNumber]);
}

int Comingle::flicker(int powerLevel, unsigned int stepTime, unsigned int totalTime) {
	int pattern[_max_pattern_steps][3];
	int *send;
	int timeAccumulator = 0;
	int i;

	for (i = 0; i < _max_pattern_steps; i+2) {
		pattern[i][0] = random(100, _device.outCount);
		pattern[i][1] = powerLevel;
		if (totalTime - timeAccumulator < stepTime) {
			pattern[i][2] = totalTime - timeAccumulator;
			break;
		} else {
			pattern[i][2] = random(100, stepTime);
			timeAccumulator += pattern[i][2];
		}
		pattern[i+1][0] = pattern[i][0];
		pattern[i+1][1] = 0;
		pattern[i+1][2] = 50;
		timeAccumulator += 50;
	}
	send = &pattern[0][0];
	return runPattern(send, i);
}

void Comingle::oscillate() {}


