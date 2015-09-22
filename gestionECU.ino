//-------------------------------------------------------------
//   ROUTINE DE CALCUL ECU                                      //
//-------------------------------------------------------------


// gestion de la depression moyenne
void gestiondepression(){
  readings[current_index] = analogRead(MAP_pin);       
  current_index++;              
 
 // on refait la moyenne des x dernieres mesures
 if (current_index >= numReadings ){              
     current_index = 0;total = 0;   
      for (int thisReading = 0; thisReading < numReadings; thisReading++) {total = total + readings[thisReading];  }  // on fait la somme 
     manifold_pressure = total / numReadings;   //on fait la moyenne 
     map_pressure_kpa = map(manifold_pressure,0,correction_pressure,0,101);  // on converti la moyenne en KPA
    newvalue=true;
 if (map_pressure_kpa < min_pressure_kpa_recorded){min_pressure_kpa_recorded = map_pressure_kpa;} // on log le mini enregistré
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
    generate_SAW(Degree_Avance_calcul);                                // Generate SAW avec le calcul precedent

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
int generate_SAW(int advance_degrees){
if (fixed == true){                                   
   map_value_us = 1536 - (25.6 * fixed_advance);
  }else{
    if (multispark && engine_rpm_average <= 1200){    // If engine rpm below 1800 and multispark has been set, add 2048us to map_value_us
        if (first_multispark ){
           first_multispark = false;
           map_value_us = 2048;
        }else{
          map_value_us = (1536 - (25.6 * advance_degrees))+2048; 
        } 
    } else{                                                // Otherwise read from map
       map_value_us = 1536 - (25.6 * advance_degrees);
    }
  }
  
  if (map_value_us < 64){                                // If map_value_us is less than 64 (smallest EDIS will accept), set map_value_us to 64us
    map_value_us = 64;
  }
  
 /* 
  // on envoie le SAW
 // 
*/
// attente 10 degre pas nécessaire ?
  int code_delay = 50;
  long ten_ATDC_delay = (((60000000/engine_rpm_average)/36)-code_delay);
  delayMicroseconds(ten_ATDC_delay);
  digitalWrite(SAW_pin,HIGH);                                 // send output to logic level HIGH (5V)
  delayMicroseconds(map_value_us);                            // hold HIGH for duration of map_value_us
  digitalWrite(SAW_pin,LOW); 

}


