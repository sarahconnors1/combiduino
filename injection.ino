
//----------------------------------------------------------
//         Calcul de l'injection
//----------------------------------------------------------

void calcul_injection(){
#if INJECTION_USED == 1  
acceleration_actuel =  MAP_ACCEL(MAP_accel);
correction_lambda_actuel = correction_lambda();
VE_actuel = VE_MAP(engine_rpm_average, map_pressure_kpa);

  // le temps d'injection est :  temps d'ouverture injecteur + le max a pleine puissance (Req_Fuel) * VE de la map en % * Correction enrichissement en % * corection lambda en %

 
  injection_time_us =  Req_Fuel_us  
  * (VE_actuel/float(100) )
  * (map_pressure_kpa / float(100) )
  * (correction_lambda_actuel /float(100)  )
  ;

//acceleration_actuel = 0;
// injection_time_us =  (injection_time_us +  previous_injection_time_us) / 2;
//previous_injection_time_us = injection_time_us;
  
tick_injection  = ( injection_time_us  + injector_opening_time_us + acceleration_actuel )* prescalertimer5 ;

debug("ms;" + String(time_loop) + ";reqfuel;" + String(Req_Fuel_us) + ";ptRPM;" + String(point_RPM) + ";ptKPA;" + String(point_KPA) + ";RPM;" + String(engine_rpm_average)
+ ";KPA;" + String(map_pressure_kpa) + ";MAPAccel;" + String(MAP_accel) + ";PhAccel;" + String(accel_state_actuel) + ";InjACC;" + String(acceleration_actuel)  + ";inj;" +
String(injection_time_us) + ";VE;" + String(VE_actuel)  +";AFR;" + String(AFR_actuel) + ";corlbda;" + String(correction_lambda_actuel ) + ";DEG;"+ String(Degree_Avance_calcul) + ";nbrsprk;" + String (nbr_spark - temp_spark_dep) ); 

 temp_spark_dep = nbr_spark;
 #endif
}

// ----------------------------------------------------------
// recherche du coefficient  a appliquer suivant la valeur LAMBDA
// ----------------------------------------------------------
int correction_lambda(){
if (correction_lambda_used == true ){
 if (nbr_spark >= last_correction_lambda_spark + correction_lambda_every_spark ){ // on corrige tous les x tour 
    last_correction_lambda_spark=nbr_spark;
    if ( (correction_lambda_actuel < max_lambda_cor)  ) { // on peut augmenter le pourcentage = enrichir 
      if (AFR_actuel > 120){
      return   (correction_lambda_actuel + increment_correction_lambda);
      }
    }
    if  (correction_lambda_actuel > min_lambda_cor){ // on peut diminuer le pourcentage = appauvrir
      if (AFR_actuel < 120){    
        return (correction_lambda_actuel - increment_correction_lambda);
      }
    }
    return correction_lambda_actuel;
  }else{
    return correction_lambda_actuel;
  }

}else{
  return 100;
}
}


// ----------------------------------------------------------
// recherche du coefficient VE a appliquer suivant la MAP
// ----------------------------------------------------------
int VE_MAP(int rpm, int pressure){
  int map_rpm_index_low = point_RPM;                      // retrouve index RPM inferieur
  int map_rpm_index_high = 0;
  int map_pressure_index_low = point_KPA;       // retrouve index pression inferieur
  int map_pressure_index_high = 0;
  int VE_min = 0;
  int VE_max = 0;
  int VE = 0;
   // calcul des valeurs des bin superieur
  if ((map_rpm_index_low >= nombre_point_RPM)|| (rpm <= rpm_axis[0]) ) { // si on est > a tour maxi ou si on est < a tour mini high = low
    map_rpm_index_high = map_rpm_index_low;
  } else{  
    map_rpm_index_high = map_rpm_index_low + 1;
  }
  
  if ((map_pressure_index_low >= nombre_point_DEP) || (pressure <= pressure_axis[0])) { // si on est > a kpa maxi ou si on est < a kpa mini high = low
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
 VE_min = map(rpm,
 rpm_axis[map_rpm_index_low],rpm_axis[map_rpm_index_high],
 fuel_map [map_pressure_index_low] [map_rpm_index_low],fuel_map [map_pressure_index_low] [map_rpm_index_high]);
  VE_max = map(rpm,
 rpm_axis[map_rpm_index_low],rpm_axis[map_rpm_index_high],
 fuel_map [map_pressure_index_high] [map_rpm_index_low],fuel_map [map_pressure_index_high] [map_rpm_index_high]);
 }else{
   VE_min = fuel_map [map_pressure_index_low] [map_rpm_index_low];
   VE_max = fuel_map [map_pressure_index_high] [map_rpm_index_low];
 }
 
 // puis entre VE_min / max et kpa
  if (map_pressure_index_low != map_pressure_index_high){
  VE = map(pressure
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
int MAP_ACCEL(int MAP_accel_){
float extrafuel = 0; // 0 = Pas d'enrichissement
   int bin = 0;

   if ( (nbr_spark < last_accel_spark + accel_every_spark) and (accel_state_actuel >0) ){ // si en cours d'acceleraion
    return acceleration_actuel;
   }else{
      if(MAP_accel_ <MAP_kpas[0]){                // check si on est dans les limites haute/basse
        bin = 0;
      } else { 
        if(MAP_accel_ >=MAP_kpas[MAP_acc_max -1]) {      // 
          bin = MAP_acc_max -1;
        }else{
          // retrouve la valeur inferieur 
          while(MAP_accel_ > MAP_kpas[bin]){bin++;} // du while
          if (bin > 0){bin--;}
       
        }
      }
    extrafuel =  float(Req_Fuel_us  / float(100) );
    extrafuel = extrafuel * MAP_acceleration[bin] ;
     if (extrafuel == 0){
      accel_state_actuel = 0;
    } else if (extrafuel > 0){ // on demarre un nouvelle accel
      accel_state_actuel = 1;
      last_accel_spark = nbr_spark;
      
    }
   
    return extrafuel;
  }
}


//-----------------------------------------------------------
//           Gestion de la sonde lambda
//-----------------------------------------------------------

// gestion de la lecture de la lambda
void lecturelambda(){
 int afr_lu = 0;
 int point_afr = 0;

#if LAMBDATYPE == 2 // narrowband
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
 
  if (afr_lu > AFR_analogique[AFR_bin_max -1]) { // borne maxi  
     AFR_actuel = AFR[AFR_bin_max -1];
  }  else if (afr_lu <AFR_analogique[0]){ // borne mini
     AFR_actuel = AFR[0];
  } else {
    point_afr = decode_afr(afr_lu);
    AFR_actuel = map(afr_lu,AFR_analogique[point_afr],AFR_analogique[point_afr + 1],AFR[point_afr],AFR[point_afr + 1]) ; // on fait une interpolation
  }
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
  if (running_mode != Stopping){
    digitalWrite(pin_pump,LOW); 
  }else{
    digitalWrite(pin_pump,HIGH);
  }
}
