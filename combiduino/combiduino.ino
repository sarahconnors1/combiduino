#pragma GCC optimize ("-O3")

#define BLUETOOTH_USED  1  // 1 si OUI sinon 0
#define INJECTION_USED  1// 1 si OUI sinon 0
#define LAMBDA_USED  1  // 1 si OUI sinon 0
#define KNOCK_USED  0  // 1 si OUI sinon 0
#define LAMBDATYPE 1  // 1= pour wideband 2= pour Narrow band
#define VACUUMTYPE 1  // 1= pour prise depression colecteur 2= pour prise depression amont papillon 
#define TPS_USED 1 // 1 pour accel base sur TPS, 0 pour accel base sur KPA
#define PID_IDLE_USED 1 // 1 utilise le PID au ralenti 0 utilise compensation simple
#define CLT_USED 1 // 1 utilise la temperature moteur pour corriger l'injection 0 pas utilisé
#define DECEL_USED 1 // 1 utilise la coupure d'injection a la décélération 0 pas utilisé
#define LOG_PERF 0 // 1 envoi une log du temps par routine 0 pas utilisé
#define OLD_LOG_USED 1 // 1 envoi les log rapide mais moins compréhensible 0 envoi l'ancien format

const byte VERSION= 50;   //version du combiduino
//-------------------------------------------- Include Files --------------------------------------------//
#include "variable.h"
#include <avr/pgmspace.h>
#include <EEPROM.h>


#if BLUETOOTH_USED == 1  
  #include <SPI.h>
  #include <boards.h>
  #include <RBL_nRF8001.h>
#endif

#if PID_IDLE_USED==1
 #include "PID_v1.h"
#endif
//--------------------------------------------Paramètre --------------------------------------------------//
//------ injection 
// phase demarrage
const byte ouverture_initiale = 6 ; // nombre de millisecond d'ouverture des injecteurs avant démarrage
const int after_start_nb_spark = 400; // enrichissement pendant x etincelle au demarrage 
const byte enrichissement_after_start = 130 ; // % d'enrichissemnt a après démarrage 

// phase normal
//unsigned int Req_Fuel_us = 8000 / nombre_inj_par_cycle;  // ouverture max des injecteurs 100% 
unsigned int Req_Fuel_us = 5000 / nombre_inj_par_cycle;  // ouverture max des injecteurs 100% 

const unsigned int injector_opening_time_us = 800; // temps d'ouverture de l'injecteur en us
const byte lissage_kpa_idle = 40 ;  // facteur lissage des KPA au ralenti
const byte lissage_kpa_running = 40 ;  // facteur lissage des KPA au regime normal

// phase ralenti
const int RPM_idle_max = 1100; // valeur RPM max pour déclencher le mode idle
const int MAP_idle_max = 45; // valeur MAP max pour déclencher le mode idle
const int TPS_idle_max = 4; // valeur MAP max pour déclencher le mode idle

boolean Idle_management = true; // gestion du ralenti controlé par PID
double RPM_idle_objectif = 950; // Objectif de ralenti

#if PID_IDLE_USED==1
  PID idlePID(  &idle_engine_rpm_average, &idle_advance, &RPM_idle_objectif, idleKp, idleKi, idleKd, DIRECT);
#endif


// phase acceleration
int MAP_kpas[8] =        {  0 ,  5,   10,  30,  60,  120}; //valeur acceleration en kpa /s
byte MAP_acceleration[8] ={ 0 ,  0,   50,  70, 120,  200} ; //valeur enrichissement en % de reqfuel
//const byte accel_mini = 8; // TPS_dot mini pour declencher un calcul d'acceleration
const byte accel_mini = 200; // TPS_dot mini pour declencher un calcul d'acceleration
const int RPM_ACC_max = 2500; // RPM maxi pour gestion enrichissement accel
const int accel_every_spark = 50;  // correction possible tous les x etincelle

// gestion TPS
const int tps_lu_max = 520; // valeur ADC max lu

//phase deceleration
const int RPM_DEC_min = 1600; // coupure d'injection si supérieur a ce regime
unsigned long time_decel = 0;
const unsigned long interval_time_decel = 2000; // declenche la deceleration si decel depuis plus de X ms


// correction par la lambda 
byte max_lambda_cor = 110; // correction maxi
byte min_lambda_cor = 90; // correction mini
const byte seuil_pourcent = 60; // seuil de representativite du point actuel pour correction si ca depasse ce seuil
const byte Kp_pourcent = 30; // taux de correction lambda 0-> 100 facteur d'apprentissage
boolean correction_lambda_used = true ; // correction lambda active ou non
const byte lambda_kpa_index_min = 11; // pas de correction lambda si superieur pour proteger le ralenti
const byte lambda_rpm_index_min = 3; // pas de correction lambda si inferieur pour proteger le ralenti

//----------------------- X- TAU ---------------------------------------

// taux d'adherence en % en fonction de la depression
const byte BAWC_min = 10; // au mini kpa pressure_axis[0]
const byte BAWC_max = 60; // au maxi kpa pressure_axis[nombre_point_DEP-1]
// taux evaporation en % en fonction de la depression
const byte BSOC_min = 20; // au mini kpa pressure_axis[0]
const byte BSOC_max = 60; // au maxi kpa pressure_axis[nombre_point_DEP-1]

// facteur de correction d adherence en fonction RPM (100 = pas de correction)
const byte AWN_min = 120; // au mini RPM rpm_axis[0]
const byte AWN_max = 60; // au maxi RPM rpm_axis[nombre_point_RPM-1]
// facteur de correction evaporation en fonction RPM (100 = pas de correction)
const byte SON_min = 70; // au mini RPM rpm_axis[0]
const byte SON_max = 110; // au maxi RPM rpm_axis[nombre_point_RPM-1]

// facteur de correction d adherence en fonction TEMPERATURE (100 = pas de correction)
const byte AWW_min = 100; // au mini CLT lowtemp
const byte AWW_max = 100; // au maxi CLT hightemp
// facteur de correction evaporation en fonction RPM (100 = pas de correction)
const byte SOW_min = 100; // au mini CLT lowtemp
const byte SOW_max = 100; // au maxi CLT hightemp




//-------------------------------------------- Initialise Parameters --------------------------------------------//
void setup() {
  
  // pour ne pas lancer de faux SAW
  detachInterrupt(1);
  detachInterrupt(0); // on desactive le pin 2
  
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

// reduction temps echantillonage analog read
#ifdef sbi
    sbi(ADCSRA,ADPS2);
    cbi(ADCSRA,ADPS1);
    cbi(ADCSRA,ADPS0);
#endif

// initialisation des capteurs
 initpressure();
 initTPS(); 
 initRPM();
  
  // port serie
  Serial.begin(115200); while (!Serial) {  }// wait for serial port to connect.  
 #if KNOCK_USED == 1  
   Serial1.begin(115200); while (!Serial1) { ; }// wait for serial port to connect. 
 #endif
 inputString.reserve(25);   //reserve 50 bytes for serial input - equates to 25 characters
 OutputString.reserve(25);
  time_loop = millis();

  // init des cartos si pas deja fait
  if (EEPROM.read(eprom_init)   !=  VERSION ) {   
 debug ("Init des MAP EEPROM");init_de_eeprom();RAZknock(); } // on charge les map EEPROM a partir de la RAM et konco moyen
 // lecture des carto
  debug ("Init des MAP RAM");
  read_eeprom(); // on charge les MAP en RAM a partir de l'eeprom et la derniÃ¨re MAP utilisÃ©e
 #if KNOCK_USED == 1
  initknock();
 #endif 

// Initialisationdu BT
#if BLUETOOTH_USED == 1  
 ble_set_name(BT_name); debug ("ready!");ble_begin();  
#endif
//init des capteurs

 inittimer(); // init des interruption Timer 5

 // initialisation du PIP interrupt 
 attachInterrupt(1, pip_interupt, FALLING);     //  interruption PIP signal de l'edis

  injection_initiale();
  initlog();

#if PID_IDLE_USED==1  
  InitPID();
#endif


// active le X-TAU
sbi(running_option,BIT_XTAU_USED);

// active le bluetooth
sbi(running_option,BIT_OUTPUT_BT);

// pas d'ego
correction_lambda_used = true;
}


//-------------------------------------------- Boucle principal --------------------------------------------//
void loop() {
   time_loop = millis();

// check entree serial
if (time_loop - time_reception > interval_time_reception){checkdesordres();time_reception = time_loop;}
//chek des valeur de knock
       getknock();
// calcul du nouvel angle d avance / injection si des valeurs ont changÃ©s
  if (BIT_CHECK(running_option,BIT_NEW_VALUE)) {deb();calcul_carto();cbi(running_option,BIT_NEW_VALUE); fin("clc");}
// puis on gere les fonctions annexe a declencher periodiquement
 
// recalcul de la dÃ©pression
  if (time_loop - time_check_depression > interval_time_check_depression){deb();gestiondepression();time_check_depression = time_loop;fin("dep");}
// recalcul du TPS
  if (time_loop - time_check_TPS > interval_time_check_TPS){deb();gestionTPS();time_check_TPS = time_loop;fin("tps");}

// recalcul du CLT
  if (time_loop - time_check_CLT > interval_time_check_CLT){deb();gestionCLT();time_check_CLT = time_loop;fin("clt");}

// recalcul de la lambda
  if (time_loop - time_check_lambda > interval_time_check_lambda){deb();lecturelambda();time_check_lambda = time_loop;fin("lbd");}

// recalcul correction AFR
  if (time_loop - time_check_AFR > interval_time_check_AFR){deb();AFR_self_learning();time_check_AFR = time_loop;fin("afr");}
  
// gestions sortie pour module exterieur ECU / EC1
  if (time_loop - time_envoi > interval_time_envoi){deb();gestionsortieECU();gestionsortieEC1();time_envoi = time_loop;fin("bt1");}

// gestions connection BLE
  if (time_loop - time_check_connect > interval_time_check_connect){deb();checkBLE();time_check_connect = time_loop;fin("bt2");}

// check demarrage de la pompe fuel
  if (time_loop - time_check_fuel_pump > interval_time_check_fuel_pump){deb();checkpump();time_check_fuel_pump = time_loop;fin("pmp");}

// gestions sortie pour megalog
  if (time_loop - time_megalog > interval_time_megalog){deb();Megalog();time_megalog = time_loop;fin("log");}

  

}

void serialEvent() {
 while (Serial.available()) {                    //whilst the serial port is available...
   inputString = Serial.readStringUntil('\n');   //... read in the string until a new line is recieved
   stringComplete = true;                     
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
  // on initialise la pression atmospherique
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

}
 
void initlog(){
  sndlog("Time;CLT;IAE;mSpark;RPM;Load;MAPdot;PhAccel;PWacc;PW;Gve;AFR;Gego;SparkAdv;TPS;TPSdot;idle;carto;PIDadv;Dec;PW_corr;Qteparoi;Xadher;Tau" ); 
  sndlog("ms;deg;%;on/off;tr/min;kpa;kpa/s;on/off;uS;uS;%;AFR;%;deg;%;%/s;on/off;nbr;deg;%;uS;uS;%;%" ); 
}

void Megalog(){


# if OLD_LOG_USED == 1
String logsend = "";
  logsend = String( time_loop/ float(1000) )  + ';'
 +  CLT + ';'
 + IAE_actuel + ';'
 +  bitRead(running_option,BIT_MS) +  ';'
 +  engine_rpm_average +  ';' 
 +  map_pressure_kpa +  ';' 
 +  MAP_accel +  ';' 
 +   bitRead(running_mode, BIT_ENGINE_MAP )  +  ';' 
 +  PW_accel_actuel_us  +  ';'
 +  injection_time_us +  ';' 
 +  VE_actuel  + ';' 
 +  AFR_actuel/ float(10)  +  ';' 
 +  correction_lambda_actuel  +  ';'
 +  Degree_Avance_calcul  +  ';'  
 +  TPS_actuel  +  ';' 
 +  TPS_accel  + ';'
 +   bitRead(running_mode, BIT_ENGINE_IDLE )  + ';'
 +  carto_actuel  + ';'
 + PID_idle_advance  + ';'
 + DEC_actuel  + ';'
 + PW_actuel + ';'
 + qte_paroi + ';' 
 + X_adher + ';'
 + Tau_evap + ';'
  ;
sndlog(logsend);


/* TEST RPM 
logsend ="";
for (int j = 0 ; j < maxpip_count; j++) { 
    logsend = logsend + String(time_readings[j]) + ";" ;
}
sndlog(logsend);
*/
#endif

# if OLD_LOG_USED == 0
char buffer[100]; // make sure this is large enough for your string + 1 for NULL char

sprintf(buffer, "%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;" , 
  (int)(time_loop/100)
 , CLT 
 , IAE_actuel 
 ,  multispark 
 ,  engine_rpm_average 
 ,  (int) (map_pressure_kpa  *100) 
 ,  MAP_accel 
 ,   bitRead(running_mode, BIT_ENGINE_MAP )   
 ,  PW_accel_actuel_us  
 ,  injection_time_us  
 ,  (int)VE_actuel  
 ,  AFR_actuel   
 ,  correction_lambda_actuel  
 ,  (int)Degree_Avance_calcul    
 ,  TPS_actuel  
 ,  TPS_accel  
 ,   bitRead(running_mode, BIT_ENGINE_IDLE )  
 ,  carto_actuel  
 , (int)PID_idle_advance  
 , (int) DEC_actuel  



);
Serial.println(buffer);
# endif

}

void InitPID(){
#if PID_IDLE_USED==1
  idlePID.SetOutputLimits((double)(-Idle_maxoutput), (double)(Idle_maxoutput)); 
  idlePID.SetTunings(idleKp, idleKi, idleKd); 
  idlePID.SetSampleTime(300); // tous les 100 millis
  idlePID.SetMode(MANUAL);
#endif  
}

void initRPM(){
  time_total=0;
  for (int j = 0 ; j < maxpip_count; j++) { 
    time_readings[j] = 65000;           //place current rpm value into current array element
    time_total= time_total + time_readings[j]; //add the reading to the total     
  }
}

