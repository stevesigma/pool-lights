/*
 Debounce

 Each time the input pin goes from LOW to HIGH (e.g. because of a push-button
 press), the output pin is toggled from LOW to HIGH or HIGH to LOW.  There's
 a minimum delay between toggles to debounce the circuit (i.e. to ignore
 noise).

 The circuit:
 * 
 * pushbutton attached from pin 2 to +5V
 * 10K resistor attached from pin 2 to ground
 */

#include <Wire.h>
#include <Time.h>
#include "DS3231.h"


//  ARDUINO NANO PINOUT:
//            ________________
//           |      NANO      |
//  LED[11] -| DP13      DP12 |- LED[10]
//          -| 3V3       DP11 |- LED[9]
//          -| REF       DP10 |- LED[8]
//  LDR     -| A0         DP9 |- LED[7]
//  PIR     -| A1         DP8 |- LED[6]
//  IR RCV  -| A2         DP7 |- LED[5]
//  BUTTON  -| A3         DP6 |- LED[4]
//  SDA     -| A4 SDA     DP5 |- LED[3]
//  SCL     -| A5 SCL     DP4 |- LED[2]
//  150WSUP -| A6         DP3 |- LED[1]
//  SANDFLT -| A7         DP2 |- LED[0] (12o'clock)
//          -| 5V         GND |- -
//          -| RST        RST |- 
//  -       -| GND        DP0 |- 
//  +       -| VIN        DP1 |- 
//           |________________|
//

// constants won't change. They're used here to
// set pin numbers:
const int LDR_PIN     = A0;	// PHOTORESISTOR
const int PIR_PIN     = A1;	// PIR sensor 
const int IRR_PIN     = A2;	// VS1838 IR Receiver
const int BUTTON_PIN  = A3; // the number of the pushbutton pin
//  A4 i2c SDA
//  A5 i2c SCL
const int LEDSUP_PIN  = A6; // 150W LED SUPPLY
const int FILTER_PIN  = A7; // SAND FILTER

int LED_PIN[] = {2,3,4,5,6,7,8,9,10,11,12,13};	// pin numbers
const int NUM_LEDS = 12;


// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// CH mode
int chStep = 1;
int chLed = 0;
int chDelay = 20;
int chRound = 0;	// count rounds in one direction
unsigned long chLastTime = 0;

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 1000;    // the debounce time; increase if the output flickers

int mode = 0;
int step = 0;

time_t t;



//
// S E T U P
//
void setup() {
  int i, j;
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LEDSUP_PIN, OUTPUT);
  pinMode(FILTER_PIN, OUTPUT);

  // SET PIN MODE TO OUTPUT = RELAYS
  for (i =0; i<NUM_LEDS; i++) {
    pinMode(LED_PIN[i], OUTPUT); // LED i
  }

  updateTime(); // DS3231 function

  // Round trip = turn all leds on one by one
  for (i =0; i<NUM_LEDS; i++) {
    setOnlyLed(i);
    delay(300);
  }

  // Turn leds one by one ten times
  for (j=0; j<10; j++) {
  	for (i =0; i<NUM_LEDS; i++) {
  		digitalWrite(LED_PIN[i],HIGH);
  	}
  	delay(200);
  	for (i =0; i<NUM_LEDS; i++) {
  		digitalWrite(LED_PIN[i],LOW);
  	}
  	delay(500);
  }
}


//
// L O O P
//
void loop() {
	// read the state of the switch into a local variable:
	int reading = analogRead(BUTTON_PIN);
  

	// check to see if you just pressed the button
	// (i.e. the input went from LOW to HIGH),  and you've waited
	// long enough since the last press to ignore any noise:

	// If the switch changed, due to noise or pressing:
	if (reading != lastButtonState) {
		// reset the debouncing timer
		lastDebounceTime = millis();
	}

 	if ((millis() - lastDebounceTime) > debounceDelay) {
 		// whatever the reading is at, it's been there for longer
    	// than the debounce delay, so take it as the actual current state:

		// if the button state has changed:
		if (reading != buttonState) {
			buttonState = reading;

			// only toggle the LED if the new button state is HIGH
			if (buttonState == HIGH) {
				ledState = !ledState;
			}
		}
    
//		digitalWrite(step, LOW);
//		step = step < 11 ? step + 1 : 0;
//		digitalWrite(step, HIGH);

		Serial.println();
	}

	//if (millis() % (1000*60) < (1000

	//chDelay = 500;

	// light f leds in circles vary direction:
  	if ((millis() - chLastTime) > chDelay) {
  		// do a change
  		
      	if (chLed > NUM_LEDS-2 || chLed <0 ) {	// at the end increase chRound
      		chRound++;
      		
      		chLed = (chLed > NUM_LEDS-1 ? -1 : NUM_LEDS-1);
      		if (chRound > 20) {
	      		chStep = -chStep;	// reverse directiion

	      		chDelay = (chDelay < 2000 ? chDelay + 20 : 20);
	      		Serial.println(chDelay);

	      		chRound = 0;
      		}
      	}
      	if ( (chLed + chStep) < NUM_LEDS && (chLed + chStep) >= 0) {
	       	chLed += chStep;
      	} else {
      		chLed = (chStep > 0 ? 0 : NUM_LEDS-1);
      	}
       	
      	setOnlyLed(chLed);
        chLastTime = millis();
  	}

	// save the reading.  Next time through the loop,
	// it'll be the lastButtonState:
	lastButtonState = reading;
}


void displayChase() {
  int i,j;

	setAllLed(LOW); // ALL OFF

  // Chase 2 dots against themselves
  for (j=0;j<10;j++) {
    for (i=0;i<NUM_LEDS;i++) {
    	setAllLed(LOW); // ALL OFF
      digitalWrite(LED_PIN[i],HIGH);                // 0,1,2,3,4,5,6,7,8,9,10,11
      digitalWrite(LED_PIN[abs(NUM_LEDS-i)],HIGH);  // 11,10,9,8,7,6,5,4,3,2,1,0
    }
    delay(500);
  }
  
  // Chase 1, 2, 3, 4, 5, 6 leds around
  
}

//       *
//     .   *
//   .       *
//  .         *
//   *       *
//     *   *
//       *
// Display time: turn on hour leds
//               flash minute led
//
void displayTime() {
  int h, m, i, j;

  t = now();
	h = hour(t) % 12;    // 1-23
	m = minute(t) / 5;  // 00-59
  
	Serial.print(h, DEC);
	Serial.print(":");
	Serial.println(m, DEC);
	

	// Flashing 12 o'clock led will inform, we are displaying time:
	setAllLed(LOW); // ALL OFF
	for (i=0;i<NUM_LEDS;i++) {
    digitalWrite(LED_PIN[0],HIGH); // ON
    delay(100);
    digitalWrite(LED_PIN[0],LOW); // OFF
    delay(100);
	}
	setAllLed(LOW); // ALL OFF
	delay(100);


  // H O U R S = turn all hour leds one by one:
  for (i=1;i<NUM_LEDS;i++) {
    if (i<=h) {
      digitalWrite(LED_PIN[i],HIGH); // ON
    } else {
	    digitalWrite(LED_PIN[i],LOW); // OFF
    }
    delay(500);  // until next hour led
  }
  delay(5000);
	setAllLed(LOW); // ALL OFF
	delay(5000);


  // M I N U T E S = turn all 5-minute leds
  for (i=0;i<NUM_LEDS;i++) {
    if (i<m) {
      digitalWrite(LED_PIN[i],HIGH); // ON
    } else {
	    digitalWrite(LED_PIN[i],LOW); // OFF
    }
    delay(100);  // until next hour led
  }
  delay(1000);
  setAllLed(LOW); // ALL OFF
  delay(1000);
  // Flash the same again at once
  for (j=0;j<5;j++) {
    for (i=0;i<m;i++) {
      digitalWrite(LED_PIN[i],HIGH); // ON
    }
    delay(1000);
    setAllLed(LOW); // ALL OFF
  }
  delay(5000);
  setAllLed(HIGH); // ALL OFF
  delay(1000);
}


//
// Turn them all ON or OFF
//
void setAllLed(boolean on) {
  int i;
  for (i=0;i<NUM_LEDS;i++) {
    digitalWrite(LED_PIN[i],on); // OFF||ON
  }
}

//
// Set all leds OFF except "n", which set ON
//
void setOnlyLed(int n) {
  int i;

  if (n==0) {
  	Serial.println();
  }
  Serial.print(n);
  Serial.print(", ");
  
  for (i=0;i<NUM_LEDS;i++) {
    if (n==i) {
	    digitalWrite(LED_PIN[i],HIGH); // ON
    } else {
	    digitalWrite(LED_PIN[i],LOW); // OFF
    }
  }
  //delay(100);
}

  
  
  
  
  
  
  
  
  
  
