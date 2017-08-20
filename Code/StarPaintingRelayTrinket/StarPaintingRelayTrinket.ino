//StarPaintingTrinket
//Turn on the power relay when it is dark and the motion
//sensor detects motion.

//To upload to your Gemma or Trinket:
//1) Select the proper board from the Tools->Board Menu
//2) Select USBtinyISP from the Tools->Programmer
//3) Plug in the Gemma/Trinket, make sure you see the green LED lit
//4) For windows, install the USBtiny drivers
//5) Press the button on the Gemma/Trinket - verify you see
//the red LED pulse. This means it is ready to receive data
//6) Click the upload button above within 10 seconds

int motionSensorPin = 0; // PIR sensor, digital input
int relayPin = 1; // Activate the relay that provides power to the star painting
int analogIn1Pin = 1; // Light sensor, analog input

int lightSensorVal;  //analog reading from light sensor
int lightSensorThresh=333;

int loopDelay_ms = 100;
long onTime_ms = 300000;    //painting will stay on for 300 seconds after motion is detected
long currOnTime_ms;        //current time since last motion was detected

// the setup routine runs once when you press reset:
void setup() {
// initialize the digital pin as an output.
pinMode(motionSensorPin, INPUT);
pinMode(relayPin, OUTPUT);
}
// the loop routine runs over and over again forever:
void loop() 
{
  //Read the light sensor value
  //Use analogRead(1) for analog input connected to GPIO 2.
  //It is confusing because it is GPIO 2 on the Trinket.
  lightSensorVal = analogRead(1);  
  
//  int blinkCount = lightSensorVal/10;
//  for(int i=0;i<blinkCount;i++)
//  {
//    digitalWrite(relayPin, HIGH);
//    delay(250);
//    digitalWrite(relayPin, LOW);
//    delay(250);
//  }
    
  if(lightSensorVal>lightSensorThresh)
  {
    //If it is dark, check for motion
    int motionSensorVal = digitalRead(motionSensorPin);
    if(motionSensorVal==HIGH)
    {
      currOnTime_ms=0;    //restart the timer
      digitalWrite(relayPin, HIGH);
    }
    
    currOnTime_ms = currOnTime_ms+loopDelay_ms;
    
    //Check if the timer has expired
    if(currOnTime_ms>onTime_ms)
    {
      digitalWrite(relayPin, LOW);
    }
  }
  else
  {
    //If it is not dark, turn off the painting
    digitalWrite(relayPin, LOW);
  }
    
  delay(loopDelay_ms);
}

