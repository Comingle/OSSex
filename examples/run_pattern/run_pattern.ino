#include <Comingle.h>

Comingle dev(1);
int pattern[][3] = {
    {-1, 200, 2000},
    {-1, 0, 2000},
    {-1, 200, 2000},
    {-1, 0, 2000}
  };
unsigned int patternSize = sizeof(pattern) / sizeof(int) / 3;

void setup() {
  int ret;
  ret = dev.runPattern(*pattern, patternSize);
}

void loop() {
}
