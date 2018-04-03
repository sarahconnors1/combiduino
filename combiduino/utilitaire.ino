//-----------------------------------
//         UTILITAIRE
//----------------------------------
// gestion du mode debug avec renvoi vers la console
void debug(String str){if (BIT_CHECK(ECU.running_option,BIT_DEBUG) ){ Serial.println(str);} }

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

//------------------------------------------------------------------------------------
//                      Routine log du temps par routine
//------------------------------------------------------------------------------------

void deb(){
#if LOG_PERF == 1 
  temp=micros();
# endif  
}

void fin(String s){
#if LOG_PERF == 1 
  Serial.println(s + ";" + String(micros() - temp)) ;
# endif  
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
   count_pressure = 0;  sum_pressure = 0;
  // on normalise 
  #if VACUUMTYPE == 2 // avant papillon
    if (kpa < pressure_axis[0]){kpa = pressure_axis[0]; }
  #endif
  #if VACUUMTYPE == 1 // collecteur
    if (kpa < pressure_axis[nombre_point_DEP-1]) {kpa = pressure_axis[nombre_point_DEP-1]; }
  #endif
  
  // nouvelle valeur   
   sbi(ECU.running_option,BIT_NEW_VALUE); // on declenche un calcul d'injectrion
   ECU.map_pressure_kpa = ECU.map_pressure_kpa + ( (kpa - ECU.map_pressure_kpa) * lissage_kpa_running / float(100) ); // on lisse
  }   
   
}  
//-------------------------------------
// MESURE TPS
//-------------------------------------
void gestionTPS(){
  sum_TPS += analogRead(TPS_pin);
  count_TPS++;
 if (count_TPS >= nbre_mesure_TPS){
   int TPS_moyen = sum_TPS / count_TPS;  
   ECU.var1 = TPS_moyen;  // pour avoir le mini/maxi dans les logs
   if (TPS_moyen<tps_lu_min){TPS_moyen=tps_lu_min;} // check valeur mini
   if (TPS_moyen>tps_lu_max){TPS_moyen=tps_lu_max;} // check valeur maxi
   
   int TPS = map(TPS_moyen,tps_lu_min,tps_lu_max,0,100);  // on converti la moyenne en %  
   count_TPS = 0; sum_TPS = 0;
   
   // on met  à jour 
   sbi(ECU.running_option,BIT_NEW_VALUE); // nouveau calcul d'injection demandé
   ECU.TPS_actuel = ECU.TPS_actuel + ( (TPS - ECU.TPS_actuel) * lissage_TPS_running / float(100) ); // on lisse
   ECU.TPS_actuel = TPS;
 }
}

//-------------------------------------
// MESURE Temperature
//-------------------------------------
void gestionCLT(){
  int CLT_lu = analogRead(CLT_pin);
   ECU.CLT = map(CLT_lu,lowtemp_lu,hightemp_lu, lowtemp, hightemp);  // on converti la moyenne en %  
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

#if LAMBDATYPE == 2 // narrowband 
    if (afr_lu > AFR_analogique[AFR_bin_max -1]) { // borne maxi  
      ECU.AFR_actuel = AFR[AFR_bin_max -1];
    }  else if (afr_lu <AFR_analogique[0]){ // borne mini
      ECU.AFR_actuel = AFR[0];
    } else {
      point_afr = decode_afr(afr_lu);
      ECU.AFR_actuel = map(afr_lu,AFR_analogique[point_afr],AFR_analogique[point_afr + 1],AFR[point_afr],AFR[point_afr + 1]) ; // on fait une interpolation
    }
#endif

#if LAMBDATYPE == 1 //wideband
  byte AFR_temp ;
  AFR_temp = map(afr_lu,0,1023,86,190) ; // on fait une interpolation lineaire
  ECU.AFR_actuel = ECU.AFR_actuel + ( (AFR_temp - ECU.AFR_actuel) * lissage_AFR / float(100) );
#endif

  }
} 


//--------------------------------------------
// calcul du nombre de KPA / TPS par seconde 
//--------------------------------------------
void gestionTPSMAPdot(){
// Pour TPS ///////////////////////////////
  ECU.TPSdot = (ECU.TPS_actuel - previous_TPS) * float(1000)/ (interval_time_check_TPSMAPdot) ; // calcul en %/S
  if (ECU.TPSdot < -50 ){ECU.TPSdot = -50;} // sanity check
  previous_TPS =  ECU.TPS_actuel;

// pour KPA ///////////////////
      ECU.MAPdot = (ECU.map_pressure_kpa - previous_map_pressure_kpa) * float(1000) / (interval_time_check_TPSMAPdot) ; // calcul en kpa/S
  if (ECU.MAPdot < -50 ){ECU.MAPdot = -50;} // sanity check
  previous_map_pressure_kpa =  ECU.map_pressure_kpa;

// Calcul du facteur  d'acceleration qui servira pour la pompe de reprise
  sbi(ECU.running_option,BIT_NEW_VALUE); // nouveau calcul d'injection demandé
  ECU.TPSMAPdot = (ECU.MAPdot * MAPdot_weight + ECU.TPSdot * TPSdot_weight) /100; 
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


// routine de calcul des RPM moyen basé sur les x dernier pip
void calculRPM(){
  ECU.engine_rpm_average = (30000000 * maxpip_count) / (time_total );

// calcul de la valeur mini entre 2 PIP  
if  ( (BIT_CHECK(ECU.running_mode, BIT_ENGINE_RUN))  // moteur tournant
      and !(BIT_CHECK(ECU.running_mode, BIT_ENGINE_CRANK))  // pas de cranking
    and ECU.engine_rpm_average>1100 //pour eviter overflox
     ){   
        debouncePIP = time_total * debouncepercent / maxpip_count ;
     }else{
        debouncePIP = 65500; 
     }
}

