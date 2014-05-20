# comingle-arduino

This is a basic arduino library for interacting with Comingle devices. 


* [Bootstrap](http://twitter.github.com/bootstrap/) 2.0.4+
* [jQuery](http://jquery.com/) 1.7.1+

These are the specific versions bootstrap-datepicker is tested against (`js` files) and built against (`css` files).  Use other versions at your own risk.

# Available functions

## Constructor
```
Comingle dev;
```

## Set an output to a particular power level.
```
dev.setOutput(outNumber, powerLevel);
```

**-1** can be given as an **outNumber** as shorthand for "apply **powerLevel** to all outputs". Otherwise, the output will be set according to the following formula: 
```
outNumber = abs(outNumber) % dev.outCount;
```

Below is a table of how example **outNumber** arguments would work in a 4-output (**outCount** == 4, and outputs number 0, 1, 2, and 3) device:

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

**powerLevel**s are constrained to a value from 0 to 255 inclusive. Some devices may have outputs that are capable of negative **powerlevel**s, such as bidirectional motors. These devices have the **bothWays** flag set to **true** and will have **powerLevel** constrained to values between -255 and 255 inclusive. A **powerLevel** of 0 will turn the output off.


