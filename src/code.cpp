//Pin connected to ST_CP of 74HC595
int latchPin = 8;  // PB0
//Pin connected to SH_CP of 74HC595
int clockPin = 12;   // PB4
//Pin connected to DS of 74HC595
int dataPin = 11;   // PB3

int potPin = 2;     // PC2   // select the input pin for the potentiometer
int val = 0;       // variable to store the value coming from the sensor

float n = 0;
int bit;
boolean bith1;
boolean bith2;
// Bit 9
int ledh1 = 7;   //  PD7
// Bit 10
int ledh2 = 13;   //  PB5

void setup() {
//Start Serial for debuging purposes
  Serial.begin(9600);
//set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  pinMode(ledh1, OUTPUT);
  pinMode(ledh2, OUTPUT);
}


void loop() {
     val = analogRead(potPin);    // read the value from the rotary
     n = val / 100;  // convert max 1024 to 0 - 10.
     
     bit = 1;
     // Calcul combien de bits de 1 à 10 il faut.
     n = pow(2, n);
     while ( n > 2 ) {
         n = n / 2;
         bit++;
     }
     
     Serial.println(bit);

    // les Bit 9 et 10 sont directement sur l'arduino.
    // Prise en main "manuellele".
    // surement faisable avec des shift et and. à revoir.
    if (bit == 9) {
      bit = 8;
      bith1 = true;
      bith2 = false; 
    } else if (bit == 10) {
      bit = 8;
      bith1 = true;
      bith2 = true; 
    } else {
      bith1 = false;
      bith2 = false;
    }

    // détermine quel chiffre afficher 0x00 to 0xFF
    // en fonctions des bits de 1 à 7. 
    if (bit > 1){
      bit=pow(2, bit);
    }

    // Mets à jour l'affichage.
    digitalWrite(latchPin, 0);
    shiftOut(dataPin, clockPin, bit);
    digitalWrite(latchPin, 1);
    digitalWrite(ledh1, bith1);
    digitalWrite(ledh2, bith2);

    // Wait and Loop.
    delay(100);
}

void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first,
  //on the rising edge of the clock,
  //clock idles low
  //internal function setup
  int i=0;
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
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);
    //if the value passed to myDataOut and a bitmask result
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {
      pinState= 0;
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
