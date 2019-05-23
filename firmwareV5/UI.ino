/**********************************************************
  LINEAR LAB POWER SUPPLY SOFTWARE VERSION 5.0 - UI (4/4)
  First released to the public on MAY 2019
  Written by JuanGg on DECEMBER 2018-APRIL 2019
  https://juangg-projects.blogspot.com/
This work is licensed under GNU General Public License v3.0
***********************************************************/


void displayFloat(TM1637Display disp, float num)
{
  if(num>=0 && num <1000)
  {
  int pos  = String(num).indexOf('.');
  disp.showNumberDecEx(round(num*pow(10, 4-pos)), (0x80 >> pos-1), true);
  }
  else if (num > 0 && num < 10000)
    disp.showNumberDecEx(round(num), true);
  else
    disp.showNumberHexEx(0xeeee); 
}

void displayVoltage(TM1637Display disp, float voltage)
{
  if(voltage < 0)
    voltage = 0;
  if(voltage<(maxVoltage+5))
  {
    disp.showNumberDecEx(voltage*100, 0b01000000, true);
  }
  else
    disp.showNumberHexEx(0xeeee);
}

void displayCurrent(TM1637Display disp, float current)
{
  if(current<0)
    current = 0;
  if(current<(maxCurrent+20))
  {
    if(constantCurrent)
      disp.showNumberDecEx(current, 0b11110000, true);
    else
      disp.showNumberDec(current, true);
  }
  else
    disp.showNumberHexEx(0xeeee);
}

void displayTemp(TM1637Display disp, float temp)
{
  disp.setSegments(degreesC,2,2); //Display the Variable value;
  disp.showNumberDec(temp,true,2,0);
}

void displayPower(TM1637Display disp, float power)
{
  disp.setSegments(wattsW,1,3); //Display the Variable value;
  disp.showNumberDecEx(round(power*10),0b01000000, true,3,0);
  Serial.println(power);
   //disp.showNumberDecEx(power*100, 0b01000000, true);
}

void displayError(TM1637Display disp1, TM1637Display disp2, int error)
{
  disp1.showNumberHexEx(0xeeee);
  disp2.showNumberHexEx(0xe000 + error);
}

#if MODE == 0
void readEncoder()
{
  if(digitalRead(encoderPinB) == LOW)
    encoderPos++;
  else
    encoderPos--;
}
#endif

void readAndUpdateKeys()
{
  if(millis()-prevKeyMillis > 10)
  {
    for(int i=0;i<3;i++)
    {
      pinMode(keys[i], INPUT_PULLUP);
      btnStates[i] = !(digitalRead(keys[i])== HIGH);
      pinMode(keys[i], OUTPUT);
      digitalWrite(keys[i],!ledStates[i]);
    }
    prevKeyMillis = millis();
  }
}

