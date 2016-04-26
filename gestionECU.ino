//-------------------------------------------------------------
//   ROUTINE DE CALCUL ECU                                      //
//-------------------------------------------------------------


void calcul_carto(){
  // calcul de la position actuelle dans la carto
  
 point_RPM = decode_rpm(engine_rpm_average);                      // retrouve index RPM
 point_KPA = decode_pressure(map_pressure_kpa);       // retrouve index pression

gestionTPSMAPdot(); // calcul de l'acceleration kpa/s et tps %/s
  
  // check si le moteur tourne
if ( (micros() - pip_old) > 100000){engine_rpm_average = 0;}

  // Check si en cours de dÃ©marrage 
if ( (engine_rpm_average < rev_mini) && (engine_rpm_average > 0) ) {
   sbi(running_mode,BIT_ENGINE_CRANK);
}else{
   cbi(running_mode,BIT_ENGINE_CRANK);
}

if (engine_rpm_average == 0){
  cbi(running_mode,BIT_ENGINE_RUN);
}else{
  sbi(running_mode,BIT_ENGINE_RUN); 
}

// mode idle 
if ( (engine_rpm_average <= RPM_idle_max) and (engine_rpm_average > rev_mini) and (TPS_actuel <= TPS_idle_max) ){
  sbi(running_mode,BIT_ENGINE_IDLE); 
}else{
  cbi(running_mode,BIT_ENGINE_IDLE);
}


// recalcul des nouvelles valeur avance et injection
    calculdelavance();
    calcul_injection();


}

void calculdelavance(){
// calcul du nombre de degrÃƒÂ© d'avance suivant la map
// puis pour le saw du dÃƒÂ©lai en microseconds
// puis en tick du timer (1 tick = 0,5 us)
  
Degree_Avance_calcul = IGN_MAP(engine_rpm_average, map_pressure_kpa) + correction_degre ;  

if (fixed == true){                                   
   map_value_us = 1536 - (25.6 * fixed_advance);
  }else{
    if (multispark && engine_rpm_average <= 1200){    // If engine rpm below 1800 and multispark has been set, add 2048us to map_value_us
        if (first_multispark ){map_value_us = msvalue;}else{map_value_us = (1536 - (25.6 * Degree_Avance_calcul))+msvalue; } // multispark 
    } else{ map_value_us = 1536 - (25.6 * Degree_Avance_calcul);}                           // Otherwise read from map
  }
  if (map_value_us < 64){ map_value_us = 64; }    // If map_value_us is less than 64 (smallest EDIS will accept), set map_value_us to 64us

 tick = map_value_us * 2; // pour le timer  
// calcul du delai nÃ©cessaire pour envoyer le SAW x degrÃ© aprÃ¨s le PIP
delay_ignition =   ((60000000L /engine_rpm_average)* angle_delay_SAW /360L ) * 2 ; 
}


//--------------------------------------------------------------
// Gestion de l'interpolation du degré d'avance suivant les points adjacent de la MAP 
//--------------------------------------------------------------

float IGN_MAP(int rpm, int pressure){


  int map_rpm_index_low = point_RPM;                      // retrouve index RPM inferieur
  int map_rpm_index_high = 0;
  int map_pressure_index_low = point_KPA;       // retrouve index pression inferieur
  int map_pressure_index_high = 0;
  float IGN_min = 0;
  float IGN_max = 0;
  float IGN = 10;

#if VACUUMTYPE ==1 // prise sur collecteur   
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
 // lowkpa      A              B        IGN_min = interpolation A et B
 // highjpa     C              D        IGN_max = interpolation C et D

 // d'abord VE versus RPM
 if (map_rpm_index_low != map_rpm_index_high){ // si < x tour/min ou > y tour/min -> si on est dans la MAP
    IGN_min = mapfloat(rpm, rpm_axis[map_rpm_index_low],rpm_axis[map_rpm_index_high], ignition_map [map_pressure_index_low] [map_rpm_index_low],ignition_map [map_pressure_index_low] [map_rpm_index_high]);
    IGN_max = mapfloat(rpm, rpm_axis[map_rpm_index_low],rpm_axis[map_rpm_index_high], ignition_map [map_pressure_index_high] [map_rpm_index_low],ignition_map [map_pressure_index_high] [map_rpm_index_high]);
 }else{
    IGN_min = ignition_map [map_pressure_index_low] [map_rpm_index_low];
    IGN_max = ignition_map [map_pressure_index_high] [map_rpm_index_low];
 }
 
 // puis entre VE_min / max et kpa
  if (map_pressure_index_low != map_pressure_index_high){
  IGN = mapfloat(pressure ,pressure_axis[map_pressure_index_low],pressure_axis[map_pressure_index_high], IGN_min, IGN_max);
  }else{
    IGN = IGN_min;
  } 
#endif
#if VACUUMTYPE == 2 // prise en amont papillon
 IGN = ignition_map[point_KPA][point_RPM]
#endif


   
return IGN;
}




// gestion de la nouvelle dÃ©pression
void gestiondepression(){
sum_pressure += analogRead(MAP_pin);
count_pressure++;
if (count_pressure >= nbre_mesure_pressure){
   int kpa_moyen = sum_pressure / count_pressure;  
   int kpa = map(kpa_moyen,0,correction_pressure,kpa_0V,101);  // on converti la moyenne en KPA   
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
  kpa = map_pressure_kpa + ( (kpa - map_pressure_kpa) * lissage_kpa / float(100) );

// nouvelle valeur   
  map_pressure_kpa = kpa;
  last_MAP_time = millis();

}   
   
}  

// calcul du nombre de KPA / TPS par seconde 
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

void gestionTPS(){
  sum_TPS += analogRead(TPS_pin);
  count_TPS++;
// debug(String(analogRead(TPS_pin)));
 if (count_TPS >= nbre_mesure_TPS){
   int TPS_moyen = sum_TPS / count_TPS;  
   int TPS = map(TPS_moyen,tps_lu_min,tps_lu_max,0,100);  // on converti la moyenne en %  
   count_TPS = 0;
   sum_TPS = 0;
   if (TPS != TPS_actuel ) {newvalue=true;}
   TPS_actuel = TPS;
   last_TPS_time = millis();
 }
}

//-------------------------------------------- retrouve l'index du tableau pour les RPM --------------------------------------------// 
int decode_rpm(int rpm_) { // renvoi la valeur infÃ©rieur du bin
  int map_rpm = 0;
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
//--------------------------------------------retrouve l index du tableau de la pression --------------------------------------------//
#if VACUUMTYPE == 2
int decode_pressure(int pressure_) { // renvoi la valeur infÃ©rieur du bin
   int map_pressure = 0;
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
int decode_pressure(int pressure_) { // renvoi la valeur infÃ©rieur du bin
   int map_pressure = 0;
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


//-------------------------------------------- Function to generate SAW signal to return to EDIS --------------------------------------------//
//-------------------------------------------- PIP signal interupt --------------------------------------------// 
void pip_interupt()  {
  unsigned int timeout_injection = 0;
  
 //gestion plus simple des rpm moyens
if ((digitalRead(interrupt_X) == LOW) and ( (micros() - pip_old) > debounce ) ) {   
    pip_old = micros();pip_count++;nbr_spark++;
    if (pip_count >= maxpip_count) {
        engine_rpm_average = (30000000 * maxpip_count) / (micros() - timeold ); 
        pip_count = 0;
        newvalue=true;
        timeold = micros();
    }
  
  if (first_multispark ){
    first_multispark = false; 
  }



//---------------------
//gestion des interruption 
//timer 5A pour ignition
//timer 5B / 5C pour injection 
//---------------------

#if SAW_WAIT== 0   
unsigned int timeout_ignition = TCNT5 + tick; 
  OCR5A = timeout_ignition; 
  TIMSK5 |= (1 << OCIE5A);  // enable timer compare interrupt
   // on envoie le SAW
  digitalWrite(SAW_pin,HIGH);  // send output to logic level HIGH (5V)
  #if KNOCK_USED == 1
    digitalWrite(pin_ignition,HIGH); // pour le knock 
    ignition_on = true;
  #endif
#endif

#if SAW_WAIT== 1
// on retarde le SAW de X microseconds
unsigned int timeout_ignition = TCNT5 + delay_ignition; 
OCR5A = timeout_ignition; 
TIMSK5 |= (1 << OCIE5A);  // enable timer compare interrupt
ignition_mode = IGNITION_PENDING;
#endif

#if INJECTION_USED == 1 
  // gestion dy cylindre en cours d'allumage
  cylindre_en_cours++;
  if (cylindre_en_cours>4){cylindre_en_cours=1;}

  #if ALTERNATESQUIRT == 0 // Si gestion des injecteurs simultanÃ©s
    if ( (cylinder_injection[cylindre_en_cours - 1] == true)||(BIT_CHECK(running_mode, BIT_ENGINE_CRANK))|| !(BIT_CHECK(running_mode, BIT_ENGINE_RUN) ) ) { // si on doit injecter ou tous les tours au demmarrage
      timeout_injection = TCNT5 + tick_injection; 
      OCR5B = timeout_injection;            // compare match register 
      TIMSK5 |= (1 << OCIE5B);  // enable timer compare interrupt
      digitalWrite(pin_injection,HIGH);  // send output to logic level HIGH (5V)
      OCR5C = timeout_injection;            // compare match register 
      TIMSK5 |= (1 << OCIE5C);  // enable timer compare interrupt
      digitalWrite(pin_injection2,HIGH);  // send output to logic level HIGH (5V)
    }   
  #endif

  #if ALTERNATESQUIRT == 1 // Si gestion des injecteurs alternÃ©
    if ( (cylinder_injection[cylindre_en_cours - 1] == true)||(BIT_CHECK(running_mode, BIT_ENGINE_CRANK))|| !(BIT_CHECK(running_mode, BIT_ENGINE_RUN) ) ) { // si on doit injecter ou tous les tours au demmarrage
      timeout_injection = TCNT5 + tick_injection; 
      OCR5B = timeout_injection;            // compare match register 
      TIMSK5 |= (1 << OCIE5B);  // enable timer compare interrupt
      digitalWrite(pin_injection,HIGH);  // send output to logic level HIGH (5V)
    } 
    if ( (cylinder_injection2[cylindre_en_cours - 1] == true)||(BIT_CHECK(running_mode, BIT_ENGINE_CRANK))|| !(BIT_CHECK(running_mode, BIT_ENGINE_RUN) ) ) { // si on doit injecter ou tous les tours au demmarrage
      timeout_injection = TCNT5 + tick_injection; 
      OCR5C = timeout_injection;            // compare match register 
      TIMSK5 |= (1 << OCIE5C);  // enable timer compare interrupt
      digitalWrite(pin_injection2,HIGH);  // send output to logic level HIGH (5V)
    }  
  #endif // de altenate squirt
 #endif // de injection used
 }
}

//Timer 5 A pour arreter le SAW
#if SAW_WAIT == 0
  ISR(TIMER5_COMPA_vect){ 
    digitalWrite(pin_ignition,LOW);
    digitalWrite(SAW_pin,LOW);    // on arrte le saw
    ignition_on = false;  
  } 
#endif

#if SAW_WAIT == 1
ISR(TIMER5_COMPA_vect){
  if (ignition_mode == IGNITION_PENDING) { // on lance le SAW 
    unsigned int timeout_ignition = TCNT5 + tick; 
    OCR5A = timeout_ignition; 
    TIMSK5 |= (1 << OCIE5A);  // enable timer compare interrupt
    digitalWrite(SAW_pin,HIGH);  // send output to logic level HIGH (5V)
    ignition_mode = IGNITION_RUNNING;
    #if KNOCK_USED == 1
      digitalWrite(pin_ignition,HIGH); // pour le knock 
      ignition_on = true;
    #endif 
  }else{ // on arrete le SAW
    digitalWrite(SAW_pin,LOW);    // on arrte le saw
    #if KNOCK_USED == 1
      digitalWrite(pin_ignition,LOW);  
      ignition_on = false;  
    #endif    
  }
} 
#endif


ISR(TIMER5_COMPB_vect){ //Timer 5 B pour arreter l injection
  digitalWrite(pin_injection,LOW);    // on arrte l'injection
} 

ISR(TIMER5_COMPC_vect){ //Timer 5 C pour arreter l injection  
  digitalWrite(pin_injection2,LOW);    // on arrte l'injection
} 
   

 

