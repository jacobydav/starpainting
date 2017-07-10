//This code is used to test creating the star twinkle effect
//by varying the LED intensity.

//Circuit:
//PWM pin -> 270 Ohm resistor -> LED -> Ground
//Potentiometer
//5V->potentiometer
//Wipter->Analog 0
//Gnd->potentiometer

byte pwm1Pin = 9;
byte pwm2Pin = 9;
byte maxInten1;
byte minInten1;
byte fadeStep1;
byte currInten1;
byte currDir1;    //Fading in or fading out

byte potValIn = 0; //analog 0 used to read value of potentiometer
int minScaleVal = 100;
int maxScaleVal = 1000;
int delayValMin = 1;
int delayValMax = 100;

void setup() 
{
  Serial.begin(9600);
  pinMode(pwm1Pin, OUTPUT);
  
  maxInten1 =random(0,255);
  minInten1 = random(0,maxInten);
  fadeStep1 = 10;
  currInten1 = minInten1;
}

void loop() 
{
  long delayValIn = analogRead(potValIn);  
  if(delayValIn<minScaleVal)
    delayValIn=minScaleVal;
  if(delayValIn>maxScaleVal)
    delayValIn = maxScaleVal;
   // Serial.print("delayValIn: ");
 // Serial.println(delayValIn);
    
  //Scale the value
  int delayValScaled = delayValMin + ((delayValIn-minScaleVal)*(delayValMax-delayValMin))/(float)(maxScaleVal-minScaleVal);
  //Serial.print("delayValScaled: ");
 //Serial.println(delayValScaled);
  
  //fade in
    
  for(int i=minInten;i<maxInten;i+=fadeStep)
  {
    analogWrite(pwm1Pin, i);
    delay(delayValScaled);
  }
  
  //fade out  
  for(int i=maxInten;i<minInten;i-=fadeStep)
  {
    analogWrite(pwm1Pin, i);
    delay(delayValScaled);
  }
  
}
