//-------------------------------------------------------------
//   ROUTINE DE CALCUL ECU                                      //
//-------------------------------------------------------------


void calcul_carto(){
  // calcul de la position actuelle dans la carto
 int map_rpm_index = decode_rpm(engine_rpm_average);                      // retrouve index RPM
 int map_pressure_index = decode_pressure(map_pressure_kpa);       // retrouve index pression
 if ((map_rpm_index != point_RPM) or (map_pressure_index != point_KPA) ){
  bin_carto_change = true;  
 }
  point_RPM = map_rpm_index;            // retrouve index RPM
  point_KPA = map_pressure_index;       // retrouve index pression

 // calcul du mode actuel du moteur
  // Stopping 1
  // Cranking  2
  // Idling  3
  // Running  10
    
  // check si le moteur tourne
  if ( (micros() - pip_old) > 100000){engine_rpm_average = 0;}

  running_mode = Running;
  if (engine_rpm_average == 0){
    running_mode = Stopping;
  } else  if (engine_rpm_average < rev_mini){
    running_mode = Cranking;
  }

// recalcul des nouvelles valeur avance et injection
    calculdelavance();
    calcul_injection();
  
}

void calculdelavance(){
// calcul du nombre de degrÃ© d'avance suivant la map
// puis pour le saw du dÃ©lai en microseconds
// puis en tick du timer (1 tick / 0,5 us)
  
Degree_Avance_calcul = ignition_map[point_KPA][point_RPM] + correction_degre ;  

if (fixed == true){                                   
   map_value_us = 1536 - (25.6 * fixed_advance);
  }else{
    if (multispark && engine_rpm_average <= 1200){    // If engine rpm below 1800 and multispark has been set, add 2048us to map_value_us
        if (first_multispark ){map_value_us = msvalue;}else{map_value_us = (1536 - (25.6 * Degree_Avance_calcul))+msvalue; } // multispark 
    } else{ map_value_us = 1536 - (25.6 * Degree_Avance_calcul);}                           // Otherwise read from map
  }
  if (map_value_us < 64){ map_value_us = 64; }    // If map_value_us is less than 64 (smallest EDIS will accept), set map_value_us to 64us
 tick = map_value_us * 2; // pour le timer  
}


// gestion de la nouvelle dépression
void gestiondepression(){
sum_pressure += analogRead(MAP_pin);
count_pressure++;
if (count_pressure >= nbre_mesure_pressure){
   int kpa_moyen = sum_pressure / count_pressure;  
   int kpa = map(kpa_moyen,0,correction_pressure,kpa_0V,101);  // on converti la moyenne en KPA   
   count_pressure = 0;
   sum_pressure = 0;
   if (kpa != map_pressure_kpa ) {newvalue=true;}
   map_pressure_kpa = kpa;
   MAP_accel = (map_pressure_kpa - previous_map_pressure_kpa) * MAP_check_per_S; // calcul en Kpa/S
   previous_map_pressure_kpa =  map_pressure_kpa;

  
}   
   
}  



//-------------------------------------------- retrouve l'index du tableau pour les RPM --------------------------------------------// 
int decode_rpm(int rpm_) { // renvoi la valeur inférieur du bin
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
int decode_pressure(int pressure_) { // renvoi la valeur inférieur du bin
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
int decode_pressure(int pressure_) { // renvoi la valeur inférieur du bin
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

#if KNOCK_USED == 1
 digitalWrite(pin_ignition,HIGH); // pour le knock 
 ignition_on = true;
#endif

//---------------------
//gestion des interruption 
//timer 5A pour ignition
//timer 5B pour injection 
//---------------------
   
unsigned int timeout_ignition = TCNT5 + tick; 
  OCR5A = timeout_ignition; 
  TIMSK5 |= (1 << OCIE5A);  // enable timer compare interrupt
   // on envoie le SAW
  digitalWrite(SAW_pin,HIGH);  // send output to logic level HIGH (5V)

#if INJECTION_USED == 1 
 // gestion dy cylindre en cours d'allumage
cylindre_en_cours++;
if (cylindre_en_cours>4){cylindre_en_cours=1;}

if ( (cylinder_injection[cylindre_en_cours - 1] == true)||(running_mode == Cranking)||(running_mode == Stopping) ) { // si on doit injecter ou tous les tours au demmarrage
  unsigned int timeout_injection = TCNT5 + tick_injection; 
  OCR5B = timeout_injection;            // compare match register 
  TIMSK5 |= (1 << OCIE5B);  // enable timer compare interrupt
  digitalWrite(pin_injection,HIGH);  // send output to logic level HIGH (5V)
}   
#endif
  }
}



ISR(TIMER5_COMPA_vect){ 
//Timer 5 B pour arreter le SAW
    digitalWrite(pin_ignition,LOW);
    digitalWrite(SAW_pin,LOW);    // on arrte le saw
 ignition_on = false;  
 //   TIMSK5 &= ~(1 << OCIE5A); //Turn off this output
 } 

 ISR(TIMER5_COMPB_vect){ 
//Timer 5 B pour arreter l injection
    digitalWrite(pin_injection,LOW);    // on arrte l'injection
 //   TIMSK5 &= ~(1 << OCIE5B); //Turn off this output
 } 
   

 
