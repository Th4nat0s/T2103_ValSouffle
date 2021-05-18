/*
____   ____      .__      _________              _____  _____.__
\   \ /   /____  |  |    /   _____/ ____  __ ___/ ____\/ ____\  |   ____
 \   Y   /\__  \ |  |    \_____  \ /  _ \|  |  \   __\\   __\|  | _/ __ \
  \     /  / __ \|  |__  /        (  <_> )  |  /|  |   |  |  |  |_\  ___/
   \___/  (____  /____/ /_______  /\____/|____/ |__|   |__|  |____/\___  >
               \/               \/                                     \/
                    Copyleft Thanat0s Since march 2021

Code for Atmega328p microcontroller.
https://github.com/Th4nat0s/T2103_ValSouffle

Greets fly to SMHOSY, Entercar68

                  ATMEL Atmega328p / (ARDUINO Uno)

                             +-\/-+
            RST        PC6  1|    |28  PC5  (A5)    SCL
            Rx  (D0 )  PD0  2|    |27  PC4  (A4)    SDA
            Tx  (D1 )  PD1  3|    |26  PC3  (A3)
                (D2 )  PD2  4|    |25  PC2  (A2)
    PWM T2      (D3~)  PD3  5|    |24  PC1  (A1)
                (D4 )  PD4  6|    |23  PC0  (A0)
                       VCC  7|    |22  GND
                       GND  8|    |21  AREF
         Xtal1  (   )  PB6  9|    |20  AVCC
         Xtal2  (   )  PB7 10|    |19  PB5  (D13 )  SCK
        PWM T0  (D5~)  PD5 11|    |18  PB4  (D12 )  MISO
        PWM T0  (D6~)  PD6 12|    |17  PB3  (D11~)  MOSI PWM T2
                (D7 )  PD7 13|    |16  PB2  (D10~)       PWM T1
                (D8 )  PB0 14|    |15  PB1  (D 9~)       PWM T1
                             +----+

                 20 mA per pin / 200 mA overall

*/

#include <Arduino.h>

/*
____   ____            .__      ___.   .__
\   \ /   /____ _______|__|____ \_ |__ |  |   ____   ______
 \   Y   /\__  \\_  __ \  \__  \ | __ \|  | _/ __ \ /  ___/
  \     /  / __ \|  | \/  |/ __ \| \_\ \  |_\  ___/ \___ \
   \___/  (____  /__|  |__(____  /___  /____/\___  >____  >
               \/              \/    \/          \/     \/
*/

// Connections to the  74HC595 ( 8 Bit led driver )
const int latchPin = 8;  // PB0 connected to ST_CP
const int clockPin = 12;   // PB4 connected to SH_CP
const int dataPin = 11;   // PB3 connected to DS

// Connections to the leds
const int ledh1 = 7;   //  PD7 connected to Bit 9 - Led
const int ledh2 = 13;   //  PB5 connected to Bit 10 - Led
const int pfft = 0;  // PD0 connection to pffft led.

// Input pins
const int potPin = 0;     // PC2 selector, input pin for the potentiometer
const int ButtonPin = 3;  // PD3, manual blower button

// Output pins
const int t_pfft = 4;  // PD4 connection to the blower driver.

// Variables will change:
int but_pushed;
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button


unsigned int val = 0;       // variable to store the value coming from the sensor
//float n = 0;
int bit;
bool bith1;
bool bith2;

unsigned long action = millis();

int pwr_slct;  // <- Int = 16 Bytes. 

// Valeurs Min/Max Potar (For 10K varistor)
unsigned int max_pot = 700; //1024 ;// 877;
unsigned int min_pot = 26;
unsigned int borne_max;
unsigned int borne_min;


// Configuration for the delays of the blower...
// Timers are in milliseconds.. 1sec = 1000ms
// 0 is special it means OFF !
//                Position  0    1       2      3      4      5     6     7     8    9   10
unsigned int pfftwait[] =  {0, 60000, 30000, 25000, 20000, 10000, 5000, 2000, 1000, 500 ,250};
// Time to Pfft in millseconds..
unsigned int pffttime[] = {0,  3000,  3000,  3000,  3000,  1000,  1000, 500,  500,  500, 250};


int pffstatus = 0; // status of the blower 0 Sleeping... 1 pffting...
unsigned int cpfftwait; // current wait selected time
unsigned int cpffttime; //


/*
.___       .__  __
|   | ____ |__|/  |_
|   |/    \|  \   __\
|   |   |  \  ||  |
|___|___|  /__||__|
         \/
*/

void setup() {
  //Start Serial for debuging purposes
  Serial.begin(9600);
  // Set various pins to in or output
  pinMode(latchPin, OUTPUT);
  pinMode(ledh1, OUTPUT); // Led 9
  pinMode(ledh2, OUTPUT);  // Led 10
  pinMode(pfft, OUTPUT);   // Led showing blow
  pinMode(t_pfft, OUTPUT);  // Transistor output for Blowing
  pinMode(ButtonPin, INPUT);  // Input button
  digitalWrite(pfft, 1);  // Active le pfft.
  delay(1000);
  digitalWrite(pfft, 0);  // Active le pfft.
  // Calcul des min / max potar.
  borne_max = (((max_pot - min_pot) / 11) * 10) + min_pot;
  borne_min = ((max_pot - min_pot) / 11) + min_pot;
}

void readbutton() {
  // read the pushbutton input pin update the status "but_pushed":
  buttonState = digitalRead(ButtonPin);
  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
     buttonPushCounter++;
     but_pushed = true;
    } else {
      // if the current state is LOW then the button went from on to off:
      but_pushed = false;
    }
  }
  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;
}

void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first,
  //on the rising edge of the clock,
  //clock idles low
  //internal function setup
  int i = 0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);
  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);
  //for each bit in the byte myDataOut&#xFFFD;
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights.
  for (i = 7; i >= 0; i--)  {
    digitalWrite(myClockPin, 0);
    //if the value passed to myDataOut and a bitmask result
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1 << i) ) {
      pinState = 1;
    }
    else {
      pinState = 0;
    }
    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }
  //stop shifting
  digitalWrite(myClockPin, 0);
}

/*
   _____         .__
  /     \ _____  |__| ____
 /  \ /  \\__  \ |  |/    \
/    Y    \/ __ \|  |   |  \
\____|__  (____  /__|___|  /
        \/     \/        \/
*/

void loop() {
  val = analogRead(potPin);    // read the value from the rotary
  readbutton();  // read the value from the button.

  // Convertis la valeur du potar en 11 Positions possibles.
  pwr_slct = (val - min_pot ) / ((max_pot - min_pot) / 11) ;

  Serial.println();
  Serial.print ( "Pwr Select: ");
  Serial.print (pwr_slct);


  // If Min or Max
  if (val < borne_min)  pwr_slct = 0 ;
  if (val >= borne_max)  pwr_slct = 10 ;

  // Convert power selected in LED bits.
  bit = 0; 
  for (int i = pwr_slct-1; i >= 0; i--)  {  // Down count !
    bit = bit | (1 << i);
  }

  // Select value for bit 9 & 10..
  bith1 = (bit >> 8) & 0x1;
  bith2 = (bit >> 9) & 0x1;
  bit = bit & 0xff;  // Push bit back to 8 Bits before printing.

  // Mets à jour l'affichage des leds.
  digitalWrite(latchPin, 0);
  shiftOut(dataPin, clockPin, bit);
  digitalWrite(latchPin, 1);
  digitalWrite(ledh1, bith1);  // Led 9 and 10
  digitalWrite(ledh2, bith2);

  // Determine time of sleeping and pffting based on power selected.
  cpffttime = pffttime[pwr_slct];
  cpfftwait = pfftwait[pwr_slct];

  Serial.print ( "val: ");
  Serial.print (cpffttime);
  Serial.print(" button:  ");
  Serial.print(but_pushed);


  unsigned long t = millis();  // Get time. 
  // If power is to 0 ( stop )... do nothing.
  if (pwr_slct == 0 ) {
      digitalWrite(pfft, 0);  // deActive le pfft.
      digitalWrite(t_pfft, 0);  // deActive le pfft.
      pffstatus = 0;
  } else {
    // if not to 0... 
    // Ovverride avec le bouton blow on off
    if (but_pushed == 0) {
      digitalWrite(pfft, 1);  // Active le pfft.
      digitalWrite(t_pfft, 1);  // Active le pfft.
      pffstatus = 1;
      action = t + action ; // Invalidate timer.
      } else {

      // if not position 0 and button released then... look a time
      // Wait and Loop.
      // If power select is <> of 0...
      if (pffstatus == 1 and pwr_slct != 0 ) {
        // Wait le temps qu'il faut.
        if ( ( t - action ) > cpffttime ) {
          action = t;  // Schedule next action
          Serial.print("Wait");  
          digitalWrite(pfft, 0);  // Stop le pfft.
          digitalWrite(t_pfft, 0);  // Stop le pfft.
          pffstatus = 0;    // Set le status on pffft pas.
        }
      } else if (pwr_slct != 0) {
        // Souffle le temps qu'il faut.
        if ( ( t - action ) > cpfftwait  ) {
          action = t;
          Serial.print("Souffle");  
          digitalWrite(pfft, 1);  // Active le pfft.
          digitalWrite(t_pfft, 1);  // Active le pfft.
          pffstatus = 1;    // set le status on pfuiiiit 
        }
      }
    }
  }
}

