//-- INCLUDE-------
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>


//************ Declaration de l'écran ***********************************************
// select the pins used on the LCD panel
const int lcdSelect = 2;

LiquidCrystal lcd(8, 9, lcdSelect, 5, 6, 7);

// gestion des boutons
int lcd_key     = 0;
#define btnMENU   1
#define btnRIGHT  2
#define btnLEFT   3
#define btnUP     4
#define btnDOWN   5
#define btnNONE   0
int adc_key_in  = 0;
int pinbouton = A0;  // pin des boutons


// ****************** GESTION CARTE SD ***********************************************
const int chipSelect = 4;
String filename = "log";
const int EEPROM_file_number = 1;
boolean logheader_received = false;   // flag pour vérifier la reception des entete de log
long nbr_log = 0;
const int max_length_log = 100;
// ******************* gestion des menus *********************************************
const int nombre_de_menu = 2;
int menu_actuel = 1;
#define menu_principal  1
#define menu_1  2
#define menu_2  3
boolean rebuild_menu = true;
boolean refresh_menu = true;
// gestion des sous menu du menu 1
int sous_menu_1 = 1;
const int nombre_de_sous_menu_1 = 2;
#define sous_menu_1_carto  1
#define sous_menu_1_MS  2

// gestion des variables sous menu
int carto_demande = 1;
int carto_actuel = 1 ;
boolean carto_change = false;
const int carto_max = 3;

boolean multispark_demande = true;
boolean multispark_actuel = true;
boolean multispark_change = false;


//******************** Structure des logs***********************************************

#define pos_time_loop 0
#define pos_CLT 1 
#define pos_IAE_actuel  2 
#define pos_BIT_MS  3 
#define pos_engine_rpm_average  4 
#define pos_map_pressure_kpa  5  
#define pos_MAP_accel 6 
#define pos_BIT_ENGINE_MAP  7   
#define pos_PW_accel_actuel_us  8  
#define pos_injection_time_us 9    
#define pos_VE_actuel 10  
#define pos_AFR_actuel  11   
#define pos_correction_lambda_actuel  12  
#define pos_Degree_Avance_calcul  13    
#define pos_TPS_actuel  14  
#define pos_TPS_accel 15  
#define pos_BIT_ENGINE_IDLE 16  
#define pos_carto_actuel  17  
#define pos_PID_idle_advance  18  
#define pos_DEC_actuel  19  
#define pos_PW_actuel 20 
#define pos_qte_paroi 21 
#define pos_X_adher 22 
#define pos_Tau_evap  23 
#define pos_AFR_objectif  24
#define pos_point_KPA  25
#define pos_point_RPM  26

// ******************** gestion des variable ECU ***************************************
int KPA = 100;
int RPM = 9000;
int avance = 10 ;
int AFR = 147;

//Serial input initailisation
String inputString = "";            // a string to hold incoming data
boolean stringComplete = false;     // whether the string is complete
boolean debugging = false;
String debugstring="";

// ************************ variable loop ***********************************************
unsigned long time_loop = 0;
unsigned long timeold = 0;
unsigned const delai_rafraichissement = 300 ; // toute les secondes
unsigned const delai_envoiECU = 1000 ; // toute les secondes
unsigned long timeold_envoiECU = 0;

void setup()
{
 lcd.begin(16, 2);              // start the library
 delay(2000);
 RAZ_affichage();
 LcdPrint(0,0, "Combiduino init"); 

 time_loop = millis();
  Serial.begin(115200);  
 inputString.reserve(200);   

 init_SD_file();
 debug("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    LcdPrint(0,0,"init failed.    ");
    return;
  }
  debug("card initialized.");

  delay(2000);
  gestion_affichage_des_menus();
  RAZ_affichage();
}


unsigned const delai_logheader = 1000 ; // toute les secondes
unsigned long timeold_logheader = 0;
 
void loop(){
time_loop = millis();
//serialEvent();
decodeECU();

// rafraichissemnt des menus complet
  if (time_loop - timeold >= delai_rafraichissement ){timeold = time_loop;gestion_des_actions();refresh_menu = true;gestion_affichage_des_menus(); }

// gestion init des log
 if (time_loop - timeold_envoiECU >= delai_envoiECU ){timeold_envoiECU = time_loop;envoiECU();}

}
 
// ----------------------------------
//  Recuperation des infos de l'ECU
//-----------------------------------

void serialEvent() {
  if (Serial.available()) {
   inputString = Serial.readStringUntil('\n');   //... read in the string until a new line is recieved
   stringComplete = true;                     
  }
}

//---------------------------------
// Gestion ENTREES/SORTIES vers l'ECU
//---------------------------------
void decodeECU(){
inputString.trim();

// check des double lignes de data et annulation si necessaire 
if  ( ( logheader_received == true) && (inputString.length() >  max_length_log) ) { 
    inputString = "";
    stringComplete = false; 
}

// check des double ligne d'entete et annulation si deja recu
if  ( ( logheader_received == true) && inputString.startsWith("Time") ) {
    inputString = "";
    stringComplete = false; 
}

// check si on recoit une entete
if (inputString.startsWith("Time") ) { // entete recu
     logheader_received = true;
}

if ( (stringComplete == true ) && ( logheader_received == true) ) { 
      write_SD_log();
      nbr_log++; 
      RPM = getValue(inputString,';',pos_engine_rpm_average).toInt();
      KPA = getValue(inputString,';',pos_map_pressure_kpa).toInt();
      AFR = getValue(inputString,';',pos_AFR_actuel).toInt(); 
      avance = getValue(inputString,';',pos_Degree_Avance_calcul).toInt(); 
      multispark_actuel = getValue(inputString,';',pos_BIT_MS ).toInt() == 1? true : false ;
      stringComplete = false;
      inputString = ""; 
}
 
}



void envoiECU(){

// demande entete de log
 if (!logheader_received){
    Serial.println("init log" );
  }

 // envoi de la nouvelle carto a l'ECU
  if (carto_change == true ){
   carto_change = false;
   carto_actuel = carto_demande;
   Serial.println("carto " + String(carto_demande) );
   message("  carto changé");
   rebuild_menu = true;
   refresh_menu = true;
   gestion_affichage_des_menus();
  }
  
  // envoi du multispark à l'ECU
  if (multispark_change == true ){
     multispark_change = false;
     multispark_actuel = multispark_demande;
     if (multispark_demande == true){
       Serial.println("ms on");
       message(" multispark ON");
     }else{
       Serial.println("ms off");
       message(" multispark OFF");
     }    
   rebuild_menu = true;
   refresh_menu = true;
   gestion_affichage_des_menus();
  }
  
}






//----------------------
// lecture des boutons
//----------------------
int read_LCD_buttons(){
 adc_key_in = analogRead(pinbouton);      // read the value from the sensor 
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnMENU;   
 return btnNONE;  // when all others fail, return this...
}
//---------------------------------------
// RAZ des 2 lignes d' affichage 
//---------------------------------------
void RAZ_affichage(){ 
lcd.clear();
}

