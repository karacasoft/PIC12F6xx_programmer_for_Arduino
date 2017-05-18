#include "PIC12F6XXProg.h"

#include <Arduino.h>

/* Constructor */
PIC12F6XXProg::PIC12F6XXProg(uint8_t dataPin, uint8_t clockPin) {
	this->dataPin = dataPin;
	this->clockPin = clockPin;
	this->programmerFlags = 0x00;
}

PIC12F6XXProg::~PIC12F6XXProg() {

}


/**
 * Sets up the pins, flags and programmer side PC counter.
 * Must be called after applying high voltage to the MCLR pin.
 * 
 */
void PIC12F6XXProg::begin() {
	pinMode(this->dataPin, OUTPUT);
	pinMode(this->clockPin, OUTPUT);

	digitalWrite(this->dataPin, LOW);
	digitalWrite(this->clockPin, LOW);
	this->programmerFlags &= (FLAG_INPUT_MODE ^ 0xFF);
	this->pcCurrent = 0x00;
}

/**
 * Writes a program into the program memory from current location 
 * until the end of the program. Uses a 16-bit unsigned integer array
 * as the program bits. Takes the least significant 14 bits of every
 * integer. When the integer with value 0xFFFF is encountered. The
 * programming is terminated.
 * @return
 * The number of bytes written on the program memory, or -1 if
 * a verification error occurs.
 */
int16_t PIC12F6XXProg::writeProgramMemory(uint16_t *program) {

	this->bulkEraseProgramMemory();
	bool programming = true;
	int i = 0;
	while(programming)
	{
		if(program[i] == 0xFFFF) {
			programming = false;
			break;
		}
		this->loadProgramData(program[i]);
		this->beginProgramming();
		delay(20);
		uint16_t data = this->readProgramData();
		if(data != program[i]) {
			return -1;
		}

		this->incrementAddress();
		i++;
		
	}
	return i;
	
}


/* Mid level routines */

/**
 * Sends the command to increment the PC by 1. Increments the internal
 * PC counter of this class, too.
 */
void PIC12F6XXProg::incrementAddress() {
	this->writeCommand(0x06);
	this->pcCurrent++;
	delayMicroseconds(3);
}

/**
 * Loads a word to the program memory.
 *
 * beginProgramming should be called after using this.
 */
void PIC12F6XXProg::loadProgramData(uint16_t data) {
	this->writeCommand(0x02);
	data = data << 1;
	delayMicroseconds(3);
	this->write(data);
	delayMicroseconds(3);
}

/**
 * Writes the loaded word/data into the memory.
 * 
 */
void PIC12F6XXProg::beginProgramming() {
	this->writeCommand(0x08);
	delay(20);
}

/**
 * Completely erases the program memory.
 *
 */
void PIC12F6XXProg::bulkEraseProgramMemory() {
	this->writeCommand(0x09);
	delay(10);
}

/**
 * Reads the word that is pointed by the PC on program memory.
 *
 */
uint16_t PIC12F6XXProg::readProgramData() {
	this->writeCommand(0x04);
	delayMicroseconds(3);
	return this->read();
}

/**
 * Load configuration command.
 *
 */
void PIC12F6XXProg::loadConfig(uint16_t config) {
	this->writeCommand(0x00);
	delayMicroseconds(3);
	config = config << 1;
	this->write(config);
	delayMicroseconds(3);
}

/**
 * Reads the word that is pointed by the PC on data memory.
 * 
 */
uint8_t PIC12F6XXProg::readData() {
	this->writeCommand(0x05);
	delayMicroseconds(3);
	return (uint8_t)(this->read() | 0x00FF);
}




/* Low level routines */
/**
 * Checks if the dataPin is currently in input mode.
 * Switches it to input mode if it is not.
 */
void PIC12F6XXProg::ensureInputMode() {
	if((programmerFlags & FLAG_INPUT_MODE) == 0x00) {
		pinMode(this->dataPin, INPUT);
		programmerFlags = programmerFlags ^ FLAG_INPUT_MODE;
	}
}

/**
 * Checks if the dataPin is currently in output mode.
 * Switches it to output mode if it is not.
 */
void PIC12F6XXProg::ensureOutputMode() {
	if((programmerFlags & FLAG_INPUT_MODE) == 0x01) {
		pinMode(this->dataPin, OUTPUT);
		programmerFlags = programmerFlags ^ FLAG_INPUT_MODE;
	}
}

/**
 * Writes the low 6 bits of the given 8-bit integer.
 */
void PIC12F6XXProg::writeCommand(uint8_t command) {
	int index = 0;
	while(index < 6) {
		if((command & 0x01) == 0x00) {
			this->write0();
		} else {
			this->write1();
		}
		command = command >> 1;
		index++;
	}
}

/**
 * Writes a full 16-bit integer in 16 cycles.
 *
 */
void PIC12F6XXProg::write(uint16_t data) {
	int index = 0;
	while(index < 16) {
		if((data & 0x01) == 0x00) {
			this->write0();
		} else {
			this->write1();
		}
		data = data >> 1;
		index++;
	}
}

/**
 * Writes a full 8-bit integer in 8 cycles.
 *
 */
void PIC12F6XXProg::write(uint8_t data) {
	int index = 0;
	while(index < 8) {
		if((data & 0x01) == 0x00) {
			this->write0();
		} else {
			this->write1();
		}
		data = data >> 1;
		index++;
	}
}

/**
 * Reads a full 16-bit integer. shifts it by one to get rid of the start 
 * and end bit.
 *
 */
uint16_t PIC12F6XXProg::read() {
	this->ensureInputMode();
	uint16_t data = 0x00;
	for (int i = 0; i < 16; ++i)
	{
		digitalWrite(this->clockPin, LOW);
		delayMicroseconds(3);
		if(digitalRead(this->dataPin) == HIGH) {
			data |= 0x8000;
		}
		data = data >> 1;
		digitalWrite(this->clockPin, HIGH);
		delayMicroseconds(3);
	}
	digitalWrite(this->clockPin, LOW);

	return data >> 1;
}


/**
 * Writes one bit with value 1.
 *
 */
void PIC12F6XXProg::write1() {
	this->ensureOutputMode();
	digitalWrite(this->clockPin, HIGH);
	delayMicroseconds(3);
	digitalWrite(this->dataPin, HIGH);
	delayMicroseconds(3);
	digitalWrite(this->clockPin, LOW);
	delayMicroseconds(3);
}

/**
 * Writes one bit with value 0.
 *
 */
void PIC12F6XXProg::write0() {
	this->ensureOutputMode();
	digitalWrite(this->clockPin, HIGH);
	delayMicroseconds(3);
	digitalWrite(this->dataPin, LOW);
	delayMicroseconds(3);
	digitalWrite(this->clockPin, LOW);
	delayMicroseconds(3);
}