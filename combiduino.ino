#pragma GCC optimize ("-O2")

 
//-------------------------------------------- Include Files --------------------------------------------//
#include <EEPROM.h>
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>


//-------------------------------------------- Global variables --------------------------------------------//

// declaration pour injection 
byte AFR[8] ={180 ,170 ,150 ,147 ,140,130,120,110} ; //valeur AFR * 10
byte AFR_analogique[8] ={10,21,41,47,144,164,175,185}; //valeur lu par le capteur avec echelle 1024


const int AFR_bin_max = 8;
int AFR_actuel =147; // valeur AFR 100 -> 190

unsigned int Req_Fuel_us = 10200;  // ouverture max des injecteurs 100% 
unsigned int injection_time_us = 0; // temps d'injection corrigé pour le timer
const int pin_lambda = A5;





// declaration du debugging
boolean debugging = true; // a mettre a falsdebuge pour ne pas debugger
String debugstring = "";


//declaration des pins
int interrupt_X = 3;                // PIP EDIS pin
int SAW_pin = 13;                   // SAW EDIS pin
int MAP_pin = A0;                   // depression
int pin_ignition = 12;  // pin du début d'ignition
//----------------------------
//declaration bluetooth
//----------------------------
// pin 8 et 9
const boolean init_BT = true; // pour creer la config BT a mettre a false normalement
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
//volatile int engine_rpm;            // vitesse actuelle
volatile float map_value_us;                 // duree du SAW EDIS en microseconds
int rev_limit = 4550;               // Max tour minute
int rev_mini = 500; // min tour minute
volatile int Degree_Avance_calcul = 10;              // Degree_Avance_calculÃ© suivant la cartographie a10 par defaut pour demmarragee
boolean output = true;
boolean fixed = false;              // declare whether to use fixed advance values
int fixed_advance = 15;             // Avance fixe
boolean multispark = true;         // multispark
boolean first_multispark = true;         // 1er allumage en multispark
int correction_degre = 0; // correction de la MAP
volatile unsigned long tick = 0; // nombre de tick du timer pour le saw
const int msvalue = 2025 ; // normaly 2048 

volatile boolean newvalue = true;  // check si des nouvelles valeurs RPM/pression ont ete calculÃ©s
//gestion simplifie des rpm
volatile unsigned long timeold = 0;
volatile unsigned int pip_count = 0;
volatile unsigned int engine_rpm_average = 0;  // Initial average engine rpm pour demarrage
const unsigned int maxpip_count = 20;  //on fait la moyenne tout les x pip


// declaration pour le KNOCK
long knockvalue[23] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // accumulation des valuer de knock
long knockcount[23] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // accumulation des count de knock
long knockmoyen[23] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // accumulation des count moyen

volatile unsigned int cylindre_en_cours = 1; // cylindre en cours d'allumage
volatile boolean ignition_on = false; // gere si le SAW est envoyé non terminé

// Gestion du knock sensor
long total_knock = 0;
long knock_moyen = 0; // moyend de la période
int count_knock = 0;
int delta_knock = 0; // ecart par rapport a la moyenne enregistré
const int count_knock_max = 8; // tout les 8 pip = 4 tour
long var1 = 0;
long var2 = 0;
boolean knock_record = false; // enregistrement des valeur normal du capteur
boolean knock_active = false; // utilisation de la fonction knock


// variable pression moyenne
const int numReadingsKPA = 10;         // nombre index de pression
int current_indexKPA = 0;
unsigned long totalKPA = 0;            // ttotal des pressions
unsigned int manifold_pressure = 100;        // pression initial 100kpa= atmos
int correction_pressure = 400;   // gestion de la valeur du capteur a pression atmo se met a jour lors de lancement
int map_pressure_kpa;
int min_pressure_kpa_recorded = 100 ; // log du mini enregistrÃ©

// variable de loop
unsigned long time_loop = 0;
unsigned long time_envoi = 0;
const unsigned long interval_time_envoi = 200; // ECU / EC1 toute les 1 secondes
unsigned long time_check_depression = 0;
const unsigned long interval_time_check_depression = 100; // Depression 10 fois / seconde
unsigned long time_check_lambda = 0;
const unsigned long interval_time_check_lambda = 100; // lambda 10 fois / seconde





const int nombre_point_RPM = 23; // nombre de point de la MAP
const int nombre_point_DEP = 17; // nombre de point de la MAP
const int nombre_carto_max = 5; // nombre de carto a stocker

int carto_actuel = 1; //cartographie en cours

//----------------------------
// Variable pour eeprom
//----------------------------
int EEPROM_lignecarto[nombre_point_RPM] = { 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // ligne pour ecriture/lecture dans EEPROM
int EEPROM_ligneRPM[nombre_point_RPM] = { 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // ligne pour ecriture/lecture dans EEPROM
int EEPROM_ligneKPA[nombre_point_DEP] = { 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // ligne pour ecriture/lecture dans EEPROM
const int nbr_byte_par_int = 1; // 1 byte
const int eeprom_nombre_max_ligne = 17;
const int eeprom_nombre_max_carto = 8;
const int nbr_byte_par_ligne = nombre_point_RPM * nbr_byte_par_int ; //23 * 2 byte
const int debut_eeprom = 50; //Pour laisser un peu de vide
const int taille_carto = 500; // carto complete avec kpa + rpm
const int debut_kpa = 400; //Pour ecrire les kpa au 400 de chaque MAP
const int debut_rpm = 420; //Pour ecrire les rpm au 420 de chaque MAP
// gestion parametre de eeprom
const int eprom_carto_actuel = 0; // emplacement dans EEPROM
const int eprom_init = 1; // emplacement dans EEPROM de init =100 OK sinon on ecrase les cartos
const int eprom_nom_BLE = 10; // emplaceement eeprom du  nom du BLE
const int eprom_rev_max = 21; // emplaceement eeprom du  REV MAX
const int eprom_rev_min = 23; // emplaceement eeprom du  REV MIN
const int eprom_debug = 2; // emplaceement eeprom du  debug
const int eprom_knock = 6; // emplaceement eeprom du knock sensor
const int eprom_reboot = 7; // emplacement eeprom du knock moyen

const int eprom_ms = 3; // emplaceement eeprom du  multispark
const int eprom_avance = 4; // emplaceement eeprom de avance initiale
const int eprom_adresseknock = 25; // emplacement eeprom du knock moyen

boolean init_eeprom = true; // si true on re ecrit les carto au demarrage
//-------------------------------------------- Initialise Parameters --------------------------------------------//
void setup() {
  // comptage du nbr de reboot
  rebootcount();
  
  // pour ne pas lancer de faux SAW
  detachInterrupt(1);
  

  pinMode(SAW_pin, OUTPUT);                                                 //  SAW_pin as a digital output
  pinMode(pin_ignition, OUTPUT); 
  pinMode(interrupt_X, INPUT);
 digitalWrite (interrupt_X, HIGH);

 pinMode(pin_lambda, INPUT);
 digitalWrite (pin_lambda, LOW );

  // port serie
  Serial.begin(115200);  
  while (!Serial) { ; }// wait for serial port to connect. 
  
   Serial1.begin(115200);  
  while (!Serial1) { ; }// wait for serial port to connect. 
  
  inputString.reserve(50);   //reserve 200 bytes for serial input - equates to 25 characters
  OutputString.reserve(50);

 
  time_loop = millis();

  // init des cartos si pas deja fait
  if (String(EEPROM.read(eprom_init) )  != "100" ) { init_eeprom = true; }else{init_eeprom = false;} // on lance l'init de l'eeprom si necessaire   
//---------POUR INIT CARTE -------------------------------  
 // init_eeprom = true; // A DE TAGGER POUR INITIALISATION 
// ------------------------------------------------------     
if (init_eeprom == true) {debug ("Init des MAP EEPROM");init_de_eeprom();RAZknock(); } // on charge les map EEPROM a partir de la RAM et konco moyen
 // lecture des carto
  debug ("Init des MAP RAM");
  read_eeprom(); // on charge les MAP en RAM a partir de l'eeprom et la derniÃ¨re MAP utilisÃ©e
  initknock();
 cli(); // stop les interruptions 
   // Initialisationdu BT
  ble_set_name(BT_name);
  ble_begin();  
 
 initpressure();
 // initialisation du PIP interrupt 
   attachInterrupt(1, pip_interupt, FALLING);                                //  interruption PIP signal de l'edis
 sei(); // c'est parti
 debug ("ready!");
debug("reboot nbr " + String(var2));
}


//-------------------------------------------- Boucle principal --------------------------------------------//
void loop() {
   time_loop = millis();


// check entree serial
  checkdesordres();


//chek des valeur de knock
       getknock();

// calcul du nouvel angle d avance si des valeurs ont changÃ©s
  if (newvalue == true) { calculdelavance(); newvalue = false;  }

// puis on gere les fonctions annexe a declencher periodiquement

  // recalcul de la dÃ©pression
  if (time_loop - time_check_depression > interval_time_check_depression){gestiondepression();time_check_depression = time_loop;}

  // recalcul de la lambda
  if (time_loop - time_check_lambda > interval_time_check_lambda){lecturelambda();time_check_lambda = time_loop;}


// gestions sortie pour module exterieur ECU / EC1
  if (time_loop - time_envoi > interval_time_envoi){gestionsortieECU();gestionsortieEC1();time_envoi = time_loop;}

ble_do_events();
}




void initpressure(){
  correction_pressure = 0;
  // on initialise la pression atmospherique
  for (int j = 0 ; j < 10; j++) { 
    correction_pressure += analogRead(MAP_pin);
  }
 correction_pressure =  (correction_pressure / 10);
}
 



