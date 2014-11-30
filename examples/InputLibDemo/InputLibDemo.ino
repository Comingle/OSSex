#include <OneButton.h>
#include <OSSex.h>

/* Demos all the Input Stuff to Serial
 * Example by Craig Durkin/Comingle, May 9, 2014
 * {♥} COMINGLE
*/

//#include "C:\Users\Digital Naturalism\Documents\GitHub\OSSex\OSSex.h"

void setup() {
  Toy.setID(0);
  Toy.addPattern(fadeCos);
  Toy.addPattern(pulse);
  Toy.attachClick(click);  

  Serial.begin(9600);
  Toy.setupInput(0,10);
    Toy.setupInput(1,10);
Toy.calibrateInput(0, 2100); 
//Toy.calibrateInput(1, 3000); 

for(int p = 0; p<Toy.device._defaultInputBufferSize;p++){
 Serial.print(Toy.device.inputs[0].buffer[p]);
 Serial.print(",");
}
Serial.println(" fullbuffer");
}

void loop() {
  Toy.updateInput(0);
  Toy.updateInput(1);
  
      Serial.print("index ");
  Serial.print(Toy.device.inputs[0].index);     //Note, we should address the confusion of how to address inputs 1 and 2
  Serial.print(" | ");
  
    Serial.print("indexval ");
  Serial.print(Toy.device.inputs[0].buffer[Toy.device.inputs[0].index]);     //Note, we should address the confusion of how to address inputs 1 and 2
  Serial.print(" | ");
 
    Serial.print("oldread ");
  Serial.print(Toy.getInput(0));
  Serial.print(" | ");
 
 
  Serial.print("rawval ");
  Serial.print(Toy.device.inputs[0].rawValue);     //Note, we should address the confusion of how to address inputs 1 and 2
  Serial.print(" | ");
  
  Serial.print("bufferMin ");
  Serial.print(Toy.device.inputs[0].bufferMin);   
  Serial.print(" | ");
  
    Serial.print("bufferMax ");
  Serial.print(Toy.device.inputs[0].bufferMax);   
  Serial.print(" | ");
  
    Serial.print("bufferAVG ");
  Serial.print(Toy.device.inputs[0].bufferAVG);   
  Serial.print(" | ");
  
      Serial.print("buffertotal ");
  Serial.print(Toy.device.inputs[0].buffertotal);   
  Serial.print(" | ");
  
    Serial.print("calAVG ");
  Serial.print(Toy.device.inputs[0].avg);   
  Serial.print(" | ");
  
     Serial.print("calmin ");
  Serial.print(Toy.device.inputs[0].min);   
  Serial.print(" | ");
  
     Serial.print("calmax ");
  Serial.print(Toy.device.inputs[0].max);   
  Serial.print(" | ");
  
  
    Serial.println();

  delay(10);
}

void click() {
  Toy.cyclePattern();
}

int step[3];
// fade all of the motors from low to high over and over
int *fadeCos(int seq) {
  step[0] = -1;
  step[2] = 50;
  step[1] = round(127 * cos((seq / (8*PI))-PI) + 127);
  return step;
}

// Randomly blip an output on for a short burst.
int* pulse(int seq) {
  if (seq % 2) {
    step[0] = -1;
    step[1] = 0;
  } else {
    step[0] = random(0,3);
    step[1] = 144;
  }
  step[2] = 70;
  return step;
}
