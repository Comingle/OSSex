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

Below is a table of how example `outNumber` arguments would work in a 4-output (`dev.outCount` == 4, and outputs number 0, 1, 2, and 3) device:

| outNumber | Actual Output |
|:---------:|:-------------:|
| -2        | 2             |
| -1        | All outputs   |
| 0         | 0             |
| 1         | 1             |
| 2         | 2             |
| 3         | 3             |
| 4         | 0             |
| 5         | 1             |
| 6         | 2             |
...

`powerLevel`s are constrained to a value from 0 to 255 inclusive. Some devices may have outputs that are capable of negative `powerLevel`s, such as bidirectional motors. These devices have the `bothWays` flag set to `true` and will have `powerLevel` constrained to values between -255 and 255 inclusive. A `powerLevel` of 0 will turn the output off.

`setOutput` returns 1 currently.

## Set an LED to a particular power level
```arduino
dev.setLED(int ledNumber, int powerLevel);
```

`setLED` sets a given `ledNumber` to a given `powerLevel`. This function does not yet support the **-1** shorthand to apply `powerLevel` to all LEDs. It constrains `powerLevel` to be from 0 to 255 inclusive, with `powerLevel` of 0 turning the LED off.

`setLED` returns 1 currently.

## Run an output pattern
```arduino
dev.runPattern(int* pattern, unsigned int patternLength);
```

`runPattern` allows you to set a sequence of outputs at given power levels for given time durations. `pattern` is a two-dimensional array `p[m][3]`, where `m` is the number of steps in the sequence, and each step is a 3-element array consisting of:
```
{outNumber, powerLevel, timeInMillis}
```

`patternLength` is `m`.

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

`runPattern` uses the internal TimerX interrupts available on a variety of Atmel processors. As such, it can run a pattern a single time (such as the example above), or it can run multiple times by being placed in a loop or in `loop()`. `runPattern` will not return until the pattern is completed.
