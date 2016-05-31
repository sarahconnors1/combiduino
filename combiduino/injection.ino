

//----------------------------------------------------------
//         Calcul de l'injection
//----------------------------------------------------------

void calcul_injection(){

#if INJECTION_USED == 1  

#if TPS_USED == 1
  PW_accel_actuel_us =  MAP_ACCEL(TPS_accel);
#endif
#if TPS_USED == 0
  PW_accel_actuel_us =  MAP_ACCEL(MAP_accel);
#endif

correction_lambda_actuel = correction_lambda();
VE_actuel = VE_MAP();
int WAS_actuel = WAS();
IAE_actuel = IAE();


  // le temps d'injection est :  temps d'ouverture injecteur + le max a pleine puissance (Req_Fuel) * VE de la map en % * Correction enrichissement en % * corection lambda en %
 
   injection_time_us =  Req_Fuel_us  
  * (VE_actuel/float(100) )
  * (map_pressure_kpa / float(100) )
  * (correction_lambda_actuel /float(100)  )
  * (WAS_actuel / float(100)  )
   * (IAE_actuel / float(100)  )
  ;

tick_injection  = ( injection_time_us  + injector_opening_time_us + PW_accel_actuel_us )* prescalertimer5 ;
#endif
}

//--------------------------------------------------------
// Enrichissemnt a prÃ¨s dÃ©marrage pendant une pÃ©riode donnÃ©e
//--------------------------------------------------------
int WAS(){
if (nbr_spark < after_start_nb_spark ){ // on corrige tous les x tour 
  return enrichissement_after_start;
}else{
  return 100;
}
}

//--------------------------------------------------------
// Enrichissemnt suivant la temperatur moteur
//--------------------------------------------------------
int IAE(){
int IAEtemp = 100;

// pas de surenrichissement si afterstart en cours

#if CLT_USED==1
  if (CLT < IAElowtemp ) {return lowtemp_enrich;}
  if (CLT > IAEhightemp ) {return hightemp_enrich;}
  IAEtemp =  map(CLT ,IAElowtemp,IAEhightemp, lowtemp_enrich, hightemp_enrich);  // on converti la moyenne en % 
#endif

  return IAEtemp;
}


// ----------------------------------------------------------
// recherche du coefficient VE a appliquer suivant la MAP
// ----------------------------------------------------------
float VE_MAP(){
  float VE = 0;
  //                              rpm_percent_low   rpm_percent_high
 //                                 lowRPM             highRPM
 //  kpa_percent_high    highkpa      C                  D       
 //  kpa_percent_low     lowkpa       A                  B       
 
VE = binC * fuel_map [pressure_index_high] [rpm_index_low] / float(100)
   + binD * fuel_map [pressure_index_high] [rpm_index_high] / float(100)
   + binA * fuel_map [pressure_index_low] [rpm_index_low] / float(100)
   + binB * fuel_map [pressure_index_low] [rpm_index_high] / float(100)
   ;
return VE;
}

//----------------------------------------------------
//     Gestion de l'enrichiseemnt a l'acceleration
//----------------------------------------------------

// renvoi une valeur dependante de l'acceleration (kpa/s ) 
// on compare l'acceleration avec le tableau MAP_kpas
// on renvoie l'enrichissement du tableau MAP_acceleration

int MAP_ACCEL(int BASE_accel){
 float extrafuel = 0; // 0 = Pas d'enrichissement
 byte bin = 0;
 byte bin1 = 0;
 int acc_ = 0;
if ( (engine_rpm_average >RPM_ACC_max) or ( !BIT_CHECK(running_mode,BIT_ENGINE_RUN) )) { // pas d'acceleration si > a un certain regime ou arrete
  return 0;
}else{

if ( (nbr_spark < last_accel_spark + accel_every_spark) and ( BIT_CHECK(running_mode,BIT_ENGINE_MAP)) and (saved_accel>BASE_accel)    ){ 
  // si en cours d'acceleraion et accel pas fini et acceleration actuel < accerleration initiale
if  (nbr_spark > last_accel_spark + accel_every_spark/2 ){ // diminution par 2 de la qté essence en fin d'accel
    PW_accel_actuel_us = PW_accel_actuel_us / 2;
}
  return PW_accel_actuel_us;

  
  
}else{ // si pas en cours d'accell
  if (BASE_accel > accel_mini){
    if(BASE_accel <MAP_kpas[0]){                // check si on est dans les limites haute/basse
       bin = 0;
       bin1 = 0;
    } else { 
      if(BASE_accel >=MAP_kpas[MAP_acc_max -1]) {      // 
          bin = MAP_acc_max -1;
          bin1 = MAP_acc_max -1;
      }else{
          // retrouve la valeur inferieur 
         while(BASE_accel > MAP_kpas[bin]){bin++;} // du while
         if (bin > 0){bin--;}
         bin1 = bin +1;
      }
    }
  
    if (bin == bin1){
      acc_ = MAP_acceleration[bin] ;  
    }else{
      // interpolation
      acc_ = map(BASE_accel , MAP_kpas[bin] , MAP_kpas[bin+1], MAP_acceleration[bin], MAP_acceleration[bin+1] );
    }   
    extrafuel =  float(Req_Fuel_us  / float(100) );
    extrafuel = extrafuel * acc_ ;
        
    if (extrafuel == 0){
      cbi(running_mode,BIT_ENGINE_MAP);
    } else { // on demarre un nouvelle accel
      sbi(running_mode,BIT_ENGINE_MAP);
      last_accel_spark = nbr_spark;    
    }
    saved_accel = BASE_accel; // on sauvegarde l'accel actuel 
    return extrafuel;
 
  }else{ // si inferieur au mini
   // si en dessou du mini
   cbi(running_mode,BIT_ENGINE_MAP);
    return 0;  
  }
 }
}
}


void injection_initiale(){
  // ouverture initiale pour purger les injecteurs
    digitalWrite(pin_injection,HIGH);  // send output to logic level HIGH (5V)
      delay(ouverture_initiale);
    digitalWrite(pin_injection,LOW);  // send output to logic level HIGH (5V)  
}

void checkpump(){
 
  // check pour arreter la pompe
  if (BIT_CHECK(running_mode, BIT_ENGINE_RUN) || BIT_CHECK(running_mode, BIT_ENGINE_CRANK) )  { // si c est pas arrete (running ou cranking)
    digitalWrite(pin_pump,LOW); 
  }else{
    digitalWrite(pin_pump,HIGH);
  }
}



// ----------------------------------------------------------
// recherche du coefficient  a appliquer suivant la valeur LAMBDA
// ----------------------------------------------------------
byte correction_lambda(){
float EGO_temp = 100;  
if (  (correction_lambda_used == true ) and (!BIT_CHECK(running_mode,BIT_ENGINE_MAP)  )  and (!BIT_CHECK(running_mode,BIT_ENGINE_IDLE ) )  ) { //correction lambda et pas d'acceleration en cours et pas au ralenti 
// correction ralenti  if (  (correction_lambda_used == true ) and (!BIT_CHECK(running_mode,BIT_ENGINE_MAP)  )    ) { //correction lambda et pas d'acceleration en cours et pas au ralenti 
  
  //                              rpm_percent_low   rpm_percent_high
 //                                 lowRPM             highRPM
 //  kpa_percent_high    highkpa      C                  D       
 //  kpa_percent_low     lowkpa       A                  B       
 
EGO_temp = binC * Ego_map [pressure_index_high] [rpm_index_low] / float(100)
   + binD * Ego_map [pressure_index_high] [rpm_index_high] / float(100)
   + binA * Ego_map [pressure_index_low] [rpm_index_low] / float(100)
   + binB * Ego_map [pressure_index_low] [rpm_index_high] / float(100)
   ;
    return EGO_temp; // correction historique
  }else{
    return 100;
  }
}

//----------------------------------------------------------------
// Calcul de la valeur de correction lambda a  appliquer au passee
//----------------------------------------------------------------
void AFR_self_learning(){
  int delay_bin = 0; // nombre de bin dans le passÃ©
  int delay_bin_number = 0; // numero du bin a analysÃ© dans le passÃ©
  byte objectif = 130;
  byte Ego = 100;
  float correction = 0;
  float erreur = 0;
  byte point_RPM_hist = 0; // point RPM historique 
  byte point_KPA_hist = 0; // point KPA historique 

// 1ere etape on stock les points RPM, KPA actuelle pour utilisation future
// si c'est non representatif on met 99 
AFR_log_bin_actuel++;
if (AFR_log_bin_actuel >= AFR_log_maxbin) {AFR_log_bin_actuel = 0;}

if (  (correction_lambda_used == true ) and (!BIT_CHECK(running_mode,BIT_ENGINE_MAP)  )  and (!BIT_CHECK(running_mode,BIT_ENGINE_IDLE ) ) and (BIT_CHECK(running_mode,BIT_ENGINE_RUN ) )  ) { //correction lambda et pas d'acceleration en cours et pas au ralenti

  if (binA > seuil_pourcent){ //test representatif
    AFR_log_RPM[AFR_log_bin_actuel] = point_RPM;        
    AFR_log_KPA[AFR_log_bin_actuel] = point_KPA;
  }else if (binB > seuil_pourcent){ //test representatif
    AFR_log_RPM[AFR_log_bin_actuel] = rpm_index_high;        
    AFR_log_KPA[AFR_log_bin_actuel] = pressure_index_low;
  }else if (binC > seuil_pourcent){ //test representatif
    AFR_log_RPM[AFR_log_bin_actuel] = rpm_index_low;        
    AFR_log_KPA[AFR_log_bin_actuel] = pressure_index_high;
  }else if (binD > seuil_pourcent){ //test representatif
    AFR_log_RPM[AFR_log_bin_actuel] = rpm_index_high;        
    AFR_log_KPA[AFR_log_bin_actuel] = pressure_index_high;  
  } 
  else{
    AFR_log_RPM[AFR_log_bin_actuel] = 99;        
    AFR_log_KPA[AFR_log_bin_actuel] = 99;
  }
}else{
  AFR_log_RPM[AFR_log_bin_actuel] = 99;        
  AFR_log_KPA[AFR_log_bin_actuel] = 99;
}
//Serial.println("ecriture pt RPM" + String(point_RPM) + " pt KPA " + String(point_KPA) + " indice " +  String(AFR_log_bin_actuel) + "|"); 

// 2eme etape
// on retrouve le point historique basÃ© sur les RPM actuels
// si c'est reprÃ©sentatif (<>99) on cherche l'AFR objectif, on compare avec AFR actuel et on modifie le point de correction 
delay_bin = AFR_delay[point_RPM]; // de combien on doit se decaler dans le passee
delay_bin_number = AFR_log_bin_actuel - delay_bin; // bin a prendre en compte
if (delay_bin_number < 0) {delay_bin_number =  AFR_log_maxbin + delay_bin_number;}
 
point_RPM_hist = AFR_log_RPM[delay_bin_number];
point_KPA_hist = AFR_log_KPA[delay_bin_number];



if (point_RPM_hist != 99){ // si point valide
  objectif = AFR_map[point_KPA_hist][point_RPM_hist]; // objectif de la carto AFR
  Ego = Ego_map[point_KPA_hist][point_RPM_hist]; // correction historique
    erreur = 100 - ( (objectif - AFR_actuel) * float(100)  / float(objectif) ) ;  // calcul de l'erreur en cours
   
  correction = Ego - ((100 - erreur) * float(Kp_pourcent)) / float(100);          // correction historique - (ecart historique/erreur actuel) * Kp%
//Serial.print(" pt RPM hist " +  String(point_RPM_hist) + "|" +  String(point_KPA_hist) + "|" + "objectif" + String(objectif) );
//Serial.println("AFR act"  + String(AFR_actuel) + "erreur " + String(erreur) + " coor hist " + String(Ego) + " coorrec " + String(correction) );

// pour gerer l'arrondi superieur si objectif < afr actuel (trop pauvre) 
if (objectif < AFR_actuel){
  correction = correction + 1;
}


//Serial.println("objectif AFR " + String(objectif) + "AFR actuel " + String(AFR_actuel) + " crr histor " + String(Ego) + " erreur " +  String(erreur) + "| new corr " +  String(correction) + "|");
    
    if (correction > max_lambda_cor){correction = max_lambda_cor;}
    if (correction < min_lambda_cor){correction = min_lambda_cor;}
  
    Ego_map[point_KPA][point_RPM] = correction; // MAJ historique 
 

} // si point valide

}





