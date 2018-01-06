#include <skywriter.h>      // skywriter include


// *********** Sigfox Includes ************
#include <SME_basic.h>
#include <Wire.h>
#include <Arduino.h>
#include <sl868a.h>
#include <HTS221.h>
#include <LSM9DS1.h>
// ********** End of Sigfox Includes ************

// ********** Sigfox Initialize *****************
long long int next_millis = 0;
long long int last = 0;

struct data {
  char id;
  bool buttonPressed;
  short x;
  short y;
  short z;
};

struct data2 {
  char id;
  short temp;
  float latitude;
  float longitude;
};

  data frame;
  data2 frame2;
// ***************** Endo of Sigfox *********************


// ***************Skywriter Initialize ******************
unsigned int max_x, max_y, max_z;
unsigned int min_x, min_y, min_z;
// *********** End of Skywriter Initialize *************


// ******************* Other **********************
const int BabyInSeatPin =  9;   // Output to the BOB Arduino indicating if there is a baby in the seat or not
const int InRangePin = 8;       // Input from BOB Arduino to indicate if the FOB is in Range This goes high 5 seconds after the last message from the FOB is received by BOB arduino
const int InRangeIndicatorPin = 7;   // Pin used to output to an LED indicating that the alarm has been activated (High Active)

int FOBOutOfRangeAndBabyInSeatState = 0;       // FOB out of range for too long and baby in seat
int SendAlarmState =  0;         // 1 = Send Sigfox message
int SigFoxMessageSent = 0;       // 1 indicates that a Sigfox message has already been sent to the cloud so no need to send again for at least 10 minutes
int BabyInSeatState = 0;         // 1 = Baby in seat
int InRangeState = 0;            //  1 if Fob is in Ragne 0 if out of range

// ************************** Fob Out of Range Timeout **************************
unsigned long previousFobInRangeMillis = 0;     // will store last time LED was updated
const long FobOutOfRangeTimeout = 6000;  // If the FOB is out of range for more than a minute then start sending sigfox messages
unsigned long currentMillis;          // Used to store current time in milliseconds
// ******************************************************************************

// *************************** SigFox Retry Timeout *****************************
// Retry sending Sigfox message every 10 minutes
unsigned long previousSigFoxMillis = 0;     // will store last time LED was updated
const long SigFoxTimeout = 600000;  // Retry sending the Sigfox message every 10 minutes 
// ******************************************************************************

// ************************** Baby in Seat Timeout ******************************
// If baby has not been detected for 5 seconds then BabyInSeatState is re-set to 0
unsigned long previousBabyInSeatMillis = 0;     // will store last time LED was updated
const long BabyInSeatTimeout = 5000;  // Retry sending the Sigfox message every 10 minutes 
// ******************************************************************************



  

// ********** the setup function runs once when you press reset or power the board *********
void setup() {

// ************** Beginning of Sigfox Setup ************************
 // SerialUSB.begin(115200);
  smeHumidity.begin();
  smeGps.begin();
  smeAccelerometer.begin();
  
  //SerialUSB.println("Initialize SIGFOX");
 // while (!initSigfox()) {
  //  SerialUSB.println("Could not initialize, retrying...");
 // }
  SigFox.begin(19200);
  //while (!SerialUSB);
  sigfoxCommandMode();
  pinMode(13, INPUT);
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  frame.id = '1';
  frame2.id ='2';
  frame.buttonPressed = false;
// ***************** Endo of Sigfox Setup ********************************** 


// *********** Beginning of Skywriter Setup *****************************
  Serial.begin(9600);              // Communication with the Serial Monitor
  Serial.println("Hello world!");

  Skywriter.begin(10, 11);   // Was 12, 13  but there are two SPI devices  Pins that skywriter are connected to
  Skywriter.onTouch(touch);
  Skywriter.onAirwheel(airwheel);
  Skywriter.onGesture(gesture);
  //Skywriter.onXYZ(xyz); // Makes Baby seat sensor very sensitive and maybe too sensitive ???????

// ************ End of Skywriter Setup ******************************


// ********************** Input/Output Setup **************************
   // set the BabyInSeatState digital pin as output:
    pinMode(BabyInSeatPin, OUTPUT);

    // set the InRangePin digial pin as input:
    pinMode(InRangePin, INPUT);

   // set the InRangeIndicatorPin (LED) digital pin as output:
    pinMode(InRangeIndicatorPin, OUTPUT);
// ******************* End of Input/Output setup **********************

}



// ****************************************************************
// *                       Main Loop                              * 
// ****************************************************************
void loop() { 

 
  
// ****************** Skywriter Main Loop Code **********************

    currentMillis = millis();      // Update current time in miliSeconds
    //Serial.println(currentMillis - previousBabyInSeatMillis);   // print time since last contact with Seat
    if (currentMillis - previousBabyInSeatMillis >= BabyInSeatTimeout)  // If No baby has been detected by the Skywriter for 5 seconds then set BabyInSeatState to 0 (No baby in seat)
    {
      BabyInSeatState = 0;     //Reset the status Flag before polling the skywriter. It will be set to 1 if the Skywriter detects the baby in the seat
    }else
    {
      BabyInSeatState = 1;     //Baby is in the seat
    }

    Skywriter.poll();   // Check the Skywriter for activity  (Check if the baby is in the seat), BabyInSeatState will be set to 1 if the baby is detected


   // Turn on or off LED depending on wheater the baby is in the seat or not
    if (BabyInSeatState == 1)
    {
       digitalWrite(BabyInSeatPin, HIGH);     // LED on 
    }else
    {
       digitalWrite(BabyInSeatPin, LOW);     // LED off  
    }

// **************End of Skywriter Main Loop Code ********************





// ******************  Check If the FOB is out of Range of BOB for more than FobOutRangeTimeout *******************************

    //read the state of the InRangePin digital pin:    
    InRangeState = digitalRead(InRangePin);  // Read InRange Pin from BOB (Indicates that FOB is in Bluetooth range of BOB)
     
    if (InRangeState == 1) // FOB in Range
    {
       previousFobInRangeMillis = millis();  // Reset the out of range counter as FOB in range
       SendAlarmState = 0;                   // Stop sending Sigfox messages and turn on alarm   
    }
    else   // FOB out of Range for time determined by BOB
    {
       // Do not reset the previous milis so that the counter starts counting upwards   
    }



  // Check how long the FOB has been out of range while baby is in the seat
    currentMillis = millis();      // Update current time in miliSeconds
    Serial.println(currentMillis - previousFobInRangeMillis);   // print time since last contact with Seat
 
    if (currentMillis - previousFobInRangeMillis >= FobOutOfRangeTimeout)  
    {
       digitalWrite(InRangeIndicatorPin, HIGH);     // in Range LED Indicator on 
    }else
    {
       digitalWrite(InRangeIndicatorPin, LOW);     // in Range LED Indicator on 
    }

//************************** End of Check If the FOB is in Range of BOB  **************************




// ********************************* Send a message to SIGFOX *************************************

  if (SigFoxMessageSent == 1)
  {
     currentMillis = millis();      // Update current time in miliSeconds
     //Serial.println(currentMillis - previousSigFoxMillis);   // print time since last contact with Seat
     if (currentMillis - previousSigFoxMillis >= SigFoxTimeout)
     {
        SigFoxMessageSent = 0;   // Reset the Sigfox Sent message flag after 10 minutes
        Serial.println ("Sigfox sent flag reset"); 
     }  
  }


    if (SendAlarmState == 1)     // Send sigfox messages
    {
      // SendSigfoxMessage();      // Send Message to Sigfox Cloud
      //Serial.print ("Sending Sigfox message to the cloud");
      SigFoxMessageSent = 1; // Message send so no need to send again
      previousSigFoxMillis = millis();  // Start time
    }

// ******************************** End of Send a message to SIGFOX ***************************************



}

// **************************************************************************
// *                          End of Main Loop                              *
// **************************************************************************










//************* Beginning of Sigfox Functions ************************* 

// Send Message to Sigfox Cloud
void SendSigfoxMessage() {
    frame2.temp = smeHumidity.readTemperature();
    frame.x = smeAccelerometer.readX();
    frame.y = smeAccelerometer.readY();
    frame.z = smeAccelerometer.readZ();
  
    if (smeGps.ready()) {
      frame2.latitude   = smeGps.getLatitude();
      frame2.longitude  = smeGps.getLongitude();
    }
    else {
      frame2.latitude = 52.671663;    //default if no gps signal found
      frame2.longitude = -8.653249;
    }

    //Serial.println ("Press button on Smarteverything board");
  
    if (isButtonOnePressed()) {
      SerialUSB.println("Pressed!");
      frame.buttonPressed = true;
      last = millis();
    }
    if (frame.buttonPressed == true) {
      // bool answer = sendSigfox(&frame, sizeof(data));   removed as not needed
      /*SerialUSB.print("  X = ");
      SerialUSB.print(frame.x, DEC);
      SerialUSB.print("     Y = ");
      SerialUSB.print(frame.y, DEC);
      SerialUSB.print("     Z = ");
      SerialUSB.println(frame.z, DEC);
      delay(2000);*/
      bool answer = sendSigfox(&frame2, sizeof(data2));  // added bool as deleted the first sendsigfox function call
      //  /*
      SerialUSB.print("temp ");
      SerialUSB.println(frame2.temp);
      SerialUSB.print("Latitude ");
      SerialUSB.print(frame2.latitude, DEC);
      SerialUSB.print("Longitude ");
      SerialUSB.print(frame2.longitude, DEC);  // */
      frame.buttonPressed = false;
    }
  }
// ****************** End of Sigfox main loop code ***********************




bool initSigfox() {
  SigFox.begin(19200);
  SigFox.print("+++");
  //SerialUSB.print("SigFox setup: ");
  while (!SigFox.available()) {
    delay(1000);
  }
  String status = "";
  while (SigFox.available()) {
    status += (char)SigFox.read();
  }
  //SerialUSB.println(status);
  status.toUpperCase();
  return status.indexOf("OK") >= 0;
}

void sigfoxCommandMode() {
  SigFox.print("+++");

  // Waiting for modem answer
  while (!SigFox.available()) {
    delay(100);
  }
  while (SigFox.available()) {
    char answer = (char)SigFox.read  ();
     // SerialUSB.print(answer);
  }

  //  SerialUSB.println("");
  delay(10000);
}

boolean canSend(long prev){
  if (prev==0){
    return true;
  }
  unsigned long diff = millis() - prev;
  return diff >= 600000;
}

bool sendSigfox(const void* data, uint8_t len) {
  uint8_t* bytes = (uint8_t*)data;

  //SerialUSB.println("Issuing AT command");
  SigFox.print('A');
  SigFox.print('T');
  SigFox.print('$');
  SigFox.print('S');
  SigFox.print('F');
  SigFox.print('=');
  
  SerialUSB.print('A');
  SerialUSB.print('T');
  SerialUSB.print('$');
  SerialUSB.print('S');
  SerialUSB.print('F');
  SerialUSB.print('=');
  
  //0-1 == 255 --> (0-1) > len
  for(uint8_t i = len-1; i < len; --i) {
    if (bytes[i] < 16) {
      SigFox.print("0");
      SerialUSB.print("0");
    }
    SigFox.print(bytes[i], HEX);
    SerialUSB.print(bytes[i], HEX);
  }
  SigFox.print('\r');

  SerialUSB.print('\r');
  SerialUSB.println("");


  bool error = false;
  // Waiting for modem answer
  while (!SigFox.available()) {
    delay(100);
  }
  bool firstChar = true;
  while (SigFox.available()) {
    char answer = (char)SigFox.read();
      //SerialUSB.print(answer);
    if (firstChar) {
      firstChar = false;
      if (answer == 'O') { // "OK" message
        error = false; 
      } else { // "ERROR" message 
        error = true;
      }
    }
  }
  return !error;
}
// ************* End of Sigfox Functtions ***********************************




//**************   Skywriter Functions *******************//

void xyz(unsigned int x, unsigned int y, unsigned int z){
  if (x < min_x) min_x = x;
  if (y < min_y) min_y = y;
  if (z < min_z) min_z = z;
  if (x > max_x) max_x = x;
  if (y > max_y) max_y = y;
  if (z > max_z) max_z = z;
  
  char buf[64];
  sprintf(buf, "%05u:%05u:%05u gest:%02u touch:%02u", x, y, z, Skywriter.last_gesture, Skywriter.last_touch);
  Serial.println(buf);
  BabyInSeatState= 1;                   // Baby has been detected in the seat
  previousBabyInSeatMillis = millis();  // Reset Baby in seat timer 
}

void gesture(unsigned char type){
  Serial.println("Got gesture ");
  //Serial.print(type,DEC);
  //Serial.print('\n');
  BabyInSeatState= 1;                   // Baby has been detected in the seat
  previousBabyInSeatMillis = millis();  // Reset Baby in seat timer 
}

void touch(unsigned char type){
  Serial.println("Got touch ");
  //Serial.print(type,DEC);
  //Serial.print('\n');
  BabyInSeatState= 1;                   // Baby has been detected in the seat
  previousBabyInSeatMillis = millis();  // Reset Baby in seat timer 
}

void airwheel(int delta){
  Serial.println("Got airwheel ");
  //Serial.print(delta);
  //Serial.print('\n');
  BabyInSeatState= 1;                   // Baby has been detected in the seat
  previousBabyInSeatMillis = millis();  // Reset Baby in seat timer 
}

//************* End of Skywriter Functions ****************************
