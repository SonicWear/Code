//This code is for the "Host" Arduino (with XBee Series 2 radio in API mode) attached to laptop.
//Forwards sensor data through to MaxMSP with command bytes and terminated with 0xFF.

#include <SoftwareSerial.h>
SoftwareSerial xbSerial(MOSI, 4); // RX, TX of the Xbee

void setup() 
{
  Serial.begin(9600);    //Out to Max
  xbSerial.begin(9600);  //XBee serial to board 
}

void loop() 
{
  byte buffer[100];
  int byteCount=0;
  byte inChar=0;
  byte checkSum=0;

  //Read incoming packet starting with 0x7E Start Delimiter
  if (getByte() ==  0x7E) 
  {
    byteCount = int (getByte());
    byteCount <<= 8;
    byteCount += int (getByte());

    if (byteCount == 31) 
    {
      //checkSum = 0;

      for (int i=0; i<byteCount; i++)
      {
        inChar = getByte();
        //checkSum += inChar;
        buffer[i] = inChar;
      }

      // get the checksum character
      checkSum = getByte();

//      Serial.print("\nChecksum: ");
//      Serial.println(checkSum, HEX);

//      Serial.print ("\nUnit ID: ");
//      Serial.println(buffer[10],HEX);

/*
      //print data bytes
      int ptr = 14;
      for (int count=0;count < 9; count++) 
      {
        if (buffer[ptr] < 0x10) {
          Serial.print("0");
        }
        Serial.print (buffer[ptr++],HEX);

        if (buffer[ptr] < 0x10) {
          Serial.print("0");
        }
        Serial.print (buffer[ptr++],HEX);
        Serial.print(" ");
      }
      Serial.println("");     
*/  
     
      // SEND THE BINARY DATA TO MAX
      Serial.write(0x81);  //prefix the data with 0x81 (begins message to Max)
      for (int p = 12; p < 31; p++)
      {
        Serial.write(buffer[p]);  //send sensor bytes
      }
      Serial.write(0xFF);  //suffix the data with 0xFF (ends message to Max)
      
      // TODO: Digital values? 0x82        
    }
  }
}

byte getByte()
{
  if (xbSerial.available() > 0)
  {
    byte inByte = xbSerial.read();
    
/*   if (inByte < 0x10) {
     Serial.print("0");
     }
     Serial.print(inByte, HEX);
     Serial.print(" ");
*/    
    return inByte;
  }
}
