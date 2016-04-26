
//----------------------------------------
//    GESTION EEPROM
//----------------------------------------

// constitution de eeprom
// 0 ---> 49 parametre
// 0 = carto actuel
// 1 = init a faire si <> 100
// 2 = debug 0/1
// 3 = multispark 0/1
// 4,5 => avance initiale
// 6 = gestion knock sensor 0/1 
// 7 = nombre de reboot
// 10 -> 20 nom du BLE
// 21,22 -> REV MAX
// 23,24 -> REV MIN

//25 -> 48 -> knock moyen


// 50 -----> 441 carto 1 (23*17 = 391)
// 450 ----> 469 kpa 1
// 470 ----> 549 RPM 1
// 550 ----->941 carto 2
// 950 ----> 969 kpa 2
// 970 ----> 1050 rpm 2
// 1050 ----->1441 carto 3
// 1450 ----> 1469 kpa 3
// 1470 ----> 1450 rpm 3
// 1550 ----->1941 carto 4
// 1950 ----> 1969 kpa 4
// 1970 ----> 2050 rpm 4
// 2050 ----->2441 carto 5
// 2450 ----> 2469 kpa 5
// 2470 ----> 2550 rpm 5
// 2550 ----->2941 carto EGO
//etc

//--------------------------------------------------------------
// ecriture de la carto EEPROM vers la carto en memoire RAM
//--------------------------------------------------------------
void writeego_ram_eeprom(){
// ecrit sur l'eeprom la carto actuelle
// gestion point de carto
  for (int nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++) { // on parcout les ligne de la carto EEPROM
     for (int nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++) { // on parcout les colonnes de la carto
      int adresse  = (nr_ligne * nombre_point_RPM) + nr_RPM + eprom_ego; // on retrouve l'adresse du dÃƒÂ©but de la ligne a ÃƒÂ©crirr 
      EEPROM.write(adresse ,  Ego_map [nr_ligne][nr_RPM] );
    }
  }
}
void writeego_eeprom_ram(){
// ecrit sur l'eeprom la carto actuelle
// gestion point de carto
  for (int nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++) { // on parcout les ligne de la carto EEPROM
     for (int nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++) { // on parcout les colonnes de la carto
      int adresse  = (nr_ligne * nombre_point_RPM) + nr_RPM + eprom_ego; // on retrouve l'adresse du dÃƒÂ©but de la ligne a ÃƒÂ©crirr 
     Ego_map [nr_ligne][nr_RPM] = EEPROM.read(adresse);
    }
  }
}


void writecarto_eeprom_ram(int carto_eeprom) {
// carto 1 -> 5
int nr_ligne = 0;
int nr_RPM = 0;
  carto_eeprom--; //car carto 1 -> pas de dÃƒÂ©calage

// gestion point KPA
for (nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++) { // on parcout les ligne de la carto EEPROM
  pressure_axis[nr_ligne] = read_eeprom_point_KPA (carto_eeprom , nr_ligne );
}

// gestion point RPM
for (nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++) { // on parcout les colonnes de la carto
 rpm_axis[nr_RPM] = read_eeprom_point_RPM (carto_eeprom , nr_RPM );
}

// gestion point de carto
  for (nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++) { // on parcout les ligne de la carto EEPROM
     for (nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++) { // on parcout les colonnes de la carto
      ignition_map [nr_ligne] [nr_RPM] = read_eeprom_point_carto (carto_eeprom , nr_ligne , nr_RPM );
    }

  }
}

//-------------------------------------------
// gestion EEPROM point de carto
//-------------------------------------------
// ecrit dans l'eeprom 1 point de carto

void write_eeprom_point_carto (int nr_carto , int kpaindex , int rpmindex , byte value){
// carto 0->4 // kpa index 0 -> 16  // rpm index 0 -> 22
  int adresse  = (nr_carto * taille_carto)  + (kpaindex * nombre_point_RPM) + rpmindex + debut_eeprom; // on retrouve l'adresse du dÃƒÂ©but de la ligne a ÃƒÂ©crirr 
  EEPROM.write(adresse ,  value ); 
}
// lit dans l'eeprom 1 point de carto
byte read_eeprom_point_carto (int nr_carto , int kpaindex , int rpmindex ){
// carto 0->4 // kpa index 0 -> 16  // rpm index 0 -> 22
  int adresse  = (nr_carto * taille_carto)  + (kpaindex * nombre_point_RPM) + rpmindex + debut_eeprom; // on retrouve l'adresse du dÃƒÂ©but de la ligne a ÃƒÂ©crirr 
 return  EEPROM.read(adresse); 
}

//-------------------------------------------
// gestion EEPROM point de axe KPA
//-------------------------------------------
// ecrit 1 point sur l'axe KPA
void write_eeprom_point_KPA (int nr_carto , int kpaindex , byte value){
// carto 0->4 // kpa index 0 -> 16  // rpm index 0 -> 22
  int adresse  = (nr_carto * taille_carto)  + kpaindex + debut_kpa + debut_eeprom; // on retrouve l'adresse du dÃƒÂ©but de la ligne a ÃƒÂ©crirr 
  EEPROM.write(adresse ,  value ); 
}
// lit dans l'eeprom 1 point de carto
byte read_eeprom_point_KPA (int nr_carto , int kpaindex ){
// carto 0->4 // kpa index 0 -> 16  // rpm index 0 -> 22
  int adresse  = (nr_carto * taille_carto) + debut_kpa + kpaindex  + debut_eeprom; // on retrouve l'adresse du dÃƒÂ©but de la ligne a ÃƒÂ©crirr 
 return  EEPROM.read(adresse); 
}


//-------------------------------------------
// gestion EEPROM point de axe RPM 
//-------------------------------------------
// ecrit 1 point sur l'axe RPM
void write_eeprom_point_RPM (int nr_carto , int rpmindex , int value){
// carto 0->4 // kpa index 0 -> 16  // rpm index 0 -> 22
  int adresse  = (nr_carto * taille_carto)  + rpmindex*2 + debut_rpm + debut_eeprom; // on retrouve l'adresse du dÃƒÂ©but de la ligne a ÃƒÂ©crirr 
  EEPROMWriteInt(adresse ,  value ); 
}
// lit dans l'eeprom 1 point de l axe RPM
int read_eeprom_point_RPM (int nr_carto , int rpmindex ){
// carto 0->4 // kpa index 0 -> 16  // rpm index 0 -> 22
  int adresse  = (nr_carto * taille_carto)  + rpmindex*2 + debut_rpm + debut_eeprom; // on retrouve l'adresse du dÃƒÂ©but de la ligne a ÃƒÂ©crirr 
 return  EEPROMReadInt(adresse); 
}



//------------------------------------------------------------------
//      INITIALISATION DE L'ECU 
//   TOUTE LES CARTO EEPROM -> RAM
//------------------------------------------------------------------

void init_de_eeprom() {
  // on genere les 5 MAP avec  les MAP par defaut
  byte value = 0;
  int valueint = 0;
  for (int carto = 0; carto < nombre_carto_max; carto++) {
   for (int ligne = 0; ligne < nombre_point_DEP; ligne++) {
    // ecriture point kpa
     value = pgm_read_byte(&(pressure_axis_flash[carto][ligne])); // on lit la flash
     write_eeprom_point_KPA (carto , ligne , value);
    
    for (int col = 0; col < nombre_point_RPM; col++) {
      if (ligne == 0){
      // ecriture point RPM
        valueint = pgm_read_word(&(rpm_axis_flash[carto][col])); // on lit la flash
        write_eeprom_point_RPM (carto , col , valueint);
        
      }
      
      // ecriture points de carto
      value = pgm_read_byte(&(ignition_map_flash[ligne + carto * nombre_point_DEP][col])); // on lit la flash
      write_eeprom_point_carto (carto , ligne , col , value); // on ecrit dans l'eeprom
    }
   
   }
  }

  EEPROM.write(eprom_carto_actuel, 1); // MAP en cours = 1
  EEPROM.write(eprom_init, VERSION); // init effectuÃƒÂ©
  for (int i = 0; i <=10; i++) {
  EEPROM.write(eprom_nom_BLE + i, BT_name[i]); // Nom du Bluettooth
  }
  
  EEPROM.write(eprom_debug, 1); // debug par defaut a oui
  EEPROM.write(eprom_ms, 0); // multispark par defaut a Non
  EEPROMWriteInt(eprom_rev_max, rev_limit); // rev max par defaut
  EEPROMWriteInt(eprom_rev_min, rev_mini); // rev max par defaut
  EEPROMWriteInt(eprom_avance, 0); // avance suppelmentaire par defaut
  EEPROM.write(eprom_knock, 0); // knock par defaut a non

  writeego_ram_eeprom(); // ecriture en RAM de la correction EGO
}
//-------------------------------------------------------------------
//        LECTURE DES PARAMETRE DE L'EEPROM
//            AU DEMARRAGE
//-------------------------------------------------------------------
void read_eeprom() {
  
   writeego_eeprom_ram(); // Lecture de la correction EGO
// lecture carto actuel
  carto_actuel = EEPROM.read(eprom_carto_actuel);
  if ( (carto_actuel < 0) || (carto_actuel > nombre_carto_max) ) { // carto invalide on remet la carto 1
    EEPROM.write(eprom_carto_actuel, 1); // MAP en cours = 1
    carto_actuel = 1;
  }

  // on lit l'eeprom pour les mettre en RAM
   writecarto_eeprom_ram(carto_actuel);
  debug ("Read EEPROM carto nr " + String(carto_actuel));
 

  
  // debugging
  int debugtemp = 0;
  debugtemp = EEPROM.read(eprom_debug);
  
  if  (debugtemp > 1 ){
    EEPROM.write(eprom_debug, 0); // debug par defaut a non
    debugging = false;
   } else{
     if (debugtemp == 0 ){
         debugging = false;
       }else{
         debugging = true;  
       }
  }

// knock sensor
  int knocktemp = 0;
  knocktemp = EEPROM.read(eprom_knock);
  
  if  (knocktemp > 1 ){
    EEPROM.write(eprom_knock, 0); // knock par defaut a non
  #if KNOCK_USED == 1 
    knock_active = false;
  #endif
   } else{
     if (knocktemp == 0 ){
        #if KNOCK_USED == 1 
         knock_active = false;
        #endif
       }else{
        #if KNOCK_USED == 1 
          knock_active = true; 
        #endif
       }
  }
  
  
    // multispark
  int mstemp = 0;
  mstemp = EEPROM.read(eprom_ms);
  
  if  (mstemp > 1 ){
    EEPROM.write(eprom_ms, 1); // debug par defaut a oui
    multispark = false;
   } else{
     if (mstemp == 0 ){
         multispark = false;
       }else{
         multispark = true;  
       }
  }
  
  
  // nom du bluetooth
  for (int i = 0; i <10; i++) {
     BT_name[i]=  EEPROM.read(eprom_nom_BLE + i); // Nom du Bluettooth
  }
  
  // RPM MAX
  rev_limit = EEPROMReadInt(eprom_rev_max);
  if (rev_limit == 0 || rev_limit >= 8000){
    EEPROMWriteInt(eprom_rev_max, 4500); // rev max par defaut
   } 
  
  // RPM MIN
   rev_mini = EEPROMReadInt(eprom_rev_min);
  if (rev_mini == 0 || rev_mini >= 1000){
    EEPROMWriteInt(eprom_rev_min, 500); // rev min par defaut
   } 
   
    // Avance initiale
   correction_degre = EEPROMReadInt(eprom_avance);
  if (correction_degre >= 25){
    EEPROMWriteInt(eprom_avance, 25); // rev min par defaut
    correction_degre = 25;
   } 


}




