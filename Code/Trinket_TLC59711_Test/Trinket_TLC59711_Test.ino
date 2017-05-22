/*
Test code for the Trinket_TLC_LED library which
allows a Adafruit Trinket to communicate with a TLC59711 breakout board.

Required libraries include homemade Trinket_TLC_LED which was
adapted from the Adafruit_TinyFlash library.
Required hardware includes an Adafruit Trinket microcontroller
(3.3Volt version), a TLC59711 breakout board.
As written, this is specifically for the Trinket; would need
modifications on other boards (Arduino Uno, etc.).
Trinket :   
3.3V   Gnd   Pin #0   Pin #2
TLC59711 board:      
Vcc   Gnd    DI        CI  
V+ of TLC59711 board to an external power supply (5-17 Volts)
Connect ground of external power supply and Trinket.

Select the 8Mhz Trinket as the board and the USBtinyISP as the programmer.
*/
#include <Trinket_TLC_LED.h>

//Pins begin
//Note: Pins do not have to be defined. They are hard coded in the library.
//Pins end

//Global variables begin
#define NUM_LEDS 12
uint16_t pwmCurrVal[NUM_LEDS];    //Current value (0-65536)
uint8_t currLED;
uint16_t currVal;
Trinket_TLC_LED tlc_LED_driver;
//Global variables end

void setup() {    
  // put your setup code here, to run once:
  currLED=0;
  currVal=0;
  //Initialize values that are the same for all LEDs
  for(int i=0;i<NUM_LEDS;i++)
  {
    pwmCurrVal[i] = 0;
  }
  
  uint32_t bytes;
  if(!(bytes = tlc_LED_driver.begin())) 
  {     
    // tlc_LED_driver init error
    // This will also fail if the board is not a Trinket.
    // blink the onboard red LED to alert the user
    for(int i=0;i<20;i++)
    {
      digitalWrite(1, LOW);
      delay(250);
      digitalWrite(1, HIGH);
      delay(250);      
    }
  }
}

//Fade in 1 LED at a time
void loop() 
{  
  //Loop through each each LED and initialize to 0
  for(int i=0;i<NUM_LEDS;i++)
    tlc_LED_driver.setPWM(i,0);
  //Set the current value for the current LED we are changing
  tlc_LED_driver.setPWM(currLED,currVal);
    
  tlc_LED_driver.write();
  
  currVal += 5000;
  if(currVal>50000)
    currLED++;
  if(currLED>=NUM_LEDS)
    currLED=0;
  
  delay(200);
    
}

