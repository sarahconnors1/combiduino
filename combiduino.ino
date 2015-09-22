//-------------------------------------------- Include Files --------------------------------------------//
#include <EEPROM.h>
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
//-------------------------------------------- Global variables --------------------------------------------//

// declaration du debugging
boolean debugging = true; // a mettre a falsdebuge pour ne pas debugger
String debugstring = "";

//declaration des pins
int interrupt_X = 3;                // PIP EDIS pin
int SAW_pin = 13;                   // SAW EDIS pin
int MAP_pin = A0;                   // depression

//----------------------------
//declaration bluetooth
//----------------------------
// pin 8 et 9
const boolean init_BT = true; // pour creer la config BT a mettre a false normalement
String OutputString = "";
//Serial input initailisation
String inputString = "";            // a string to hold incoming data
boolean stringComplete = false;     // whether the string is complete

// ----------------------------
// declaration pour l ECU
//-----------------------------
volatile int engine_rpm;            // vitesse actuelle
volatile float map_value_us;                 // duree du SAW EDIS en microseconds
int rev_limit = 4550;               // Max tour minute
int rev_mini = 500; // min tour minute
volatile int Degree_Avance_calcul = 10;              // Degree_Avance_calculé suivant la cartographie a10 par defaut pour demmarragee
boolean output = true;
boolean fixed = false;              // declare whether to use fixed advance values
int fixed_advance = 15;             // Avance fixe
boolean multispark = true;         // multispark
boolean first_multispark = true;         // 1er allumage en multispark

volatile boolean newvalue = true;  // check si des nouvelles valeurs RPM/pression ont ete calculés
//gestion simplifie des rpm
volatile unsigned long timeold = 0;
volatile unsigned int pip_count = 0;
volatile unsigned int engine_rpm_average = 1000;  // Initial average engine rpm pour demarrage
const unsigned int maxpip_count = 20;  //on fait la moyenne tout les x pip

unsigned int point_RPM_actuel = 0; // pour la sortie BT
unsigned int point_KPA_actuel = 0; // pour la sortie BT


// variable pression moyenne
const int numReadings = 10;         // nombre index de pression
int readings[numReadings];          // Array des pressions
int current_index = 0;
unsigned long total = 0;            // ttotal des pressions
unsigned int manifold_pressure = 100;        // pression initial 100kpa= atmos
int correction_pressure = 400;   // gestion de la valeur du capteur a pression atmo se met a jour lors de lancement
int map_pressure_kpa;
int min_pressure_kpa_recorded = 100 ; // log du mini enregistré

// variable de loop
unsigned long time_loop_old = 0;
int loop_count = 0;
float loop_frequence = 0;




const int nombre_point_RPM = 23; // nombre de point de la MAP
const int nombre_point_DEP = 17; // nombre de point de la MAP
const int nombre_carto_max = 5; // nombre de carto a stocker
int correction_degre = 0; // correction de la MAP
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
const int eprom_carto_actuel = 0; // emplacement dans EEPROM
const int eprom_init = 1; // emplacement dans EEPROM de init =100 OK sinon on ecrase les cartos
boolean init_eeprom = true; // si true on re ecrit les carto au demarrage
//-------------------------------------------- Initialise Parameters --------------------------------------------//
void setup() {
  timeold = 0;

  attachInterrupt(1, pip_interupt, FALLING);                                //  interruption PIP signal de l'edis
  pinMode(SAW_pin, OUTPUT);                                                 //  SAW_pin as a digital output
  pinMode(interrupt_X, INPUT);
  digitalWrite (interrupt_X, HIGH);

  for (int thisReading = 0; thisReading < numReadings; thisReading++)       //populate manifold pressure averaging array with zeros
    readings[thisReading] = 100;
  // on initialise la pression atmospherique
  correction_pressure = analogRead(MAP_pin);


  // port serie
  Serial.begin(38400);                                                       // Initialise serial communication at 38400 Baud
  inputString.reserve(1000);   //reserve 200 bytes for serial input - equates to 25 characters

  // Initialisationdu BT
  ble_set_name("Combiduino");
  ble_begin();

  time_loop_old = millis();

  // init des cartos si pas deja fait
  if (String(EEPROM.read(eprom_init) )  != "100" ) {
    init_eeprom = true; // on lance init
  }else{ 
    init_eeprom = false;
  }
    
  // init_eeprom = true; // a detagger pour re initialisation
  
  if (init_eeprom == true) {debug ("Init des MAP EEPROM");init_de_eeprom(); } // on charge les map EEPROM a partir de la RAM
  
  debug ("Init des MAP RAM");
  read_eeprom(); // on charge les MAP en RAM a partir de l'eeprom et la dernière MAP utilisée
  debug ("ready!");
}


//-------------------------------------------- Boucle principal --------------------------------------------//
void loop() {
  loop_count++;

  // check entree serial
  checkdesordres();

  // calcul du nouvel angle d avance si des valeurs ont changés
  if (newvalue == true) {
    Degree_Avance_calcul = rpm_pressure_to_spark(engine_rpm_average, map_pressure_kpa);
    newvalue = false;
  }

  // gestions sortie pour module exterieur
  if ((loop_count == 40) or (loop_count == 60)) {
    gestionsortie();
  }

  // recalcul de la dépression
  gestiondepression();

  // fin de boucle
  if (loop_count > 100) {
    loop_count = 0;
    time_loop_old = millis();
  }


  ble_do_events();
}





