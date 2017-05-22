#ifndef _TINYFLASH_H_
#define _TINYFLASH_H_

#include <Arduino.h>

class Adafruit_TinyFlash {
 public:
#ifdef __AVR_ATtiny85__
  Adafruit_TinyFlash(uint8_t cs = 3);
#else
  Adafruit_TinyFlash(uint8_t cs = 10);
#endif
  uint32_t          begin(void);
  boolean           beginRead(uint32_t addr),
                    writePage(uint32_t addr, uint8_t *data),
                    eraseChip(void),
                    eraseSector(uint32_t addr);
  uint8_t           readNextByte(void);
  void              endRead(void);
  //PWM functions begin
  void setPWM(uint8_t chan, uint16_t pwm);
  void spiwriteMSB(uint32_t d);
  void write(void);
  //PWM functions end
 private:
  boolean           waitForReady(uint32_t timeout = 100L),
                    writeEnable(void);
  void              writeDisable(void),
                    cmd(uint8_t c);
#ifndef __AVR_ATtiny85__
  volatile uint8_t *cs_port;
#endif
  uint8_t           cs_mask;
  //PWM stuff begin
  uint16_t *pwmbuffer;
  int8_t numdrivers;
  int8_t _clk;
  int8_t _dat;
  uint8_t BCr, BCg, BCb;
  //PWM stuff end
};

#endif // _TINYFLASH_H_
