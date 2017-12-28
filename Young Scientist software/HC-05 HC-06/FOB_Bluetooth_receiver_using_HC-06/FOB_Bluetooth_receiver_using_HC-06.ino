//  Sketc: basicSerialWithNL_001
// 
//  Uses hardware serial to talk to the host computer and software serial 
//  for communication with the Bluetooth module
//  Intended for Bluetooth devices that require line end characters "\r\n"
//
//  Pins
//  Arduino 5V out TO BT VCC
//  Arduino GND to BT GND
//  Arduino D9 to BT RX through a voltage divider
//  Arduino D8 BT TX (no need voltage divider)
//
//  When a command is entered in the serial monitor on the computer 
//  the Arduino will relay it to the bluetooth module and display the result.
//
 
 
#include <SoftwareSerial.h>
SoftwareSerial BTserial(8, 9); // RX | TX
 
const long baudRate = 38400; 
char c=' ';
boolean NL = true;
 
void setup() 
{
    Serial.begin(9600);
    Serial.print("Sketch:   ");   Serial.println(__FILE__);
    Serial.print("Uploaded: ");   Serial.println(__DATE__);
    Serial.println(" ");

    pinMode(10,OUTPUT); // key pin  puts decice in at mode
    digitalWrite(10,HIGH);

    pinMode(11,INPUT);   // Output to the Smarteverything to tell it that the Bluetooth FOB is still in range

 
    BTserial.begin(baudRate);  
    Serial.print("BTserial started at "); Serial.println(baudRate);
    Serial.println(" ");
}
 
void loop()
{
   
    // Read from the Bluetooth module and send to the Arduino Serial Monitor
    if (BTserial.available())
    {
        c = BTserial.read();
        Serial.print("Charecter read back from: ");
        Serial.write(c);  // Echo the response to the terminal
        Serial.println(" ");
    }

    if  (c == 'K')
    {
        digitalWrite(12, HIGH); // Indicates that the FOB is still responding
        Serial.println("Key Fob has responded");
    }else
    {
        digitalWrite(12, LOW);  // Indicates no response from FOB (FOB out of range)
        Serial.println("No Response from Keyfob or Keyfob out of range");
    }
    
    c = ' ';
   
    delay(1000);  // send message every second
 
}
