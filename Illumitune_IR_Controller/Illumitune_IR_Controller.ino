/*
 3-30-2011
 Spark Fun Electronics 2011
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 IR is usually something like 40kHz
 
 Potentially build in an average so that if a sensor goes off line (unplugged), the system handles it well.
 
 Without a sensor attached, the system will 'give up' after MAXHITS (50) hits positive. This means that the system
 will begin to ignore a sensor that has run to a rail. But it will not ignore a sensor that is giving random readings.
 
 Gotcha: D13 can't be used as an input because there's a status LED on that pin. Ok for output, not ok for input.
 Removing the current limiting resistor on the main board fixed the problem.
 
 Everything now runs at 38400 because Arduino 21 and Uno don't play well at 38400bps.
 
 IR LEDs can be seen on most smart phone's camera. Use it to test the system!
 
 If a button gets hit more than 20 times in a row, disable it for X minutes? 
 
 From 4/9/11 trip to WOW, channel 4 is giving me fits. There may be an issue where we need to wait some time between 
 checking each channel? Trying to replicate the problem on my desk, tilting the IR receiver slightly out of alignment
 gives similar sorts of crazy output. Could it be an alignment issue? Check that the top IR LED is seated as low as 
 possible. Consider using channel-1 LED because that's a new/fresh LED.
 
 The output looks like: 
 35
 25
 18
 25
 19
 18
 14
 etc
 There must be a way average these and ignore anything less than MINIMUMHITS
 
 When I debug over USB, works great. When I power from main board, it activates 5/6 channels. Chaning the delay
 seems to help. Possibly check alternate channels instead of channels next to each other.
 
 possibly bring 5V USB wall wart to power board
 
 4/17/11: I found out that the lower AC power cords were causing havoc with the IR receivers. Today I replaced all
 the CAT5 cables with STP CAT5. It seems to help only a little. I also added 100uF and 1000uF caps to the IR receiver
 boards. But I still have two channels that are triggering oddly.
 
 Things to do: 
 Add IR out to main board to turn on the amplifier automatically. 
 More documentation on how to fix things if channel breaks (channel triggers on phantom).
 Build more IR LED strings to replace old LEDs.
 Modify more IR receivers.
 (done) Modify the Light controller code to work with new PCB.
 
 Possible IR issues:
 Top lens is dirty. Try removing it.
 Top LED is crooked. Add heat shrink to straighten it.
 Incoming IR signal is wacky. Add external 10k pull-ups. (I'm skipping this idea for now)
 Incoming IR signal is wacky. Bring DSO and look at power and signal lines.
 
 At home: Wait wait! The IR LEDs are being driven at 10mA with 330ohm resistor. Checking datasheet, they can be 
 run up to 50mA! Ahah! A quick test and LED runing at 20mA responds much better. I re-designed the IR controller 
 with NPNs and 71 ohm resistors to run LEDs at max 50mA. It will be a few weeks before I see new PCBs.
 
 I flip the firmware so that it reports 'success' of the IR beam. A larger number is shown when no hand is
 breaking beam.
 
 4-26-2011: Major breakthroughs. There was a bad timing issue in the channel test procedure. The code was written where channels
 7 and 8 where much slower than 1 and 2. With a logic analyzer, everything is the same now. Check wikipedia, looks like
 32 clicks is enough to get the channel to work correctly. It seems to be working very well tonight.
 
 4-25-2012:
 Update to Arduino v1.0. I have not yet pushed this new compiled code to the board.
 */

int irDetector1 = 10; //Left
int irDetector2 = 11; //Right
int irDetector3 = 12;
int irDetector4 = 13;
int irDetector5 = 17;//A3;
int irDetector6 = 16;//A2;
int irDetector7 = 15;//A1;
int irDetector8 = 14;//A0;

int irEmitter1 = 2;
int irEmitter2 = 3;
int irEmitter3 = 4;
int irEmitter4 = 5;
int irEmitter5 = 6;
int irEmitter6 = 7;
int irEmitter7 = 8;
int irEmitter8 = 9;

int StatusLED = A4;

int Hits[10]; //This array holds the number of times a channel is activated in a row
//We use this to disable a channel if it gets too many sequential 'hits'

int MAXHITS = 2; //This is the number of hits before a channel is ignored. 

//32 is the standard RC5 int NUMBEROFCHECKS = 32; //This is the number times we cycle a beam. 
int NUMBEROFCHECKS = 50; //This is the number times we cycle a beam. 
//Increasing this number slows down the system but helps prevent false positive breaks

//int MAXSEEN = 0; //This is the max number that we have to see to consider the beam broken
//Normally the IR detector will report NUMBEROFCHECKS as the number of hits. Any less than that means the beam
//is probably broken. However, I normally reduce the MINIMUMHITS by 5 so the system is a bit more robust

long ThisSecond = 0;

#define TRUE  0
#define FALSE  1
boolean AnnounceHits = FALSE;

void setup() {
  Serial.begin(38400);
  Serial.println("Illumitune IR String Controller v2.0"); 

  pinMode(StatusLED, OUTPUT);

  pinMode(irEmitter1, OUTPUT);
  pinMode(irEmitter2, OUTPUT);
  pinMode(irEmitter3, OUTPUT);
  pinMode(irEmitter4, OUTPUT);
  pinMode(irEmitter5, OUTPUT);
  pinMode(irEmitter6, OUTPUT);
  pinMode(irEmitter7, OUTPUT);
  pinMode(irEmitter8, OUTPUT);

  pinMode(irDetector1, INPUT);
  pinMode(irDetector2, INPUT);
  pinMode(irDetector3, INPUT);
  pinMode(irDetector4, INPUT);
  pinMode(irDetector5, INPUT);
  pinMode(irDetector6, INPUT);
  pinMode(irDetector7, INPUT);
  pinMode(irDetector8, INPUT);

  digitalWrite(irDetector1, HIGH);
  digitalWrite(irDetector2, HIGH);
  digitalWrite(irDetector3, HIGH);
  digitalWrite(irDetector4, HIGH);
  digitalWrite(irDetector5, HIGH);
  digitalWrite(irDetector6, HIGH);
  digitalWrite(irDetector7, HIGH);
  digitalWrite(irDetector8, HIGH);

  //LED max current testing
  /*while(1) {
   digitalWrite(irEmitter6, HIGH);
   digitalWrite(StatusLED, HIGH);
   delay(2000);
   
   digitalWrite(irEmitter6, LOW);
   digitalWrite(StatusLED, LOW);
   delay(2000);
   }*/

  //Turn on all the IR LEDs so that we can see what's going on from a smart phone
  demoIRLEDs();

  //This is a test routine that displays what the IRs are detecting
  //Do not call in production - it is an infinite loop
  //test_IRs();

  //Prefill the hits array - this was needed during testing
  for(int channel = 1 ; channel < 9 ; channel++)
    Hits[channel] = MAXHITS;

  //Let the world know we are online
  Serial.println("Channels indicate IR beam break");
}

void loop() {

  //Blink the status LED every second
  if( (millis()/1000) != ThisSecond) {
    ThisSecond = millis()/1000; //millis() will roll over every 50 days, but it should roll correctly
    if(digitalRead(StatusLED) == 0) 
      digitalWrite(StatusLED, HIGH);
    else
      digitalWrite(StatusLED, LOW);
  }

  //Test the beams
  for(int x = 1 ; x < 9 ; x++) {
    int channel = x;

    //Don't check two channels next to each other
    /*if(x == 1) channel = 1;
     if(x == 2) channel = 5;
     if(x == 3) channel = 2;
     if(x == 4) channel = 6;
     if(x == 5) channel = 3;
     if(x == 6) channel = 7;
     if(x == 7) channel = 4;
     if(x == 8) channel = 8;*/

    if(test_channel(channel) == 0) {
      //Record this hit
      Hits[channel]++;

      //Report this hit
      if(Hits[channel] < MAXHITS)
        AnnounceHits = TRUE; //We now need to report over serial once we're done testing the channels
      else
        Hits[channel] = MAXHITS; //This limits the size of hits to any given channel. Otherwise it'll wrap at 32,000 or there abouts after 30 seconds
    }
    else
      Hits[channel] = 0; //If we detect nothing, then reset the hits to this channel

    delay(1); //We have to have some amount of time in between IR flooding/checking, 1 to 5ms works well
  }

  //See if we have new channel hits to report over serial
  if(AnnounceHits == TRUE) {
    AnnounceHits = FALSE;

    //Wiggle the status LED
    if(digitalRead(StatusLED) == 0) 
      digitalWrite(StatusLED, HIGH);
    else
      digitalWrite(StatusLED, LOW);

    Serial.print("$");
    for(int channel = 1 ; channel < 9 ; channel++)
      if( (Hits[channel] < MAXHITS) && (Hits[channel] > 0) )  Serial.print(channel, DEC); //Report that this channel has been hit!
    Serial.print("#");
    Serial.println();
  }
}

//Tests a given IR channel to see if it is currently broken or clear
//Returns 1 if channel is clear
//Returns 0 if something is breaking the beam

//http://en.wikipedia.org/wiki/RC-5
//We need on for 8us, off for 20us
//We need 32 of these clicks, total of 889us
//Then a pause of 889us
//This will signify bit '1'

//For the illumitune we use 38kHz IR receivers, not 36kHz. Let's tweak to 38kHz.
//We need on for 8us, off for 20us
int test_channel(int channel_number) {

  for(int x = 0 ; x < NUMBEROFCHECKS ; x++) {

    switch(channel_number) {
      case(1): 
      digitalWrite(irEmitter1, HIGH); 
      break;
      case(2): 
      digitalWrite(irEmitter2, HIGH); 
      break;
      case(3): 
      digitalWrite(irEmitter3, HIGH); 
      break;
      case(4): 
      digitalWrite(irEmitter4, HIGH); 
      break;
      case(5): 
      digitalWrite(irEmitter5, HIGH); 
      break;
      case(6): 
      digitalWrite(irEmitter6, HIGH); 
      break;
      case(7): 
      digitalWrite(irEmitter7, HIGH); 
      break;
      case(8): 
      digitalWrite(irEmitter8, HIGH); 
      break;
    }

    //Delay exactly 7.8us, delayMicrosecond() doesn't work for this short of a delay
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

    switch(channel_number) {
      case(1): 
      digitalWrite(irEmitter1, LOW); 
      break;
      case(2): 
      digitalWrite(irEmitter2, LOW); 
      break;
      case(3): 
      digitalWrite(irEmitter3, LOW); 
      break;
      case(4): 
      digitalWrite(irEmitter4, LOW); 
      break;
      case(5): 
      digitalWrite(irEmitter5, LOW); 
      break;
      case(6): 
      digitalWrite(irEmitter6, LOW); 
      break;
      case(7): 
      digitalWrite(irEmitter7, LOW); 
      break;
      case(8): 
      digitalWrite(irEmitter8, LOW); 
      break;
    }

    //Delay exactly 18.5us, delayMicrosecond() isn't accurate enough
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    __asm__("nop\n\t""nop\n\t""nop\n\t");

  }

  if(channel_number == 1) 
    if(digitalRead(irDetector1) == HIGH) return(0); //Beam is broken!
  if(channel_number == 2) 
    if(digitalRead(irDetector2) == HIGH) return(0); //Beam is broken!
  if(channel_number == 3) 
    if(digitalRead(irDetector3) == HIGH) return(0); //Beam is broken!
  if(channel_number == 4) 
    if(digitalRead(irDetector4) == HIGH) return(0); //Beam is broken!
  if(channel_number == 5) 
    if(digitalRead(irDetector5) == HIGH) return(0); //Beam is broken!
  if(channel_number == 6) 
    if(digitalRead(irDetector6) == HIGH) return(0); //Beam is broken!
  if(channel_number == 7) 
    if(digitalRead(irDetector7) == HIGH) return(0); //Beam is broken!
  if(channel_number == 8) 
    if(digitalRead(irDetector8) == HIGH) return(0); //Beam is broken!

  return(1); //Beam is intact

}

//This routine helps debug a single channel
//It's an infinite loop. Don't call in production code
void test_IRs(void) {

  while(1) {
    Serial.print("Beams: ");

    for(int channel_number = 1 ; channel_number < 9 ; channel_number++) {

      for(int x = 0 ; x < NUMBEROFCHECKS ; x++) {

        switch(channel_number) {
          case(1): 
          digitalWrite(irEmitter1, HIGH); 
          break;
          case(2): 
          digitalWrite(irEmitter2, HIGH); 
          break;
          case(3): 
          digitalWrite(irEmitter3, HIGH); 
          break;
          case(4): 
          digitalWrite(irEmitter4, HIGH); 
          break;
          case(5): 
          digitalWrite(irEmitter5, HIGH); 
          break;
          case(6): 
          digitalWrite(irEmitter6, HIGH); 
          break;
          case(7): 
          digitalWrite(irEmitter7, HIGH); 
          break;
          case(8): 
          digitalWrite(irEmitter8, HIGH); 
          break;
        }

        //Delay exactly 7.8us, delayMicrosecond() doesn't work for this short of a delay
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");

        switch(channel_number) {
          case(1): 
          digitalWrite(irEmitter1, LOW); 
          break;
          case(2): 
          digitalWrite(irEmitter2, LOW); 
          break;
          case(3): 
          digitalWrite(irEmitter3, LOW); 
          break;
          case(4): 
          digitalWrite(irEmitter4, LOW); 
          break;
          case(5): 
          digitalWrite(irEmitter5, LOW); 
          break;
          case(6): 
          digitalWrite(irEmitter6, LOW); 
          break;
          case(7): 
          digitalWrite(irEmitter7, LOW); 
          break;
          case(8): 
          digitalWrite(irEmitter8, LOW); 
          break;
        }

        //Delay exactly 18.5us, delayMicrosecond() isn't accurate enough
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t");
        __asm__("nop\n\t""nop\n\t""nop\n\t");

      }

      //Show what we found
      Serial.print(channel_number, DEC);
      Serial.print(":");

      if(channel_number == 1)
        if(digitalRead(irDetector1) == HIGH) 
          Serial.print("--");
        else
          Serial.print("00");
      if(channel_number == 2)
        if(digitalRead(irDetector2) == HIGH) 
          Serial.print("--");
        else
          Serial.print("00");
      if(channel_number == 3)
        if(digitalRead(irDetector3) == HIGH) 
          Serial.print("--");
        else
          Serial.print("00");
      if(channel_number == 4)
        if(digitalRead(irDetector4) == HIGH) 
          Serial.print("--");
        else
          Serial.print("00");
      if(channel_number == 5)
        if(digitalRead(irDetector5) == HIGH) 
          Serial.print("--");
        else
          Serial.print("00");
      if(channel_number == 6)
        if(digitalRead(irDetector6) == HIGH) 
          Serial.print("--");
        else
          Serial.print("00");
      if(channel_number == 7)
        if(digitalRead(irDetector7) == HIGH) 
          Serial.print("--");
        else
          Serial.print("00");
      if(channel_number == 8)
        if(digitalRead(irDetector8) == HIGH) 
          Serial.print("--");
        else
          Serial.print("00");

      Serial.print(" ");
    }
    Serial.println();
    
    //delay(10);
  }
}

#define ON HIGH
#define OFF LOW
void demoIRLEDs(void) {
  Serial.println("IRs are demoing - break out your smart phone to see");

  //Turn on all LEDs
  digitalWrite(irEmitter1, ON);
  digitalWrite(irEmitter2, ON);
  digitalWrite(irEmitter3, ON);
  digitalWrite(irEmitter4, ON);
  digitalWrite(irEmitter5, ON);
  digitalWrite(irEmitter6, ON);
  digitalWrite(irEmitter7, ON);
  digitalWrite(irEmitter8, ON);

  //Use this to turn the LEDs on permanently so that you can inspect each one with a smart phone
  //Serial.println("LEDs always on");
  //while(1);
  delay(1000);

  //Turn off all LEDs
  digitalWrite(irEmitter1, OFF);
  digitalWrite(irEmitter2, OFF);
  digitalWrite(irEmitter3, OFF);
  digitalWrite(irEmitter4, OFF);
  digitalWrite(irEmitter5, OFF);
  digitalWrite(irEmitter6, OFF);
  digitalWrite(irEmitter7, OFF);
  digitalWrite(irEmitter8, OFF);

  //Turn on one LED to verify the channel is under control
  //digitalWrite(irEmitter4, ON);
  //while(1);

  //This is only for debug! Do not uncomment

  //Toggle each LED
  /*
#define QUICKWAIT  1000
   while(1) { //Infinite loop so that you can see the LEDs under individual control
   digitalWrite(irEmitter1, ON);
   delay(QUICKWAIT);
   digitalWrite(irEmitter1, OFF);
   
   digitalWrite(irEmitter2, ON);
   delay(QUICKWAIT);
   digitalWrite(irEmitter2, OFF);
   
   digitalWrite(irEmitter3, ON);
   delay(QUICKWAIT);
   digitalWrite(irEmitter3, OFF);
   
   digitalWrite(irEmitter4, ON);
   delay(QUICKWAIT);
   digitalWrite(irEmitter4, OFF);
   
   digitalWrite(irEmitter5, ON);
   delay(QUICKWAIT);
   digitalWrite(irEmitter5, OFF);
   
   digitalWrite(irEmitter6, ON);
   delay(QUICKWAIT);
   digitalWrite(irEmitter6, OFF);
   
   digitalWrite(irEmitter7, ON);
   delay(QUICKWAIT);
   digitalWrite(irEmitter7, OFF);
   
   digitalWrite(irEmitter8, ON);
   delay(QUICKWAIT);
   digitalWrite(irEmitter8, OFF);
   }
   */
}

