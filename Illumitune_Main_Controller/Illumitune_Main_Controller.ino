/*
 3-30-2011
 Spark Fun Electronics 2011
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 The harp mainboard takes in serial commands from the IR sensors and a button. When $34# comes in, musical
 instruments 3 and 4 are played once. Not twice. When the button is pressed, the array of instruments is changed.
 
 Everything now runs at 38400 because Arduino 21 and Uno don't play well at 38400bps.
 
 Things to do:
 1) We may want to build a watch dog into the main board and the peripheral boards. Added 8/3/11
 
 2) We may want to build in a 'play with me function' where the unit starts making noise if people don't use it after
 a certain amount of time. Done 4/11/11
 
 3) We should play a note every time the button is hit. Done 4/10/11
 
 4) Enable remote reset of boards. Done 4/11/11
 
 5) Add IR output from the main board so that we can turn on the amplifier without having to open the side panel. Done 4/24/11
 
 4-13-2011: Fixed a bad bug with the incomingIRs array and incomingSpot variable getting too big. Moved everything to 115200bps.
 We need to test and then set the screen saver timeout
 
 4-24-2011: Added IR control of stereo.
 
 Good sounds: 
 
 Piano - plays notes 70 to 78
 Bank 0x00
 0
 6
 14
 75
 80
 91
 113
 115
 118
 124
 
 Fancy sounds, just one note each
 Bank 0x78
 30, 41, 49, 55, 58, 60, 66, 72, 78
 
 Instrument
 6
 8
 9
 19
 85
 91
 98
 104
 
 4-27-2011: Adding IR control of audio amp.
 Power:    0xC1A28877
 Standby:  0xC1A2B847
 Vol up:   0xC1A2E21D
 Vol down: 0xC1A2D22D
 
 7-27-2011: Employees are reporting that Illumitune is making loud noises,
 usually in the afternoon. I get differing reports of what fixes it, hitting the
 button? Or unplugging it? Does it occur more often after long periods of inactivity?
 
 Is it an old version of the MIDI shield that is going haywire? It is the old version,
 replace with new 1k inline version. We'll need to add IR and cut a trace.
 Modifications: Cut trace on MIS, pin 3 now goes to IR
 Add JST
 Add current resistor to IR
 Green wire to pin 5? That is now Soft TX out to the MIS. Why? Conflicted with IR control
 
 Is it the main controller going into the weeds?
 What happens if we have the main controller go into reset, will the
 IO pins high impedance cause the MIS to go noisy?
 Perhaps we can reset the MIS after every 10 mins of inactivity?
 
 Ahah! Any time resetMIDI goes low, VS1053 outputs white noise. This happens if ATmega goes into the weeds.
 We need a watch dog timer. After some research, WatchDog seems to not work on old ATmega328 duelminova boards. 
 May need Uno? Let's leave illumitune without WD, but let's never pull MIDIreset low.
 
 Need to change main so that it does not report the 'screen saver' to log every time - Fixed
 After initial boot up the scale sounds very wrong. Why? - Fixed
 
 Pulled logs as well
 
 8-3-2011: Installing new MIS.
 Watchdog works but acts a bit funny. Adding more information to logging so we know at what point
 the unit reset. 
 
 Pulled the logs. I can see where the unit is powered up/down during the day...
 
 Pulled the green LED light from channel 4(?).
 The bulb type seems to be:
 JDR Type GU-10 Base / MR16 2 Diameter / Line Voltage / 20 Watt
 The original green lamp had 21LEDs, and a very slight convex lens.
 Search for: MR16 GU-10 Green LED
 About the only place I could find one is here:
 http://www.bulbamerica.com/silver-1w-mr16-18led-gu10-base-green-bulb.html
 
 10-12-2011
 More reports of bad feedback.
 
 Pulling the logs, it looks like the unit resets after switching instruments.
 
 To see how long reset takes, we need to add a print time after the unit is up.
 
 It would also be good to correctly mark the LOG file date
 
 Whoa! Holding the instrument button causes system reset!
 Now the dog is pet while the button is held.
 
 Now something is wrong with the GPS parsing? The petDog function was printing so much
 ths GPS parser couldn't catch up.
 
 The unit seems to be WDT resetting on notes as well. Weird. I added more petting in more loops. 
 
 11-17-2011
 More reports of bad feedback. Came in at 9AM and the unit was working but making no noise? After a reset the unit starts working
 as expected.
 
 Logs show weird time stamps and often stopping logging after pressing a button. Why?
 Time stamps are working fine, but there are no more WDT resets in the logs. Dah?
 
 Why is there a BUNCH of button hits before the end of some logs? I installed a 1k pull up on the button input pin.
 
 12-14-2011
 On the last visit I installed the new RC block on the MIDI shield. There are still reports of the bad feedback noise. On this visit,
 the unit was completely down and has been for a few days. This time it was the instrument button that had jammed shut. This
 caused the system to hang infinitely.
 
 Programming in a button state so that the system does not hang.
 
 Need to bring a new button and graphite lube next time. The blue LED light is also out.
 
 3-20-2012
 Bringing new switches. Updating to Arduino v1.0.
 
 Added two new spots (spot++ check and buttonPresses check) where we may get a variable over-run.
 
 Replacing blue bulb with white bulb.
 Checking why channel 8 (purple, last channel) is not working.
 
 Adding ability to disable GPS parsing. Could this library be responsible for failure? I am going to test for a week.
 
 3-28-2012
 Adding back millis printing so we can tell what's going on/when.
 
 4-18-2012
 Unit works great at 9AM but they report the unit is continuing to hang. Removing GPS parsing doesn't seem to have made it better or worse. 
 
 I may need to go to the external MIDI unit.
 I have a new, non-IR amp to install. I forgot it this week but will bring it next.
 
 Write our own GPS parser - I am weary of crack_gps.
 Install non-IR amp
 Remove IR routines
 Use external MIDI box
 Install new main board power adapter. Original was 12V, install 9V
 Leave extra parts in red box, including microSD adapter.
 
 4-24-12
 Commiting code and eagle files to Github.
 
 To do:
 Removing IR code - done
 Log all error ridden data - done
 Check for errors from light controller (we normally just pass commands to it) - done

 */

//We may need to disable GPS to test if that library is causing the hang glitches
#define ENABLE_GPS

#include <avr/wdt.h> //We need watch dog for this program

//When we pet the WDT, let's print from what routine we pet it
#define PET_NONE  0
#define PET_MAIN  1
#define PET_BUTTON  2
#define PET_GPS  3
#define PET_PLAY  4
#define PET_NOTES  5

#include <SoftwareSerial.h>
//This was the original setup before we added the IR
//NewSoftSerial softMIDI(2, 3); //Soft TX out on 3, we don't use RX(2) in this code
SoftwareSerial softMIDI(2, 5); //Soft TX out on 5, we don't use RX(2) in this code
SoftwareSerial softLightControl(8, 9); //Soft TX out on 9, soft RX on 8 - RX is not used

#ifdef ENABLE_GPS
SoftwareSerial softGPS(11, 12); //Soft TX out on 12, soft RX on 11
#include <TinyGPS.h>
TinyGPS gps;
int local_hour_offset = 7; //Boulder Colorado (MST) is -7 hours from GMT
unsigned long age, date, time;
int year;
byte month, day, hour, minute, second, hundredths;
#endif

//Pin definitions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//IR LED = 3, IR is controlled by IRremote.h
byte resetMIDI = 4; //Tied to VS1053 Reset line
byte InputButton = 7; //Input pin for the button
byte resetIRController = 6; //Tied to the reset pin of the IR Controller Arduino
byte resetLightController = 10; //Ties to the reset pin of the Light Controller Arduino
byte ledPin = 13; //MIDI traffic inidicator
byte StatusLED = A4; //The yellow status LED on the master controller
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

char buffer[100]; //Used for GPS sprintf things

long ThisSecond = 0;
long lastMillis = 0;

char incomingIRs[20]; //This array stores the incoming IR beam breaks in the form: $12345678#
int incomingSpot = 0;

long NoteTimeout[10]; //This array stores the number of millis seconds since a note has been played
#define MINTIMEOUT  50 //This is the minimum amount of time required between notes, in ms, 250ms works ok

int ButtonPresses = 0; //This controls which set of sounds we play. 0 is the special sounds, 1 through 9 are pianos.
int ignoreButton = false;

//These are 9 interesting sounds from the special bank 0x78
//You only play each of these notes once
int SoundNumber = 0;
char SoundLookup[9] = { 
  30, 41, 49, 55, 58, 60, 66, 72, 78 };

//These are 9 cool sounding instruments from the default bank 0x00
//Each instrument has the full note range from 30? to 90?
int InstrumentNumber = 0;
//char InstrumentLookup[9] = { 
//  0, 6, 14, 75, 80, 91, 113, 115, 118 };
char InstrumentLookup[6] = { 
  0, 6, 14, 113, 115, 118 };
int MaxNumberInstruments = 6;
//I removed some of the instruments that don't auto turn off

long secondsUntilScreenSaverMode = 10; //300 = 5 minutes - amount of time before exhibit does it's own thing
long secondsSinceInteraction = 0;
int MISWatcher = 0; //Counts to 10 minutes then resets the MIS

void setup() {
  wdt_reset(); //Pet the dog
  wdt_disable(); //We don't want the watchdog during init

  Serial.begin(38400); //All debugging is at 38400 - OpenLog needs to listen at this rate as well
  Serial.println("Illumitune Master Controller v2.0"); 

  pinMode(StatusLED, OUTPUT);
  pinMode(InputButton, INPUT);
  digitalWrite(InputButton, HIGH); //Enable internal pull-up on the button input

  pinMode(resetIRController, OUTPUT);
  digitalWrite(resetIRController, LOW); //Put IR Controller into reset

  pinMode(resetLightController, OUTPUT);
  digitalWrite(resetLightController, LOW); //Put Light Controller into reset

  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, HIGH); //Hold VS1053 in reset while we get setup

  softMIDI.begin(31250); //Setup soft serial for MIDI control

  setupMIDI(); //Sets the volume and initial instrument bank

  softLightControl.begin(38400); //Setup soft serial for Light Controller

  //Act as if the button has been hit for the first time to put illumitune into default piano mode
  ButtonPresses = 0; //This controls which set of sounds we play. 0 is the special sounds, 1 through 9 are pianos.
  //This doesn't init the MIDI correctly if button gets jammed or stuck so let's init the MIDI separately
  //dealWithButton(); //Calling this function will run with the next button press, in this case, Piano #1 because BP = 0;

  setupGPS(); //Configure the GPS module
  Serial.println("GPS Configured"); 

  //demoPlay(); //Play some sounds to tell the world we are alive
  //demoPlayAll(); //Play all the sounds and instruments that we've specially selected
  //Turn the system back to normal piano notes
  ButtonPresses = 1; //Set the system up to be on normal notes, not sounds
  talkMIDI(0xB0, 0, 0x00); //Default bank GM1
  talkMIDI(0xC0, InstrumentLookup[0], 0); //Set instrument number. 0xC0 is a 1 data byte command  
  noteOn(0, 60, 60); //Center C
  delay(200);
  noteOn(0, 62, 60); //D
  delay(200);
  noteOn(0, 64, 60); //E
  delay(200);
  noteOn(0, 65, 60); //F
  delay(200);

  for(int x = 1 ; x < 9 ; x++)
    NoteTimeout[x] = 0; //Reset all the note timeouts

  digitalWrite(resetIRController, HIGH); //Bring IR Controller online
  Serial.println("IR Controller Online"); 

  digitalWrite(resetLightController, HIGH); //Bring Light Controller online
  Serial.println("Light Controller Online"); 

  if (feedgps()) lookUpDate(); //Watchdog pet both places
  printTimeDate(); //Print current time and date to log file

  //Let the world know we are online
  Serial.println("Master Init Complete");

  wdt_enable(WDTO_250MS); //Unleash the beast
}

void loop() {
  petDog(PET_MAIN); //Pet the dog

  //Blink the status LED every second
  if( (millis()/1000) != ThisSecond) {
    ThisSecond = millis()/1000; //millis() will roll over every 50 days, but it should roll correctly
    if(digitalRead(StatusLED) == 0) 
      digitalWrite(StatusLED, HIGH);
    else
      digitalWrite(StatusLED, LOW);
    secondsSinceInteraction++;
  }

  //Add time to each of the note's timeouts
  //We don't want to play notes too close to each other, so this keeps track of how much time has been 
  //between each note.
  for(int x = 1 ; x < 9 ; x++)
    NoteTimeout[x] += (millis() - lastMillis);
  lastMillis = millis();

  while(Serial.available()) {
    petDog(PET_MAIN); //Pet the dog

    char newChar = Serial.read();

    if(incomingSpot > 19) {
      incomingSpot = 0; //Wrap incomingSpot variable
      //We've received some error - log it
      petDog(PET_MAIN); //Pet the dog
      printTimeDate(); //Print current time and date to log file

      petDog(PET_MAIN); //Pet the dog
      Serial.println("Error: Too many characters received from Light Controller!");
    }
    incomingIRs[incomingSpot++] = newChar; //Record this char to the array

    if(newChar == '$') incomingSpot = 0; //Reset the incoming reading spot
    if(newChar == '#')
      parseNotes(); //If we see the end character then let's make music! Watchdog pet
    else {
      //We've received some error - log it
      petDog(PET_MAIN); //Pet the dog
      printTimeDate(); //Print current time and date to log file

      petDog(PET_MAIN); //Pet the dog
      Serial.println("Error: Bad frame received from Light Controller!");
    }
  }

  //If button is pressed, debounce and cycle to the next set of instruments
  if(digitalRead(InputButton) == LOW) dealWithButton(); //Watchdog pet

  //If button is not pressed, then we know it's not broken and we can quit ignoring it
  if(digitalRead(InputButton) == HIGH) ignoreButton = false;

  //If we have a good, current, date and time, break it out
  if (feedgps()) lookUpDate(); //Watchdog pet both places

  //If the exhibit is not interacted with, start blinking the lights
  if(secondsSinceInteraction > secondsUntilScreenSaverMode) {
    secondsSinceInteraction = 0; //Reset the time since last interaction
    playWithMe(); //!!! Watchdog pet
  }
  
  //This is an odd test where we check to see if the light controller is reporting it is reset
  if(softLightControl.available() > 2) {
    //Read in two bytes and check them
    byte char1 = softLightControl.read();
    if(char1 == 'I') { //Light controller reports 'Illumitune Light Controller v2.0 Online' each time it resets
      byte char2 = softLightControl.read();
      if(char2 == 'l'){ //Confirmed reset
        Serial.println("Error: Light controller reset");
      }
    }
  }
}

//This function pets the dog and then records at what stage it was called to EEPROM
//This allows us to know how we came out of reset and log the state
void petDog(int petStyle){
  wdt_reset(); //Pet the dog

  return; //For production code, we do not want to print the petStyle

  //Doing this WDT logging is dangerous because we can exhaust the EEPROM
  //Pretty quickly (~30 days)
  //This should only be used for testing
  //EEPROM.write(WDTSTATE, petStyle); //Stick the pet style into EEPROM so we can look it up after reset
  switch(petStyle){
  case PET_NONE: 
    Serial.println("WDT: None"); 
    break;
    //case PET_MAIN: Serial.println("WDT: Main"); break;
  case PET_BUTTON: 
    Serial.println("WDT: Button"); 
    break;
  case PET_GPS: 
    Serial.println("WDT: GPS"); 
    break;
  case PET_PLAY: 
    Serial.println("WDT: Play"); 
    break;
  case PET_NOTES: 
    Serial.println("WDT: Notes"); 
    break;
  }
}

//This function is called if the Illumitune is not interacted with after a set amount of time
//Play with me!!!
void playWithMe(void) {
  //We don't need to record this every time to the log
  printTimeDate(); //Print current time and date to log file
  Serial.println("Screen Saver");

  //Make the lights go up/down in a row
  for(int x = 1 ; x < 9 ; x++) {
    softLightControl.print("$");
    softLightControl.print(x, DEC);
    softLightControl.print("#");
    if(Serial.available()) return; //This indicates someone is playing with us!
    if(digitalRead(InputButton) == LOW) return; //This indicates someone is playing with us!
    delay(100);
    petDog(PET_PLAY); //Pet the dog
  }

  //Back down
  for(int x = 7 ; x > 0 ; x--) {
    softLightControl.print("$");
    softLightControl.print(x, DEC);
    softLightControl.print("#");
    if(Serial.available()) return; //This indicates someone is playing with us!
    if(digitalRead(InputButton) == LOW) return; //This indicates someone is playing with us!
    delay(100);
    petDog(PET_PLAY); //Pet the dog
  }
}

//This function plays a note and then adjusts the current sound bank
//We rotate through different instruments and sounds here
void dealWithButton(void) {

  if(ignoreButton == true) return; //If we are ignoring the button then bail immediately
  
  #define waitLimit  500

  int loopCount = 0;
  while(digitalRead(InputButton) == LOW) {
    //Wait for user to STOP PRESSING DA BUTTON!
    petDog(PET_BUTTON); //Pet the dog

    //The button has jammed shut before. Let's make it fail gracefully
    delay(1);
    if(loopCount++ >= waitLimit) break; 
  }

  if(loopCount >= waitLimit) 
    ignoreButton = true; //If we hold the button for more than 2 seconds, ignore it
  else
    ignoreButton = false;

  delay(200); //Debouncing
  loopCount = 0;
  while(digitalRead(InputButton) == LOW) { 
    //This switch seems to be pretty dirty so we do this twice
    petDog(PET_BUTTON); //Pet the dog

    //The button has jammed shut before. Let's make it fail gracefully
    delay(1);
    if(loopCount++ >= waitLimit) break; 
  }

  if(loopCount >= waitLimit) 
    ignoreButton = true; //If we hold the button for more than 2 seconds, ignore it
  else
    ignoreButton = false;

  if(ignoreButton == true) return; //If we're ignoring the button then don't do any instrument changes

  secondsSinceInteraction = 0; //Reset the time since last interaction

  //Play a note to indicate the button has been hit
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  talkMIDI(0xB0, 0, 0x78); //Select the bank of really fun sounds

  //For this bank 0x78, the instrument does not matter, only the note
  talkMIDI(0xC0, 2, 0); //Set instrument number. 0xC0 is a 1 data byte command

  //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
  noteOn(0, 31, 60); //Sound 31 is sticks
  //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  //Tell all the lights to turn on. They'll turn off after a short while
  softLightControl.println("$12345678#");
  petDog(PET_BUTTON); //Pet the dog

  ButtonPresses++; //Go to next instrument bank

  printTimeDate(); //Print current time and date to log file

    //We have one set of noises and 9 instruments to play
  if(ButtonPresses >= MaxNumberInstruments) ButtonPresses = 0; //Loop the variable

  if(ButtonPresses == 0) { //Play funky noises
    Serial.println("Switching to noises");

    talkMIDI(0xB0, 0, 0x78); //Select the bank of really fun sounds
    //For this bank 0x78, the instrument does not matter, only the note
    talkMIDI(0xC0, 2, 0); //Set instrument number. 0xC0 is a 1 data byte command
  }
  else{
    int instrument = ButtonPresses - 1;
    Serial.print("Switching to instrument ");
    Serial.println(instrument, DEC);

    //This is normal piano notes
    talkMIDI(0xB0, 0, 0x00); //Default bank GM1
    talkMIDI(0xC0, InstrumentLookup[instrument], 0); //Set instrument number. 0xC0 is a 1 data byte command  
  }
}

//Reads the notes from the incomingIR array and plays them
//We should see an array that looks like: 578#
void parseNotes(void) {
  petDog(PET_NOTES); //Pet the dog

  int spot = 0;
  int note = 0;

  //If we're parsing serial, then we know we're being played with
  //Reset the interaction counter
  secondsSinceInteraction = 0;

  //Show the world we're playing a note!
  if(digitalRead(StatusLED) == 0) 
    digitalWrite(StatusLED, HIGH);
  else
    digitalWrite(StatusLED, LOW);

  //Send commands to the light controller to turn on these channels
  softLightControl.print("$");

  while(incomingIRs[spot] != '#') {
    petDog(PET_NOTES); //Pet the dog

    //Serial.print(incomingIRs[spot]); //Debugging

    note = incomingIRs[spot] - '0'; //Adjust the character from a character to a number, so '5' becomes 5.
    if(note > 8) note = 8; //Notes should only be channels 1 to 8
    if(note < 1) note = 1;

    //sprintf(buffer, "%02d:%02ld", note, NoteTimeout[note]); //Debugging
    //Serial.println(buffer);

    //Note will dissapate automatically, but we don't want to play it more than once
    //So check to see if enough time has passed to play again
    if(NoteTimeout[note] > MINTIMEOUT){
      NoteTimeout[note] = 0; //Reset the amount of time to zero

      if(ButtonPresses == 0)
        noteOn(0, SoundLookup[note], 60); //Play a specific noise
      else {
        //noteOn(0, note + 70, 60); //Plays notes 70 to 77, instument is set in main loop
        if(note == 1) noteOn(0, 60, 60); //Center C
        if(note == 2) noteOn(0, 62, 60); //D
        if(note == 3) noteOn(0, 64, 60); //E
        if(note == 4) noteOn(0, 65, 60); //F
        if(note == 5) noteOn(0, 67, 60); //G
        if(note == 6) noteOn(0, 69, 60); //A
        if(note == 7) noteOn(0, 71, 60); //B
        if(note == 8) noteOn(0, 72, 60); //2nd C
      }

      petDog(PET_NOTES); //Pet the dog
      printTimeDate(); //Print current time and date to log file

      petDog(PET_NOTES); //Pet the dog
      Serial.print("Note,");
      Serial.println(note, DEC);
      petDog(PET_NOTES); //Pet the dog

      //Send commands to the light controller to turn on these channels
      softLightControl.print(note, DEC);
      petDog(PET_NOTES); //Pet the dog
    }

    spot++;
    if(spot > 8){ //This is the super safe catch in case things get crazy. Should never happen.
      Serial.println("Error: Spot incremented too far!");
      break; 
    }
  }

  //Send commands to the light controller to turn on these channels
  softLightControl.print("#"); //The closing character of the command packet
  petDog(PET_NOTES); //Pet the dog
}

//Resets Music Instrument shield, then configures it for normal operation
//You can call this at any time to reset the shield
void setupMIDI(void){
  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  //digitalWrite(resetMIDI, LOW);
  //delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);

  talkMIDI(0xB0, 0x07, 120); //0xB0 is channel message, set channel volume to near max (127)
}

//Prints the time and date, usually from GPS for logging reasons
void printTimeDate(void) {
  //Print this to the terminal and there for the logger
#ifdef ENABLE_GPS
  sprintf(buffer, "%02d/%02d/%04d, ", month, day, year);
#else
  sprintf(buffer, "[GPS disabled], ");
#endif
  Serial.print(buffer);

#ifdef ENABLE_GPS
  sprintf(buffer, "%02d:%02d:%02d, ", hour, minute, second);
  Serial.print(buffer);
#else
  //sprintf(buffer, "[GPS disabled], ");
  Serial.print(millis()/1000);
  Serial.print(", ");
#endif
}

//Plays three notes to let us know the machine is alive
void demoPlay(void) {

  //This is normal piano notes
  talkMIDI(0xB0, 0, 0x00); //Default bank GM1

  talkMIDI(0xC0, InstrumentLookup[0], 0); //Set instrument number. 0xC0 is a 1 data byte command  

  for (int note = 70 ; note < 75 ; note++) {

    //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
    noteOn(0, note, 60);
    delay(100);

    //Turn off the note with a given off/release velocity
    noteOff(0, note, 60);
    delay(100);
  }
}

//Demo all the notes for debugging
//Plays all the notes 
void demoPlayAll(void) {

  talkMIDI(0xB0, 0, 0x78); //Select the bank of really fun sounds

  //For this bank 0x78, the instrument does not matter, only the note
  talkMIDI(0xC0, 2, 0); //Set instrument number. 0xC0 is a 1 data byte command

  for (int x = 0 ; x < 9 ; x++) {
    //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
    noteOn(0, SoundLookup[x], 60);
    delay(250);

    //Turn off the note with a given off/release velocity
    noteOff(0, SoundLookup[x], 60);
    delay(250);
  }

  //This is normal piano notes
  talkMIDI(0xB0, 0, 0x00); //Default bank GM1

  for (int instrument = 0 ; instrument < 9 ; instrument++) {
    talkMIDI(0xC0, InstrumentLookup[instrument], 0); //Set instrument number. 0xC0 is a 1 data byte command  

    for (int note = 70 ; note < 78 ; note++) {

      //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
      noteOn(0, note, 60);
      delay(50);

      //Turn off the note with a given off/release velocity
      noteOff(0, note, 60);
      delay(50);
    }
  }
}

//Takes the date info from the GPS and prints it to the terminal so that we can see it in the logs
void lookUpDate(void) {

#ifdef ENABLE_GPS
  petDog(PET_GPS); //Pet the dog
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  petDog(PET_GPS); //Pet the dog

  //Correct the day if necessary
  //This is not perfect and rolls backwards poorly
  if(hour < local_hour_offset)
    if(day > 1) day--;

  //Check for daylight savings time
  if(year == 2011) {
    if(month == 3 && day > 12) hour++; //DST begins March 13th
    else if(month > 3 && month < 11) hour++;
    else if(month == 11 && day < 6) hour++; //DST ends November 6th
  }
  if(year == 2012) {
    if(month == 3 && day > 10) hour++; //DST begins March 11th
    else if(month > 3 && month < 11) hour++;
    else if(month == 11 && day < 4) hour++; //DST ends November 4th
  }
  if(year == 2013) {
    if(month == 3 && day > 9) hour++; //DST begins March 10th
    else if(month > 3 && month < 11) hour++;
    else if(month == 11 && day < 3) hour++; //DST ends November 3th
  }

  //Convert UTC hours to local current time using local_hour
  if(hour < local_hour_offset)
    hour += 24; //Add 24 hours before subtracting local offset
  hour -= local_hour_offset;
  //if(hour > 12) hour -= 12; //Get rid of military time
  //We want 24hour time for this project
#endif
}

//Pulls data in from soft serial and 
bool feedgps() {
#ifdef ENABLE_GPS
  while (softGPS.available()) {
    petDog(PET_GPS); //Pet the dog

    if (gps.encode(softGPS.read()))
      return true;
  }
#endif
  return false;
}

//This sends commands to the LS20031 to:
//Reduce it to 9600bps
//Set update rate to 5Hz
//Turn of all sentence but GPGGA
//Good info here: http://www.laptopgpsworld.com/3701-diy-gps-module-using-locosys-ls20031
//Checksum calc here: http://www.hhhh.org/wiml/proj/nmeaxor.html
void setupGPS(void) {
#ifdef ENABLE_GPS
  //Setup soft serial for GPS control
  softGPS.begin(57600);
  delay(100);

  //$PMTK104*37 - resets the unit fully. Use this to put the unit into a known 57600bps/all state

  //softGPS.println("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29"); //Turn off all but GPGGA sentence
  softGPS.println("$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"); //Turn off all but GPRMC sentence
  delay(500);

  softGPS.println("$PMTK251,9600*17"); //Set unit to 9600

  delay(500);

  softGPS.end();

  softGPS.begin(9600);
#endif  
}

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  softMIDI.write(cmd);
  softMIDI.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes 
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if( (cmd & 0xF0) <= 0xB0)
    softMIDI.write(data2);

  digitalWrite(ledPin, LOW);
}


