/**********************************************************
  LINEAR LAB POWER SUPPLY SOFTWARE VERSION 5.0 - SERIAL (3/4)
  First released to the public on MAY 2019
  Written by JuanGg on DECEMBER 2018-APRIL 2019
  https://juangg-projects.blogspot.com/
Feel free to copy and make use of this code as long as you
give credit to the original author.
***********************************************************/

#ifdef SERIAL
void receiveSerial()
{
  while(serialLink.available()>0)
  {
    recChar = serialLink.read();
    if(recChar == startMarker)
    {
      receiving = true;
      newData = false;
      dataIndex=0;
    }
    else if(receiving == true)
    {
      if(recChar != endMarker && dataIndex < dataSize-1)
      {
        receivedData[dataIndex] = recChar;
        dataIndex ++;
      }
      else
      {
        receiving = false;
        newData = true;
        receivedData[dataIndex] = 0;
      }
    }
  }  
}
#endif

void interpretSerial()
{
   #if MODE == 0
  /////////////////MASTER SPECIFIC LOOP//////////////////////////
  //Receive:
  if(newData)
  {
    if(String(receivedData) == "?e")
    {
      serialLink.print("<se");
      serialLink.print(encoderPos);
      serialLink.print(">");
    }

    if(String(receivedData) == "sdpt")
    {
      thingToSet = 4;   
      prevSetMillis = millis();
    }

    if(String(receivedData) == "sv")
    {
      for(int i=0; i<3; i++)
      {
        if(btnStates[i])
          break;
        else if(i==2)
          thingToSet = 0;
      }
    }
    newData = false;
  }
  
  #else
  //////////////////SLAVE SPECIFIC LOOP/////////////////////////
  //Receive:
  if(newData)
  {
    if(String(receivedData).substring(0,2) == "se")
    {
      encoderPos = String(receivedData).substring(2).toInt();
    }

    if(String(receivedData) == "sv")
    {
      for(int i=0; i<3; i++)
      {
        if(btnStates[i])
          break;
        else if(i==2)
          thingToSet = 0;
      }
    }
    newData = false;
  }
  //Send:
  if(millis()-prevSerialMillis > 50) 
  {
    serialLink.print("<?e>");
    prevSerialMillis = millis();
  }
  if(!digitalRead(functionBtn))
  {
    thingToSet = 4;
    serialLink.print("<sdpt>");
    prevSetMillis = millis();
  }

 
  #endif
}



