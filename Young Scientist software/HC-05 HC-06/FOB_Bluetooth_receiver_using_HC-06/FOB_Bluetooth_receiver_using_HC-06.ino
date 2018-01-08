//  Sketch: basicSerialWithNL_001
// 
//  Uses hardware serial to talk to the host computer and software serial 
//  for communication with the Bluetooth module
//
//  Pins
//  Arduino 5V out TO BT VCC
//  Arduino GND to BT GND
//  Arduino D9 to BT RX through a voltage divider
//  Arduino D8 BT TX (no need voltage divider)
//
/*  
COMMAND  RESPONSE  COMMENT
AT  OK  Used to verify communication
AT+VERSION  OKlinvorV1.8  The firmware version (version might depend on firmware)
AT+NAMExyz  OKsetname Sets the module name to “xyz”
AT+PIN1234  OKsetPIN  Sets the module PIN to 1234
AT+BAUD1  OK1200  Sets the baud rate to 1200
AT+BAUD2  OK2400  Sets the baud rate to 2400
AT+BAUD3  OK4800  Sets the baud rate to 4800
AT+BAUD4  OK9600  Sets the baud rate to 9600
AT+BAUD5  OK19200 Sets the baud rate to 19200
AT+BAUD6  OK38400 Sets the baud rate to 38400
AT+BAUD7  OK57600 Sets the baud rate to 57600
AT+BAUD8  OK115200  Sets the baud rate to 115200
AT+BAUD9  OK230400  Sets the baud rate to 230400
AT+BAUDA  OK460800  Sets the baud rate to 460800
AT+BAUDB  OK921600  Sets the baud rate to 921600
AT+BAUDC  OK1382400 Sets the baud rate to 1382400
*/
 
#include <SoftwareSerial.h>    // Used for communications with Bluetooth device
SoftwareSerial BTserial(8, 9); // RX | TX   // Bluetooth connections to Arduino
 
// Communications Variables
const long baudRate = 9600; // Bluetooth baud rate
char c = ' ';
boolean NL = true;

// Status LEDs
int BabyInSeat = 1;    //  1 = baby in seat
int InRange = 1;       //  1 = Bluetooth in range
int LedFlashing = 0;   //  1 = Led Flashing / Buzzer Buzzing
unsigned long previousOutOfRangeMillis = 0;
int OutOfRangeMilis = 5000; // 5000mS

//**** Variables for LED Blink Function **************
const int ledPin =  LED_BUILTIN;   // the number of the LED pin
int ledState = LOW;                // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 1000;        // interval at which to blink (milliseconds)
unsigned long currentMillis;

const int BuzzerPin =  12;   // the number of the Buzzer pin
const int SwitchPin =  11;   // the number of the Buzzer pin
int SwitchPressed = 0;       // 1 if Switch has been pressed, Reset if baby seat comes back into range
                             // If button has been pressed the LED continues to flash, Only the buzzer stops buzzing 
int SwitchState = 0; 
 
void setup() 
{
    Serial.begin(9600);  // terminal baudrate
    Serial.print("Sketch:   ");   Serial.println(__FILE__);
    Serial.print("Uploaded: ");   Serial.println(__DATE__);
    Serial.println(" ");

    BTserial.begin(baudRate);  
    Serial.print("BTserial started at "); Serial.println(baudRate);
    Serial.println(" ");

    // set the LedPin digital pin as output:
    pinMode(ledPin, OUTPUT);

    // set the SwitchPin Digial pin as input:
    pinMode(SwitchPin, INPUT);
}

 
void loop()
{
   
    // Read from the Bluetooth module and send to the Arduino Serial Monitor
    if (BTserial.available())
    {
        c = BTserial.read();
      //  Serial.print("Charecter read back from: ");
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
    
    Serial.print ("Time since the Fob was in range of the Seat: ");
    Serial.println(currentMillis - previousOutOfRangeMillis);   // print time since last contact with Seat
 
    if (currentMillis - previousOutOfRangeMillis >= OutOfRangeMilis)   
    {
        InRange = 0;    // If the FOB has been out of range for too long then set the InRange bit to 0
    }
    

    if (InRange == 1)         // FOB in Range of Seat
    {
        LedFlashing = 0;      //LED Never flashes if the FOB is in range   
        //Serial.println("FOB In Range ");
    }


    if (InRange == 0)  // FOB out of range of Seat
    {
        if (BabyInSeat == 1)  // FOB out of range and Baby in Seat
        {
            LedFlashing = 1;
           // Serial.println("FOB Out of Range for more than 5 Seconds and Baby in seat ");
        }
        else  // FOB out of range and Baby not in Seat
        {
            LedFlashing = 0;    
           // Serial.println("FOB Out of Range for more than 5 Seconds and Baby not in seat ");
        }     
    }


    if (LedFlashing == 1)
    {
        BlinkLED();    // Call The LED blink and buzzer sound function
    }
    else
    {
        // LED is not flashing and Buzzer is not buzzing
        digitalWrite(BuzzerPin, LOW);  // Buzzer Off connected to digital pin
        digitalWrite(ledPin, LOW);     // LED off 
        SwitchPressed = 0; 
    }

    delay(100);   //  Loops every 100mS (slows down the loop) Easier for debug, this can be removed if necessary as it slows down the switch detection 

    // read the state of the Switch Pin:
    SwitchState = digitalRead(SwitchPin);
    //Serial.println(SwitchState);
    
     if (SwitchState == LOW)
    {
         SwitchPressed = 1;    // The disable swithc has been pressed. 
         Serial.println ("Switch has been pressed ");     
    }  
}



void BlinkLED()
{
/*
  Blink without Delay
  Turns on and off a light emitting diode (LED) connected to a digital pin,
  without using the delay() function. This means that other code can run at the
  same time without being interrupted by the LED code.
*/

  // Serial.println("Blink LED Called ");

  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  currentMillis = millis();

  if (currentMillis - previousMillis >= interval) 
  { 
    previousMillis = currentMillis;   // save the last time you blinked the LED

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)         // If LED is off
    {
      ledState = HIGH;           // Turn LED on
    
      if (SwitchPressed == 0)    // Buzzer only active is switch has not been pressed
      {
          digitalWrite(BuzzerPin, HIGH); // Turn on Buzzer
          Serial.println ("Buzzer High ");
      }
    } 
     else 
    {
      ledState = LOW;               //  else turn LED off
      digitalWrite(BuzzerPin, LOW); // Buzzer Off connected to digital pin 12
      Serial.println ("Buzzer Low ");
    }
    
    digitalWrite(ledPin, ledState);  // set the LED with the ledState of the variable:
  }
}





