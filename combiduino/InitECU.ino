
//********************************************************************************************
//          ROUTINE POUR INITIALISATION
//********************************************************************************************

void initCarto(){
   // init des cartos si pas deja fait
  if (EEPROM.read(eprom_init)   !=  VERSION ) {   
 debug ("Init des MAP EEPROM");init_de_eeprom();RAZknock(); } // on charge les map EEPROM a partir de la RAM et konco moyen
 // lecture des carto
 debug ("Init des MAP RAM");
  read_eeprom(); // on charge les MAP en RAM a partir de l'eeprom et la derniÃ¨re MAP utilisÃ©e

}

void initSerial(){
   // port serie
  Serial.begin(57600); while (!Serial) {  }// wait for serial port to connect.  
 #if KNOCK_USED == 1  
   Serial1.begin(115200); while (!Serial1) { ; }// wait for serial port to connect. 
 #endif
 inputString.reserve(25);   //reserve 50 bytes for serial input - equates to 25 characters
 OutputString.reserve(25);
 //init du transfert log
 ET.begin(details(ECU), &Serial);

}

void initPIN(){
  // declaration des Pin OUTPUT
  pinMode(SAW_pin, OUTPUT);                                                 
  pinMode(pin_ignition, OUTPUT); 
  pinMode(pin_injection, OUTPUT);
  pinMode(pin_injection2, OUTPUT);
  pinMode( pin_pump, OUTPUT);
  digitalWrite(pin_pump,LOW); // gestion de l'allumage de la pompe

// declaration des Pin INTPUT  
  pinMode(interrupt_X, INPUT);
  digitalWrite (interrupt_X, HIGH);
  pinMode(pin_lambda, INPUT);
  digitalWrite (pin_lambda, LOW );
  pinMode(MAP_pin, INPUT);
  digitalWrite (MAP_pin, LOW );
  pinMode(TPS_pin, INPUT);
  digitalWrite (TPS_pin, LOW );
  pinMode(CLT_pin, INPUT);
  digitalWrite (CLT_pin, LOW );
  pinMode(VLT_pin, INPUT);
  digitalWrite (VLT_pin, LOW );


// reduction temps echantillonage analog read
#ifdef sbi
// Division par 16 + rapide
//    sbi(ADCSRA,ADPS2);
//    cbi(ADCSRA,ADPS1);
//    cbi(ADCSRA,ADPS0);
#endif

#ifdef sbi
// Division par 64 - rapide
//    sbi(ADCSRA,ADPS2);
//    sbi(ADCSRA,ADPS1);
//    cbi(ADCSRA,ADPS0);
#endif

}

void InitPID(){
#if PID_IDLE_USED==1
  idlePID.SetOutputLimits((double)(Idle_minoutput), (double)(Idle_maxoutput)); 
  idlePID.SetTunings(idleKp, idleKi, idleKd); 
  idlePID.SetSampleTime(100); // tous les 100 millis
  idlePID.SetMode(MANUAL);
#endif  
#if PID_EGO_USED==1
  egoPID.SetOutputLimits((double)(ego_minoutput), (double)(ego_maxoutput)); 
  egoPID.SetTunings(egoKp, egoKi, egoKd); 
  egoPID.SetSampleTime(100); // tous les 100 millis
  egoPID.SetMode(MANUAL);
#endif  
}

void initRPM(){
  time_total=0;
  for (int j = 0 ; j < maxpip_count; j++) { 
    time_readings[j] = 65000;           //place current rpm value into current array element
    time_total= time_total + time_readings[j]; //add the reading to the total     
  }
}


void initpressure(){
  correction_pressure = 0;
  // on initialise la pression atmospherique
  for (int j = 0 ; j < 5; j++) { 
    correction_pressure += analogRead(MAP_pin);
  }
 correction_pressure =  (correction_pressure / 5);
}

void initTPS(){
  tps_lu_min = 0;
  // on initialise la position 0 du TPS
  for (int j = 0 ; j < 5; j++) { 
    tps_lu_min += analogRead(TPS_pin);
  }
 tps_lu_min =  (tps_lu_min / 5);
}

 void inittimer(){
 // parametrage du timer 5
  TCCR5A = 0;
  TCCR5B = 0;
  TCCR5C = 0;
  TCNT5  = 0;
  TIFR5 = 0x00;
  TIMSK5 = 0 ; 
  TCCR5B |= (1 << CS51);    // 8 prescaler 
// parametrage pour le PIP pin 3 Timer 3C
 TCCR3B |= (1 << ICNC3); // Noise canceler a OUI
// initialisation du PIP interrupt 
 attachInterrupt(1, pip_interupt, FALLING);     //  interruption PIP signal de l'edis

}
 
void initlog(){
# if SD_LOG_USED == 0
  sndlog(F("Time;CLT;IAE;mSpark;RPM;Load;MAPdot;PW;AFR;Gego;SparkAdv;TPS;idle;carto;Dec;PW_corr;AFRobj;ptKPA;ptRPM") ); 
  sndlog(F("ms;deg;%;on/off;tr/min;kpa;kpa/s;uS;AFR;%;deg;%;on/off;deg;%;uS;AFR;pt;pt") ); 
# endif
}



