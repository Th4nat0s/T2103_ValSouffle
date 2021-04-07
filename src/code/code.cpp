
// Connections 
//Pin connected to ST_CP of 74HC595
int latchPin = 8;  // PB0
//Pin connected to SH_CP of 74HC595
int clockPin = 12;   // PB4
//Pin connected to DS of 74HC595
int dataPin = 11;   // PB3
int potPin = 2;     // PC2   // select the input pin for the potentiometer
// Bit 9 - Led
int ledh1 = 7;   //  PD7
// Bit 10 - Led
int ledh2 = 13;   //  PB5



int val = 0;       // variable to store the value coming from the sensor
//float n = 0;
int bit;
boolean bith1;
boolean bith2;


int pwr_slct;  // <- Int = 16 Bytes. 

// Valeurs Min/Max Potar (For 10K varistor)
int max_pot = 877;
int min_pot = 26;
int borne_max;
int borne_min;


void setup() {
  //Start Serial for debuging purposes
  Serial.begin(9600);
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(ledh1, OUTPUT);
  pinMode(ledh2, OUTPUT);

  borne_max = (((max_pot - min_pot) / 11) * 10) + min_pot;
  borne_min = ((max_pot - min_pot) / 11) + min_pot;
}


void loop() {
  val = analogRead(potPin);    // read the value from the rotary

  // Convertis la valeur du potar en 11 Positions possibles.
  pwr_slct = (val - min_pot ) / ((max_pot - min_pot) / 11) ;
  
  Serial.print ( "Pwr Select: ");
  Serial.println (pwr_slct);

  if (val < borne_min)  pwr_slct = 0 ;
  if (val >= borne_max)  pwr_slct = 10 ;
  
  // Convert power selected in LED bits.
  bit = 0; 
  for (int i = pwr_slct-1; i >= 0; i--)  {
    bit = bit | (1 << i);
  }

  // Select bit 9 & 10..
  bith1 = (bit >> 8) & 0x1;
  bith2 = (bit >> 9) & 0x1;
  
  bit = bit & 0xff;  // Push bit back to 8 Bits. 

  // Mets à jour l'affichage.
  digitalWrite(latchPin, 0);
  shiftOut(dataPin, clockPin, bit);
  digitalWrite(latchPin, 1);
  digitalWrite(ledh1, bith1);  // Led 9 and 10
  digitalWrite(ledh2, bith2);
  // Wait and Loop.
  delay(50);
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
