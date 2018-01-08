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

//  AT commands
//  https://www.itead.cc/wiki/Serial_Port_Bluetooth_Module_(Master/Slave)_:_HC-05
 
 
#include <SoftwareSerial.h>
SoftwareSerial BTserial(8, 9); // RX | TX
 
const long baudRate = 9600; 
char c=' ';
boolean NL = true;
int BOB = 0;



// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 5000;        // If the FOB fails to respond 5 times in a row then it definitely out of range
unsigned long currentMillis;
int CharKReadBack = 0; 
 
void setup() 
{
    Serial.begin(9600);
    Serial.print("Sketch:   ");   Serial.println(__FILE__);
    Serial.print("Uploaded: ");   Serial.println(__DATE__);
    Serial.println(" ");

 //   pinMode(10,OUTPUT);      // key pin  puts HC-05 Bluetooth device in AT communication mode
 //   digitalWrite(10,HIGH);   // not needed as the device has been programmed to connect to the HC-06

    pinMode(11,INPUT);       // Input from Smarteverything determine if the baby is in the seat or not
                             // High indicates that baby is in the seat


    pinMode(7,OUTPUT);       // Output from BOB to Sigfox to indicate that the FOB is in Range
                             // High indicates that FOB is in Range, Low indicates that FOB is out of Range
                         
 
    BTserial.begin(baudRate);    // Start up the communication with the Bluetooth module
    Serial.print("BTserial started at "); Serial.println(baudRate);
    Serial.println(" ");

}
 
void loop()
{
   BOB = digitalRead(11);   // check if the Baby is in the seat or not by reading input pin 11 
                            // This input is set by the Smart everything board 
 
  
    BTserial.flush();  // Flush the Bluetooth serial port to ensure that there are no unwanted characters in it
    
    
    if (BOB == HIGH) 
    {
      // Write char 1 over bluetooth to the FOB to say that a baby is in the seat
      BTserial.write("1");  
    } else 
    {
      // Write char 0 over bluetooth to the FOB to say that no baby in the seat
      BTserial.write("0"); 
    }
    c = ' ';  // clear the character 

    delay(100);  // wait for response from FOB
 
  
    // Read from the Bluetooth module and send to the Arduino Serial Monitor
    while (BTserial.available())
    { 
        c = BTserial.read();
        Serial.print("Charecter read back: ");
        Serial.println(c);  // Echo the response to the terminal
    }
   
    if  (c == 'K')
    { 
        previousMillis = currentMillis;  // Response has been got so reset the timer
        Serial.println("Key Fob has responded");
    }

    currentMillis = millis();
    Serial.println(currentMillis - previousMillis);

    if (currentMillis - previousMillis >= interval) 
    { 
      digitalWrite(7, LOW);  // Indicates no response from FOB (FOB out of range)
      Serial.println("Fob is out of Range for 5 seconds");
    } else
    {
      digitalWrite(7, HIGH); // Indicates that the FOB is still responding 
      Serial.println("Fob is in Range"); 
    }
   
    delay(900);  // send message every second  (900mS + 100mS delay earlier in the program)

   // BTserial.flush();  // Flush the Bluetooth serial port to ensure that there are no unwanted characters in it
 
}
