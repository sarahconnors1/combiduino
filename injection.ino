

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
VE_actuel = VE_MAP(engine_rpm_average, map_pressure_kpa);
int WAS_actuel = WAS();

  // le temps d'injection est :  temps d'ouverture injecteur + le max a pleine puissance (Req_Fuel) * VE de la map en % * Correction enrichissement en % * corection lambda en %
 
   injection_time_us =  Req_Fuel_us  
  * (VE_actuel/float(100) )
  * (map_pressure_kpa / float(100) )
  * (correction_lambda_actuel /float(100)  )
  * (WAS_actuel / float(100)  )
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



// ----------------------------------------------------------
// recherche du coefficient VE a appliquer suivant la MAP
// ----------------------------------------------------------
float VE_MAP(int rpm, int pressure){
  int map_rpm_index_low = point_RPM;                      // retrouve index RPM inferieur
  int map_rpm_index_high = 0;
  int map_pressure_index_low = point_KPA;       // retrouve index pression inferieur
  int map_pressure_index_high = 0;
  float VE_min = 0;
  float VE_max = 0;
  float VE = 0;
   // calcul des valeurs des bin superieur
  if ((rpm <= rpm_axis[nombre_point_RPM-1])|| (rpm <= rpm_axis[0]) ) { // si on est > a tour maxi ou si on est < a tour mini high = low
    map_rpm_index_high = map_rpm_index_low;
  } else{  
    map_rpm_index_high = map_rpm_index_low + 1;
  }
  
  if ((pressure <= pressure_axis[nombre_point_DEP-1]) || (pressure >= pressure_axis[0])) { // si on est > a kpa maxi ou si on est < a kpa mini high = low
    map_pressure_index_high = map_pressure_index_low;
  } else{  
    map_pressure_index_high = map_pressure_index_low + 1;
  }  
  // interpolation entre les 4 valeurs de la map
 //            lowRPM      highRPM
 // lowkpa      A              B        VE_min = interpolation A et B
 // highjpa     C              D        VE_max = interpolation C et D
 
 // d'abord VE versus RPM
 if (map_rpm_index_low != map_rpm_index_high){ // si < x tour/min ou > y tour/min -> si on est dans la MAP
 VE_min = mapfloat(rpm,
 rpm_axis[map_rpm_index_low],rpm_axis[map_rpm_index_high],
 fuel_map [map_pressure_index_low] [map_rpm_index_low],fuel_map [map_pressure_index_low] [map_rpm_index_high]);
  VE_max = mapfloat(rpm,
 rpm_axis[map_rpm_index_low],rpm_axis[map_rpm_index_high],
 fuel_map [map_pressure_index_high] [map_rpm_index_low],fuel_map [map_pressure_index_high] [map_rpm_index_high]);
 }else{
   VE_min = fuel_map [map_pressure_index_low] [map_rpm_index_low];
   VE_max = fuel_map [map_pressure_index_high] [map_rpm_index_low];
 }
 
 // puis entre VE_min / max et kpa
  if (map_pressure_index_low != map_pressure_index_high){
  VE = mapfloat(pressure
  ,pressure_axis[map_pressure_index_low],pressure_axis[map_pressure_index_high],
  VE_min,VE_max);
  }else{
    VE = VE_min;
  }  
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
if ( engine_rpm_average >RPM_ACC_max){ // pas d'acceleration si > a un certain regime
  return 0;
}else{

if ( (nbr_spark < last_accel_spark + accel_every_spark) and ( BIT_CHECK(running_mode,BIT_ENGINE_MAP)) and (saved_accel>BASE_accel)    ){ // si en cours d'acceleraion et accel pas fini et acceleration actuel < accerleration initiale
  return PW_accel_actuel_us;
}else{
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

//-----------------------------------------------------------
//           Gestion de la sonde lambda
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
     AFR_actuel = AFR[AFR_bin_max -1];
  }  else if (afr_lu <AFR_analogique[0]){ // borne mini
     AFR_actuel = AFR[0];
  } else {
    point_afr = decode_afr(afr_lu);
    AFR_actuel = map(afr_lu,AFR_analogique[point_afr],AFR_analogique[point_afr + 1],AFR[point_afr],AFR[point_afr + 1]) ; // on fait une interpolation
  }
#endif

#if LAMBDATYPE == 1 //wideband
 AFR_actuel = map(afr_lu,0,1020,90,190) ; // on fait une interpolation lineaire
#endif

}






} 


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
int correction_lambda(){
  if (  (correction_lambda_used == true ) and (!BIT_CHECK(running_mode,BIT_ENGINE_MAP)  )  and (!BIT_CHECK(running_mode,BIT_ENGINE_IDLE ) )  ) { //correction lambda et pas d'acceleration en cours et pas au ralenti 
    return Ego_map[point_KPA][point_RPM]; // correction historique
  }else{
    return 100;
  }
}

//----------------------------------------------------------------
// Calcul de la valeur de correction lambda Ã  appliquer au passÃ©
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

// 1ere Ã©tape on stock les points RPM, KPA actuelle pour utilisation future
// si c'est non representatif on met 99 
AFR_log_bin_actuel++;
if (AFR_log_bin_actuel >= AFR_log_maxbin) {AFR_log_bin_actuel = 0;}

if (  (correction_lambda_used == true ) and (!BIT_CHECK(running_mode,BIT_ENGINE_MAP)  )  and (!BIT_CHECK(running_mode,BIT_ENGINE_IDLE ) ) and (BIT_CHECK(running_mode,BIT_ENGINE_RUN ) )  ) { //correction lambda et pas d'acceleration en cours et pas au ralenti
  AFR_log_RPM[AFR_log_bin_actuel] = point_RPM;        
  AFR_log_KPA[AFR_log_bin_actuel] = point_KPA;
}else{
  AFR_log_RPM[AFR_log_bin_actuel] = 99;        
  AFR_log_KPA[AFR_log_bin_actuel] = 99;
}
//Serial.println("ecriture pt RPM" + String(point_RPM) + " pt KPA " + String(point_KPA) + " indice " +  String(AFR_log_bin_actuel) + "|"); 
// 2eme etape
// on retrouve le point historique basÃ© sur les RPM actuels
// si c'est reprÃ©sentatif (<>99) on cherche l'AFR objectif, on compare avec AFR actuel et on modifie le point de correction 
delay_bin = AFR_delay[point_RPM]; // de combien on doit se decaler dans le passÃ©
delay_bin_number = AFR_log_bin_actuel - delay_bin; // bin a prendre en compte
if (delay_bin_number < 0) {delay_bin_number =  AFR_log_maxbin + delay_bin_number;}
 
point_RPM_hist = AFR_log_RPM[delay_bin_number];
point_KPA_hist = AFR_log_KPA[delay_bin_number];


//Serial.println("decalage de " + String(delay_bin) + " bin number " + String(delay_bin_number) + " pt RPM hist " +  String(point_RPM_hist) + "|" +  String(point_KPA_hist) + "|");

if (point_RPM_hist != 99){ // si point valide
  objectif = AFR_map[point_KPA_hist][point_RPM_hist]; // objectif de la carto AFR
  Ego = Ego_map[point_KPA_hist][point_RPM_hist]; // correction historique
    erreur = 100 - ( (objectif - AFR_actuel) * float(100)  / float(objectif) ) ;  // calcul de l'erreur en cours
   
    correction = Ego - ((Ego - erreur) * float(Kp_pourcent)) / float(100);          // correction historique - (ecart historique/erreur actuel) * Kp%
//Serial.println("objectif AFR " + String(objectif) + "AFR actuel " + String(AFR_actuel) + " crr histor " + String(Ego) + " erreur " +  String(erreur) + "| new corr " +  String(correction) + "|");
    
    if (correction > max_lambda_cor){correction = max_lambda_cor;}
    if (correction < min_lambda_cor){correction = min_lambda_cor;}
  
    Ego_map[point_KPA][point_RPM] = correction; // MAJ historique 
 

} // si point valide

}




/*
 byte AFR_delay[nombre_point_RPM] = { 10,  10,   10,   10,    9,    9,    8,    8,    7,    6,    5,    4,    4,    3,    2,    2,    2,    2,    2,    2,    2,    2,    2}; // 23 retard en nbre de mesure de la sonde lambda
const AFR_log_maxbin = 12;
byte AFR_log_bin_actuel = 0;
byte AFR_log_RPM[AFR_log_maxbin] =          {0,0,0,0,0,0,0,0,0,0,0,0,0,0 } ; //point RPM
byte AFR_log_KPA[AFR_log_maxbin] =          {0,0,0,0,0,0,0,0,0,0,0,0,0,0 } ; //point KPA

int correction_lambda(){

byte objectif = 130;
byte Ego = 100;
float correction = 0;
float erreur = 0;

objectif = AFR_map[point_KPA][point_RPM];
Ego = Ego_map[point_KPA][point_RPM]; // correction historique

if (  (correction_lambda_used == true ) and (!BIT_CHECK(running_mode,BIT_ENGINE_MAP)  )  and (!BIT_CHECK(running_mode,BIT_ENGINE_IDLE ) )  ) { //correction lambda et pas d'acceleration en cours et pas au ralenti
 if (nbr_spark >= last_correction_lambda_spark + correction_lambda_every_spark ){ // on corrige tous les x tour 

    last_correction_lambda_spark=nbr_spark;
    erreur = 100 - ( (objectif - AFR_actuel) * float(100)  / float(objectif) ) ;  // calcul de l'erreur en cours
   
    correction = Ego - ((Ego - erreur) * float(Kp_pourcent)) / float(100);          // correction historique - (ecart historique/erreur actuel) * Kp%
    
    if (correction > max_lambda_cor){correction = max_lambda_cor;}
    if (correction < min_lambda_cor){correction = min_lambda_cor;}
  
    Ego_map[point_KPA][point_RPM] = correction; // MAJ historique 
    return correction;
  }else{
    return correction_lambda_actuel;
  }
}else{
  return 100;
}

}
*/



