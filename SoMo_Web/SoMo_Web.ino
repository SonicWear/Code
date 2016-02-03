// SoMo V5 sketch for SoMo Companion App
// 
// This sketch reads the IMU, scales values to the range 0-1024,
// and sends them over the radio using a variant of the MAX SensorBox protocol.
//
// To program, ensure the Arduino IDE is set to "Arduino Leonardo" under Tools->Board

/*This code sits on the remote devices (what the dancers will wear) 
  in a multiple dancer setup with XBee Series 1 Radios.

 Unit ID for each remote device needs to be set using the keys below:

 Unit 1: 0x31
 Unit 2: 0x32
 Unit 3: 0x33
 Unit 4: 0x34
 Unit 5: 0x35
 Unit 6: 0x36
 Unit 7: 0x37
 Unit 8: 0x38
 Unit 9: 0x39
 */

#include <SoftwareSerial.h>
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
MPU6050 accelgyro; 

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;

#define POWER 1
#define RX    8   //  Use 17 for SoMo V4
#define TX    4

#define RATE  50  // Every xx milliseconds.  Do not set to less than 20 as to not swamp receiver XBee.  50 Recommended.

SoftwareSerial mySerial(RX, TX); // RX, TX

char analogValue[20];               //array of analog values *in byte format for MAX* plus some buffer

char current = 0;    //current position of analog value in array
int digVal   = 0;    //digital pins bits are packed into a single variable
char imask   = 128;  //index bytes start with 0x80
char theEnd  = 255;  //byte to signal message end

//SET UNIT ID HERE
//*******************************
byte unitID = 0x31;
//*******************************

// Counter for the ID packet
int id_counter=0;

void setup(void) 
{
  // Turn on SoMo Power LED
  pinMode(POWER, OUTPUT);
  digitalWrite(POWER, HIGH);  

  Wire.begin();
  Serial.begin(38400);  

  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  mySerial.begin(38400);  
  
  digVal=0;
  
  for (int i = 2;i<14;i++)
  {
    digitalWrite(i,HIGH);  //enable pullups
  }
}

void loop() 
{   
  // read raw accel/gyro measurements from device
  accelgyro.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz);

  // display tab-separated accel/gyro x/y/z values
/*
  Serial.print("a/g/m:\t");
  Serial.print(ax); 
  Serial.print("\t");
  Serial.print(ay); 
  Serial.print("\t");
  Serial.print(az); 
  Serial.print("\t");
  Serial.print(gx); 
  Serial.print("\t");
  Serial.print(gy); 
  Serial.print("\t");
  Serial.print(gz); 
  Serial.print("\t");
  Serial.print(mx); 
  Serial.print("\t");
  Serial.print(my); 
  Serial.print("\t");
  Serial.println(mz);
*/
  
  // Scale inputs to 0-1024 for backwards compatibility with old SensorBox code using pure analog inputs
  int scaled_ax = ScaleMAX(ax);
  int scaled_ay = ScaleMAX(ay);
  int scaled_az = ScaleMAX(az);
  int scaled_gx = ScaleMAX(gx);
  int scaled_gy = ScaleMAX(gy);
  int scaled_gz = ScaleMAX(gz);
  int scaled_mx = ScaleMAX_mag(mx);
  int scaled_my = ScaleMAX_mag(my);
  int scaled_mz = ScaleMAX_mag(mz); 

/* 
   // display SCALED tab-separated accel/gyro x/y/z values
  Serial.print("Scaled:\t");
  Serial.print(scaled_ax); 
  Serial.print("\t");
  Serial.print(scaled_ay); 
  Serial.print("\t");
  Serial.print(scaled_az); 
  Serial.print("\t");
  Serial.print(scaled_gx); 
  Serial.print("\t");
  Serial.print(scaled_gy); 
  Serial.print("\t");
  Serial.print(scaled_gz); 
  Serial.print("\t");
  Serial.print(scaled_mx); 
  Serial.print("\t");
  Serial.print(scaled_my); 
  Serial.print("\t");
  Serial.println(scaled_mz);
*/

  current=0;  //reset analog value counter
  
  packValueMPU9150(scaled_ax); 
  packValueMPU9150(scaled_ay);
  packValueMPU9150(scaled_az);
  packValueMPU9150(scaled_mx);
  packValueMPU9150(scaled_my);
  packValueMPU9150(scaled_mz);
  packValueMPU9150(scaled_gx);
  packValueMPU9150(scaled_gy);
  packValueMPU9150(scaled_gz);

  char total = current;    
  sendMessage(total);   //send everything over radio
  
    // Blink the power LED
  digitalWrite(POWER, HIGH);  
  delay(RATE);         // Wait RATE milliseconds
  digitalWrite(POWER, LOW);     
}

void sendMessage(char total)
{
  mySerial.write(imask|0xF); //this is 0x8F
  
  // Send identifier phrase
  mySerial.print("SOMOV5");
  
  // Send UnitID
  mySerial.write(unitID);

  // Send Analog Values
  for (int i = 0;i<total;i++)
  {
    mySerial.write(analogValue[i]);   
  }
 
  // Send Digital Values
  mySerial.write((digVal&127));
  mySerial.write(digVal>>7);
  
  mySerial.write(theEnd);   //ends digital message with 255
}

void packValueMPU9150(int16_t value)
{
  int16_t tempA = value; 
  analogValue[current] = tempA & 127;
  current++;
  analogValue[current] = (tempA >> 7);
  current++;
}

// Scale the inputs
int ScaleMAX(int raw)
{
   float temp = Scale(raw, -16000, 16000);   
   return temp*1024;
}

// Scale the inputs for magnetometer only
int ScaleMAX_mag(int raw)
{
   float temp = Scale(raw, -32000, 32000);
   return temp*1024;
}

// Scale one of the inputs to between 0.0 and 1.0
float Scale(long in, long smin, long smax)
{
  // Bound
  if (in > smax) in=smax;
  if (in < smin) in=smin;
  
  // Change zero-offset
  in = in-smin;
  
  // Scale between 0.0 and 1.0 (0.5 would be halfway)
  float temp = (float)in/((float)smax-(float)smin);
  return temp;
}
