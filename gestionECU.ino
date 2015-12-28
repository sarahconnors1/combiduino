//-------------------------------------------------------------
//   ROUTINE DE CALCUL ECU                                      //
//-------------------------------------------------------------


void calculdelavance(){
// calcul du nombre de degrÃ© d'avance suivant la map
// puis pour le saw du dÃ©lai en microseconds
// puis en tick du timer (1 tick / 0,5 us)
  
Degree_Avance_calcul = rpm_pressure_to_spark(engine_rpm_average, map_pressure_kpa);  
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
    map_pressure_kpa = map(analogRead(MAP_pin),0,correction_pressure,0,101);  // on converti la moyenne en KPA
   MAP_accel = (previous_map_pressure_kpa - map_pressure_kpa) * MAP_check_per_S; // calcul en Kpa/S
   previous_map_pressure_kpa =  map_pressure_kpa;
   newvalue=true;
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


//-------------------------------------------- fonction pour retrouver le degree d avance a appliquer suivant  RPM et pression--------------------------------------------//
int rpm_pressure_to_spark(int rpm, int pressure){
  int table_value;
  int map_rpm_index = decode_rpm(rpm);                      // retrouve index RPM
  int map_pressure_index = decode_pressure(pressure);       // retrouve index pression
    
  table_value = ignition_map[map_pressure_index][map_rpm_index];
  // correction de l avance manuel
  table_value = table_value + correction_degre;
  return table_value;
}

//-------------------------------------------- Function to generate SAW signal to return to EDIS --------------------------------------------//
//-------------------------------------------- PIP signal interupt --------------------------------------------// 
void pip_interupt()  {
 //gestion plus simple des rpm moyens
if ((digitalRead(interrupt_X) == LOW) and ( (micros() - pip_old) > debounce ) ) {   
    pip_old = micros();pip_count++;
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

if (cylinder_injection[cylindre_en_cours - 1] == true){ // si on doit injecter
  
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
   

 
