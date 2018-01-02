#include <skywriter.h>   // skywriter include


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
unsigned int BabyInSeat = 0;   // 0 indicates that the baby is not in the seat

// *********** End of Skywriter Initialize *************



  

// the setup function runs once when you press reset or power the board


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
  Serial.begin(9600);
  Serial.println("Hello world!");

  Skywriter.begin(12, 13);
  Skywriter.onTouch(touch);
  Skywriter.onAirwheel(airwheel);
  Skywriter.onGesture(gesture);
  //Skywriter.onXYZ(xyz);

// ************ End of Skywriter Setup ******************************
  

}

// the loop function runs over and over again forever
void loop() { 
  
// ************ Skywriter Main Loop Function Call ******************
  Skywriter.poll();   // Check the Skywriter for activity  (Check if the baby is in the seat)
// If the baby is in the seat then  BabyInSeat variable remains active until there is no activity for 30 seconds
//  After 30 seconds the seat starts to transmit Sigfox messages with GPS co-ordinates and Temperature measurements
// *****************************************************************


// ************ Sigfox Main Functions ***********************  
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

// ****************** End of Sigfox main loop functions ***********************
}



//************* Beginning of Sigfox Functions ************************* 
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
}

void gesture(unsigned char type){
  Serial.println("Got gesture ");
  Serial.print(type,DEC);
  Serial.print('\n');
  BabyInSeat = 1;
}

void touch(unsigned char type){
  Serial.println("Got touch ");
  Serial.print(type,DEC);
  Serial.print('\n');
  BabyInSeat = 1;
}

void airwheel(int delta){
  Serial.println("Got airwheel ");
  Serial.print(delta);
  Serial.print('\n');
  BabyInSeat = 1;
}

//************* End of Skywriter Functions ****************************
