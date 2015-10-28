
#define LIN_OUT8 1 // pour linaire output 8b
#define SCALE 4 // 
#define LOG_OUT 0 // use the log output function
#define FHT_N 128 // set to 256 point fht

#include <fht.h> // include the library
//------------------------------------
// PIN SETTING
//-------------------------------------
int pin_ignition = 3; // pin qui recoit l'impulsion de l'ECU

void setup() {
  pinMode(3, INPUT);
  digitalWrite (3, HIGH);
  Serial.begin(115200); // use the serial port

  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() {
  
  while(1) { // reduces jitter
   
   
   if (digitalRead(3) == HIGH) {
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k; // put real data into bins
    }
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_lin8(); // take the output of the fht
    sei();
   for (int j = 0 ; j <= 25 ; j++) { 
    Serial.print(";" + String(fht_lin_out8[j]) );
    }
    Serial.println(" ");
  }
}

}
