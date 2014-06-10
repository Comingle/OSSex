# comingle-arduino

This is an arduino library for interacting with Comingle devices. 

# Available functions

## Constructor
```arduino
#include <Comingle.h>

Comingle dev(int deviceId);
```

Create an instance of the Comingle class. If `deviceId` is 1, it sets up a device based on an Arduino Uno / Atmega328. Any other number will set up a device for a LilyPad USB / Atmega32U4.

## Set an output to a particular power level
```arduino
dev.setOutput(int outNumber, int powerLevel);
```

**-1** can be given as an `outNumber` as shorthand for "set all outputs to `powerLevel`". Otherwise, the output will be set according to the following formula: 
```arduino
outNumber = abs(outNumber) % dev.outCount;
```

Below is a table of how example `outNumber` arguments would work in a 3-output (`dev._device.outCount` == 3, and outputs number 0, 1, and 2) device:

| outNumber | Actual Output |
|:---------:|:-------------:|
| -2        | 2             |
| -1        | All outputs   |
| 0         | 0             |
| 1         | 1             |
| 2         | 2             |
| 3         | 0             |
| 4         | 1             |
| 5         | 2             |
...

`powerLevel`s are constrained to a value from 0 to 255 inclusive. Some devices may have outputs that are capable of negative `powerLevel`s, such as bidirectional motors. These devices have the `bothWays` flag set to `true` and will have `powerLevel` constrained to values between -255 and 255 inclusive. A `powerLevel` of 0 will turn the output off.

`setOutput()` returns 1 currently.

## Set an LED to a particular power level
```arduino
dev.setLED(int ledNumber, int powerLevel);
```

`setLED()` sets a given `ledNumber` to a given `powerLevel`. This function does not yet support the **-1** shorthand to apply `powerLevel` to all LEDs. It constrains `powerLevel` to be from 0 to 255 inclusive, with `powerLevel` of 0 turning the LED off.

`setLED()` returns 1 currently.

## Read an input
```arduino
dev.getInput(int inNumber);
```

`getInput()` is a wrapper for `analogRead()`. Since the inputs are often not broken out in numerical order, `getInput()` provides an easy way to "read input 1, now read input 2", etc. without having to remember the pin mappings of a given device.

## Run an output pattern
```arduino
dev.runPattern(int* pattern, unsigned int patternLength);
```

`runPattern()` allows you to set a sequence of outputs at given power levels for given time durations. `pattern` is a two-dimensional array `p[m][3]`, where `m` is the number of steps in the sequence, and each step is a 3-element array consisting of:
```
{outNumber, powerLevel, timeInMillis}
```

`patternLength` is equal to `m` and is constrained to 16 total steps (defined by `_max_pattern_steps` in Comingle.h)

Example:

```arduino
#include <Comingle.h>

Comingle dev;

void setup() {
  int pattern[][3] = {
    {-1, 200, 2000},
    {-1, 0, 2000},
    {-1, 200, 2000},
    {-1, 0, 2000}
  }
  unsigned int patternLength = sizeof(pattern) / sizeof(int) / 3;
  dev.runPattern(*pattern, patternLength);
}

void loop() {
}
```

This pattern will turn all outputs on to a power level of 200 for 2 seconds, turn them off for 2 seconds, turn them back on for 2 seconds, and then turn them off again (technically also for two seconds).

`runPattern()` uses the internal TimerX interrupts available on a variety of Atmel processors. As such, it can run a pattern a single time (such as the example above), or it can run multiple times by being placed in a loop or in `loop()`. `runPattern()` will not return until the pattern is completed.

## Get device info
```arduino
Serial.begin(9600);
Serial.print(dev._device.outCount);
...
```

Each instance of the `Comingle` class has an exposed `_device` struct that can be accessed to determine the device's capabilities. The struct's contents are currently:

```
struct device {
  bool bothWays;                    // can outputs go both forward and backward?
  uint8_t outCount;                 // number of outputs (electrodes, motors)
  uint8_t outPins[_max_outputs];    // array mapping to output pins
  uint8_t tuoPins[_max_outputs];    // array mapping to reverse output pins;
  bool isLedMultiColor;             // do we have multicolored LEDs?
  uint8_t ledCount;                 // number of LEDs
  uint8_t ledPins[_max_leds];       // array mapping to LED output pins
  uint8_t inCount;                  // number of input pins
  uint8_t inPins[_max_inputs];      // array mapping to input pins
  int deviceId;
} _device;
```

This struct is defined in Comingle.h, along with the `_max_outputs`, `_max_leds`, `_max_inputs` and a few other constants.

## Flicker the outputs
```arduino
dev.flicker(/int/ powerLevel, /unsigned int/ stepTime, unsigned int totalTime);
```

`flicker()` randomly turns outputs on and off. It will do this `totalTime` has elapsed. It will select a random output, turn it on for a time period of at most `stepTime`, turn it off, then select another output and repeat the process.

This function is currently untested.
