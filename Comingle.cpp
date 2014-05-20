/* Comingle.cpp v0.1 - Library for controlling Arduino-based sex-toys
 * Created by Craig Durkin, May 9, 2014
 * License?
*/

#include <Arduino.h>
#include <Comingle.h>
#include <avr/interrupt.h> 
#include <avr/io.h>

Comingle ComingleDevice;

ISR(TIMER2_OVF_vect) { // should be TIMER4
	ComingleDevice.checkPattern();
};

Comingle::Comingle() {
	
	// Tonga with LilyPad USB brains
	_device.outCount = 6;  // should be 4. testing.
	_device.outPins[0] = 3; // 3,6,10,11
	_device.outPins[1] = 5; // should be 9. testing
	_device.outPins[2] = 6;
	_device.outPins[3] = 9;
	_device.outPins[4] = 10;
	_device.outPins[5] = 11;
	_device.bothWays = false;
	_device.inputCount = 4;
	_device.inputPins[0] = A2;
	_device.inputPins[1] = A3;
	_device.inputPins[2] = A4;
	_device.inputPins[3] = A5;
	
	_device.ledCount = 6;
	_device.ledPins[0] = 2;
	_device.ledPins[1] = 4;
	_device.ledPins[2] = 7;
	_device.ledPins[3] = 8;
	_device.ledPins[4] = 12;
	_device.ledPins[5] = 13;

	//Disable Timer4 until necessary.
	//TCCR4B = 0x00;   

	TCCR2B = 0x00;    
  	_tickCount = 0;
    
	for (int i = 0; i < _device.outCount; i++) {
		pinMode(_device.outPins[i], OUTPUT);
	}
	for (int i = 0; i < _device.inputCount; i++) {
		pinMode(_device.inputPins[i], INPUT);
	}
	for (int i = 0; i < _device.ledCount; i++) {
		pinMode(_device.ledPins[i], OUTPUT);
	}

}

void Comingle::checkPattern() {
	_tickCount++;
	if (_tickCount > _singlePattern[_i][2]) {
		//Serial.print('o');
		//Serial.println(_singlePattern[_i][0]);
		// Serial.print('p');
		// Serial.println(_singlePattern[_i][1]);
		// Serial.print('T');
		// Serial.println(_singlePattern[_i][2]);
		// Serial.print('i');
		// Serial.println(_i);
		// Serial.print('t');
		// Serial.println(_tickCount);
  		if (_i == _singlePatternLength) { 
    		_i = 0;
    		TCCR2B = false; // last step, so turn timer off.
  		} else {
  			setOutput(_singlePattern[_i][0], _singlePattern[_i][1]);
    		_i++;
  		}
  		_tickCount = 0;            //Resets the interrupt counter
	}
		

	//TCNT4 = TIMER4_INIT;           //Reset Timer after interrupt triggered
	//TIFR4 = 0x00;                  //Timer4 INT Flag Reg: Clear Timer Overflow Flag
  TCNT2 = 131;
  TIFR2 = 0x00;
  

}


// Set an output to a particular power level. If outNumber is -1, set all outputs to powerLevel.
// outNumber of any other negative number or a number greater than or equal to the number of available outputs will be rolled over.
// Ex: in a 4 output device, you can access outputs 0, 1, 2, and 3.
// Specifying outNumber of -3 will map to output 3. Specifying an outNumber of 5 will map to output 1.
// powerLevel can be from 0..255 in devices that aren't bidirectional, and -255..255 in birdirectional devices. 
// Negative powerLevel values are coerced to 0 in devices that aren't bidirectional.
// powerLevel of 0 turns the output off. Values greater than +/-255 get coerced to +/-255.
// XXX Add serial (Stream object) feedback from function for diagnostics
// returns -1 if outNumber is higher than number of available outputs. Returns 0 otherwise.
int Comingle::setOutput(int outNumber, int powerLevel) {
	int iterations = 1, scaledPower;

	if (outNumber == -1) {
		// set all outputs, starting at 0.
		iterations = _device.outCount;
		outNumber = 0;
	} else {
		outNumber = abs(outNumber) % _device.outCount;
	}
	
	for (int i = 0; i < iterations; i++) {
		if (_device.bothWays) {
			scaledPower = constrain(powerLevel, -255, 255);
		} else {
			scaledPower = constrain(powerLevel, 0, 255);
		}

		if (powerLevel == 0) {
			analogWrite(_device.outPins[outNumber], 0);
			if (_device.bothWays) {
				analogWrite(_device.tuoPins[outNumber], 0);
			}
		} else if (powerLevel > 0) {
			analogWrite(_device.outPins[outNumber], scaledPower);
		} else {
			analogWrite(_device.tuoPins[outNumber], scaledPower);
		}
		outNumber = i+1;
	}

	return 1;

}


// Turn an LED on or off. lightLevel can be a value from 0-32. 0 turns the LED off.
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
// Add serial (Stream object) feedback from function for diagnostics
//void Comingle::runPattern(int patternNumber) {}
int Comingle::runPattern(int* pattern, unsigned int patternLength) {
	for (int i = 0; i < patternLength; i++) {
		for (int j = 0; j < 3; j++) {
			ComingleDevice._singlePattern[i][j] = *(pattern++);
		}
	}
	ComingleDevice._singlePatternLength = patternLength;
	ComingleDevice._device = _device;
	
	for (int i = 0; i < patternLength; i++) {
		Serial.print('i');
		Serial.println(i);
		Serial.print('0');
		Serial.println(ComingleDevice._singlePattern[i][0]);
		Serial.print('1');
		Serial.println(ComingleDevice._singlePattern[i][1]);
		Serial.print('2');
		Serial.println(ComingleDevice._singlePattern[i][2]);
	}


	// let it loose
	// Thanks for Noah at arduinomega.blogspot.com for clarifying this
	// http://www.pjrc.com/teensy/atmega32u4.pdf has a reference of what these registers mean
	// These settings work out to about a 1ms interrupt on a lilypad usb with an atmega32u4
	_i = 0;
	//TCNT4  = TIMER4_INIT; //Reset Timer Count to 25 out of 255
  	//TIFR4  = 0x00;        //Timer2 INT Flag Reg: Clear Timer Overflow Flag
  	//TIMSK4 = 0x04;        //Timer4 INT Reg: Timer2 Overflow Interrupt Enable: 00000100
  	//TCCR4B = 0x05;        //Timer4 PWM4x disable, prescale / 16: 00000101
	 TCNT2 = 131;
	 TCCR2A = 0x00;
	 TIMSK2 = 0x01;
	 TIFR2 = 0x00;
	 TCCR2B = 0x05;

	while (TCCR2B) {
	}

	return 1;
}

// Define motor/led pattern
// ? Possible implementation: 
// Function takes 4 arguments: first is a pattern number to assign. can be null for one-off patterns.
// second is an MxN matrix of motor steps, where M is number of motors, N is time steps, and value is motor power level
// third is a pattern time: 2000 milliseconds and 5 columns in the matrix means each step runs for 0.4 seconds. 1 2 3 4 5 
// fourth is loop boolean -- run pattern once or run it looping
//void Comingle::definePattern(int patternNumber, int motorSeq[][], unsigned int seqTime, bool loopSeq) {}
// can't really alter the timing of each step
// 
void Comingle::definePattern(unsigned int patternNumber, int* pattern) {}



// Read input channel
// void Comingle::readInput(int channelNumber) {}
void Comingle::readInput() {}

// return basic info about device (output counts/pins)
void Comingle::deviceInfo() {}

void Comingle::fade() {}

void Comingle::oscillate() {}