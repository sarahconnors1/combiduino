
//----------------------------------------
//    GESTION EEPROM
//----------------------------------------

// constitution de eeprom
// 0 ---> 49 parametre
// 50 -----> 441 carto 1 (23*17 = 391)
// 450 ----> 469 kpa 1
// 470 ----> 549 RPM 1
// 550 ----->941 carto 2
// 950 ----> 969 kpa 2
// 970 ----> 1050 rpm 2
//etc



//-------------------------------------------------
//  GESTION CARTO COMPLETE (POINT, KPA , RPM
//      EEPROM -> RAM      RAM ->EEPROM
//-------------------------------------------------
// ecriture de la carto en  RAM --> eeprom
void writecarto_ram_eeprom(int carto_ram , int carto_eeprom) {
  int ligne;
  int offset = 0; // nr de ligne dans le tableau total
 
 // on ecrit les axe RPM et KPA 
   write_ram_eeprom_kpa(carto_ram, carto_eeprom);
   write_ram_eeprom_rpm(carto_ram, carto_eeprom);

  carto_ram--; //car carto 1 -> pas de décalage
  for (int nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++) { // on parcout les ligne de la carto
    offset = nr_ligne + (nombre_point_DEP * carto_ram); // calcul du pointeur de ligne de la MAP
    for (int nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++) { // on parcout les colonnes de la carto
      EEPROM_lignecarto[nr_RPM] = ignition_map [offset] [nr_RPM]; // on rempli le tableau Temporaire
    }
    writeeepromline(nr_ligne, carto_eeprom); // on écrit dans l'EEPROM la ligne correspondante
  }
}

// ecriture de la carto EEPROM vers la carto en memoire RAM
void writecarto_eeprom_ram(int carto_ram , int carto_eeprom) {
  int ligne;
  int offset = 0; // nr de ligne dans le tableau total
  
   // on ecrit les axe RPM et KPA 
   write_eeprom_ram_kpa(carto_ram, carto_eeprom);
   write_eeprom_ram_rpm(carto_ram, carto_eeprom);
  
  carto_ram--; //car carto 1 -> pas de décalage

  for (int nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++) { // on parcout les ligne de la carto EEPROM
    offset = nr_ligne + (nombre_point_DEP * carto_ram); // calcul du pointeur de ligne en RAM de la MAP
    readeepromline(nr_ligne, carto_eeprom); // on lit dans l'EEPROM la ligne correspondante
    for (int nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++) { // on parcout les colonnes de la carto
      ignition_map [offset] [nr_RPM] = EEPROM_lignecarto[nr_RPM]  ; // on rempli la ligne carto
    }

  }
}


//---------------------------------------------------------------
//   GESTION LIGNE DE CARTO READ / WRITE
//---------------------------------------------------------------

// ecriture d'une ligne de carto  dans l'eeprom
// on va ecrire une ligne complete de EEPROM_lignecarto vers lEEPROM  nr_carto ligne nr_ligne de l'eeprom
// attention la ligne 0 est la premiere ligne (0 -> 16)
void writeeepromline(int nr_ligne, int nr_carto) {
  int adresse = 0;

  if ( (nr_ligne > eeprom_nombre_max_ligne) || (nr_carto > eeprom_nombre_max_carto ) ) {
    return;
  }
  nr_carto-- ; // car la carto 1 -> pas de décalage
  adresse = (nr_carto * taille_carto)  + (nr_ligne * nbr_byte_par_ligne) + debut_eeprom; // on retrouve l'adresse du début de la ligne a écrirr
  debug("adreese eeprom write carto " + String(adresse));
  for (int offset = 0; offset < nombre_point_RPM; offset++) { // offset = la
    EEPROM.write(adresse + offset * nbr_byte_par_int,  EEPROM_lignecarto[offset] ); // on lit tout les points
  }
}


// lecture d'une ligne de carto  dans l'eeprom
// attention la ligne 0 est la premiere ligne (0 -> 16)
void readeepromline(int nr_ligne, int nr_carto) {
  int adresse = 0;

  if ( (nr_ligne > eeprom_nombre_max_ligne) || (nr_carto > eeprom_nombre_max_carto ) ) {
    return;
  }
  nr_carto-- ; // car la carto 1 -> pas de décalage
  adresse = (nr_carto * taille_carto)  + (nr_ligne * nbr_byte_par_ligne) + debut_eeprom; // on retrouve l'adresse du début de la ligne a écrirr
  debug("adreese eeprom read carto " + String(adresse));
  for (int offset = 0; offset < nombre_point_RPM; offset++) { // offset = la
    EEPROM_lignecarto[offset] = EEPROM.read(adresse + offset * nbr_byte_par_int ); // on lit tout les points
  }
}

//---------------------------------------------------------------
//   GESTION LIGNE DE KPA READ / WRITE
//---------------------------------------------------------------

// ecriture axe KPA de la carto X en ram vers carto Y en eeprom
void write_ram_eeprom_kpa(int carto_ram, int carto_eeprom){
  // lecture de la RAM et mise dans le buffer
   for (int offset = 0; offset < nombre_point_DEP; offset++) { 
     EEPROM_ligneKPA[offset] = pressure_axis[carto_ram - 1][offset];
   }
  // ecriture du buffer dans eeprom
  writeeepromlinekpa(carto_eeprom);
}
// ecriture axe KPA de la carto X en EEPROM vers carto Y en RAM
void write_eeprom_ram_kpa(int carto_ram, int carto_eeprom){
   // ecriture de EEPROM vers buffer
   
   readeepromlinekpa(carto_eeprom);
  
  // lecture du buffer et ecriture en RAM
   for (int offset = 0; offset < nombre_point_DEP; offset++) { 
     pressure_axis[carto_ram - 1][offset] = EEPROM_ligneKPA[offset] ;
   }
 
}


// ecriture d'une ligne de KPA  dans l'eeprom
// on va ecrire une ligne complete du tableau EEPROM_ligneKPA vers l'EEPROM  nr_carto 
void writeeepromlinekpa(int nr_carto) {
  int adresse = 0;
  if   (nr_carto > eeprom_nombre_max_carto){return;} // check

  nr_carto-- ; // car la carto 1 -> pas de décalage
  adresse = (nr_carto * taille_carto ) + debut_kpa + debut_eeprom; // on retrouve l'adresse du début de la ligne a écrirr
 debug("adreese eeprom write kpa" + String(adresse));
  for (int offset = 0; offset < nombre_point_DEP; offset++) { // offset = la
    EEPROM.write(adresse + offset * nbr_byte_par_int,  EEPROM_ligneKPA[offset] ); // on lit tout les points
  }
}

// lecture d'une ligne de kpa  dans l'eeprom
// on va lire une ligne complete de l'EEPROM vers le tableau  EEPROM_lignekpa 
void readeepromlinekpa(int nr_carto) {
  int adresse = 0;
  if  (nr_carto > eeprom_nombre_max_carto)  {return;  } // Check
  
  nr_carto-- ; // car la carto 1 -> pas de décalage
  adresse = (nr_carto * taille_carto) + debut_kpa + debut_eeprom; // on retrouve l'adresse du début de la ligne a écrirr
  debug("adreese eeprom read kpa" + String(adresse));
  for (int offset = 0; offset < nombre_point_DEP; offset++) { // offset = la
    EEPROM_ligneKPA[offset] = EEPROM.read(adresse + offset * nbr_byte_par_int ); // on lit tout les points
  }
}

//---------------------------------------------------------------
//   GESTION LIGNE DE RPM READ / WRITE
//---------------------------------------------------------------

// ecriture axe RPM de la carto X en ram vers carto Y en eeprom
void write_ram_eeprom_rpm(int carto_ram, int carto_eeprom){
  // lecture de la RAM et mise dans le buffer
   for (int offset = 0; offset < nombre_point_RPM; offset++) { 
     EEPROM_ligneRPM[offset] = rpm_axis[carto_ram - 1][offset];
   }
  // ecriture du buffer dans eeprom
  writeeepromlinerpm(carto_eeprom);
}
// ecriture axe RPM de la carto X en EEPROM vers carto Y en RAM
void write_eeprom_ram_rpm(int carto_ram, int carto_eeprom){
   // ecriture de EEPROM vers buffer
  readeepromlinerpm(carto_eeprom);
  
  // lecture du buffer et ecriture en RAM
   for (int offset = 0; offset < nombre_point_RPM; offset++) { 
     rpm_axis[carto_ram - 1][offset] = EEPROM_ligneRPM[offset] ;
   }
 
}



// ecriture d'une ligne de RPM  dans l'eeprom
// on va ecrire une ligne complete du tableau EEPROM_ligneRPM vers l'EEPROM  nr_carto 
void writeeepromlinerpm(int nr_carto) {
  int adresse = 0;
  if   (nr_carto > eeprom_nombre_max_carto){return;} // check

  nr_carto-- ; // car la carto 1 -> pas de décalage
  adresse = (nr_carto * taille_carto) + debut_rpm + debut_eeprom; // on retrouve l'adresse du début de la ligne a écrirr
    debug("adreese eeprom write rpm" + String(adresse));
  for (int offset = 0; offset < nombre_point_RPM; offset++) { // offset = la
    EEPROMWriteInt(adresse + offset * 2, EEPROM_ligneRPM[offset]); // on ecrit des vrai Int sur 2 Bytes
  }
}

// lecture d'une ligne de RPM  dans l'eeprom
// on va lire une ligne complete de l'EEPROM vers le tableau  EEPROM_lignerpm
void readeepromlinerpm(int nr_carto) {
  int adresse = 0;
  if  (nr_carto > eeprom_nombre_max_carto)  {return;  } // Check
  
  nr_carto-- ; // car la carto 1 -> pas de décalage
  adresse = (nr_carto *taille_carto) + debut_rpm + debut_eeprom; // on retrouve l'adresse du début de la ligne a écrirr
    debug("adreese eeprom read RPM" + String(adresse));
  for (int offset = 0; offset < nombre_point_DEP; offset++) { // offset = la
    EEPROM_ligneRPM[offset] =EEPROMReadInt(adresse + offset * 2)  ; // on lit tout les points avec des vrais Int donc 2 bytes
  }
}


//------------------------------------------------------------------
//      INITIALISATION DE L'ECU 
//   TOUTE LES CARTO EEPROM -> RAM
//------------------------------------------------------------------

void init_de_eeprom() {
  // on génére les 5 MAP avec  les MAP par defaut
  for (int carto = 1; carto <= nombre_carto_max; carto++) {
    writecarto_ram_eeprom(carto , carto);
  //  write_ram_eeprom_kpa(carto, carto);
  //  write_ram_eeprom_rpm(carto, carto);
  }
  EEPROM.write(eprom_carto_actuel, 1); // MAP en cours = 1
  EEPROM.write(eprom_init, 100); // init effectué
}

void read_eeprom() {
  // on lit toute les carto de l'eeprom pour les mettre en RAM
  for (int carto = 1; carto <= nombre_carto_max; carto++) {
    writecarto_eeprom_ram(carto , carto);
//    write_eeprom_ram_kpa(carto, carto);
 //   write_eeprom_ram_rpm(carto, carto);
  }
  carto_actuel = EEPROM.read(eprom_carto_actuel);
  if ( (carto_actuel < 0) || (carto_actuel > nombre_carto_max) ) { // scarto invalide on remet la carto 1
    EEPROM.write(eprom_carto_actuel, 1); // MAP en cours = 1
    carto_actuel = 1;
  }
}



