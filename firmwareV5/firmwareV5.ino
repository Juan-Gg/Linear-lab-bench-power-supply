/**********************************************************
  LINEAR LAB POWER SUPPLY SOFTWARE VERSION 5.0 - MAIN (1/4)
  First released to the public on MAY 2019
  Written by JuanGg on DECEMBER 2018-APRIL 2019
  https://juangg-projects.blogspot.com/
This work is licensed under GNU General Public License v3.0
***********************************************************/

//CONFIGURATION
#define MODE 1 //0: master, 1:slave
#define SERIAL 


//SERIAL COMMS
#ifdef SERIAL
#include <SoftwareSerial.h>
SoftwareSerial serialLink(A5,A4); //A2, A3 for Rev B
boolean receiving = false;
char recChar;
#define startMarker '<'
#define endMarker '>'
#define dataSize 20
int dataIndex = 0;
char receivedData[dataSize];
boolean newData = false;
unsigned long prevSerialMillis;
#endif

//ANALOG & CALIBRATION
#define vMeas A0
#define cMeas A1
#define ccMeas A2 //A6 for Rev B
#define tMeas A3 //A7 for Rev B
#define vSet 10
#define cSet 9

#if MODE == 0 //Master constants
const float vMeasCal = 0.0288;
const float vMeasOffset = 0.00;
const float cMeasCal = 0.50;
const float cMeasOffset = 0;
const float vSetCal = 140;
const float vSetOffset = -0.08;
const float cSetCal = 6.75;
const float cSetOffset = 0;
#else //Slave constants
const float vMeasCal = 0.0278;
const float vMeasOffset = 0;
const float cMeasCal = 0.483;
const float cMeasOffset = -3;
const float vSetCal = 145;
const float vSetOffset = 0.02;
const float cSetCal = 7;
const float cSetOffset = 0;
#endif


float progVoltage = 0;
float progCurrent = 100;

#define maxVoltage 25 //V
#define voltageRes 0.05 

#define maxCurrent 500 //mA
#define currentRes 1

boolean constantCurrent;

float measVoltageSum;
float measCurrentSum;

float avgMeasVoltage;
float avgMeasCurrent;
const byte numMeasAverages = 20;
int updateRate = 10;
unsigned long prevAdqMillis = 0;


//MISC
#define fan 13
#define fanOnTemp 30
#define maxTemp 40
#define minTemp 10

// DISPLAYS & KEYBOARD
#include <TM1637Display.h>

const uint8_t degreesC[] = 
{SEG_A | SEG_B | SEG_G | SEG_F,  // º
 SEG_A | SEG_F | SEG_E | SEG_D}; //C

const uint8_t wattsW[] = {SEG_B | SEG_D | SEG_F}; //W
 
#define vDispClk 4
#define vDispData 5
#define cDispClk 6
#define cDispData 7

int keys[] = {8,12,11};

boolean btnStates[3];
boolean ledStates[3];

unsigned long prevKeyMillis = 0;

#if MODE == 0
  #define encoderPinA 2
  #define encoderPinB 3
#else
  #define functionBtn 2
#endif

long encoderPos = 0;
long prevEncoderPos = 0;

TM1637Display cDisplay(cDispClk, cDispData);
TM1637Display vDisplay(vDispClk, vDispData);

unsigned long prevDisplayMillis;

//MENU
unsigned long prevSetMillis;
int setTime = 2000;
unsigned long prevDebounceMillis;
byte thingToSet = 0;
boolean outputOn = false;


void setup() 
{
   setupPWM12();
   vDisplay.setBrightness(0x04);
   cDisplay.setBrightness(0x04);
   Serial.begin(9600);
   
   #ifdef SERIAL
    serialLink.begin(9600);
   #endif
   
   #if MODE == 0
     pinMode(encoderPinA, INPUT_PULLUP);
     pinMode(encoderPinB, INPUT_PULLUP);
     attachInterrupt(digitalPinToInterrupt(encoderPinA), readEncoder, FALLING);
   #else
     pinMode(functionBtn, INPUT_PULLUP);
   #endif

   pinMode(fan, OUTPUT);
}

void loop() 
{ 
  handleFan();
  takeMeasurements();
  readAndUpdateKeys();
  receiveSerial();
  interpretSerial();
  
  if(!checkErrors())
    handleMenu();
}



void handleMenu()
{
  for(byte i=0;i<3;i++)
  {
    if(btnStates[i])
    {
      if(millis()- prevDebounceMillis > 300)
      {
        thingToSet = i+1;
        prevEncoderPos = encoderPos;
        serialLink.print("<sv>");
        prevSetMillis = millis();
        prevDebounceMillis = millis();
      }
    }
  }

  //Back to stand-by when inactivity time expires.
  if(encoderPos != prevEncoderPos)
    prevSetMillis = millis();
  
  if (thingToSet != 0 && (millis() - prevSetMillis >= setTime))
  {
    thingToSet = 0;
  }

  //Main MENU
  switch(thingToSet)
  {
    case 0: //show measured V and I on displays
      ledStates[0] = 0;
      ledStates[1] = 0;
      
      if(millis()-prevDisplayMillis > 100)
      {
        displayVoltage(vDisplay,getMeasVoltage());
        displayCurrent(cDisplay,getMeasCurrent());
        prevDisplayMillis = millis();
      }
      break;
    case 1: //progVoltage set
      ledStates[0] = 1;
      ledStates[1] = 0;
      progVoltage += voltageRes * (encoderPos - prevEncoderPos);
      progVoltage = constrain(progVoltage,0,maxVoltage);
      if(millis()-prevDisplayMillis > 100)
      {
        displayVoltage(vDisplay,progVoltage);
        displayCurrent(cDisplay,getMeasCurrent());
        prevDisplayMillis = millis();
      }
      break;
    case 2: //progCurrent set
      ledStates[0] = 0;
      ledStates[1] = 1;
      progCurrent += currentRes * (encoderPos - prevEncoderPos);
      progCurrent = constrain(progCurrent, 0, maxCurrent);
      if(millis()-prevDisplayMillis > 100)
      {
        displayVoltage(vDisplay,getMeasVoltage());
        displayCurrent(cDisplay,progCurrent);
        prevDisplayMillis = millis();
      }
      break;
    case 3: //Output ON or OFF
      outputOn = !outputOn;
      ledStates[2] = outputOn;
      thingToSet = 0;
      break;
    case 4: //Show temp and power:
      if(millis()-prevDisplayMillis > 100)
      {
        ledStates[0] = 0;
        ledStates[1] = 0;
        displayPower(vDisplay,getMeasPower());
        displayTemp(cDisplay,getMeasTemp());
        prevDisplayMillis = millis();
      }
      break; 
  }
  prevEncoderPos = encoderPos;
  
  if(outputOn)
  {
    setVoltage(progVoltage);
    setCurrent(progCurrent);
  }
  else
  {
    setVoltage(0);
    setCurrent(0);
  }
  constantCurrent = (analogRead(ccMeas) < 550);
}

void handleFan()
{
  if(getMeasTemp() > fanOnTemp)
    analogWrite(fan, constrain(map(getMeasTemp(), fanOnTemp, maxTemp-5, 160, 255), 0, 255));
  else if(getMeasTemp() < fanOnTemp -2)
    analogWrite(fan, 0);
}

boolean checkErrors()
{
  int i = 0;
  if(getMeasCurrent() > (progCurrent + 10))
    i = 1;
  if(getMeasVoltage() > (progVoltage + 2))
    i = 2;
  if(getMeasTemp() > maxTemp)
    i = 3;
  if(getMeasTemp() < minTemp)
    i = 4;
  if(i !=0)
  {
    displayError(vDisplay, cDisplay, i);
    return true;
  }
  return false;
}

