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
        if (first_multispark ){
           first_multispark = false;
           map_value_us = 2048;
        }else{
          map_value_us = (1536 - (25.6 * Degree_Avance_calcul))+2048; 
        } 
    } else{                                                // Otherwise read from map
       map_value_us = 1536 - (25.6 * Degree_Avance_calcul);
    }
  }
if (map_value_us < 64){                                // If map_value_us is less than 64 (smallest EDIS will accept), set map_value_us to 64us
    map_value_us = 64;
}
 tick = map_value_us * 2; // pour le timer  

}




// gestion de la depression moyenne
void gestiondepression(){
  totalKPA = totalKPA + analogRead(MAP_pin);       
  current_indexKPA++;              
 
 // on refait la moyenne des x dernieres mesures
 if (current_indexKPA >= numReadingsKPA ){              
     manifold_pressure = totalKPA / numReadingsKPA;   //on fait la moyenne 
     map_pressure_kpa = map(manifold_pressure,0,correction_pressure,0,101);  // on converti la moyenne en KPA
    newvalue=true;
 if (map_pressure_kpa < min_pressure_kpa_recorded){min_pressure_kpa_recorded = map_pressure_kpa;} // on log le mini enregistrÃ©
 
 current_indexKPA = 0;totalKPA = 0;  // on remet a 0 pour prochaine lecture
 }
}
//-------------------------------------------- PIP signal interupt --------------------------------------------// 
void pip_interupt()  {
 //gestion plus simple des rpm moyens
  pip_count++;
  if (pip_count >=maxpip_count) {
    engine_rpm_average = (30000000 * maxpip_count) / (micros() - timeold ); 
    pip_count = 0;
    newvalue=true;
    timeold = micros();
  }
  
  // gestion dy cylindre en cours d'allumage
  cylindre_en_cours++;
  if (cylindre_en_cours>4){cylindre_en_cours=1;}
  
  generate_SAW();                                // Generate SAW avec le calcul precedent

}

//-------------------------------------------- retrouve l'index du tableau pour les RPM --------------------------------------------// 
int decode_rpm(int rpm_) {
  int map_rpm = 0;
   if(rpm_ <rev_mini){                // check si on est dans les limites haute/basse
     map_rpm = 1;
   } else { 
     if(rpm_ >=rev_limit) {      // 
       map_rpm = nombre_point_RPM - 1;      
     }else{
       // retrouve la valeur inferieur 
       while(rpm_ > rpm_axis[carto_actuel][map_rpm]){map_rpm++;} // du while
       if (map_rpm > 0){map_rpm--;}
     }
   point_RPM_actuel = map_rpm + 1;
    return map_rpm;
  }
}
//--------------------------------------------retrouve l index du tableau de la pression --------------------------------------------//
int decode_pressure(int pressure_) {
   int map_pressure = 0;
   if(pressure_ < pressure_axis[carto_actuel][0]){
     map_pressure = 0;
   }else if (pressure_ > pressure_axis[carto_actuel][nombre_point_DEP -1]) {
     map_pressure = nombre_point_DEP -1 ;
   }else{
     // retrouve la valeur inferieur 
     while(pressure_ > pressure_axis[carto_actuel][map_pressure]){map_pressure++;}
     if (map_pressure > 0){map_pressure--;}
   }
   point_KPA_actuel = map_pressure + 1 ;
   return map_pressure;
}


//-------------------------------------------- fonction pour retrouver le degree d avance a appliquer suivant  RPM et pression--------------------------------------------//
int rpm_pressure_to_spark(int rpm, int pressure){
  int table_value;
  int map_rpm_index = decode_rpm(rpm);                      // retrouve index RPM
  int map_pressure_index = decode_pressure(pressure);       // retrouve index pression
  
  // gestion de la multi cartographie
  map_pressure_index = map_pressure_index + (carto_actuel -1) * nombre_point_DEP ; // on decale de X lignes
  
  table_value = ignition_map[map_pressure_index][map_rpm_index];
  // correction de l avance manuel
  table_value = table_value + correction_degre;
  return table_value;
}

//-------------------------------------------- Function to generate SAW signal to return to EDIS --------------------------------------------//
void generate_SAW(){
   digitalWrite(pin_ignition,HIGH);
  
  delayMicroseconds(200); // on attend un peu 
// on envoie le SAW
  digitalWrite(SAW_pin,HIGH);                                 // send output to logic level HIGH (5V)
  ignition_on = true;

// gestion du timer 5 pour arreter le SAW
  TCCR5A = 0;
  TCCR5B = 0;
  TCNT5  = 0;

  OCR5A = tick;            // compare match register 
  TCCR5B |= (1 << WGM52);   // CTC mode
  TCCR5B |= (1 << CS51);    // 8 prescaler 
  TIMSK5 |= (1 << OCIE5A);  // enable timer compare interrupt
}


ISR(TIMER5_COMPA_vect){ 
//Timer 5 B pour arreter le SAW
    digitalWrite(pin_ignition,LOW);
    digitalWrite(SAW_pin,LOW);    // on arrte le saw
   TCCR5A = 0x00;          //Disbale Timer5 while we set it up
   TCCR5B = 0x00;          //Disbale Timer5 while we set it up
   TCCR5C = 0x00;          //Disbale Timer5 while we set it up
 ignition_on = false;  
 } 
   
 
// attente 10 degre pas nÃ©cessaire ?
//  int code_delay = 50;
//  long ten_ATDC_delay = (((60000000/engine_rpm_average)/36)-code_delay);
//  delayMicroseconds(ten_ATDC_delay);
  //delayMicroseconds(map_value_us);                            // hold HIGH for duration of map_value_us
 //digitalWrite(SAW_pin,LOW); 

//delayMicroseconds(map_value_us);                            // hold HIGH for duration of map_value_us
//digitalWrite(SAW_pin,LOW); 
 
