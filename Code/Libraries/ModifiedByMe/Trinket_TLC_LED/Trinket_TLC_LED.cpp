// Library for Adafruit Trinket to communicate with Adafruit 12-Channel 16-bit PWM LED Driver - SPI Interface - TLC59711.
// The library is a modification of the Adafruit TinyFlash library written by Limor Fried and Phillip Burgess.

//I will include this because it was in the TinyFlash library: MIT license

#include "Trinket_TLC_LED.h"

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

// Constructor
Trinket_TLC_LED::Trinket_TLC_LED(uint8_t cs) {
	numdrivers = 1;	//This code has only been tested with 1 TLC59711. Change this value if you chain multiple boards together.
  numLEDChans = 12;   //For the TLC59711
	_clk = 2;	//PWM
	_dat = 0;	//PWM
	BCr = BCg = BCb = 0x7F;	//PWM
	pwmbuffer = (uint16_t *)calloc(2, 12*numdrivers);	//PWM

	cs_mask = digitalPinToBitMask(cs);
}

// Select chip and issue command (don't deselect; data may follow)
void Trinket_TLC_LED::cmd(uint8_t c) {
	CHIP_SELECT
	(void)spi_xfer(c);
}

// Initialize SPI pins, validate chip is present
uint32_t Trinket_TLC_LED::begin(void) {
	uint8_t manID, devID;

  //Check that we are using an ATtiny
  #ifndef __AVR_ATtiny85__
  return 0;
  #endif

	PORTB &= ~(_BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2) | cs_mask);
	DDRB  &= ~_BV(PORTB0); // DI (NOT MISO)
	DDRB  |=  _BV(PORTB1)  // DO (NOT MOSI)
              |   _BV(PORTB2)  // SCK
              |   cs_mask;     // CS

	pinMode(_clk, OUTPUT);	//PWM
    pinMode(_dat, OUTPUT);	//PWM

	return 1;
}

// Poll status register until busy flag is clear or timeout occurs
boolean Trinket_TLC_LED::waitForReady(uint32_t timeout) {
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

// Private function used by write and erase operations
boolean Trinket_TLC_LED::writeEnable(void) {
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
void Trinket_TLC_LED::writeDisable(void) {
	cmd(CMD_WRITEDISABLE);
	CHIP_DESELECT
}

void  Trinket_TLC_LED::spiwriteMSB(uint32_t d) 
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

//Writes the updated PWM values to the TLC59711
//I don't remember what examples, libraries, etc I used as a reference
//to modify this to work with the TLC59711. Sorry.
void Trinket_TLC_LED::write(void) {
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
  for (uint8_t n=0; n<numdrivers; n++) 
  {
    spiwriteMSB(command >> 24);
    spiwriteMSB(command >> 16);
    spiwriteMSB(command >> 8);
    spiwriteMSB(command);

    // 12 channels per TLC59711
    for (int8_t c=numLEDChans-1; c >= 0 ; c--) 
    {
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

//Set the value for a PWM channel.
//The new values will not be used until write() is called.
void Trinket_TLC_LED::setPWM(uint8_t chan, uint16_t pwm) {
  if (chan<0 || chan > numLEDChans*numdrivers) 
    return;

  pwmbuffer[chan] = pwm;  
}