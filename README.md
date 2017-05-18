# PIC12F6xx_programmer_for_Arduino
Incomplete, but working, PIC12F6xx arduino programmer

# Introduction
I recently get my hands on a PIC12F683 microprocessor. And I decided to use my Arduino as a programmer for this. I implemented the read/write operations for only the program memory side.

# Usage
Here's some code:
```
#include "PIC12F6XXProg.h"

uint16_t program[] = {
  0x2805, // GOTO 0x05
  0x0000, 0x0000, 0x0000, 0x0000, // NOP x 4
  0x1683, // BSF STATUS, RP0
  0x1005, // BCF GPIO, 0
  0x019F,
  0x1283,
  0x1419,
  
  0x1D05,
  0x1405,
  0x1905,
  0x1005,
  0x280A,
  0xFFFF // programmer terminator, will not be written to the processor
};

PIC12F6XXProg programmer(2, 3);

void setup() {
  Serial.begin(9600);
  programmer.begin();

  Serial.println("Writing in 3 seconds!! Put high voltage on the MCLR pin...");
  delay(3000);
  Serial.println("Programming...");
  delay(10);
  programmer.writeProgramMemory(program);
  
  Serial.println("Programming completed!");
  pinMode(2, INPUT);
  pinMode(3, INPUT);
}

void loop() {}
```
The program loaded here writes HIGH to GP0 if the GP2 is given HIGH, or LOW otherwise.

Refer to the datasheet of your microprocessor for the complete instruction set. 
