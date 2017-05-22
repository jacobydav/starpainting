// Barebones library for Winbond W25Q80BV serial flash memory.  Written
// with the limited space of the ATtiny85 in mind, but works on regular
// Arduinos too.  Provides basic functions needed for audio playback
// system only: chip and sector erase, block write, sequential byte read.
// Other than possibly adding support for other Winbond flash in the
// future, the plan is to NOT bloat this out with all bells and whistles;
// functions like block reads or buffered writes can be implemented in
// client code where RAM can be better managed in the context of the
// overall application (1 flash page = 1/2 the ATtiny85's RAM).
// Written by Limor Fried and Phillip Burgess for Adafruit Industries.
// MIT license.

#include "Adafruit_TinyFlash.h"

#define CMD_PAGEPROG     0x02
#define CMD_READDATA     0x03
#define CMD_WRITEDISABLE 0x04
#define CMD_READSTAT1    0x05
#define CMD_WRITEENABLE  0x06
#define CMD_SECTORERASE  0x20
#define CMD_CHIPERASE    0x60
#define CMD_ID           0x90

#define STAT_BUSY        0x01
#define STAT_WRTEN       0x02

// Currently rigged for W25Q80BV only
#define CHIP_BYTES       1L * 1024L * 1024L

#ifdef __AVR_ATtiny85__

#define CHIP_SELECT   PORTB &= ~cs_mask;
#define CHIP_DESELECT PORTB |=  cs_mask;
#define SPIBIT                      \
  USICR = ((1<<USIWM0)|(1<<USITC)); \
  USICR = ((1<<USIWM0)|(1<<USITC)|(1<<USICLK));
static uint8_t spi_xfer(uint8_t n) {
	USIDR = n;
	SPIBIT
	SPIBIT
	SPIBIT
	SPIBIT
	SPIBIT
	SPIBIT
	SPIBIT
	SPIBIT
	return USIDR;
}

#else

#include <SPI.h>
#define CHIP_SELECT   *cs_port &= ~cs_mask;
#define CHIP_DESELECT *cs_port |=  cs_mask;
#define spi_xfer(n)   SPI.transfer(n)

#endif

// Constructor
Adafruit_TinyFlash::Adafruit_TinyFlash(uint8_t cs) {
	numdrivers = 1;	//PWM
	_clk = 2;	//PWM
	_dat = 0;	//PWM
	BCr = BCg = BCb = 0x7F;	//PWM
	pwmbuffer = (uint16_t *)calloc(2, 12*numdrivers);	//PWM
#ifndef __AVR_ATtiny85__
	cs_port = portOutputRegister(digitalPinToPort(cs));
#endif
	cs_mask = digitalPinToBitMask(cs);
}

// Select chip and issue command (don't deselect; data may follow)
void Adafruit_TinyFlash::cmd(uint8_t c) {
	CHIP_SELECT
	(void)spi_xfer(c);
}

// Initialize SPI pins, validate chip is present
uint32_t Adafruit_TinyFlash::begin(void) {
	uint8_t manID, devID;

#ifdef __AVR_ATtiny85__
	PORTB &= ~(_BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2) | cs_mask);
	DDRB  &= ~_BV(PORTB0); // DI (NOT MISO)
	DDRB  |=  _BV(PORTB1)  // DO (NOT MOSI)
              |   _BV(PORTB2)  // SCK
              |   cs_mask;     // CS
#else
	SPI.begin();
	// Resistor-based 5V->3.3V logic conversion is a little sloppy, so:
	SPI.setClockDivider(SPI_CLOCK_DIV8); // 500 KHz
#endif

	//cmd(CMD_ID);
	//for(uint8_t i=0; i<4; i++) manID = spi_xfer(0); // 3 dummy bytes first
	//devID = spi_xfer(0);
	//CHIP_DESELECT

	// Chip capacity is hardcoded for now
	//return ((manID == 0xEF) && (devID == 0x13)) ? CHIP_BYTES : 0L;

	pinMode(_clk, OUTPUT);	//PWM
    pinMode(_dat, OUTPUT);	//PWM

	return 1;
}

// Poll status register until busy flag is clear or timeout occurs
boolean Adafruit_TinyFlash::waitForReady(uint32_t timeout) {
	uint8_t  status;
	uint32_t startTime = millis();

	do {
		cmd(CMD_READSTAT1);
		status = spi_xfer(0);
		CHIP_DESELECT
		if((millis() - startTime) > timeout) return false;
	} while(status & STAT_BUSY);

	return true;
}

// Set up a read operation (no data is returned yet)
boolean Adafruit_TinyFlash::beginRead(uint32_t addr) {

	if((addr >= CHIP_BYTES) || !waitForReady()) return false;

	cmd(CMD_READDATA);
	(void)spi_xfer(addr >> 16);
	(void)spi_xfer(addr >>  8);
	(void)spi_xfer(addr      );
	// Chip is held in selected state until endRead()

	return true;
}

// Read next byte (call N times following beginRead())
uint8_t Adafruit_TinyFlash::readNextByte(void) {
	return spi_xfer(0);
}

// Stop read operation
void Adafruit_TinyFlash::endRead(void) {
	CHIP_DESELECT
}

// Erase the whole chip.  Boom, gone.  Use with caution.
boolean Adafruit_TinyFlash::eraseChip(void) {

	if(!waitForReady() || !writeEnable()) return false;

	// Might want to have this clear the block protect bits

	cmd(CMD_CHIPERASE);
	CHIP_DESELECT

	if(!waitForReady(10000L)) return false; // Datasheet says 6S max

	writeDisable();

	return true;
}

// Erase one 4K sector
boolean Adafruit_TinyFlash::eraseSector(uint32_t addr) {

	if(!waitForReady() || !writeEnable()) return false;

	cmd(CMD_SECTORERASE);
	(void)spi_xfer(addr >> 16); // Chip rounds this down to
	(void)spi_xfer(addr >>  8); // prior 4K sector boundary;
	(void)spi_xfer(0         ); // lowest bits are ignored.
	CHIP_DESELECT

	if(!waitForReady(1000L)) return false; // Datasheet says 400ms max

	writeDisable();

	return true;
}

// Private function used by write and erase operations
boolean Adafruit_TinyFlash::writeEnable(void) {
	uint8_t status;

	cmd(CMD_WRITEENABLE);
	CHIP_DESELECT

	// Verify write-enable status
	cmd(CMD_READSTAT1);
	status = spi_xfer(0);
	CHIP_DESELECT
	return (status & STAT_WRTEN) ? true : false;
}

// Companion to above function, used after write and erase operations
void Adafruit_TinyFlash::writeDisable(void) {
	cmd(CMD_WRITEDISABLE);
	CHIP_DESELECT
}

// Writes one page: 256 bytes, starting at 256 byte boundary.  There are
// no other options.  This is the ONLY write method provided by the library;
// other capabilities (if needed) may be implemented in client code.
boolean Adafruit_TinyFlash::writePage(uint32_t addr, uint8_t *data) {
	if((addr >= CHIP_BYTES) || !waitForReady() || !writeEnable())
		return false;

	cmd(CMD_PAGEPROG);
	(void)spi_xfer(addr >> 16);
	(void)spi_xfer(addr >>  8);
	(void)spi_xfer(0); // If len=256, page boundary only (datasheet 7.2.21)
	for(int i=0; i<256; i++) {
		(void)spi_xfer(data[i]);
	}
	CHIP_DESELECT // Write occurs after the CS line is de-asserted

	delay(3);     // Max page program time according to datasheet

	if(!waitForReady()) return false;

	writeDisable();

	return true;
}

void  Adafruit_TinyFlash::spiwriteMSB(uint32_t d) 
{
  if (_clk >= 0) {
    uint32_t b = 0x80;
    //  b <<= (bits-1);
    for (; b!=0; b>>=1) {
      digitalWrite(_clk, LOW);
      if (d & b)  
	digitalWrite(_dat, HIGH);
      else
	digitalWrite(_dat, LOW);
      digitalWrite(_clk, HIGH);
    }
  } else {
    spi_xfer(d);//SPI.transfer(d);
  }
}

void Adafruit_TinyFlash::write(void) {
  uint32_t command;

  // Magic word for write
  command = 0x25;

  command <<= 5;
  //OUTTMG = 1, EXTGCK = 0, TMGRST = 1, DSPRPT = 1, BLANK = 0 -> 0x16
  command |= 0x16;

  command <<= 7;
  command |= BCr;

  command <<= 7;
  command |= BCg;

  command <<= 7;
  command |= BCb;

  cli();
  for (uint8_t n=0; n<numdrivers; n++) {
    spiwriteMSB(command >> 24);
    spiwriteMSB(command >> 16);
    spiwriteMSB(command >> 8);
    spiwriteMSB(command);

    // 12 channels per TLC59711
    for (int8_t c=11; c >= 0 ; c--) {
      // 16 bits per channel, send MSB first
      spiwriteMSB(pwmbuffer[n*12+c]>>8);
      spiwriteMSB(pwmbuffer[n*12+c]);
    }
  }

  if (_clk >= 0)
    delayMicroseconds(200);
  else
    delayMicroseconds(2);
  sei();
}

void Adafruit_TinyFlash::setPWM(uint8_t chan, uint16_t pwm) {
  if (chan > 12*numdrivers) return;
  pwmbuffer[chan] = pwm;  
}