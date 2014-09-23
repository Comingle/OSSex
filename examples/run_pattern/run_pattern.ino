/* Runs a simple pattern
 * Example by Craig Durkin/Comingle, May 9, 2014
 * {♥} COMINGLE
*/

#include <OSSex.h>

int pattern[][3] = {
    {-1, 200, 1000},
    {-1, 0, 1000},
};
unsigned int patternSize = sizeof(pattern) / sizeof(int) / 3;

void setup() {
  Toy.setID(0);
}

void loop() {
  Toy.runShortPattern(*pattern, patternSize);
}
