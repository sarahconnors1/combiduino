long temp=0;
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#define BIT_CHECK(var,pos) ((var) & (1<<(pos)))
//-------------------------------------------- Global variables --------------------------------------------//
//declaration des pins
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

// gestion different mode du moteur
byte running_mode = 0;
#define BIT_ENGINE_RUN      0     // Engine running
#define BIT_ENGINE_CRANK    1   // Engine cranking
#define BIT_ENGINE_ASE      2    // after start enrichment (ASE)
#define BIT_ENGINE_WARMUP   3  // Engine in warmup
#define BIT_ENGINE_ACC      4    // in TPS acceleration mode
#define BIT_ENGINE_DCC      5    // in deceleration mode
#define BIT_ENGINE_MAP      6    // in MAP acceleration mode
#define BIT_ENGINE_IDLE     7  // idle on

// declaration pour injection 
unsigned int VE_actuel = 20; // VE du dernier calcul

volatile unsigned int tick_injection = 0; // pour le timer
const byte nombre_inj_par_cycle = 2; // le nombre d'injection pour 1 cycle complet 2 tour

unsigned int injection_time_us = 0; // temps d'injection corrigé pour le timer
volatile unsigned int cylindre_en_cours = 1; // cylindre en cours d'allumage
boolean cylinder_injection[4] = {true,false,true,false}; // numero de cylindre cylindre pour injection bank1
boolean cylinder_injection2[4] = {false,true,false,true}; // numero de cylindre cylindre pour injection bank2
const byte prescalertimer5 =2 ; //prescaler /8 a 16mhz donc 1us= 2 tick

// declaration acceleration
long  last_accel_spark = 0 ; //pas de correction au demarrage
//byte accel_state_actuel = 0;  //etat du moteur 0=pas d'accel 1=accel 2=decel 
const int MAP_acc_max = 6; // nombre d'indice du tableua MAP_kpas
unsigned int PW_accel_actuel_us = 0; // accel du dernier calcul en microsecond
int saved_accel=0; // acceleration lors du declenchement de l'accel

//declaration TPS
int TPS_actuel = 100;
int previous_TPS = 100;
int TPS_accel = 100 ;  // TPS/s d'acceleration
int sum_TPS = 0;
byte count_TPS = 0;
const byte nbre_mesure_TPS = 3;
const int TPS_check_per_S = 8 ; // nombre de calcul par seconde d'acceleration / depression
long last_TPS_time = 10; // dernier calcul du TPS
long previous_TPS_time = 0; // avant dernier calcul TPS

// declaration pour la lambda
const int AFR_bin_max = 9;
#if LAMBDATYPE == 2
int AFR_analogique[AFR_bin_max] ={45,   93  ,186 ,214 ,651, 744, 791, 835,1023}; //valeur lu par le capteur avec echelle 1,1V 1024
byte AFR[AFR_bin_max] =          {190 ,170  ,150 ,147 ,140, 130, 120, 110,90} ; //valeur AFR * 10
# endif
#if LAMBDATYPE == 1
int AFR_analogique[AFR_bin_max] ={0 , 200, 300, 400, 540, 664, 716, 818, 1000}; //valeur lu par 0V=9 5V=19
byte AFR[AFR_bin_max] =          {90, 110, 120, 130, 147, 155, 160, 170, 190 } ; //valeur AFR * 10
# endif

int AFR_actuel =147; // valeur AFR 100 -> 190
int correction_lambda_actuel = 100;
boolean correction_lambda_used = true; // correction lambda active ou non
const int increment_correction_lambda = 1;
const int correction_lambda_every_spark = 30;  // correction possible tous les x etincelle
long  last_correction_lambda_spark = 1000 ; //pas de correction au demarrage
int sum_lambda = 0;
byte count_lambda = 0;
const byte nbre_mesure_lambda = 2;


// declaration du debugging
boolean debugging = true; // a mettre a falsdebuge pour ne pas debugger
String debugstring = "";



//----------------------------
//declaration bluetooth
//----------------------------

//const boolean init_BT = true; // pour creer la config BT a mettre a false normalement
char BT_name [10] = "Combi";
String OutputString = "";

//Serial input initailisation
String inputString = "";            // a string to hold incoming data
boolean stringComplete = false;     // whether the string is complete
String inputString3 = "";            // a string to hold incoming data
boolean stringComplete3 = false;     // whether the string is complete

// ----------------------------
// declaration pour l ECU
//-----------------------------
int point_RPM = 0; // index dans la carto
int point_KPA = 16; // index dans la carto


volatile long nbr_spark = 0;
volatile float map_value_us;                 // duree du SAW EDIS en microseconds
volatile boolean SAW_requested = false;
unsigned int rev_limit = 4550;               // Max tour minute
unsigned int rev_mini = 500; // min tour minute
volatile int Degree_Avance_calcul = 10;              // Degree_Avance_calculÃ© suivant la cartographie a10 par defaut pour demmarragee
boolean output = true;
boolean fixed = false;              // declare whether to use fixed advance values
const int fixed_advance = 15;             // Avance fixe
boolean multispark = true;         // multispark
volatile boolean first_multispark = true;         // 1er allumage en multispark
int correction_degre = 0; // correction de la MAP
volatile unsigned int tick = 0; // nombre de tick du timer pour le saw
const int msvalue = 2025 ; // normaly 2048 
volatile boolean newvalue = true;  // check si des nouvelles valeurs RPM/pression ont ete calculÃ©s
//gestion simplifie des rpm
volatile unsigned long timeold = 0;
const unsigned int debounce = 4000; // temps mini acceptable entre 2 pip 4ms-> 7500 tr/min 
volatile unsigned long pip_old = 0; // durée du dernier pip pour le debounce
volatile unsigned int pip_count = 0;
volatile unsigned int engine_rpm_average = 0;  // Initial average engine rpm pour demarrage
const unsigned int maxpip_count = 20;  //on fait la moyenne tout les x pip
const int nombre_point_RPM = 23; // nombre de point de la MAP
const int nombre_point_DEP = 17; // nombre de point de la MAP
const int nombre_carto_max = 5; // nombre de carto a stocker
int carto_actuel = 1; //cartographie en cours
volatile boolean ignition_on = false; // gere si le SAW est envoyé non terminé

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
#endif

// variable pression moyenne
int correction_pressure = 400;   // gestion de la valeur du capteur a pression atmo se met a jour lors de lancement
int map_pressure_kpa = 100;
int previous_map_pressure_kpa = 100;
int MAP_accel = 100 ;  // kpa/s d'acceleration
int sum_pressure = 0;
byte count_pressure = 0;
const byte nbre_mesure_pressure = 3;
const int MAP_check_per_S = 16 ; // nombre de calcul par seconde d'acceleration / depression
const int kpa_0V = 0; // Kpa reel lorsque le capteur indique 0V
const byte lissage_kpa = 80 ;  // facteur lissage des KPA 100= pas de lissage 50 = fort lissage
long last_MAP_time = 10; // dernier calcul du TPS
long previous_MAP_time = 0; // avant dernier calcul TPS

// variable de loop
long var1 = 0;
long var2 = 0;
unsigned long time_loop = 0;
unsigned long time_reception = 0;
const unsigned long interval_time_reception = 100; // check des ordres tout les 100 ms secondes

unsigned long time_envoi = 0;
const unsigned long interval_time_envoi = 500; // ECU / EC1 toute les 1 secondes
unsigned long time_check_depression = 0;
const unsigned long interval_time_check_depression = 1000 / (MAP_check_per_S * nbre_mesure_pressure); // Depression x fois / seconde
unsigned long time_check_lambda = 0;
const unsigned long interval_time_check_lambda = 100; // lambda 5 fois / seconde
unsigned long time_check_connect = 0; 
const unsigned long interval_time_check_connect = 30000; // reconnection  1 fois /  30 seconde

unsigned long time_check_fuel_pump = 0; 
const unsigned long interval_time_check_fuel_pump = 5000; // check de la pompe de fuel

unsigned long time_check_TPS = 0;
const unsigned long interval_time_check_TPS = 1000 / (TPS_check_per_S * nbre_mesure_TPS); // TPS x fois / seconde

unsigned long time_log = 0;
const unsigned long interval_time_log = 250; // envoi des log 4 fois par seconde

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
