#include <Wire.h>
#include <skywriter.h>

unsigned int max_x, max_y, max_z;
unsigned int min_x, min_y, min_z;

void setup() {
  Serial.begin(9600);
  Serial.println("Hello world!");
  
  Skywriter.begin(12, 13);
  Skywriter.onTouch(touch);
  Skywriter.onAirwheel(airwheel);
  Skywriter.onGesture(gesture);
  Skywriter.onXYZ(xyz);
}

void loop() {
  Skywriter.poll();
}

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
}

void touch(unsigned char type){
  Serial.println("Got touch ");
  Serial.print(type,DEC);
  Serial.print('\n');
}

void airwheel(int delta){
  Serial.println("Got airwheel ");
  Serial.print(delta);
  Serial.print('\n');
}
