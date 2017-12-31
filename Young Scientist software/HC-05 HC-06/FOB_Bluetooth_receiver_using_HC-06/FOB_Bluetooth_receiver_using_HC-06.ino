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
SoftwareSerial BTserial(8, 9); // RX | TX   // Bluetooth connections to Arduino
 
// Communications Variables
const long baudRate = 9600; // Bluetooth baud rate
char c = ' ';
boolean NL = true;

// Status LEDs
int BabyInSeat = 1;    //  1 = baby in seat
int InRange = 1;       //  1 = Bluetooth in range
int LedFlashing = 0;   //  1 = Led Flashing / Buzzer Buzzing
int ButtonPressed = 0; //  1 = Button Pressed to disable the Buzzer and LED
unsigned long previousOutOfRangeMillis = 0;
int OutOfRangeMilis = 5000; // 5000mS

//**** Variables for LED Blink Function **************
const int ledPin =  LED_BUILTIN;   // the number of the LED pin
int ledState = LOW;                // ledState used to set the LED
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 1000;        // interval at which to blink (milliseconds)
unsigned long currentMillis;
 
void setup() 
{
    Serial.begin(9600);  // terminal baudrate
    Serial.print("Sketch:   ");   Serial.println(__FILE__);
    Serial.print("Uploaded: ");   Serial.println(__DATE__);
    Serial.println(" ");

    BTserial.begin(baudRate);  
    Serial.print("BTserial started at "); Serial.println(baudRate);
    Serial.println(" ");

    // set the digital pin as output:
    pinMode(ledPin, OUTPUT);
}
 
void loop()
{
   
    // Read from the Bluetooth module and send to the Arduino Serial Monitor
    if (BTserial.available())
    {
        c = BTserial.read();
       // Serial.print("Charecter read back from: ");
      //  Serial.write(c);  // Echo the response to the terminal
      //  Serial.println(" ");
    }

    if (c == '1')   // Baby in seat
    {
        BabyInSeat = 1;   // Baby is in the seat
        // Acknowledge receipt of a character by writing char K over bluetooth to the BOB Master
        BTserial.write("K"); 

        Serial.println("Baby in Seat ");

        InRange = 1;   // FOB is in range
        
        // Reset the out of Range Timer
        previousOutOfRangeMillis = millis();  // Update the timer to with the last time a message was received
    }

    if (c == '0')  // Baby not in seat
    {
        BabyInSeat = 0;   // Baby is not in the seat
        // Acknowledge receipt of a character by writing char K over bluetooth to the BOB Master
        BTserial.write("K"); 

        Serial.println("Baby not in Seat ");

        InRange = 1;   // FOB is in range
        
        // Reset the out of Range Timer
        previousOutOfRangeMillis = millis();  // Update the timer to with the last time a message was received
    }


    c = ' ';   // clear the value in the char c

    // Check how long the FOB has been out of range
    currentMillis = millis();  // Update current time in miliSeconds
    if (currentMillis - previousOutOfRangeMillis >= OutOfRangeMilis)   
    {
        InRange = 0;    // If the FOB has been out of range for too long then set the InRange bit to 0
        Serial.println('FOB Out of Range for more than 10 seconds');
    }
    
     Serial.println(currentMillis - previousOutOfRangeMillis);
     Serial.println(InRange);


    if (InRange == 1)
    {
        if (BabyInSeat == 1)
        {
            LedFlashing = 0;    
        }
        
        if (BabyInSeat == 0)
        {
            LedFlashing = 0;    
        } 
        //Serial.println("FOB In Range ");
    }


    if (InRange == 0)  // FOB out of range
    {
        if (BabyInSeat == 1)  // FOB out of range and Baby in Seat
        {
            LedFlashing = 1;
        }
        
        if (BabyInSeat == 0)  // FOB out of range and Baby not in Seat
        {
            LedFlashing = 0;    
        } 
        Serial.println("FOB Out of Range for more than 10 Seconds ");
    }


    if (LedFlashing == 1)
    {
        BlinkLED;    // Call The LED blink and buzzer sound function
    }else
    {
        digitalWrite(12, LOW);     // Buzzer Off connected to digital pin 1
        digitalWrite(ledPin, LOW); // LED off  
    }

    delay(100);   //  slows down the number of loops 

  
}


void BlinkLED()
{
/*
  Blink without Delay

  Turns on and off a light emitting diode (LED) connected to a digital pin,
  without using the delay() function. This means that other code can run at the
  same time without being interrupted by the LED code.

  The circuit:
  - Use the onboard LED.
  - Note: Most Arduinos have an on-board LED you can control. On the UNO, MEGA
    and ZERO it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN
    is set to the correct LED pin independent of which board is used.
    If you want to know what pin the on-board LED is connected to on your
    Arduino model, check the Technical Specs of your board at:
    https://www.arduino.cc/en/Main/Products
*/

  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
      digitalWrite(12, HIGH); // Buzzer On connected to digital pin 12
    } else {
      ledState = LOW;
      digitalWrite(12, LOW); // Buzzer Off connected to digital pin 12
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}





