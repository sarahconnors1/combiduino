

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
DEC_actuel = Decel();
AFR_objectif = AFR_map[point_KPA][point_RPM];
  // le temps d'injection est :  temps d'ouverture injecteur + le max a pleine puissance (Req_Fuel) * VE de la map en % * Correction enrichissement en % * corection lambda en %
 
   injection_time_us =  Req_Fuel_us  
  * (VE_actuel/float(100) )
  * (map_pressure_kpa / float(100) )
  * (correction_lambda_actuel /float(100)  )
  * (WAS_actuel / float(100)  )
   * (IAE_actuel / float(100)  )
   * (DEC_actuel / float(100)  )
  ;

// pour le Xtau
Calcul_PW_actuel_corrige_XTAU();

tick_injection  = ( PW_actuel + injector_opening_time_us  )* prescalertimer5 ;
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
// gestion de la deceleration
//--------------------------------------------------------
byte Decel(){
#if DECEL_USED == 1
if (BIT_CHECK(running_mode,BIT_ENGINE_DCC) ){ // coupure totale si décélération totale
  return 0;
}else{
  return 100;
}
#endif

#if DECEL_USED == 0
 return 100;
#endif

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
// on renvoie l'enrichissement du tableau MAP_acceleration x req fuel

int MAP_ACCEL(int BASE_accel){
 float extrafuel = 0; // 0 = Pas d'enrichissement
 byte bin = 0;
 byte bin1 = 0;
 int acc_ = 0;

 // pas d'acceleration si > a un certain regime ou arrete
if ( (engine_rpm_average >RPM_ACC_max) or ( !BIT_CHECK(running_mode,BIT_ENGINE_RUN) )) {return 0;} 

 // si en cours d'acceleraion et accel pas fini et acceleration actuel < accerleration initiale 
if ( (nbr_spark < last_accel_spark + accel_every_spark) and ( BIT_CHECK(running_mode,BIT_ENGINE_MAP)) and (saved_accel>BASE_accel)    ){  
   if  (nbr_spark > last_accel_spark + accel_every_spark/2 ){ // diminution par 2 de la qté essence en fin d'accel
       PW_accel_actuel_us = PW_accel_actuel_us / 2;
   }
      return PW_accel_actuel_us;
}
 // si acceleraition actuelle en dessous du mini on retourne 0
if (BASE_accel < accel_mini){
   cbi(running_mode,BIT_ENGINE_MAP);
   return 0;  
}

 // calcul d une nouvelle acceleration  
 // si pas en cours d'accelleration ou nouvelle accel superieur a la précédente

// Retrouve le bin a appliquer
if(BASE_accel <MAP_kpas[0]){ bin =  bin1 = 0;  }              // check si on est dans les limites basse
else if(BASE_accel >=MAP_kpas[MAP_acc_max -1]) {bin = bin1 = MAP_acc_max -1; }     // check si on est dans les limites haute
else{ // retrouve la valeur inferieur 
   while(BASE_accel > MAP_kpas[bin]){bin++;} 
   if (bin > 0){bin--;} bin1 = bin +1;
}

// interpolation   
if (bin == bin1){
   acc_ = MAP_acceleration[bin] ;  
}else{
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
}

//------------------------------------------------------
// ouverture initiale pour purger les injecteurs
//------------------------------------------------------
void injection_initiale(){
    digitalWrite(pin_injection,HIGH);  // send output to logic level HIGH (5V)
    delay(ouverture_initiale);
    digitalWrite(pin_injection,LOW);  // send output to logic level HIGH (5V)  
}

//-----------------------------------------------------
// check pour arreter la pompe
//-----------------------------------------------------
void checkpump(){
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
EGO_temp = binC * Ego_map [pressure_index_high] [rpm_index_low] / float(100)
         + binD * Ego_map [pressure_index_high] [rpm_index_high] / float(100)
         + binA * Ego_map [pressure_index_low] [rpm_index_low] / float(100)
         + binB * Ego_map [pressure_index_low] [rpm_index_high] / float(100);
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

//*********************         1ere ETAPE                 ***************
// on stock les points RPM, KPA actuelle pour utilisation future
// si c'est non representatif on met 99 
//-------------------------------------------------------------------------
AFR_log_bin_actuel++;
if (AFR_log_bin_actuel >= AFR_log_maxbin) {AFR_log_bin_actuel = 0;}

AFR_log_RPM[AFR_log_bin_actuel] = 99;        
AFR_log_KPA[AFR_log_bin_actuel] = 99;
if (  (correction_lambda_used == true ) and (!BIT_CHECK(running_mode,BIT_ENGINE_MAP)  )  
      and (!BIT_CHECK(running_mode,BIT_ENGINE_IDLE ) )
      and (!BIT_CHECK(running_mode,BIT_ENGINE_DCC ) )
      and (!BIT_CHECK(running_mode,BIT_ENGINE_DCC_MAPdot ) )
      and (BIT_CHECK(running_mode,BIT_ENGINE_RUN ) )  
      and (IAE_actuel == 100) // moteur chaud
      and ( (point_KPA < lambda_kpa_index_min) or (point_RPM > lambda_rpm_index_min) ) // zone adjacente au ralenti
      ) { //correction lambda et pas d'acceleration/deceleration en cours et pas au ralenti

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
}
//Serial.println("ecriture pt RPM" + String(point_RPM) + " pt KPA " + String(point_KPA) + " indice " +  String(AFR_log_bin_actuel) + "|"); 



//*********************         2eme ETAPE                 ***************
// on retrouve le point historique base sur les RPM actuels
// si c'est representatif (<>99) on cherche l'AFR objectif dans la MAP objectif,
// on compare avec AFR actuel et on modifie le point de correction 
//--------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------
//                ROUTINE X-TAU
//-------------------------------------------------------------------------------
/*
 * Byte Tau_evap           pourcentage d'évaporation du film par seconde
 * unsigned int  Tau_dt    pourcentage d evaporation sur un cycle * 100 pour stockage en entier  -> Tau_evap / 2*(60 * RPM)       
 * Byte X_adher            pourcentage de carburant qui colle a la paroi a chaque injection
 *  int qte_paroi  Quantite actuellement sur la paroi 
 *  = quantite precedente sur la paroi
 *  + nouvelle quantite adherente sur la paroi (PW_precedent * X_adher/100) 
 *  - quantité evaporée de la paroi sur la periode (qte_paroi_previous * 100/Tau_dt)
 * 
 *  int qte_paroi_previuos  Quantite precedente sur la paroi
 * 
 * unsigned int PW_actuel corrigé par le xtau
 *  =( PW de la MAP                          ( (injection_time_us)
 * - quantit evaporé disponible pour le cylindre    (qte_paroi * 100/Tau_dt) )
 * ) * facteur de non adherence                      * 100/ (100- X_adher)
 * + acceleration de la pompe de reprise



*/

void Calcul_PW_actuel_corrige_XTAU(){
// routine principale qui calcul le Pulse width corrigé 
// basé sur le PW demandé par la carto (injection_time_us)  sans tenir compte de l'enrichissement de la pompe de reprise
// A la fin du calcul PW_actuel tient compte aussi de l'enrichissement ( PW corrige + PW accel)
  Calcul_variable_XTAU(); 

  if (BIT_CHECK(running_option,BIT_XTAU_USED) and BIT_CHECK(running_mode, BIT_ENGINE_RUN) and  !BIT_CHECK(running_mode, BIT_ENGINE_CRANK) and (!BIT_CHECK(running_mode,BIT_ENGINE_IDLE ) ) ){
     PW_actuel = ( ( injection_time_us - (  (qte_paroi/Tau_dt) * float(100) ) ) 
     / (100 - X_adher) )
    * 100
    ;
    
    if (PW_actuel < 0){ 
      PW_actuel = 0;
    }
    
    if (    PW_actuel * Xtau_max_decel  < injection_time_us  ){
      PW_actuel = injection_time_us /  Xtau_max_decel ;
    }
  
 }else{
    PW_actuel = injection_time_us;
 }

  PW_actuel +=  PW_accel_actuel_us;

}
void Calcul_qte_paroi(){
// calcul la quantité restante sur la paroi (unite = PW pulse width)
// cette routine doit être appelé à chaque cycle aka injection du cylindre 1
// elle tient compte de l'ajout sur la paroi de la derniere injection ( injection carto + pompe de reprise) 
if (recalcul_paroi == true){
qte_paroi = qte_paroi_previous
 +  ( float(PW_previous/float(100) )  * X_adher )  
 - ( (qte_paroi_previous /Tau_dt) * float(100) );
 
  if (qte_paroi < 0){
    qte_paroi=0;
  }
  qte_paroi_previous = qte_paroi;
  PW_previous = PW_actuel; 
  recalcul_paroi = false;
}

}



void Calcul_variable_XTAU(){
byte BAWC = 0;
byte BSOC = 0;
byte AWN = 100;
byte SON = 100;
byte AWW = 100;
byte SOW = 100;

// interpolation lineaire basee sur la depression actuelle
BAWC = map(pressure_axis[point_KPA], pressure_axis[nombre_point_DEP-1] ,pressure_axis[0], BAWC_min, BAWC_max);
BSOC = map(pressure_axis[point_KPA], pressure_axis[nombre_point_DEP-1] ,pressure_axis[0], BSOC_min, BSOC_max);

// facteur de corection base sur les RPM
AWN = map(rpm_axis[point_RPM] ,rpm_axis[0],rpm_axis[nombre_point_RPM - 1], AWN_min, AWN_max);
SON = map(rpm_axis[point_RPM] ,rpm_axis[0],rpm_axis[nombre_point_RPM - 1], SON_min, SON_max);

// facteur de correction basé sur la temperature CLT
#if CLT_USED == 1
if (CLT <  lowtemp){
 AWW = AWW_min; SOW = SOW_min; 
}else if (CLT > hightemp){
 AWW = AWW_max;SOW = SOW_max; 
}else{
AWW = map(CLT ,lowtemp ,hightemp , AWW_min, AWW_max);
SOW = map(CLT ,lowtemp ,hightemp , SOW_min, SOW_max);
}
#endif
  
// calcul du taux d adherence final
X_adher = BAWC * (AWN / float(100) ) * (AWW / float(100) );
// calcul du taux evaporation final
Tau_evap = BSOC * (SON / float(100) ) * (SOW / float(100) );

// calcul le temps nécessaire entre 2 injection du meme cylindre (sur 1 cycle = 2 tours
  if ( engine_rpm_average> 0){
  Tau_dt =  engine_rpm_average /120;
   Tau_dt = Tau_evap  * Tau_dt   ;
  }else{
    Tau_dt = 10; // valeur arbitraire pour eviter les div par 0
  }  
}






