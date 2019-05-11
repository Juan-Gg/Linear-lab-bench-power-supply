/**********************************************************
  LINEAR LAB POWER SUPPLY SOFTWARE VERSION 5.0 - ANALOG (2/4)
  First released to the public on MAY 2019
  Written by JuanGg on DECEMBER 2018-APRIL 2019
  https://juangg-projects.blogspot.com/
Feel free to copy and make use of this code as long as you
give credit to the original author.
***********************************************************/

//PWM DACs
void setupPWM12()
{
  DDRB |= _BV(PB1) | _BV(PB2);        // set pins as outputs 
  TCCR1A = _BV(COM1A1) | _BV(COM1B1)  // non-inverting PWM 
           | _BV(WGM11);              //mode 14: fast PWM, TOP=ICR1 
  TCCR1B = _BV(WGM13) | _BV(WGM12)
           | _BV(CS10);               // no prescaling 
  ICR1 = 0xfff;                      // TOP counter value
}

// 12-bit version of analogWrite(). Works only on pins 9 and 10.
void analogWrite12(uint8_t pin, uint16_t val)
{
  val = constrain(val,0,4096);
  switch (pin) {
    case  9: OCR1A = val; break;
    case 10: OCR1B = val; break;
  }
}

//Voltage-Current-Temp Measurement
void takeMeasurements()
{
  if(millis() - prevAdqMillis >= updateRate)
  {
    
    measVoltageSum -= avgMeasVoltage;
    measVoltageSum += ((analogRead(vMeas) * vMeasCal) + vMeasOffset);
    avgMeasVoltage = measVoltageSum / numMeasAverages;

    measCurrentSum -= avgMeasCurrent;
    measCurrentSum += ((analogRead(cMeas) * cMeasCal) + cMeasOffset);
    avgMeasCurrent = measCurrentSum / numMeasAverages;

    prevAdqMillis = millis();
  }
}

float getMeasVoltage()
{
  return avgMeasVoltage;
}

float getMeasCurrent()
{
  return avgMeasCurrent;
}

float getMeasPower()
{
  return getMeasVoltage() * (getMeasCurrent()/1000);
}

float getMeasTemp()
{
  return int(-35 * log(analogRead(tMeas)) + 242);
}

void setVoltage(float voltage)
{
  analogWrite12(vSet, constrain((voltage + vSetOffset) * vSetCal, 0, 4096));
}

void setCurrent(float current)
{
  analogWrite12(cSet, constrain((current+cSetOffset) * cSetCal,0,4096));
}
