/*
This code controls the LEDs for the star painting.

Required libraries include Adafruit_TinyFlash
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
*/
#include <Adafruit_TinyFlash.h>

//Pins begin
//Note: If using Pin #1 for button input, remember that this pin
//      is connected to the red LED. So connect 5V to one side of a button
//      and connect the other side to Pin #1. The value of Pin #1 will be High when
//      the button is pressed.
//#define startBtnPin 1
//#define motionBtnPin 4
//Pins end

//Global variables begin
#define NUM_LEDS 12
//The LED intensity will vary randomly. It will fluctuate around
//the value set in pwmMeanVal.
uint16_t pwmMeanVal[NUM_LEDS];    //Mean value (0-65536)
uint16_t pwmCurrVal[NUM_LEDS];    //Current value (0-65536)
//Max value for this LED. This is used to keep all of the LEDs
//at the same general brightness.
uint16_t pwmMax[NUM_LEDS];
//Min value for this LED. This is used to keep all of the LEDs
//at the same general brightness.
uint16_t pwmMin[NUM_LEDS];

uint16_t pwmInc[NUM_LEDS];
//The new pwm value. If it is much larger than the current value, then
//it will be reached in increments.
uint16_t pwmFinalVal[NUM_LEDS];

bool pwmChangeInProgress[NUM_LEDS];

uint8_t pwmChangeAddOrSub[NUM_LEDS];

Adafruit_TinyFlash flash;
//Global variables end

void setup() {    
  // put your setup code here, to run once:
  //Initialize values that are unique to each LED
  //LED 0
  pwmMeanVal[0] = 35000;
  pwmMax[0]=60000;
  pwmMin[0]=5000;
  pwmInc[0]=2000;
  //LED 1
  pwmMeanVal[1] = 35000;
  pwmMax[1]=60000;
  pwmMin[1]=5000;
  pwmInc[1]=2000;
  //LED 2
  pwmMeanVal[2] = 35000;
  pwmMax[2]=60000;
  pwmMin[2]=5000;
  pwmInc[2]=2000;
  //LED 3
  pwmMeanVal[3] = 35000;
  pwmMax[3]=60000;
  pwmMin[3]=5000;
  pwmInc[3]=2000;
  //LED 4
  pwmMeanVal[4] = 35000;
  pwmMax[4]=60000;
  pwmMin[4]=5000;
  pwmInc[4]=2000;
  //LED 5
  pwmMeanVal[5] = 35000;  
  pwmMax[5]=60000;
  pwmMin[5]=5000;
  pwmInc[5]=2000;
  //LED 6
  pwmMeanVal[6] = 35000;  
  pwmMax[6]=60000;
  pwmMin[6]=5000;
  pwmInc[6]=2000;
  //LED 7
  pwmMeanVal[7] = 35000;  
  pwmMax[7]=60000;
  pwmMin[7]=5000;
  pwmInc[7]=2000;
  //LED 8
  pwmMeanVal[8] = 35000;  
  pwmMax[8]=60000;
  pwmMin[8]=5000;
  pwmInc[8]=2000;
  //LED 9
  pwmMeanVal[9] = 35000;  
  pwmMax[9]=60000;
  pwmMin[9]=5000;
  pwmInc[9]=2000;
  //LED 10
  pwmMeanVal[10] = 35000;  
  pwmMax[10]=60000;
  pwmMin[10]=5000;
  pwmInc[10]=2000;
  //LED 11
  pwmMeanVal[11] = 35000;  
  pwmMax[11]=60000;
  pwmMin[11]=5000;
  pwmInc[11]=2000;
  //Initialize values that are the same for all LEDs
  for(int i=0;i<NUM_LEDS;i++)
  {
    pwmCurrVal[i] = 1000;
    pwmChangeAddOrSub[i]=1;
    pwmChangeInProgress[i]=false;
    pwmFinalVal[i]=0;
  }
  
  uint32_t bytes;
  if(!(bytes = flash.begin())) 
  {     // Flash init error?
    for(int i=0;i<20;i++)
    {
      digitalWrite(1, LOW);
      delay(250);
      digitalWrite(1, HIGH);
      delay(250);      
    }
  }
}

void loop() 
{
//  digitalWrite(1, LOW);
//  delay(1000);
//  digitalWrite(1, HIGH);
//  delay(1000);
//  digitalWrite(1, LOW);
//  delay(1000);
  
  //uint8_t currColor = random(1,4);
//  if(pwmDir[0]==0)
//    pwmCurrVal[0]=pwmVal[0]+pwmInc[0];
//  else if(pwmDir[0]==1)
//    pwmVal[0]=pwmVal[0]-pwmInc[0];
//    
//  if(pwmVal[0]>pwmMax[0])
//  {
//     pwmVal[0] = pwmMax[0];
//     pwmDir[0]=1;
//  }
//  if(pwmVal[0]<pwmMin[0])
//  {
//     pwmVal[0] = pwmMin[0];
//     pwmDir[0]=0; 
//  }
  
  //Loop through each each LED
  for(int currLED=0;currLED<NUM_LEDS;currLED++)
  {
    if(pwmChangeInProgress[currLED]==false)
    {
      //add or subtract a value from the mean
      //Find out if we should add or subtract or not change
      //1=add, 2=subtract, 3=nothing
      pwmChangeAddOrSub[currLED] = random(1,3);
      uint16_t valRange;
      uint16_t valChange; 
      switch(pwmChangeAddOrSub[currLED])
      {
        case 1:
          valRange = pwmMax[currLED]-pwmMeanVal[currLED];
          valChange = random(1,valRange);
          pwmFinalVal[currLED] = pwmCurrVal[currLED] + valChange;
          //Begin updating the value. If the new pwm value is large, then we will incrementally
          //reach the value.
          if(valChange>pwmInc[currLED])
          {
            pwmCurrVal[currLED] = pwmCurrVal[currLED] + pwmInc[currLED];
            pwmChangeInProgress[currLED]=true;
          }
          else
            pwmCurrVal[currLED] = pwmCurrVal[currLED] + valChange;
        break;
        case 2:
          valRange = pwmMeanVal[currLED]-pwmMin[currLED];
          valChange = random(1,valRange);
          pwmFinalVal[currLED] = pwmCurrVal[currLED] - valChange;
          //Begin updating the value. If the new pwm value is large, then we will incrementally
          //reach the value.
          if(valChange>pwmInc[currLED])
          {
            pwmCurrVal[currLED] = pwmCurrVal[currLED] - pwmInc[currLED];
            pwmChangeInProgress[currLED]=true;
          }
          else
            pwmCurrVal[currLED] = pwmCurrVal[currLED] - valChange;
        break;
        default:
          //Do nothing
          break;
      }
    }
    else
    {
      switch(pwmChangeAddOrSub[currLED])
      {
        case 1:
          pwmCurrVal[currLED] = pwmCurrVal[currLED] + pwmInc[currLED];
          if(pwmCurrVal[currLED]>pwmFinalVal[currLED])
          {
            pwmCurrVal[currLED] = pwmFinalVal[currLED];
            pwmChangeInProgress[currLED]=false;
          }
        break;
        case 2:
          pwmCurrVal[currLED] = pwmCurrVal[currLED] - pwmInc[currLED];
          if(pwmCurrVal[currLED]<pwmFinalVal[currLED])
          {
            pwmCurrVal[currLED] = pwmFinalVal[currLED];
            pwmChangeInProgress[currLED]=false;
          }
        break;
        default:
          //Do nothing
          break;
      }
    }
    
    flash.setPWM(currLED,pwmCurrVal[currLED]);
  }
  flash.write();
  
  delay(200);
    
}

