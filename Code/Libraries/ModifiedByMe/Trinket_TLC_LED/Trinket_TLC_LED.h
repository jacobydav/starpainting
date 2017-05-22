#ifndef _TRINKETLEDDRIVER_H_
#define _TRINKETLEDDRIVER_H_

#include <Arduino.h>

class Trinket_TLC_LED {
 public:

  Trinket_TLC_LED(uint8_t cs = 3);

  uint32_t          begin(void);
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

  uint8_t           cs_mask;
  //PWM stuff begin
  uint16_t *pwmbuffer;
  int8_t numdrivers;    //How many TLC59711 boards are chained together. Currently only 1 board is supported. The code may work with more, but I have not tested it.
  int8_t numLEDChans;   //How many LED channels. Currently only the TLC59711 is supported so this will be 12.
  int8_t _clk;
  int8_t _dat;
  uint8_t BCr, BCg, BCb;
  //PWM stuff end
};

#endif // _TRINKETLEDDRIVER_H_
