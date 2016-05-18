long temp=0;
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#define BIT_CHECK(var,pos) ((var) & (1<<(pos)))
//-------------------------------------------- Global variables --------------------------------------------//
// nr serie EDIS4 F3CF-12A359-AA 5039B 4J10L4F
//-------------------declaration des pins
const int interrupt_X = 3;      // PIP EDIS pin
const int BLE_reset_pin = 4;     // pin 4 reboot BLE
const int pin_pump = 5;              // pin 5 activation de la pompe 
// pin 8 et 9 BLE
const int pin_injection = 11; // pin pour la carte MOSFET Injecteur
const int pin_injection2 = 10; // pin pour la carte MOSFET Injecteur
const int pin_ignition = 12;  // pin du début d'ignition pour le knock
const int SAW_pin = 13;           // SAW EDIS pin

const int MAP_pin = A0;            // depression
const int pin_lambda = A5;         // Signal lambda
const int TPS_pin = A1;            // TPS
const int CLT_pin = A2;            // CLT temperature huile

// -------------------gestion different mode du moteur
byte running_mode = 0;
#define BIT_ENGINE_RUN      0     // Engine running
#define BIT_ENGINE_CRANK    1   // Engine cranking
#define BIT_ENGINE_ASE      2    // after start enrichment (ASE)
#define BIT_ENGINE_WARMUP   3  // Engine in warmup
#define BIT_ENGINE_ACC      4    // in TPS acceleration mode
#define BIT_ENGINE_DCC      5    // in deceleration mode
#define BIT_ENGINE_MAP      6    // in MAP acceleration mode
#define BIT_ENGINE_IDLE     7  // idle on

//----------------------declaration pour injection 
const byte nombre_inj_par_cycle = 2; // le nombre d'injection pour 1 cycle complet 2 tour
const byte prescalertimer5 =2 ; //prescaler /8 a 16mhz donc 1us= 2 tick
float VE_actuel = 20; // VE du dernier calcul

volatile unsigned int tick_injection = 0; // pour le timer
unsigned int injection_time_us = 0; // temps d'injection corrigé pour le timer
volatile byte cylindre_en_cours = 1; // cylindre en cours d'allumage
boolean cylinder_injection[4] = {true,false,true,false}; // numero de cylindre cylindre pour injection bank1
boolean cylinder_injection2[4] = {false,true,false,true}; // numero de cylindre cylindre pour injection bank2

//----------------------declaration acceleration
const int MAP_acc_max = 6; // nombre d'indice du tableua MAP_kpas
long  last_accel_spark = 0 ; //dernier declenchement d'acceleration
unsigned int PW_accel_actuel_us = 0; // accel du dernier calcul en microsecond
int saved_accel=0; // acceleration lors du declenchement de l'accel

//----------------------declaration TPS
const byte nbre_mesure_TPS = 5;
const int TPS_check_per_S = 4 ; // nombre de calcul par seconde d'acceleration / depression
int TPS_actuel = 0;
int previous_TPS = 0;
int TPS_accel = 0 ;  // TPS/s d'acceleration
int sum_TPS = 0;
byte count_TPS = 0;
long last_TPS_time = 10; // dernier calcul du TPS
long previous_TPS_time = 0; // avant dernier calcul TPS
const byte TPS_ecart_representatif = 1; // ecart a ne pas prendre en compte car non représentatif

//--------------------declaration pour la lambda
const int AFR_bin_max = 9;
#if LAMBDATYPE == 2
int AFR_analogique[AFR_bin_max] ={45,   93  ,186 ,214 ,651, 744, 791, 835,1023}; //valeur lu par le capteur avec echelle 1,1V 1024
byte AFR[AFR_bin_max] =          {190 ,170  ,150 ,147 ,140, 130, 120, 110,90} ; //valeur AFR * 10
# endif
int AFR_actuel =147; // valeur AFR 100 -> 190
int sum_lambda = 0;
byte count_lambda = 0;
const byte nbre_mesure_lambda = 4;


//--------------------declaration du debugging
boolean debugging = true; // a mettre a falsdebuge pour ne pas debugger
String debugstring = "";

//--------------------declaration bluetooth
char BT_name [10] = "Combi";
String OutputString = "";
boolean output = true;

//--------------------Serial input initailisation
String inputString = "";            // a string to hold incoming data
boolean stringComplete = false;     // whether the string is complete

//------------------- declaration pour l ECU
const byte maxpip_count = 10;  //on fait la moyenne tout les x pip
const int nombre_point_RPM = 23; // nombre de point de la MAP
const int nombre_point_DEP = 17; // nombre de point de la MAP
const int nombre_carto_max = 5; // nombre de carto a stocker
const unsigned int debounce = 4000; // temps mini acceptable entre 2 pip 4ms-> 7500 tr/min 
const int fixed_advance = 15;             // Avance fixe


unsigned int delay_ignition = 1000; // nombre de us entre PIP et le debut du SAW
const int angle_delay_SAW = 10 ; // nombre de degre d'attente avant envoyer le SAW 
byte ignition_mode = 0; // gestion etat allumage
#define IGNITION_STOP     0     // SAW pas en cours
#define IGNITION_PENDING  1     // SAW programmé
#define IGNITION_RUNNING  2   // SAW en cours

byte point_RPM = 0; // index dans la carto
byte point_KPA = 16; // index dans la carto
volatile long nbr_spark = 0; // nombre de spark depuis demarrage
volatile float map_value_us;                 // duree du SAW EDIS en microseconds
volatile boolean SAW_requested = false;
unsigned int rev_limit = 4550;               // Max tour minute
unsigned int rev_mini = 500; // min tour minute
float Degree_Avance_calcul = 10;              // Degree_Avance_calculÃ© suivant la cartographie a 10 par defaut pour demmarragee

boolean fixed = false;              // declare whether to use fixed advance values
boolean multispark = true;         // multispark
volatile boolean first_multispark = false;         // 1er allumage en multispark
const int RPM_max_multispark = 1400; // multispark géré jusqy'a x rpm
int correction_degre = 0; // correction de la MAP
const int msvalue = 2040 ; // normaly 2048

volatile unsigned int tick = 0; // nombre de tick du timer pour le saw
volatile boolean newvalue = true;  // check si des nouvelles valeurs RPM/pression ont ete calculÃ©s
//volatile unsigned long timeold = 0;
volatile unsigned long pip_old = 0; // durée du dernier pip pour le debounce
volatile byte pip_count = 0;
volatile unsigned int engine_rpm_average = 0;  // Initial average engine rpm pour demarrage
byte carto_actuel = 1; //cartographie en cours
volatile boolean ignition_on = false; // gere si le SAW est envoyé non terminé

//rpm averaging array
//const int num_rpm_readings = 10;               //size of rpm averaging array
volatile unsigned int time_readings[maxpip_count]  ;   // array of rpm readings
volatile unsigned long time_total = 0;          // the running rpm total


// declaration pour interpolation
byte binA = 0;
byte binB = 0;
byte binC = 0;
byte binD = 0;
byte rpm_index_low = 0;                      // retrouve index RPM inferieur
byte rpm_index_high = 0;
byte pressure_index_low = 0;       // retrouve index pression inferieur
byte pressure_index_high = 0;

// declaration pour le KNOCK
#if KNOCK_USED == 1
long knockvalue[23] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // accumulation des valuer de knock
long knockcount[23] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // accumulation des count de knock
long knockmoyen[23] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // accumulation des count moyen
long total_knock = 0;
long knock_moyen = 0; // moyend de la période
int count_knock = 0;
int delta_knock = 0; // ecart par rapport a la moyenne enregistré
const int count_knock_max = 8; // tout les 8 pip = 4 tour
boolean knock_record = false; // enregistrement des valeur normal du capteur
boolean knock_active = false; // utilisation de la fonction knock
String inputString3 = "";            // a string to hold incoming data
boolean stringComplete3 = false;     // whether the string is complete
#endif

// variable pression moyenne
int correction_pressure = 400;   // gestion de la valeur du capteur a pression atmo se met a jour lors de lancement
float map_pressure_kpa = 100;
float previous_map_pressure_kpa = 100;
int MAP_accel = 100 ;  // kpa/s d'acceleration
int sum_pressure = 0;
byte count_pressure = 0;
const byte nbre_mesure_pressure = 5;
const int MAP_check_per_S = 10 ; // nombre de calcul par seconde d'acceleration / depression
const int kpa_0V = 0; // Kpa reel lorsque le capteur indique 0V

long last_MAP_time = 10; // dernier calcul du TPS
long previous_MAP_time = 0; // avant dernier calcul TPS

//--------------------correction lambda
byte correction_lambda_actuel = 100;

const int increment_correction_lambda = 1;
const int correction_lambda_every_spark = 30;  // correction possible tous les x etincelle
long  last_correction_lambda_spark = 1000 ; //pas de correction au demarrage
                                  //600, 800, 1000, 1200, 1400, 1600, 1800, 2000, 2200, 2400, 2600, 2800, 3000, 3200, 3400, 3600, 3800, 4000, 4200, 4400, 4600, 4800, 5000}, //
                                  
byte AFR_delay[nombre_point_RPM] = { 6 ,   6,    6,    5,    5,    5,    5,    4,    4,    4,    4,    4,    4,    3,    2,    2,    2,    2,    2,    2,    2,    2,    2}; // 23 retard en nbre de mesure de la sonde lambda
const byte AFR_log_maxbin = 12;
byte AFR_log_bin_actuel = 0;
byte AFR_log_RPM[AFR_log_maxbin] =          {99,99,99,99,99,99,99,99,99,99,99,99 } ; //point RPM
byte AFR_log_KPA[AFR_log_maxbin] =          {99,99,99,99,99,99,99,99,99,99,99,99 } ; //point KPA

// correction Ralenti
float PID_idle_advance = 0;
#if PID_IDLE_USED== 0
const byte Idle_maxbin = 7;
int Idle_step[Idle_maxbin] = {-150,-100,-50,0,50,75,100 } ; // ecart par rapport au ralenti ideal
int Idle_adv[Idle_maxbin] = {2,1,0,0,-1,-1,-2 }; // degré avance de compensation
#endif

#if PID_IDLE_USED== 1
const double Idle_maxoutput = 5; // nombre de degre max a ajouter ou enlever
const double idle_coeff = 0.01; 
//double idleKp=2 * idle_coeff, idleKi=0.1 * idle_coeff, idleKd=0.5 * idle_coeff;
double idleKp=1.5 * idle_coeff, idleKi=0.1 * idle_coeff, idleKd=0.5 * idle_coeff;
double idle_advance = 0;
double idle_engine_rpm_average = 0;
#endif

// temperature huile et enrichissement
// la règle est 3% de moins d'air par 10 degre Celsius de temperature

int CLT = 20; // temperature huile moteur
byte IAE_actuel = 100; // Intake enrichissement 
const int lowtemp_lu = 660; // valeur capteur a basse temperature
const int lowtemp = 20; //temperature en degre
const int hightemp_lu = 290; // valeur capteur a haute temperature
const int hightemp = 60; //temperature en degre

const int IAElowtemp = 20; //temperature en degre
const byte lowtemp_enrich = 115; // coeff enirchissemnt a temperature low temp 
const int IAEhightemp = 50; //temperature en degre
const byte hightemp_enrich = 100; // coeff enirchissemnt a temperature high temp 
// 


// variable de loop
long var1 = 0;
long var2 = 0;
unsigned long time_loop = 0;
unsigned long time_reception = 0;
const unsigned long interval_time_reception = 100; // check des ordres tout les 100 ms secondes
unsigned long time_envoi = 0;
const unsigned long interval_time_envoi = 250; // ECU / EC1 toute les 1 secondes
unsigned long time_check_depression = 0;
const unsigned long interval_time_check_depression = 1000 / (MAP_check_per_S * nbre_mesure_pressure); // Depression x fois / seconde
unsigned long time_check_lambda = 0;

const unsigned long interval_time_check_lambda = 50 / nbre_mesure_lambda; // lambda  50ms
unsigned long time_check_connect = 0; 

const unsigned long interval_time_check_AFR = 200 ; // recalcul correction tous les 50ms
unsigned long time_check_AFR = 2000; 

const unsigned long interval_time_check_connect = 30000; // reconnection  1 fois /  30 seconde

unsigned long time_check_fuel_pump = 0; 
const unsigned long interval_time_check_fuel_pump = 2000; // check de la pompe de fuel

unsigned long time_check_TPS = 0;
const unsigned long interval_time_check_TPS = 1000 / (TPS_check_per_S * nbre_mesure_TPS); // TPS x fois / seconde

unsigned long time_check_CLT = 0;
const unsigned long interval_time_check_CLT = 1000 ; // CLT 1 fois / seconde


unsigned long time_megalog = 0;
const unsigned long interval_time_megalog = 100; // envoi des log 10 fois par seconde

//----------------------------
// Variable pour eeprom
//----------------------------

const int debut_eeprom = 50; //Pour laisser un peu de vide
const int taille_carto = 500; // carto complete avec kpa + rpm
const int debut_kpa = 400; //Pour ecrire les kpa au 400 de chaque MAP
const int debut_rpm = 420; //Pour ecrire les rpm au 420 de chaque MAP
// gestion parametre de eeprom
const int eprom_carto_actuel = 0; // emplacement dans EEPROM
const int eprom_init = 1; // emplaceement version
const int eprom_nom_BLE = 10; // emplaceement eeprom du  nom du BLE
const int eprom_rev_max = 21; // emplaceement eeprom du  REV MAX
const int eprom_rev_min = 23; // emplaceement eeprom du  REV MIN
const int eprom_debug = 2; // emplaceement eeprom du  debug
const int eprom_knock = 6; // emplaceement eeprom du knock sensor
const int eprom_reboot = 7; // emplacement eeprom du knock moyen

const int eprom_ms = 3; // emplaceement eeprom du  multispark
const int eprom_avance = 4; // emplaceement eeprom de avance initiale
const int eprom_adresseknock = 25; // emplacement eeprom du knock moyen

const int eprom_ego = 2550; // emplacement eeprom des correction EGO
