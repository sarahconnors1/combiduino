#pragma GCC optimize ("-O3")

#define BLUETOOTH_USED  0  // 1 si OUI sinon 0
#define INJECTION_USED  1// 1 si OUI sinon 0
#define LAMBDA_USED  1  // 1 si OUI sinon 0
#define KNOCK_USED  0  // 1 si OUI sinon 0
#define LAMBDATYPE 1  // 1= pour wideband 2= pour Narrow band
#define VACUUMTYPE 1  // 1= pour prise depression colecteur 2= pour prise depression amont papillon 
#define TPS_USED 0 // 1 pour accel base sur TPS, 0 pour accel base sur KPA
#define PID_IDLE_USED 1 // 1 utilise le PID au ralenti 0 utilise compensation simple
#define CLT_USED 1 // 1 utilise la temperature moteur pour corriger l'injection 0 pas utilisé
#define DECEL_USED 1 // 1 utilise la coupure d'injection a la décélération 0 pas utilisé
#define LOG_PERF 0 // 1 envoi une log du temps par routine 0 pas utilisé
#define SD_LOG_USED 2 // 1 envoi les log rapide pour le SD mais moins compréhensible/  0 envoi l'ancien format /2 envoi par easytransfert
#define PID_EGO_USED 1 // 1 utilise le PID EGO
const byte VERSION= 52;   //version du combiduino
//-------------------------------------------- Include Files --------------------------------------------//
#include <EasyTransfer.h>
EasyTransfer ET; 

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
unsigned int Req_Fuel_us = 5000 / nombre_inj_par_cycle;  // ouverture max des injecteurs 100% 
const unsigned int injector_opening_time_us = 800; // temps d'ouverture de l'injecteur en us
const byte lissage_kpa_running = 50 ;  // facteur lissage des KPA au regime normal
const byte lissage_TPS_running = 50 ;  // facteur lissage du TPS au regime normal

// phase ralenti
const int RPM_idle_max = 1100; // valeur RPM max pour déclencher le mode idle
const int MAP_idle_max = 45; // valeur MAP max pour déclencher le mode idle
const int TPS_idle_max = 4; // valeur MAP max pour déclencher le mode idle
boolean Idle_management = true; // gestion du ralenti controlé par PID
double RPM_idle_objectif = 950; // Objectif de ralenti

// phase acceleration
const byte MAPdot_weight = 60;const byte TPSdot_weight = 40; // pondération pour l'acceleration  
int MAP_kpas[8] =        {  0 ,  40,   80,  120,  160,  200}; //valeur de TPSMAPdot
byte MAP_acceleration[8] ={ 0 ,  25,   50,  70, 120,  200} ; //valeur enrichissement en % de reqfuel
//const byte accel_mini = 8; // TPS_dot mini pour declencher un calcul d'acceleration
const byte accel_mini = 40; // TPSMAP_dot mini pour declencher un calcul d'acceleration
const int RPM_ACC_max = 2500; // RPM maxi pour gestion enrichissement accel
const int accel_every_spark = 50;  // correction possible tous les x etincelle

// gestion TPS
const int tps_lu_max = 520; // valeur ADC max lu

//phase deceleration
const int RPM_DEC_min = 1800; // coupure d'injection si supérieur a ce regime
unsigned long time_decel = 0;
const unsigned long interval_time_decel = 2000; // declenche la deceleration si decel depuis plus de X ms

//gestion lambda
const byte lissage_AFR = 60 ;  // facteur lissage des AFR


// Apprentissage par la lambda 
byte max_lambda_cor = 110; // correction maxi
byte min_lambda_cor = 90; // correction mini
const byte seuil_pourcent = 60; // seuil de representativite du point actuel pour correction si ca depasse ce seuil
const byte Kp_pourcent = 30; // taux de correction lambda 0-> 100 facteur d'apprentissage
boolean correction_lambda_used = true ; // correction lambda active ou non
const byte lambda_kpa_index_min = 11; // pas de correction lambda si superieur pour proteger le ralenti
const byte lambda_rpm_index_min = 3; // pas de correction lambda si inferieur pour proteger le ralenti

//correction PID lambda
const double ego_minoutput= -10;
const double ego_maxoutput=  10;
const double ego_coeff = 0.01; 
double egoKp=2 * ego_coeff, egoKi=0.1 * ego_coeff, egoKd=0.5 * ego_coeff;

//----------------------- X- TAU ---------------------------------------
const float Xtau_max_decel = 1.2; // facteur pour limiter l appauvrissemnt maximum en fin d accel du x tau 1.3 = 30%
// taux d'adherence en % en fonction de la depression
const byte BAWC_min = 20; // au mini kpa pressure_axis[nombre_point_DEP-1]
const byte BAWC_max = 60; // au maxi kpa pressure_axis[0]
// taux evaporation en % en fonction de la depression
const byte BSOC_min = 10; // au mini kpa pressure_axis[nombre_point_DEP-1] 
const byte BSOC_max = 60; // au maxi kpa pressure_axis[0]

// facteur de correction d adherence en fonction RPM (100 = pas de correction)
const byte AWN_min = 130; // au mini RPM rpm_axis[0]
const byte AWN_max = 60; // au maxi RPM rpm_axis[nombre_point_RPM-1]
// facteur de correction evaporation en fonction RPM (100 = pas de correction)
const byte SON_min = 60; // au mini RPM rpm_axis[0]
const byte SON_max = 120; // au maxi RPM rpm_axis[nombre_point_RPM-1]

// facteur de correction d adherence en fonction TEMPERATURE (100 = pas de correction)
const byte AWW_min = 120; // au mini CLT lowtemp
const byte AWW_max = 100; // au maxi CLT hightemp
// facteur de correction evaporation en fonction TEMPERATURE (100 = pas de correction)
const byte SOW_min = 100; // au mini CLT lowtemp
const byte SOW_max = 120; // au maxi CLT hightemp



#if PID_IDLE_USED==1
  PID idlePID(  &idle_engine_rpm_average, &idle_advance, &RPM_idle_objectif, idleKp, idleKi, idleKd, DIRECT);
#endif


#if PID_EGO_USED==1
  PID egoPID(  &AFR_actuel_double, &correction_lambda_actuel_double, &AFR_objectif_double, egoKp, egoKi, egoKd, REVERSE);
#endif

//-------------------------------------------- Initialise Parameters --------------------------------------------//
void setup() {
  
// pour ne pas lancer de faux SAW
  detachInterrupt(1);
  detachInterrupt(0); // on desactive le pin 2

// initialisation des capteurs
 initPIN();
 initpressure();initTPS();initRPM();
 initSerial();
 time_loop = millis();
 initCarto();
   
 #if KNOCK_USED == 1
  initknock();
 #endif 

// Initialisationdu BT
#if BLUETOOTH_USED == 1  
 ble_set_name(BT_name); debug ("ready!");ble_begin();  
#endif


 inittimer(); // init des interruption Timer 5
 injection_initiale();
 initlog();
 InitPID();

// active le X-TAU
sbi(ECU.running_option,BIT_XTAU_USED);

// active le bluetooth
sbi(ECU.running_option,BIT_OUTPUT_BT);

// pas d'ego
correction_lambda_used = true;

// pas de debug
cbi(ECU.running_option,BIT_DEBUG);
}


//-------------------------------------------- Boucle principal --------------------------------------------//
void loop() {
   time_loop = millis();

//*******************************
// a chaque boucle 
getknock(); //chek des valeur de knock
Calcul_qte_paroi(); // a chaque tour
if (BIT_CHECK(ECU.running_option,BIT_NEW_VALUE)) {deb();calcul_carto(); fin("clc");} // calcul du nouvel angle d avance / injection si des valeurs ont changÃ©s
//*******************************


//***************************************************************
// puis on gere les fonctions annexe a declencher periodiquement
//***************************************************************
 
// recalcul de la dÃ©pression
  if (time_loop - time_check_depression > interval_time_check_depression){deb();gestiondepression();time_check_depression = time_loop;fin("dep");}
// recalcul du TPS
  if (time_loop - time_check_TPS > interval_time_check_TPS){deb();gestionTPS();time_check_TPS = time_loop;fin("tps");}
// recalcul de la lambda
  if (time_loop - time_check_lambda > interval_time_check_lambda){deb();lecturelambda();time_check_lambda = time_loop;fin("lbd");}
// recalcul du CLT
  if (time_loop - time_check_CLT > interval_time_check_CLT){deb();gestionCLT();time_check_CLT = time_loop;fin("clt");}
// recalcul correction AFR
  if (time_loop - time_check_AFR_correction > interval_time_check_AFR_correction){deb();AFR_self_learning();time_check_AFR_correction = time_loop;fin("afr");}
// check demarrage de la pompe fuel
  if (time_loop - time_check_fuel_pump > interval_time_check_fuel_pump){deb();checkpump();time_check_fuel_pump = time_loop;fin("pmp");}

// recalcul du TPSdot et MAPdot
  if (time_loop - time_check_TPSMAPdot > interval_time_check_TPSMAPdot){deb();gestionTPSMAPdot();time_check_TPSMAPdot = time_loop;fin("TPSMAPdot");}


//**************************************************
// Gestion ENTREE/SORTIE
//**************************************************
// gestions sortie pour megalog
  if (time_loop - time_megalog > interval_time_megalog){deb();Megalog();time_megalog = time_loop;fin("log");}
// gestions sortie pour module exterieur ECU / EC1
  if (time_loop - time_envoi > interval_time_envoi){deb();gestionsortieECU();gestionsortieEC1();time_envoi = time_loop;fin("bt1");}
// check entree serial
if (time_loop - time_reception > interval_time_reception){checkdesordres();time_reception = time_loop;}
// gestions connection BLE
  if (time_loop - time_check_connect > interval_time_check_connect){deb();checkBLE();time_check_connect = time_loop;fin("bt2");}
  

}

void serialEvent() {
 while (Serial.available()) {                    //whilst the serial port is available...
   inputString = Serial.readStringUntil('\n');   //... read in the string until a new line is recieved
   stringComplete = true;                     
 }
}


