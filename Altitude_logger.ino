
/*----------------------------------------------------------------------
    Altitude Logger by Thornhill
  ------------------------------------------------------------------------
  Altitude logger using the Adafruit_MPL3115A2 barometric pressure sensor.
  Will display the maximum height recorded during active operation via a
  series of LED flashes. Base altitude is recorded & set via a long button press.
  Maximum height is displayed with a short button press, where the LEDs will
  cycle through their sequence once. Maximum height is reset during the
  recording of the base altitude.

  4 LEDs represent thousands, hundreds, tens, and ones. During readout the LEDs
  will flash according to the number of thousands, hundred, tens, and ones in
  that reading.

  e.g: a height of 415 m will equal:
      Led1(0 flashes), Led2(4 flashes), Led3(1 flash), Led4(5 flashes)

      
  ------------------------------------------------------------------------

 *** Attribution/References ***

  Uses a modified library for the Adafruit MPL3115A2
  (https://github.com/adafruit/Adafruit_MPL3115A2_Library), incorperating a way to
  calibrate the sensor based on sea level pressure for that particular day
  (https://github.com/adafruit/Adafruit_MPL3115A2_Library/pull/4).

  ----------------------------------------------------------------------*/

#include <Wire.h>
#include <Adafruit_MPL3115A2.h>

Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

#define SeaPressure 1021.56 //Sea Level pressure (pascals) for that particular day
#define height_limit 120 // ~ 400ft limit
#define BUTTON  3 //button pin


float max_altm; //max altitude
float base_altm; //the base altitude
int height = 0;

byte LED_array[] = {12, 11, 10, 9}; //thou,hund,ten,one
byte counter = 0;
byte flashes = 0; //how many times to flash the LED
int ledState = LOW;

volatile boolean short_press = false;
volatile boolean long_press = false;

unsigned long firstTime;
int current;
byte previous = HIGH;

void setup() {
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);

  Serial.begin(9600);

  baro.setSeaPressure (SeaPressure); //set Sea Level pressure for calibration
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(1, interrupt, CHANGE);
}

void loop() {

  if (! baro.begin()) {
    Serial.println("Couldnt find sensor");
    return;
  }

  float altm = baro.getAltitude();

  //Mostly for debugging
  /*
    Serial.print("alt:"); Serial.println(altm);
    Serial.print("max:");Serial.println(max_altm);
    Serial.print("base:"); Serial.println(base_altm);
    Serial.println("------------");
    float tempC = baro.getTemperature();
    Serial.print(tempC); Serial.println("*C");
    Serial.println("------------");*/

  //keep hold of the highest altitude recorded
  if (altm > max_altm) {
    max_altm = altm;
  }
  else {
    max_altm = max_altm;
  }

  height = (int) max_altm - (int) base_altm; //height based on max altm recorded
  height = (int) max_altm - (int) base_altm; //get the altitude from ground

  //Height limit
  //Turn on LED to show we've reached the height limit
  if ((altm - base_altm)  <= height_limit) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }

  //Display the height
  if (short_press == true) {
    flash(height);
    short_press = ! short_press;
  }

  //Reset & set base altitude
  else if (long_press == true) {
    height = 0; //reset that height
    base_altm = altm; //make the current altitude the base altitude
    max_altm = altm;
    LED_confirm(500);
    long_press = ! long_press;
  }


  base_altm = base_altm;
  previous = current;
}//END LOOP


/*-------------------------------
   LEDs - Confirm flash
  -------------------------------*/
//Turn all the LEDs on to show confirmation
void LED_confirm(int delay_interval) {
  byte thisPin;
  byte confirm_counter = 0;
  while (confirm_counter < 1) {
    for (thisPin = 9; thisPin <= 12; thisPin++) {
      digitalWrite(thisPin, HIGH);
    }
    delay(delay_interval);
    for (thisPin = 9; thisPin <= 12; thisPin++) {
      digitalWrite(thisPin, LOW);
    }
    confirm_counter++;
  }

}

/*-------------------------------
   LEDs - Flash height
  -------------------------------*/

void flash(int height) {

  if (height == 0) {
    LED_confirm(200);
  }

  byte thousands = height / 1000;
  byte hundreds = (height % 1000) / 100;
  byte tens = (height % 100) / 10;
  byte ones = (height % 10) / 1;

  byte flash_array[] = {thousands, hundreds, tens, ones};

  for (byte i = 0; i < 4; i++) {
    flashes = flash_array[i];

    while (counter < flashes) {

      digitalWrite(LED_array[i], HIGH);
      delay(400);
      digitalWrite(LED_array[i], LOW);
      delay(400);
      counter++;
    }
    counter = 0;
  }
}

/*-------------------------------
          Interrupt
  -------------------------------*/

void interrupt() {

  current = digitalRead(14);

  // if the state changes to low signal, remember the start time
  if (current == LOW && previous == HIGH) {
    firstTime = millis();
  }

  int millis_held = (millis() - firstTime);

  //if (current == HIGH && previous == LOW) {
  if (millis_held > 20) {
    if (millis_held < 500) {
      short_press = true;
      long_press = false;
    }
    if (millis_held > 500) {
      short_press = false;
      long_press = true;
    }
  }
  millis_held = 0;
}




