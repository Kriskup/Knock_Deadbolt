#include <Servo.h>
Servo myservo;

// set pin numbers:
const int knock = A0;        // piezoelectric
const int buttonPin1 = 8;    // the number of the pushbutton pin
const int buttonPin2 = 2;    // the "remote"
const int ledPin1 = 9;       // the number of the LED pins
const int ledPin2 = 3;
const int ledPin3 = 4;
const int ledR = 5;
const int ledG = 6;
const int ledB = 7;

// set integers
const int threshold = 5;     //knock threshold in 10 bit 0-5 volts
const int fade = 50;          // time for knock signal to zero
const int tollerance = 80;    // tempo tollerance in ms
const int maxLength = 17;     // max num of knock timings ie. knock number -1
const int knockLimit = 11000;  // length of knock password in seconds

// variables will change:
int pos = 0;                 // servo position locked = 100: unlocked = 5
int lockstate = 0;           // locked = 0: unlocked = 1
int speakerVolt = 0;
int buttonState1 = 0;        // variable for reading the pushbutton status
int buttonState2 = 0;
int tArray[maxLength];       // array from 0 to 17
int count = 0;               // counts position on array
int easterEgg = 0;
int gameOver = 0;
unsigned long limit;         // wait for knock value
unsigned long time;          // knock timing value
long security;

void setup() {
  // set the LED pins as outputs
  // the for() loop saves some extra coding
  for (count = 3; count < 8; count++) {
    pinMode(count, OUTPUT);
    digitalWrite(count, LOW);
  }
  pinMode(ledPin1, OUTPUT);
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledR, HIGH);
  myservo.attach(10);           // attaches the servo on pin 10 to the servo object
  pinMode(10, OUTPUT);
  // initialize the pushbutton pins as an input:
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  Serial.begin(9600);       // use the serial port
  myservo.write(160);
  myservo.write(140); 
  randomSeed(analogRead(1));
//  attachInterrupt(0, remote, FALLING); // interupt would not play tone
                    // changed to if statement
}


void loop() {
  // read the state of the pushbutton value:
  buttonState1 = digitalRead(buttonPin1);
  // check if the pushbutton is pressed:
  // if it is, the buttonState is HIGH:
  if (buttonState1 == HIGH) {          // record password loop
    digitalWrite(ledPin1, HIGH);       // turn LED1 on:
    recordPassword();                  // Save knock array password
  }
  else {
    digitalWrite(ledPin1, LOW);        // turn LED1 off
    buttonState2 = digitalRead(buttonPin2);
    if (buttonState2 == HIGH) { remote(); }
    else {
      if (lockstate == 0) {
        testPassword();                  // check knock password
      }
      else {
        lockKnock();
      }
    }
  }
}

void recordPassword() {
  for (count=0; count<maxLength; count++) {
    tArray[count] = 0;
  }
  count = 0;        //Reset counter and array with button push
  easterEgg = 0;
  limit = millis();
  Serial.println("Record Password");
  while (millis()-limit < knockLimit) {
    // wait for all knocks
    speakerVolt = analogRead(knock);
    buttonState1 = digitalRead(buttonPin1);
    if (buttonState1 == HIGH && millis()-limit > 1000) {
      limit = -knockLimit;
      while (buttonState1 == HIGH) {
        buttonState1 = digitalRead(buttonPin1);
      }
    }
    // if the sensor reading is greater than the threshold:
    else if (speakerVolt > threshold) {
      Serial.print("Knock");
      if (count == 0) {
        // first knock begins timer
        time = millis();
        Serial.println();
      }
      // for remaining knocks record timings
      else if (count < maxLength) {
        tArray[count-1] = millis() - time;  // save time from previous knock
        time = millis();                    // reset timer
        Serial.print(" - ");
        Serial.print(count);
        Serial.print(" - (time)");
        Serial.println(tArray[count-1]);
      }
      //LED 2 turns on for innactivity period, also indicating knock
      digitalWrite(ledPin2, HIGH);
      delay(fade);
      digitalWrite(ledPin2, LOW);
      count ++;
    }
  }
  Serial.println("Password Recorded");
  for (count=0; count<maxLength; count++) {
    Serial.print(tArray[count]);
    Serial.print("-");
  }
  gameOver = 0;
  Serial.println();
  Serial.println();
}

void testPassword() {
  speakerVolt = analogRead(knock);
  // if the sensor reading is greater than the threshold:
  if (speakerVolt > threshold) {
    // knock detected
    Serial.println("Knock Check");
    time = millis();        // reset timers for both timing and knock time limit
    limit = millis();
    digitalWrite(ledPin3, HIGH);
    // turn on knock detection LED
    if (tArray[0] == 0) {        // if no knock password set, don't open door
      count = maxLength + 1;      // maxLength + 1 results in a fail after while loop
    }
    else { count = 0;}
    // if values are recorded reset counter
    //LED 2 turns on for innactivity period, also indicating knock
    digitalWrite(ledPin2, HIGH);
    delay(fade);
    digitalWrite(ledPin2, LOW);
    // time limit avoids staying stuck in while after less than required knocks entered
    security = random(2*tollerance, 3000);
    // random number prevents being able to guess next timing through time system times
    // out from previous knock
      
    while (count < maxLength) {
      // waits for knocks. Stops for pass or fail
      // pass is count = maxLength, fail is count = maxLength + 1
      if (millis() - time > tArray[count] + security) {
//        Serial.print("(count)");
//        Serial.print(count);
//        Serial.print(" - (array)");
//        Serial.print(tArray[count]);
        count = maxLength + 1;
//        Serial.print(" - (security)");
//        Serial.println(security);
      }
      else {
        if (tArray[count] != 0) {
          // see else
          speakerVolt = analogRead(knock);
          // if the sensor reading is greater than the threshold:
          if (speakerVolt > threshold) {
            int a = millis() - time;  // space saver
            if (a < tArray[count] + tollerance && a > tArray[count] - tollerance) {
              time = millis();
              count ++;
              security = random(2*tollerance, 3000);  // new random timeout after each knock
            }
            else {
              count = maxLength + 1;  // fail
            }
            //LED 2 turns on for innactivity period, also indicating knock
            digitalWrite(ledPin2, HIGH);
            delay(fade);
            digitalWrite(ledPin2, LOW);
            Serial.print("Knock - (time)");
            Serial.println(a);  // time of knock
          }
        }
        else {    // if next entry is a 0 password is correct YAY!
          time = millis();
          while (millis() - time < 1000) {    // wait for additional knocks
            speakerVolt = analogRead(knock);
            // if the sensor reading is greater than the threshold:
            if (speakerVolt > threshold) {
              // if extra knocks are found pass is wrong
              count = maxLength + 1;
              time = 0;  // exits while loop
              //LED 2 turns on for innactivity period, also indicating knock
              digitalWrite(ledPin2, HIGH);
              delay(fade);
              digitalWrite(ledPin2, LOW);
            }
            else {
              count = maxLength;    // correct password
            }
          }
        }
      }
    }
    digitalWrite(ledPin3, LOW);
    if (count == maxLength) {
      Serial.println("You unlocked the door!");
      Serial.println();
      easterEgg++;
      gameOver = 0;
      unlockDoor();
    }
    else if (count == maxLength + 1) {
      Serial.println("AH AH AH, YOU DIDN'T SAY THE MAGIC WORD!");
      Serial.println();
      easterEgg = 0;
      gameOver++;
      error();
    }
  }
}

void lockKnock() {
  speakerVolt = analogRead(knock);
  // if the sensor reading is greater than the threshold:
  if (speakerVolt > threshold) {
    // knock detected
    Serial.println("Lock Door?");
    digitalWrite(ledPin2, HIGH);
    delay(fade);
    digitalWrite(ledPin2, LOW);
    time = millis();        // reset timing
    count = 0;
    while (count < 1) {
      if (millis() - time < 1500) {
        speakerVolt = analogRead(knock);
        // if the sensor reading is greater than the threshold:
        if (speakerVolt > threshold) {
          digitalWrite(ledPin2, HIGH);
          delay(fade);
          digitalWrite(ledPin2, LOW);
          lockDoor();
          count = 1;
          Serial.println("Door Locked");
          Serial.println();
        }
      }
      else {
        count = 1;
        Serial.println("Door not Locked");
        Serial.println();
      }
    }
  }
}

void remote() {
  if (lockstate == 0){
    unlockDoor();
    Serial.println("Door Unlocked");
    Serial.println();
  }
  else {
    lockDoor();
    Serial.println("Door Locked");
    Serial.println();
  }
}

void lockDoor() {
  int melody[] = {784, 330 };
  int noteDurations[] = {8, 8 };
  for (int note = 0; note < 3; note ++) {    // play notes
    tone(12, melody[note], 1000/noteDurations[note]);
    delay(1300/noteDurations[note]);
  }
  myservo.write(160);         // move servo
  myservo.write(140); 
//  delay (500);
  digitalWrite(ledR,HIGH);    // changes multicolor LED to red
  digitalWrite(ledG,LOW);
  lockstate = 0;
}

void unlockDoor() {
  if (easterEgg < 4) {
    int melody[] = {659, 784, 1319, 1047, 1175, 1568 };
    int noteDurations[] = {10,8,10,10,10,10};
    for (int note = 0; note < 6; note ++) {    // play notes
      tone(12, melody[note], 1000/noteDurations[note]);
      delay(1300/noteDurations[note]);
    }
  }
  else {    // Play Rockie theme
    int melody[] = {131,131,131,131,131,165,131,131, 165,165,165,165,165,196,165,165,
            165,165,165,165,165, 175,165,165,165,165,165, 165,165,196,220,220,196,220,247,
            165, 165,165,165,196,220,220,196,220,247,165, 147,131,147,131,147,165,
            131,131,123,123,110,110,98,175,165};
    int noteDurations[] = {4,8,8,4,8,4,8,2, 4,8,8,4,8,4,8,2, 
                          8,8,8,8,2, 2,8,8,8,8,8, 8,8,4,1,8,8,8,4,1,
                          8,8,8,4,1,8,8,8,4,1, 8,8,4,8,8,2,
                          4,8,4,8,4,8,4,8,1};
    easterEgg = 0;
    for (int note = 0; note < 61; note ++) {    // play notes
      tone(12, melody[note], 1000/noteDurations[note]);
      delay(1300/noteDurations[note]);
    }
  }
  myservo.write(15);          // move servo
//  delay (500);
  digitalWrite(ledR,LOW);    // changes multicolor LED to green
  digitalWrite(ledG,HIGH);
  lockstate = 1;
}

void error() {
  if (gameOver == 5 && tArray[0] != 0) {    // Game over
    int melody[] = {262,0,196,0,165,220,247,220,208,233,208,196};
    float noteDurations[] = {4,2,4,2,2,3,4,3,3,2,1.5,1};
    for (int note = 0; note < 11; note ++) {    // play notes
      tone(12, melody[note], (1000/noteDurations[note])/(1+note/10));
      delay((1300/noteDurations[note])/(1+note/10));
    }
    tone(12,196,1000);
    for (count=0; count<maxLength; count++) {
      tArray[count] = 0;
    }
    gameOver = 0;
  }
  else if (tArray[0] == 0) {
    tone(12,1109,500);
  }
  else {
    tone(12, 131, 1000/4);
  }
}
