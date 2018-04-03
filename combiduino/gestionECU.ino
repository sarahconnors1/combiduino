//------------------------------------------------------------- //
//   ROUTINE DE CALCUL PRINCIPAL ECU                            //
//------------------------------------------------------------- //

void calcul_carto(){
// Etat du moteur actuel 
calculRPM();

// check si le moteur tourne
if ( (micros() - pip_old) > 100000){ECU.engine_rpm_average = 0;}

// Check si en cours de dÃ©marrage 
if ( (ECU.engine_rpm_average < rev_mini) && (ECU.engine_rpm_average > 0) ) {
   sbi(ECU.running_mode,BIT_ENGINE_CRANK);}else{cbi(ECU.running_mode,BIT_ENGINE_CRANK);}

// check si running 
if (ECU.engine_rpm_average == 0){
  cbi(ECU.running_mode,BIT_ENGINE_RUN);}else{sbi(ECU.running_mode,BIT_ENGINE_RUN);}

// Check si WARM UP
if (ECU.CLT > IAEhightemp ) {sbi(ECU.running_mode,BIT_ENGINE_WARMUP);}else{cbi(ECU.running_mode,BIT_ENGINE_WARMUP);}

// check du mode idle
#if TPS_USED==1  
if ( (ECU.engine_rpm_average <= RPM_idle_max) and (ECU.engine_rpm_average > rev_mini) and (ECU.TPS_actuel <= TPS_idle_max) and (ECU.map_pressure_kpa <= MAP_idle_max) ){
# endif
#if TPS_USED==0  
if ( (ECU.engine_rpm_average <= RPM_idle_max) and (ECU.engine_rpm_average > rev_mini) and (ECU.map_pressure_kpa <= MAP_idle_max) ){
# endif
  sbi(ECU.running_mode,BIT_ENGINE_IDLE); 
}else{
  cbi(ECU.running_mode,BIT_ENGINE_IDLE);
  #if PID_IDLE_USED==1 
    idlePID.SetMode(MANUAL);
  #endif
}

// check de la deceleration 
if ( (ECU.engine_rpm_average > RPM_DEC_min) 
  and (ECU.map_pressure_kpa <= pressure_axis[nombre_point_DEP-1] + 1 ) ){ // depression forte
    if (time_decel == 0){
      time_decel = time_loop;
    }
    if ( (time_loop - time_decel) > interval_time_decel ){
      sbi(ECU.running_mode,BIT_ENGINE_DCC); 
      ECU.qte_paroi = 0; // pour le xtau il n'y a plus rien sur la paroi
    }
} else {
    cbi(ECU.running_mode,BIT_ENGINE_DCC);
    time_decel = 0; 
}




// recalcul des nouvelles valeur avance et injection
// calcul de la position actuelle dans la carto
 ECU.point_RPM = decode_rpm(ECU.engine_rpm_average);          // retrouve index RPM
 ECU.point_KPA = decode_pressure(ECU.map_pressure_kpa);       // retrouve index pression 
 calculinterpolation(ECU.engine_rpm_average, ECU.map_pressure_kpa); // calcul pourcentage a appliquer par bin 
 calculdelavance();
 calcul_injection();


 cbi(ECU.running_option,BIT_NEW_VALUE); // calcul effectué
}


//-------------------------------------------------------------------------------
//  Routine d interpolation dans la MAP
//-------------------------------------------------------------------------------
void calculinterpolation(int rpm, int pressure){
// interpolation entre les 4 valeurs de la map
//                              rpm_percent_low   rpm_percent_high
//                                 lowRPM             highRPM
//  kpa_percent_high    highkpa      C                  D       
//  kpa_percent_low     lowkpa       A                  B       
byte rpm_percent_low = 0;                     
byte rpm_percent_high = 0;
byte kpa_percent_low = 0;                     
byte kpa_percent_high = 0;
  
rpm_index_low = ECU.point_RPM;            // retrouve index RPM inferieur
rpm_index_high = 0;
pressure_index_low = ECU.point_KPA;       // retrouve index pression inferieur
pressure_index_high = 0;
  
// calcul des valeurs des bin superieur et interpolation des RPM et des KPA
if ((rpm >= rpm_axis[nombre_point_RPM-1])|| (rpm <= rpm_axis[0]) ) { // si on est > a tour maxi ou si on est < a tour mini high = low
      rpm_index_high = rpm_index_low;  
      rpm_percent_low = 100;
      rpm_percent_high = 0;
} else{ 
      rpm_index_high = rpm_index_low + 1;    
      rpm_percent_low = float((rpm_axis[rpm_index_high] - rpm) * 100. / (rpm_axis[rpm_index_high] - rpm_axis[rpm_index_low] ) ); // interpolation des RPM (low /high) pour obtenir un % e
      rpm_percent_high = 100 - rpm_percent_low;
}
  
if ((pressure <= pressure_axis[nombre_point_DEP-1]) || (pressure >= pressure_axis[0])) { // si on est > a kpa maxi ou si on est < a kpa mini high = low
      pressure_index_high = pressure_index_low;
      kpa_percent_low = 100;
      kpa_percent_high = 0;
} else{
      pressure_index_high = pressure_index_low - 1;   
      kpa_percent_low = float ((pressure_axis[pressure_index_high] - pressure) * 100. / (pressure_axis[pressure_index_high] - pressure_axis[pressure_index_low] ) ); // interpolation des RPM (low /high) pour obtenir un % e
      kpa_percent_high = 100 - kpa_percent_low;
}

// calcul des valeurs des Bins adjacents
binC = float(rpm_percent_low *  kpa_percent_high / 100. );
binD = float(rpm_percent_high *  kpa_percent_high / 100. );
binB = float(rpm_percent_high *  kpa_percent_low / 100. );
binA = 100 - binC - binD - binB;
}


//-------------------------------------------------------------------------
//      Routine principal de calcul de l'avance
//-------------------------------------------------------------------------
void calculdelavance(){
// calcul du nombre de degrees d'avance suivant la map
// puis pour le saw du delai en microseconds
// puis en tick du timer (1 tick = 0,5 us)

ECU.PID_idle_advance = Idle_correction();
ECU.Degree_Avance_calcul = IGN_MAP() + correction_degre + ECU.PID_idle_advance  ;  

if (BIT_CHECK(ECU.running_option,BIT_FIXED_ADV)){                                   
   map_value_us = 1536 - (25.6 * fixed_advance);
}else{
   if (BIT_CHECK(ECU.running_option,BIT_MS) && ECU.engine_rpm_average <= RPM_max_multispark){    // If engine rpm below 1800 and multispark has been set, add 2048us to map_value_us
        if (BIT_CHECK(ECU.running_option,BIT_FIRST_MS) ){map_value_us = msvalue;}else{map_value_us = (1536 - (25.6 * ECU.Degree_Avance_calcul))+msvalue; } // multispark 
   } else{
        map_value_us = 1536 - (25.6 * ECU.Degree_Avance_calcul);  // Otherwise read from map
   }                          
  }
if (map_value_us < 64){ map_value_us = 64; }    // If map_value_us is less than 64 (smallest EDIS will accept), set map_value_us to 64us


 tick = map_value_us * 2; // pour le timer  
// calcul du delai necessaire pour envoyer le SAW x degre apres le PIP
delay_ignition =   ((60000000L /ECU.engine_rpm_average)* (angle_delay_SAW ) /360L ) * 2 ; 
}

//---------------------------------------------------------------------------------------------
// Recherche de la correction en degré a appliquer a l'avance pour maintenir un ralenti stable
//---------------------------------------------------------------------------------------------
float Idle_correction(){
if (!BIT_CHECK(ECU.running_mode,BIT_ENGINE_IDLE )||Idle_management == false )  { // si on est en mode normal on renvoie 0
  return 0;
}
#if PID_IDLE_USED==0 
byte bin = 0;
int ecart = ECU.ECU.engine_rpm_average - RPM_idle_objectif; 
// on recherche l'index de correction basé sur l'écart entre ralenti objectif et ralenti réel
if ( ecart >= Idle_step[Idle_maxbin - 1] ) {
  bin = Idle_maxbin - 1;
} else if ( ecart <= Idle_step[0] ) {
   bin = 0;
} else {
    while(ecart > Idle_step[bin]){bin++;} // du while
    if (bin > 0){bin--;}
}
return Idle_adv[bin];
#endif

#if PID_IDLE_USED==1
 idlePID.SetMode(AUTOMATIC);
 idle_engine_rpm_average = (double)(ECU.engine_rpm_average);
 idlePID.Compute();
 return idle_advance;
#endif
  
}

//--------------------------------------------------------------
// Gestion de l'interpolation du degré d'avance suivant les points adjacent de la MAP 
//--------------------------------------------------------------
float IGN_MAP(){
  float IGN = 10;

#if VACUUMTYPE ==1 // prise sur collecteur   
IGN = binC * ignition_map [pressure_index_high] [rpm_index_low] / float(100)
   + binD * ignition_map [pressure_index_high] [rpm_index_high] / float(100)
   + binA * ignition_map [pressure_index_low] [rpm_index_low] / float(100)
   + binB * ignition_map [pressure_index_low] [rpm_index_high] / float(100)
   ;  
#endif
#if VACUUMTYPE == 2 // prise en amont papillon
 IGN = ignition_map[ECU.point_KPA][ECU.point_RPM];
#endif
   
return IGN;
}




//-------------------------------------------- Function to generate SAW signal to return to EDIS --------------------------------------------//
//-------------------------------------------- PIP signal interupt --------------------------------------------// 
void pip_interupt()  {
  unsigned int timeout_injection = 0;
  
 //gestion plus simple des rpm moyens
 unsigned long delai = micros() - pip_old ; // ecart avec le précedent pip   

// Test du faux PIP
if  ( (delai < debouncePIP) // ecart entre 2 PIP trop petit   
    and (debouncePIP < 65000) ){
      pip_fault ++;
      return;
}
 
if ((digitalRead(interrupt_X) == LOW) ) {   
    pip_old = micros();
    time_total= time_total - time_readings[pip_count]; //subtract the previous reading in current array element from the total reading      //set time of current reading as the new time of the previous reading
    time_readings[pip_count] = delai;           //place current rpm value into current array element
    time_total= time_total + time_readings[pip_count]; //add the reading to the total     
    pip_count++;nbr_spark++;
    if (pip_count >= maxpip_count) {pip_count = 0;sbi(ECU.running_option,BIT_NEW_VALUE);}
    
  if (BIT_CHECK(ECU.running_option,BIT_FIRST_MS) ){
    cbi(ECU.running_option,BIT_FIRST_MS); 
  }




//---------------------
//gestion des interruption 
//timer 5A pour ignition
//timer 5B / 5C pour injection 
//---------------------


// on retarde le SAW de X microseconds
unsigned int timeout_ignition = TCNT5 + delay_ignition; 
OCR5A = timeout_ignition; 
TIMSK5 |= (1 << OCIE5A);  // enable timer compare interrupt
ignition_mode = IGNITION_PENDING;

#if INJECTION_USED == 1 
  // gestion dy cylindre en cours d'allumage
  cylindre_en_cours++; if (cylindre_en_cours>4){cylindre_en_cours=1;}

  // gestion du XTAU
  if (cylindre_en_cours == 1){
    recalcul_paroi= true;
      
  }

    if ( (cylinder_injection[cylindre_en_cours - 1] == true)||(BIT_CHECK(ECU.running_mode, BIT_ENGINE_CRANK))|| !(BIT_CHECK(ECU.running_mode, BIT_ENGINE_RUN) ) ) { // si on doit injecter ou tous les tours au demmarrage
      timeout_injection = TCNT5 + tick_injection; 
      OCR5B = timeout_injection;            // compare match register 
      TIMSK5 |= (1 << OCIE5B);  // enable timer compare interrupt
      digitalWrite(pin_injection,HIGH);  // send output to logic level HIGH (5V)
    } 
    if ( (cylinder_injection2[cylindre_en_cours - 1] == true)||(BIT_CHECK(ECU.running_mode, BIT_ENGINE_CRANK))|| !(BIT_CHECK(ECU.running_mode, BIT_ENGINE_RUN) ) ) { // si on doit injecter ou tous les tours au demmarrage
      timeout_injection = TCNT5 + tick_injection; 
      OCR5C = timeout_injection;            // compare match register 
      TIMSK5 |= (1 << OCIE5C);  // enable timer compare interrupt
      digitalWrite(pin_injection2,HIGH);  // send output to logic level HIGH (5V)
    }  
 #endif // de injection used
 }
 
}

//------------------------------------
//Timer 5 A pour arreter le SAW
//------------------------------------
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


ISR(TIMER5_COMPB_vect){ //Timer 5 B pour arreter l injection
  digitalWrite(pin_injection,LOW);    // on arrte l'injection
} 

ISR(TIMER5_COMPC_vect){ //Timer 5 C pour arreter l injection  
  digitalWrite(pin_injection2,LOW);    // on arrte l'injection
} 
   

 


