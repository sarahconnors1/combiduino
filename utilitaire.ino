//-----------------------------------
//         UTILITAIRE
//----------------------------------
// gestion du mode debug avec renvoi vers la console
void debug(String str){if (debugging == true){ Serial.println(str);} }

void sndlog(String str){ Serial.println(str);} 

// gestion du parsing
String getValue(String data, char separator, int index){
    // spliting a string and return the part nr index
    // split by separator
    int stringData = 0;        //variable to count data part nr 
    String dataPart = "";      //variable to hole the return text
    for(unsigned int i = 0; i<data.length(); i++) {    //Walk through the text one letter at a time
      if(data[i]==separator) { stringData++; //Count the number of times separator character appears in the text
      }else if(stringData==index) {dataPart.concat(data[i]); //get the text when separator is the rignt one  
      }else if(stringData>index) {return dataPart;break;//return text and stop if the next separator appears - to save CPU-time
      }
    }
    return dataPart; //return text if this is the last part
}
//-----------------------------------------------------------------------------------------------
//                        ROUTINE EPROM
//-----------------------------------------------------------------------------------------------

//This function will write a 2 byte integer to the eeprom at the specified address and address + 1
void EEPROMWriteInt(int p_address, int p_value){
     byte lowByte = ((p_value >> 0) & 0xFF);
     byte highByte = ((p_value >> 8) & 0xFF);

     EEPROM.write(p_address, lowByte);
     EEPROM.write(p_address + 1, highByte);
}

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int EEPROMReadInt(int p_address){
     byte lowByte = EEPROM.read(p_address);
     byte highByte = EEPROM.read(p_address + 1);

     return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
 }
 
//-----------------------------------------------------------------------------------------------
//                        ROUTINE INTERPOLATION
//-----------------------------------------------------------------------------------------------

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//-------------------------------------------- Triggered when serial input detected --------------------------------------------//

 # if KNOCK_USED == 1
 void serialEvent1() {
 while (Serial1.available()) {                    //whilst the serial port is available...
   inputString3 = Serial1.readStringUntil('\n');   //... read in the string until a new line is recieved
   stringComplete3 = true;                      
 }
 }
#endif

//-----------------------------------------------------------------------------------------------
//                       GESTION DES MESURES
//-----------------------------------------------------------------------------------------------


//-------------------------------------
// MESURE DEPRESSION
//-------------------------------------
void gestiondepression(){
sum_pressure += analogRead(MAP_pin);
count_pressure++;
if (count_pressure >= nbre_mesure_pressure){
   int kpa_moyen = sum_pressure / count_pressure;  
   float kpa = mapfloat(kpa_moyen,0,correction_pressure,kpa_0V,101);  // on converti la moyenne en KPA   
   count_pressure = 0;
   sum_pressure = 0;
  // on normalise 
  #if VACUUMTYPE == 2 // avant papillon
    if (kpa < pressure_axis[0]){kpa = pressure_axis[0]; }
  #endif
  #if VACUUMTYPE == 1 // collecteur
    if (kpa < pressure_axis[nombre_point_DEP-1]) {kpa = pressure_axis[nombre_point_DEP-1]; }
  #endif
   if (kpa != map_pressure_kpa ) {newvalue=true;}
  // on lisse
  if (BIT_CHECK(running_mode,BIT_ENGINE_IDLE )){ // si on est en mode idle
    lissage_kpa = lissage_kpa_idle;
  }else{
    lissage_kpa = lissage_kpa_running;
  }
  
  kpa = map_pressure_kpa + ( (kpa - map_pressure_kpa) * lissage_kpa / float(100) );

// nouvelle valeur   
  map_pressure_kpa = kpa;
  last_MAP_time = millis();

}   
   
}  
//-------------------------------------
// MESURE TPS
//-------------------------------------
void gestionTPS(){
  sum_TPS += analogRead(TPS_pin);
  count_TPS++;
// debug(String(analogRead(TPS_pin)));
 if (count_TPS >= nbre_mesure_TPS){
   int TPS_moyen = sum_TPS / count_TPS;  
   //var1 = TPS_moyen;
   if (TPS_moyen<tps_lu_min){TPS_moyen=tps_lu_min;} // check valeur mini
   
   int TPS = map(TPS_moyen,tps_lu_min,tps_lu_max,0,100);  // on converti la moyenne en %  
   count_TPS = 0;
   sum_TPS = 0;
   if (TPS != TPS_actuel ) {newvalue=true;}

   if ( abs(TPS - TPS_actuel) > TPS_ecart_representatif ){ // si ecart est representatif
   TPS_actuel = TPS;
   }
   
   last_TPS_time = millis();

   gestionTPSMAPdot();
 }
}

//-------------------------------------
// MESURE Temperature
//-------------------------------------
void gestionCLT(){
  int CLT_lu = analogRead(CLT_pin);
   CLT = map(CLT_lu,lowtemp_lu,hightemp_lu, lowtemp, hightemp);  // on converti la moyenne en %  
  // Serial.println(" temp degre " + String(CLT) + " CLT lu = " + String(CLT_lu) );
}

//-----------------------------------------------------------
//  MESURE LAMBDA
//-----------------------------------------------------------

// gestion de la lecture de la lambda
void lecturelambda(){
int afr_lu = 0;
#if LAMBDATYPE == 2 // narrowband
 int point_afr = 0;
 analogReference(INTERNAL1V1);
 afr_lu = analogRead(pin_lambda);   
 analogReference(DEFAULT);
#endif

#if LAMBDATYPE == 1 //wideband
 afr_lu = analogRead(pin_lambda);
#endif

sum_lambda += afr_lu;
count_lambda++;


  if (count_lambda >= nbre_mesure_lambda){
    afr_lu = sum_lambda / count_lambda;
    count_lambda = sum_lambda = 0;
//Serial.println("lambda " + String(afr_lu) );
#if LAMBDATYPE == 2 // narrowband 
    if (afr_lu > AFR_analogique[AFR_bin_max -1]) { // borne maxi  
      AFR_actuel = AFR[AFR_bin_max -1];
    }  else if (afr_lu <AFR_analogique[0]){ // borne mini
      AFR_actuel = AFR[0];
    } else {
      point_afr = decode_afr(afr_lu);
      AFR_actuel = map(afr_lu,AFR_analogique[point_afr],AFR_analogique[point_afr + 1],AFR[point_afr],AFR[point_afr + 1]) ; // on fait une interpolation
    }
#endif

#if LAMBDATYPE == 1 //wideband
  AFR_actuel = map(afr_lu,0,1023,86,190) ; // on fait une interpolation lineaire
#endif

  }
} 


//--------------------------------------------
// calcul du nombre de KPA / TPS par seconde 
//--------------------------------------------
void gestionTPSMAPdot(){

// Pour TPS ///////////////////////////////
  if ( (last_TPS_time - previous_TPS_time) > 0 ){
    TPS_accel = (TPS_actuel - previous_TPS) * float(1000)/ (last_TPS_time - previous_TPS_time) ; // calcul en %/S
  }else{
    TPS_accel = TPS_accel; // si pas de nouvel valeur
  }   
  if (TPS_accel < -50 ){TPS_accel = -50;} // sanity check
  previous_TPS =  TPS_actuel;
  previous_TPS_time = last_TPS_time;

// pour KPA ///////////////////
  if ( (last_MAP_time - previous_MAP_time) > 0 ){
      MAP_accel = (map_pressure_kpa - previous_map_pressure_kpa) * float(1000) / (last_MAP_time - previous_MAP_time) ; // calcul en %/S
  }else{
      MAP_accel = MAP_accel; // si pas de nouvel valeur
  }   
  if (MAP_accel < -50 ){MAP_accel = -50;} // sanity check

  previous_map_pressure_kpa =  map_pressure_kpa;
  previous_MAP_time = last_MAP_time;
}

//------------------------  
// RETROUVE POINT RPM
//------------------------
byte decode_rpm(int rpm_) { // renvoi la valeur infÃ©rieur du bin
  byte map_rpm = 0;
   if(rpm_ <rpm_axis[0]){                // check si on est dans les limites haute/basse
     map_rpm = 0;
   } else { 
     if(rpm_ >=rpm_axis[nombre_point_RPM - 1]) {      // 
       map_rpm = nombre_point_RPM - 1;      
     }else{
       // retrouve la valeur inferieur 
       while(rpm_ > rpm_axis[map_rpm]){map_rpm++;} // du while
       if (map_rpm > 0){map_rpm--;}
     }
   }
  return map_rpm;
}

//------------------------  
// RETROUVE POINT KPA
//------------------------
#if VACUUMTYPE == 2
byte decode_pressure(int pressure_) { // renvoi la valeur infÃ©rieur du bin
   byte map_pressure = 0;
   if(pressure_ < pressure_axis[0]){
     map_pressure = 0;
   }else if (pressure_ > pressure_axis[nombre_point_DEP -1]) {
     map_pressure = nombre_point_DEP -1 ;
   }else{
     // retrouve la valeur inferieur 
     while(pressure_ > pressure_axis[map_pressure]){map_pressure++;}
     if (map_pressure > 0){map_pressure--;}
   }
   return map_pressure;
}
#endif

#if VACUUMTYPE == 1
byte decode_pressure(int pressure_) { // renvoi la valeur infÃ©rieur du bin
   byte map_pressure = 0;
   if(pressure_ > pressure_axis[0]){
     map_pressure = 0;
   }else if (pressure_ < pressure_axis[nombre_point_DEP -1]) {
     map_pressure = nombre_point_DEP -1 ;
   }else{
     // retrouve la valeur inferieur 
     while(pressure_ < pressure_axis[map_pressure]){map_pressure++;}
   }
   return map_pressure;
}
#endif

//------------------------  
// RETROUVE POINT AFR
//------------------------
#if LAMBDATYPE == 2 // narrowband 
 // Renvoi le bin AFR (0 -> 8)
int decode_afr(int afr_) {
  int map_afr = 0;
   if(afr_ <AFR_analogique[0]){                // check si on est dans les limites haute/basse
     map_afr = 0;
   } else { 
     if(afr_ >=AFR_analogique[AFR_bin_max -1]) {      // 
       map_afr = AFR_bin_max - 1;      
     }else{
       // retrouve la valeur inferieur 
       while(afr_ > AFR_analogique[map_afr]){map_afr++;} // du while
       if (map_afr > 0){map_afr--;}
     }
    return map_afr;
  }
  return map_afr;
}
#endif 

