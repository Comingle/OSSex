/* Runs a simple Pattern
 * Example by Craig Durkin/Comingle, May 9, 2014
 * {♥} COMINGLE
*/

#include <Comingle.h>

Comingle dev(0);
int pattern[][3] = {
    {-1, 200, 1000},
    {-1, 0, 1000},
};
unsigned int patternSize = sizeof(pattern) / sizeof(int) / 3;

void setup() {
}

void loop() {
  int ret;
  ret = dev.runPattern(*pattern, patternSize);
}
