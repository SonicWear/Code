//This code is for the "Host" Arduino (with XBee Series 2 radio in API mode) attached to laptop.
//Sends sensor data through to MaxMSP.

#include <SoftwareSerial.h>
SoftwareSerial xbSerial(MOSI, 4); // RX, TX of the Xbee

void setup() 
{
  Serial.begin(9600);    //UART to Serial Monitor
  xbSerial.begin(9600);  //Xbee serial to board
  Serial1.begin(9600);   //Out to Max
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
      Serial.println("Bytecount OK");

      //checkSum = 0;

      for (int i=0; i<byteCount; i++)
      {
        inChar = getByte();
        //checkSum += inChar;
        buffer[i] = inChar;
      }

      // get the checksum character
      checkSum = getByte();   // was checkSum +- inChar;  ????
      Serial.print("\nChecksum: ");
      Serial.println(checkSum, HEX);

      Serial.print ("\nUnit ID: ");
      Serial.println(buffer[10],HEX);

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
      Serial1.write(0x81);  //prefix the data with 0x81 (begins message to Max)
      for (int p = 10; p < 32; p++) 
      {
        Serial1.write(buffer[p]);  //send bytes 10-31
      }
      Serial1.write(0xFF);  //suffix the data with 0xFF (ends message to Max)
      
      // TODO: Digital values? 0x82        
    }
  }
}

byte getByte()
{
  if (xbSerial.available() > 0)
  {
    byte inByte = xbSerial.read();
    
    if (inByte < 0x10) {
     Serial.print("0");
     }
     Serial.print(inByte, HEX);
     Serial.print(" ");
    
    return inByte;
  }
}
