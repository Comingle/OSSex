#include <Comingle.h>

Comingle dev;
int pattern[][3] = {
    {-1, 201, 2000},
    {-1, 0, 2000},
    {-1, 203, 2000},
    {-1, 0, 2000},
    {0, 200, 2000},
    {1, 200, 2000},
    {2, 200, 2000},
    {3, 200, 2000},
    {4, 200, 2000},
    {5, 200, 2000},
    {0, 0, 2000},
    {1, 0, 2000},
    {2, 0, 2000},
    {3, 0, 2000},
    {4, 0, 2000},
    {5, 0, 2000},
  };
unsigned int patternSize = sizeof(pattern) / sizeof(int) / 3;

void setup() {
  int ret;
  ret = dev.runPattern(*pattern, patternSize);
}

void loop() {
}
