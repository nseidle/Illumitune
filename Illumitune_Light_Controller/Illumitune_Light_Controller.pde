/*
 4-8-2011
 Spark Fun Electronics 2011
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 The light controller board takes in serial commands from the main board. When $34# comes in, lights 3 and 4
 are turned on. They are turned off after some amount of time.
 
 Once a unit is plugged in, it should tell the user where to find more info and what the 
 system can do from the debug terminal.
 
 This code works with the prototype light controller created from a proto shield. You'll need to re-map the pins
 if you are using the custom PCB shield version.
 
 (Uno + Arduino 0022) = bad. Grrr... 57600bps messing up. Let's change everything back to 38400.
 
 4-13-11: Moving up to 115200bps. Fixed a bad bug in the incomingSpot variable going above 20.
 
 */

#include <NewSoftSerial.h>
//Original NewSoftSerial softIncomingSerial(11, 10); //Soft RX in on pin D11. Soft TX out on D10 but we don't use TX in this code
NewSoftSerial softIncomingSerial(2, 3); //Soft RX in on pin D3. Soft TX out on D2 but we don't use TX in this code

#define OFF LOW
#define ON HIGH
//21 43
int StatusLED = A0;
/*int Channel1 = 3; This works with the orignal hand built board
int Channel2 = 2;
int Channel3 = 5;
int Channel4 = 4;
int Channel5 = 7;
int Channel6 = 6;
int Channel7 = 9;
int Channel8 = 8;*/

int Channel1 = 4;
int Channel2 = 5;
int Channel3 = 6;
int Channel4 = 7;
int Channel5 = 8;
int Channel6 = 9;
int Channel7 = 10;
int Channel8 = 11;

long ThisSecond = 0;
long lastMillis;

char incomingLights[20]; //This array stores the incoming channels to turn on in the form: $12345678#
int incomingSpot = 0;

long LightTimeon[10]; //This array stores the number of millis seconds since a light has been turned on
#define MAXTIMEON  500 //This is the maximum amount of time allowed before light turns off

void setup() {

  pinMode(StatusLED, OUTPUT);

  pinMode(Channel1, OUTPUT);
  pinMode(Channel2, OUTPUT);
  pinMode(Channel3, OUTPUT);
  pinMode(Channel4, OUTPUT);
  pinMode(Channel5, OUTPUT);
  pinMode(Channel6, OUTPUT);
  pinMode(Channel7, OUTPUT);
  pinMode(Channel8, OUTPUT);

  Serial.begin(38400); //For debugging

  softIncomingSerial.begin(38400); //For incoming light commands

  //Let the world know we are online
  Serial.println("Illumitune Light Controller v2.0 Online"); 

  //Run the lights through demo mode to prove they are working
  lightsDemo();

  //Let the world know what we do
  Serial.println("When the unit receives $34#, it will turn on lights 3 and 4 (pull those lights low).");
  Serial.println("After 250ms (software adjustable), the light controller turns the light off.");

  incomingSpot = 0;

  //Pre-load the timeon array so that the lights are off intially.
  for(int x = 0 ; x < 8 ; x++)
    LightTimeon[x] = MAXTIMEON;
}

void loop() {

  //Blink the status LED every second
  if( (millis()/1000) != ThisSecond) {
    ThisSecond = millis()/1000; //millis() will roll over every 50 days, but it should roll correctly
    if(digitalRead(StatusLED) == 0) 
      digitalWrite(StatusLED, OFF);
    else
      digitalWrite(StatusLED, ON);
  }

  //Add time to each of the note's timeouts
  //We don't want to play notes too close to each other, so this keeps track of how much time has been 
  //between each note.
  for(int x = 0 ; x < 8 ; x++)
    LightTimeon[x] += (millis() - lastMillis);
  lastMillis = millis();

  while(softIncomingSerial.available()) {
    char newChar = softIncomingSerial.read();

    //This serial is used when controlling from terminal - for testing only
    //while(Serial.available()) {
    //  char newChar = Serial.read();

    if(incomingSpot > 19) incomingSpot = 0;
    incomingLights[incomingSpot++] = newChar; //Record this char to the array

    if(newChar == '$') incomingSpot = 0; //Reset the incoming reading spot
    if(newChar == '#') parseLights(); //If we see the end character then let's make music!
  }

  //Check to see if any lights need to turn off

  if(digitalRead(Channel1) == ON) //Is the channel currently on?
    if(LightTimeon[0] > MAXTIMEON) {
      digitalWrite(Channel1, OFF);
      Serial.println("Channel 1 Off");
    }

  if(digitalRead(Channel2) == ON) //Is the channel currently on?
    if(LightTimeon[1] > MAXTIMEON) {
      digitalWrite(Channel2, OFF);
      Serial.println("Channel 2 Off");
    }

  if(digitalRead(Channel3) == ON) //Is the channel currently on?
    if(LightTimeon[2] > MAXTIMEON) {
      digitalWrite(Channel3, OFF);
      Serial.println("Channel 3 Off");
    }

  if(digitalRead(Channel4) == ON) //Is the channel currently on?
    if(LightTimeon[3] > MAXTIMEON) {
      digitalWrite(Channel4, OFF);
      Serial.println("Channel 4 Off");
    }

  if(digitalRead(Channel5) == ON) //Is the channel currently on?
    if(LightTimeon[4] > MAXTIMEON) {
      digitalWrite(Channel5, OFF);
      Serial.println("Channel 5 Off");
    }

  if(digitalRead(Channel6) == ON) //Is the channel currently on?
    if(LightTimeon[5] > MAXTIMEON) {
      digitalWrite(Channel6, OFF);
      Serial.println("Channel 6 Off");
    }

  if(digitalRead(Channel7) == ON) //Is the channel currently on?
    if(LightTimeon[6] > MAXTIMEON) {
      digitalWrite(Channel7, OFF);
      Serial.println("Channel 7 Off");
    }

  if(digitalRead(Channel8) == ON) //Is the channel currently on?
    if(LightTimeon[7] > MAXTIMEON) {
      digitalWrite(Channel8, OFF);
      Serial.println("Channel 8 Off");
    }
}

//Reads the lights from the incomingLights array and turns them on
//We should see an array that looks like: 578#
void parseLights(void) {
  int spot = 0;
  int note = 0;

  //Show the world we're playing a note!
  if(digitalRead(StatusLED) == 0) 
    digitalWrite(StatusLED, ON);
  else
    digitalWrite(StatusLED, OFF);

  while(incomingLights[spot] != '#') {

    if(incomingLights[spot] == '1') digitalWrite(Channel1, ON);
    if(incomingLights[spot] == '2') digitalWrite(Channel2, ON);
    if(incomingLights[spot] == '3') digitalWrite(Channel3, ON);
    if(incomingLights[spot] == '4') digitalWrite(Channel4, ON);
    if(incomingLights[spot] == '5') digitalWrite(Channel5, ON);
    if(incomingLights[spot] == '6') digitalWrite(Channel6, ON);
    if(incomingLights[spot] == '7') digitalWrite(Channel7, ON);
    if(incomingLights[spot] == '8') digitalWrite(Channel8, ON);

    Serial.print("Channel ");
    Serial.print(incomingLights[spot]);
    Serial.println(" on!");

    //Reset timeon
    if(incomingLights[spot] == '1') LightTimeon[0] = 0;
    if(incomingLights[spot] == '2') LightTimeon[1] = 0;
    if(incomingLights[spot] == '3') LightTimeon[2] = 0;
    if(incomingLights[spot] == '4') LightTimeon[3] = 0;
    if(incomingLights[spot] == '5') LightTimeon[4] = 0;
    if(incomingLights[spot] == '6') LightTimeon[5] = 0;
    if(incomingLights[spot] == '7') LightTimeon[6] = 0;
    if(incomingLights[spot] == '8') LightTimeon[7] = 0;

    spot++;
  }
}

//Turn each light on for a little bit of time
void lightsDemo(void) {
  //Turn all lights off
  digitalWrite(Channel1, OFF);
  digitalWrite(Channel2, OFF);
  digitalWrite(Channel3, OFF);
  digitalWrite(Channel4, OFF);
  digitalWrite(Channel5, OFF);
  digitalWrite(Channel6, OFF);
  digitalWrite(Channel7, OFF);
  digitalWrite(Channel8, OFF);

  delay(500);

  //Turn each channel on for a split second
#define QUICKON  250
  digitalWrite(Channel1, ON);
  delay(QUICKON);
  digitalWrite(Channel1, OFF);

  digitalWrite(Channel2, ON);
  delay(QUICKON);
  digitalWrite(Channel2, OFF);

  digitalWrite(Channel3, ON);
  delay(QUICKON);
  digitalWrite(Channel3, OFF);

  digitalWrite(Channel4, ON);
  delay(QUICKON);
  digitalWrite(Channel4, OFF);

  digitalWrite(Channel5, ON);
  delay(QUICKON);
  digitalWrite(Channel5, OFF);

  digitalWrite(Channel6, ON);
  delay(QUICKON);
  digitalWrite(Channel6, OFF);

  digitalWrite(Channel7, ON);
  delay(QUICKON);
  digitalWrite(Channel7, OFF);

  digitalWrite(Channel8, ON);
  delay(QUICKON);
  digitalWrite(Channel8, OFF);

  //Turn all lights on
  digitalWrite(Channel1, ON);
  digitalWrite(Channel2, ON);
  digitalWrite(Channel3, ON);
  digitalWrite(Channel4, ON);
  digitalWrite(Channel5, ON);
  digitalWrite(Channel6, ON);
  digitalWrite(Channel7, ON);
  digitalWrite(Channel8, ON);

  delay(500);

  //Turn all lights off
  digitalWrite(Channel1, OFF);
  digitalWrite(Channel2, OFF);
  digitalWrite(Channel3, OFF);
  digitalWrite(Channel4, OFF);
  digitalWrite(Channel5, OFF);
  digitalWrite(Channel6, OFF);
  digitalWrite(Channel7, OFF);
  digitalWrite(Channel8, OFF);
}



