#include <stddef.h>
#include <stdint.h>

#define FLAG_INPUT_MODE 0x01

#define DEFAULT_CONFIG 0x3FFF

#define CONFIG_OSC_MODE_INTOSC1 0x3FFC
#define CONFIG_OSC_MODE_INTOSC2 0x3FFD
#define CONFIG_OSC_MODE_EC 0x3FFB

class PIC12F6XXProg
{
	uint8_t programmerFlags;
	uint8_t dataPin;
	uint8_t clockPin;


public:
	uint8_t pcCurrent;

	PIC12F6XXProg(uint8_t dataPin, uint8_t clockPin);
	~PIC12F6XXProg();
	
	void begin();

	int16_t writeProgramMemory(uint16_t *program);

	/* Mid level routines */
	void incrementAddress();
	void loadProgramData(uint16_t data);
	void beginProgramming();

	void bulkEraseProgramMemory();

	uint16_t readProgramData();
	uint8_t readData();

	void loadConfig(uint16_t config);

	/* Low level data pass interface */
	void ensureInputMode();
	void ensureOutputMode();
	void writeCommand(uint8_t command);
	void write(uint16_t data);
	void write(uint8_t data);
	uint16_t read();
	void write1();
	void write0();

};
