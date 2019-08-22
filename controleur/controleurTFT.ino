// NE pas oublier de changer la taille du buffer dans hardwareserial.h
//#define SERIAL_RX_BUFFER_SIZE 256

#define SD_USED 1

//-- INCLUDE-------
#include <UTFT.h>
#include <SPI.h>
#if SD_USED == 1
  #include <SD.h>
#endif
#include <EEPROM.h>
#include <URTouch.h>
//#include <EasyTransfer.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
#define BIT_CHECK(var,pos) ((var) & (1<<(pos)))

//************ Declaration de l'écran ***********************************************
// select the pins used on the LCD panel
UTFT myGLCD(ITDB32S,38,39,40,41);
// Declare which fonts we will be using
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];


//********** Declaration du tactile **************************************************
#define TOUCH_ORIENTATION  PORTRAIT
URTouch  myTouch( 6, 5, 4, 3, 2);
int x=0;
int y = 0;
// gestion des boutons
int btncount  = 0;
const int btncount_duree = 3; // duree du bouton
int btn_actuel  = 0;
#define btnHIT   1
#define btn2     2
#define btn3     3
#define btnNONE   0
boolean screen_pressed = false;


// ******************* gestion des menus *********************************************
const int nombre_de_menu = 2;
int menu_actuel = 1;
#define menu_principal  1
#define menu_1  2
#define menu_2  3
boolean rebuild_menu = true;
boolean refresh_menu = true;
int count_menu = 0;
int count_menu_max = 4;

// gestion des sous menu du menu 1
int sous_menu_1 = 1;
const int nombre_de_sous_menu_1 = 2;
#define sous_menu_1_carto  1
#define sous_menu_1_MS  2




boolean multispark_demande = true;
boolean multispark_actuel = true;
boolean multispark_change = false;


//******************** Structure des logs***********************************************

typedef struct RECEIVE_DATA_STRUCTURE_t {
  unsigned long time_loop;        // 4
  int CLT ;                       // 2
  byte IAE_actuel ;               // 1
  byte running_option ;           //1 
  unsigned int  engine_rpm_average ;  // 2 
  float map_pressure_kpa  ;       // 4
 // int  MAPdot  ;                  // 2
  unsigned int  injection_time_us  ;  // 2 
  float VE_actuel ;               // 4
  int AFR_actuel ;                // 2
  byte correction_lambda_actuel ; //1  
  float Degree_Avance_calcul ;    // 4
  int TPS_actuel ;                // 2
 // int TPSdot ;                    // 2
  byte running_mode ;             // 1
  byte carto_actuel ;             // 1
  float PID_idle_advance ;        // 4
  byte DEC_actuel  ;              // 1
  int PW_actuel ;                 // 2  
  int qte_paroi ;                 // 2  
  byte X_adher ;                  // 1
  byte Tau_evap   ;               // 1
  byte AFR_objectif ;             // 1
  byte point_KPA ;                // 1
  byte point_RPM ;                // 1
//  int TPSMAPdot ; //                 2    somme des TPSdot et MAPdot pour gerer l'acceleration
//  unsigned int PW_accel_actuel_us ; // 2 accel du dernier calcul en microsecond
  byte VLT ; //                      1   voltage actuel de la batterie
  unsigned int  tps_ouverture_injecteur_us ; // 2 temps d'ouverture des injecteurs suivant la trension
  byte MCE_actuel;                // 1  enrichissement manuel
//  float var1 ;                    // 4 valeur libre TPS ADC
//  float var2;                     // 4 valeur libre 
};
typedef union RECEIVE_DATA_STRUCTURE_TOT {
 RECEIVE_DATA_STRUCTURE_t value;
 byte packet[sizeof(RECEIVE_DATA_STRUCTURE_t)];
};

RECEIVE_DATA_STRUCTURE_TOT ECUdata;
 byte Buffer[sizeof(RECEIVE_DATA_STRUCTURE_TOT) + 64];  // Serial buffer



// ****************** GESTION CARTE SD ***********************************************
const int chipSelect = 53;
String filename = "log";
 byte num_file = 0;
const int EEPROM_file_number = 1;
boolean logheader_received = false;   // flag pour vérifier la reception des entete de log
long nbr_log = 0;
const byte flush_every_cycle =( 512/sizeof(RECEIVE_DATA_STRUCTURE_t)) ;
int flush_cycle = 0; 
const int max_length_log = 100;
unsigned int erreur_reception = 0;


boolean event_requested = false; // flag pour indiquer qu'on a marquer un evenement
#if SD_USED == 1
File dataFile;
#endif
// ******************** gestion des variable ECU ***************************************
int KPA = 99;
int RPM = 900;
int avance = 99 ;
int AFR = 147;
int TPS = 100;
int IAE = 100;
int CLT = 99;
int VLT = 120;
int IOT = 800;
int CorL = 100;
int var1 = 0;
int var2 = 0;
float IDL = -2.1;
byte ENGINE_IDLE_OLD = 1;
byte ENGINE_WARMUP_OLD = 1;
byte ENGINE_DCC_OLD = 1;
byte BIT_EGO_ACTIVE_OLD = 1;


byte running_mode = 255;
#define BIT_ENGINE_RUN      0     // Engine running
#define BIT_ENGINE_CRANK    1   // Engine cranking
#define BIT_ENGINE_DCC_MAPdot  2    // rien ?
#define BIT_ENGINE_WARMUP   3    // Engine in warmup
#define BIT_ENGINE_ACC      4    // in acceleration mode
#define BIT_ENGINE_DCC      5    // in deceleration mode avec coupure
#define BIT_EGO_ACTIVE      6    // correction lambda possible
#define BIT_ENGINE_IDLE     7    // idle on

// gestion des differente option activable
byte running_option = 0;
#define BIT_MS   0     // multispark fait
#define BIT_XTAU_USED   1   // X-TAU fait
#define BIT_DEBUG       2    // DEBUG ON/OFF fait
#define BIT_FIXED_ADV   3    // fixed advance fait
#define BIT_OUTPUT_BT   4    // Output Bluetooth fait
#define BIT_FIRST_MS    5    // first multispark fait
#define BIT_NEW_VALUE   6    // new value temps de recalculer fait
#define BIT_LOG_END     7    // gestion des envoi des log en 2 parties

// gestion enrichissement
const byte MCE1 = 94;
const byte MCE2 = 98;
const byte MCE3 = 100;
const byte MCE4 = 103;
const byte MCE5 = 107;
boolean MCEchange_requested=false;
byte MCE_requested=100;
byte MCE_OLD = 107;


// gestion carto allumage
int carto_demande = 1;
int carto_OLD = 9 ;
boolean carto_change = false;

//Serial input initailisation
String inputString = "                                                                                                ";            // a string to hold incoming data
boolean stringComplete = false;     // whether the string is complete
boolean debugging = true;
boolean SDinitfailed = false;
//String debugstring="";




// ************************ variable loop ***********************************************
unsigned long time_loop = 0;
unsigned long timeold = 0;
unsigned const delai_rafraichissement = 1000 ; // toute les secondes
unsigned const delai_envoiECU = 500 ; // toute les secondes
unsigned long timeold_envoiECU = 0;

void setup()
{
  myGLCD.InitLCD();
  myGLCD.setFont(BigFont);

 // myTouch.InitTouch(TOUCH_ORIENTATION);
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
 
 RAZ_affichage();
 LcdPrint(0,120, "COMBIDUINO"); 

 time_loop = millis();
 Serial.begin(57600);  
 Serial1.begin(57600);  

debug("Initializing SD card..." + String(sizeof(ECUdata) ) );
#if SD_USED == 1
  if (!SD.begin(chipSelect)) {
    LcdPrint(0,0,"init failed.    ");
    SDinitfailed = true;
    delay(10000);
    return;
  }
#endif  


 init_SD_file();
 debug("card initialized.");
 LcdPrint(0,0,"Card initialized.    ");
 delay(5000);
  
  RAZ_affichage();
  gestion_affichage_des_menus();
 initlog();

}


void loop(){
time_loop = millis();
lectureecran();
decodeECU();
// rafraichissemnt des menus complet
  if (time_loop - timeold >= delai_rafraichissement ){timeold = time_loop;refresh_menu = true;gestion_affichage_des_menus(); }

// gestion envoi vers le combiduino
 if (time_loop - timeold_envoiECU >= delai_envoiECU ){timeold_envoiECU = time_loop;envoiECU();}
}
 











