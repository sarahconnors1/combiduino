//---------------------------------------------------------------
//  GESTION DES ENTREE SORTIE VERS DEBUG OU IPHONE
//---------------------------------------------------------------


//-------------------------------------------- Triggered when serial input detected --------------------------------------------//
void serialEvent() {
 while (Serial.available()) {                    //whilst the serial port is available...
   inputString = Serial.readStringUntil('\n');   //... read in the string until a new line is recieved
   stringComplete = true;                      
 }
 }
 
 void serialEvent1() {
 while (Serial1.available()) {                    //whilst the serial port is available...
   inputString3 = Serial1.readStringUntil('\n');   //... read in the string until a new line is recieved
   stringComplete3 = true;                      
 }
 }

//-------------Check des donnÃ©e entrante par le port serie-----------------------------
void checkdesordres(){
  Read_BT(); // lecture du BT
  
  if (stringComplete){   
  if(inputString == "fixed") {
        debug("Fixed Advance Selected");
        fixed = true;
   }
   else if (inputString == "map") {
        debug("Ignition Advance Selected");
        fixed = false;
   }
   else if (inputString == "dbg on") {
        debug("debug ON");
        debugging = true;
   }
   else if (inputString == "dbg off") {
        debug("debug OFF");
        debugging = false;
   }
   
   else if (inputString == "ms on") {
         debug("Multispark Enabled");
         multispark = true;
         first_multispark = true;
   }
   else if (inputString == "ms off") {
         debug("Multispark Disabled");
         multispark = false;
   }
   else if (inputString == "output on"){
      output = true; 
    }
    else if (inputString == "output off"){
      output = false;
   }
    else if (inputString == "plus"){
      correction_degre = correction_degre + 3;
   }
    else if (inputString == "moins"){
      correction_degre = correction_degre - 3;
   }
    else if (inputString.startsWith("map;") ) {// change la carto actuelle
    //parm 1 nr de carto RAM
      changement_carto_ram();
    }
    else if (inputString.startsWith("chg;") ) {// change la 1 point de la carto x
    //parm 1 nr de carto RAM
    //parm 2 point RPM 0 -> 22
    //parm 3 point KPA 0 -> 16
    //parm 4 degre
      changement_point_carto_ram();
    }
    else if (inputString.startsWith("writecarto;") ) {// ecrit la carto en ram X dans la carto Y eeprom
    //parm 1 nr de carto RAM
    //parm 2 nr de carto EEPROM
      carto_ram_vers_eeprom();
    }
    else if (inputString.startsWith("readcarto;") ) {// ecrit la carto EEPROM X dans la carto en RAM Y 
    //parm 1 nr de carto RAM
    //parm 2 nr de carto EEPROM
      carto_eeprom_vers_ram();
    }
    else if (inputString.startsWith("sndcarto;") ) {//envoie la carto eeprom => IPHONE
      //parm 1 nr de carto EEPROM
      send_carto_iphone();
    }
    else if (inputString.startsWith("sndkpa;") ) {//envoie l axe KPA eeprom => IPHONE
      //parm 1 nr de carto EEPROM
      send_kpa_iphone();
    }
    else if (inputString.startsWith("gtkpa;") ) {//envoie l axe KPA iphone => ECU
      //parm 1 nr de carto EEPROM
      // parm 2 point kpa
      // parm 3 valeur
      get_kpa_iphone();
    }
    else if (inputString.startsWith("sndrpm;") ) {//envoie l axe RPM eeprom => I PHONE
      //parm 1 nr de carto EEPROM
      send_rpm_iphone();
    }
    else if (inputString.startsWith("gtrpm;") ) {//envoie l axe RPM iphone => ECU
      //parm 1 nr de carto EEPROM
      // parm 2 point rpm
      // parm 3 valeur
      get_rpm_iphone();
    }
    else if (inputString.startsWith("gt1") ) {//envoie le setting a l'iphone
      //parm retour carto demarrage 
      // debug 1/0 (oui/non)
      //rev mini
      // rev maxi
      // retour exemple "gt1;5;0;1000;6000
      send_setting1_iphone();
    }
     else if (inputString.startsWith("st1;") ) {//ecrit le nouveau setting dans eeprom 
      //parm retour carto demarrage 
      // debug 1/0 (oui/non)
      //rev mini
      // rev maxi
      //  exemple "st1;5;0;1000;6000
      send_setting1_ecu();
    }
      else if (inputString.startsWith("gt2") ) {//envoie le setting a l'iphone
      //parm retour nom bluettooth 
      // retour exemple "gt2;combiduino
      send_setting2_iphone();
    }
     else if (inputString.startsWith("st2;") ) {//ecrit le nouveau setting dans eeprom 
      //parm retour nom bluetooth
      //  exemple "st2;combiduino
      send_setting2_ecu();
     }
    else if (inputString.startsWith("gt3") ) {//envoie le setting a l'iphone
      //parm retour  
      // multispark 1/0 (oui/non)
      // correction avance
      // knock actif 1/0
      send_setting3_iphone();
    }
     else if (inputString.startsWith("st3;") ) {//ecrit le nouveau setting dans eeprom 
      //parm retour 
      // multispark 1/0 (oui/non)
      // correction avance
      // knock actif 1/0
      //  exemple "st3;0;3;1
      send_setting3_ecu();
    }
    else if (inputString.startsWith("initkpa") ) {//re initialise la reference kpa 
      initpressure();
      debug("init kpa ok");
    }
    else if (inputString.startsWith("recknk on") ) {//record knock moyen 
      recordknock();
      debug("record knock");
    }
    else if (inputString.startsWith("recknk off") ) {//stop record knock moyen et save en EEPROM 
      saveknock();
      debug("save knock");
    }
    else if (inputString.startsWith("knk off") ) {//stop gestion du cliquetis 
      knock_active = false;
      debug("knock off");
    }
    else if (inputString.startsWith("knk on") ) {//stop gestion du cliquetis 
      knock_active = false;
      debug("knock off");
    }

   // RAZ c est traite 
     inputString = "";
     stringComplete = false;
   
 }
}

//---------------------------------------
//  GESTION DES COMMANDES
//---------------------------------------


//-----------------------------------------------------------------
//       ENVOI DES SETTINGS DE L'IPHONE => ECU
//-----------------------------------------------------------------

// Maj des settings dans l'eeprom a partir de l'Iphone
void send_setting1_ecu(){
    String carto="1";
  String debug = "0";
  String rpmmin = "1000";
  String rpmmax = "6000";
  carto = getValue(inputString, ';', 1) ; // 1er parametre
  debug = getValue(inputString, ';', 2) ; // 2er parametre
  rpmmin = getValue(inputString, ';', 3) ; // 3er parametre
  rpmmax = getValue(inputString, ';', 4) ; // 4er parametre
  
  Serial.println("debug|" + debug + "|");
  
   if ( (carto.toInt() > 0) and (carto.toInt() <= nombre_carto_max) ) {
     EEPROM.write(eprom_carto_actuel, carto.toInt()); // MAP en cours = 1
     carto_actuel = carto.toInt();
   }   
   if ( (debug.toInt() >= 0) and (debug.toInt() <= 1) ) {
     EEPROM.write(eprom_debug, debug.toInt()); // debug 
     debugging = debug.toInt() == 0 ? false : true;
     
   }
   if (rpmmin.toInt() < 1000) {
     EEPROMWriteInt(eprom_rev_min, rpmmin.toInt() ); // rev min
   }
   if (rpmmax.toInt() > 1000 and rpmmax.toInt() < 9999) {
     EEPROMWriteInt(eprom_rev_max, rpmmax.toInt() ); // rev max 
   }
}

void send_setting2_ecu(){

  char BT [14]= "combi";
   String nameBT = "                         ";
 
   nameBT= getValue(inputString, ';', 1) + "              ";
   nameBT.toCharArray(BT, 12) ; // 1er parametre
  

   
  for (int i = 0; i <9; i++) {
    if (BT_name[i] != BT[i]) {
      debug("chg" + String(i) + String(BT_name[i]) + String (BT[i]) );
      BT_name[i] = BT[i];
      EEPROM.write(eprom_nom_BLE + i, BT_name[i]); // Nom du Bluettooth
    }
  }
 
}  

void send_setting3_ecu(){  
  String ms = "0";
  String avance = "0";
  String knck = "1";
  ms = getValue(inputString, ';', 1) ; // 1er parametre
  avance = getValue(inputString, ';', 2) ; // 2er parametre
   knck = getValue(inputString, ';', 3) ; // 3er parametre
// multispark
   if ( (ms.toInt() >= 0) || (ms.toInt() <= 1) ) {
     EEPROM.write(eprom_ms, ms.toInt()); // debug 
     multispark = ms.toInt() == 0 ? false : true;
   }
// correction avance   
   if (avance.toInt() < 25) {
     EEPROMWriteInt(eprom_avance, avance.toInt() ); // rev min
     correction_degre = avance.toInt();
   }
   // knock actif
   if ( (knck.toInt() >= 0) || (knck.toInt() <= 1) ) {
     EEPROM.write(eprom_adresseknock, knck.toInt()); // debug 
     knock_active = knck.toInt() == 0 ? false : true;
   }
  
}


//-----------------------------------------------------------------
//       ENVOI DES SETTINGS A L'IPHONE
//-----------------------------------------------------------------


// Envoie le setting 1 a l'iphone
void send_setting1_iphone() {
OutputString="";
  OutputString = "gt1;" + String(EEPROM.read(eprom_carto_actuel)) + ";" + String(EEPROM.read(eprom_debug))+ ";" + String(EEPROMReadInt(eprom_rev_min)) +  ";" + String(EEPROMReadInt(eprom_rev_max))  ; 
        Send_to_BT(OutputString); 
}

// Envoie le setting 2 a l'iphone
void send_setting2_iphone() {
OutputString="";
  OutputString = "gt2;" + String(BT_name); 
        Send_to_BT(OutputString); 
}

// Envoie le setting 3 a l'iphone  
  void send_setting3_iphone() {
  byte ms = EEPROM.read(eprom_ms);
   int av = EEPROMReadInt(eprom_avance);
 byte kn = EEPROM.read(eprom_adresseknock);
   
    OutputString="";
    OutputString = "gt3;" + String(ms) + ";" + av +";" + String(kn)   ; 
        Send_to_BT(OutputString); 
}


// changement de carto actuelle en carto X
void changement_carto_ram(){
  String carto="1";
  carto = getValue(inputString, ';', 1) ; // 1er parametre
  if ( (carto.toInt() >0) && (carto.toInt() <= nombre_carto_max) ){
    carto_actuel = carto.toInt();
    EEPROM.write(eprom_carto_actuel,carto_actuel); // SAV de la derniere carto en cours
  debug("carto change" + carto);
  }else{
    debug("carto invalide" + carto);
  }
}



// Ecriture de la carto X en RAM vers carto Y EEPROM
void carto_ram_vers_eeprom(){
  String cartoram="1";
  String cartoeeprom="1";
  cartoram = getValue(inputString, ';', 1) ; // 1er parametre
  cartoeeprom = getValue(inputString, ';', 2) ; // 2eme parametre
  if ( (cartoram.toInt() >0) && (cartoram.toInt() <= nombre_carto_max) && (cartoeeprom.toInt() >0) && (cartoeeprom.toInt() <=eeprom_nombre_max_carto)   ){
     writecarto_ram_eeprom(cartoram.toInt() , cartoeeprom.toInt());
    debug("carto sauvegarde ram"+ cartoram + "eeprom" + cartoeeprom  );
  }else{
    debug("ordre invalide");
  }
}

// Ecriture de la carto X EEPROM vers carto Y en RAM
void carto_eeprom_vers_ram(){
  String cartoram="1";
  String cartoeeprom="1";
  cartoram = getValue(inputString, ';', 1) ; // 1er parametre
  cartoeeprom = getValue(inputString, ';', 2) ; // 2eme parametre
  if ( (cartoram.toInt() >0) && (cartoram.toInt() <= nombre_carto_max) && (cartoeeprom.toInt() >0) && (cartoeeprom.toInt() <=eeprom_nombre_max_carto)   ){
     writecarto_eeprom_ram(cartoram.toInt() , cartoeeprom.toInt());
    debug("carto sauvegarde");
  }else{
    debug("ordre invalide");
  }
}
//--------------------------------------------------------
//  ENVOI DONNEE VERS IPHONE
//-------------------------------------------------------

// lit une carto EEPROM ----> IPHONE
// format "cep,nrcarto,nrligne, point RPM,degre"
void send_carto_iphone(){
    String cartoeeprom="1";
    OutputString = "";
  cartoeeprom = getValue(inputString, ';', 1) ; // 2eme parametre
  if (  (cartoeeprom.toInt() >0) && (cartoeeprom.toInt() <=eeprom_nombre_max_carto)   ){
    
    for (int nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++){ // on parcout les ligne de la carto EEPROM
    readeepromline(nr_ligne,cartoeeprom.toInt()); // on lit dans l'EEPROM la ligne correspondante mise dans le tableau EEPROM_lignecarto
      for (int nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++){ // on parcout les colonnes de la carto
        OutputString = "cep;" + String(cartoeeprom.toInt()) + ";" + String(nr_ligne) + ";" + String(nr_RPM) + ";"  + String(EEPROM_lignecarto[nr_RPM])  ; // on rempli la ligne carto   
        Send_to_BT(OutputString);  
      }
     OutputString ="";
    }
  }else{
    debug("ordre invalide");
  }
}

// lit un axe KPA EEPROM ----> IPHONE
// format "cek,nrcarto,nrligne, kpa"
void send_kpa_iphone(){
    String cartoeeprom="1";
    OutputString = "";
    cartoeeprom = getValue(inputString, ';', 1) ; // 1eme parametre
  if (  (cartoeeprom.toInt() >0) && (cartoeeprom.toInt() <=eeprom_nombre_max_carto)   ){
   
    readeepromlinekpa(cartoeeprom.toInt()); // on lit l EEPROM --->BUFFER
    for (int nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++){ // on parcout les colonnes de la carto
        OutputString = "cek;" + String(cartoeeprom.toInt()) + ";" + String(nr_ligne) + ";"  + String(EEPROM_ligneKPA[nr_ligne])  ; // on rempli la ligne carto   
        Send_to_BT(OutputString);  
      }
     OutputString ="";
  }else{
    debug("ordre invalide");
  }
}

// lit un axe RPM EEPROM ----> IPHONE
// format "cer,nrcarto,nrligne, kpa"
void send_rpm_iphone(){
    String cartoeeprom="1";
    OutputString = "";
    cartoeeprom = getValue(inputString, ';', 1) ; // 1eme parametre
  if (  (cartoeeprom.toInt() >0) && (cartoeeprom.toInt() <=eeprom_nombre_max_carto)   ){
   
    readeepromlinerpm(cartoeeprom.toInt()); // on lit l EEPROM --->BUFFER
    for (int nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++){ // on parcout les colonnes de la carto
        OutputString = "cer;" + String(cartoeeprom.toInt()) + ";" + String(nr_RPM) + ";"  + String(EEPROM_ligneRPM[nr_RPM])  ; // on rempli la ligne carto   
        Send_to_BT(OutputString);  
      }
     OutputString ="";
  }else{
    debug("ordre invalide");
  }
}

//-------------------------------------------------
// Reception des donnÃ©e de carto , axes RPM & Kpa
//     IPHONE ===>> ECU
//------------------------------------------------

// envoi de point KPA IPHONE => ECU
void get_kpa_iphone(){
  int adresse = 0;
  int nr_carto = 0;
  String carto="1";
  String point_kpa = "0";
  String kpa = "0";
  
  carto = getValue(inputString, ';', 1) ; // 1er parametre
  point_kpa = getValue(inputString, ';', 2) ; // 3er parametre
  kpa = getValue(inputString, ';', 3) ; // 4er parametre
  
  if ( (carto.toInt() >0) && (carto.toInt() <= nombre_carto_max)  && (point_kpa.toInt() <= nombre_point_DEP -1) && (kpa.toInt() >= 0 ) ){
    nr_carto = carto.toInt(); 
    nr_carto-- ; // car la carto 1 -> pas de dÃ©calage
    pressure_axis[nr_carto][point_kpa.toInt()] = kpa.toInt();  // on ecrit en RAM
    
    adresse = (nr_carto * taille_carto ) + debut_kpa + debut_eeprom; // on retrouve l'adresse du dÃ©but de la ligne a Ã©crirr
    EEPROM.write(adresse + point_kpa.toInt() * nbr_byte_par_int,  kpa.toInt() ); // on ecrit en EEPROM
  }
}
// envoi de point RPM IPHONE => ECU
void get_rpm_iphone(){
  int adresse = 0;
  int nr_carto = 0;
  String carto="1";
  String point_rpm = "0";
  String rpm = "0";
  
  carto = getValue(inputString, ';', 1) ; // 1er parametre
  point_rpm = getValue(inputString, ';', 2) ; // 3er parametre
  rpm = getValue(inputString, ';', 3) ; // 4er parametre
  
  if ( (carto.toInt() >0) && (carto.toInt() <= nombre_carto_max)  && (point_rpm.toInt() <= nombre_point_RPM -1) && (rpm.toInt() >= 0 ) ){
    
    nr_carto = carto.toInt(); 
    nr_carto-- ; // car la carto 1 -> pas de dÃ©calage
    rpm_axis[nr_carto][point_rpm.toInt()] = rpm.toInt();  // on ecrit en RAM
  
    adresse = (nr_carto * taille_carto) + debut_rpm + debut_eeprom; // on retrouve l'adresse du dÃ©but de la ligne a Ã©crirr
    EEPROMWriteInt(adresse + point_rpm.toInt() * 2, rpm.toInt() ); // on ecrit des vrai Int sur 2 Bytes
  }
}

// changementd'un point de carto X en RAM
void changement_point_carto_ram(){
  String carto="1";
  String point_rpm = "0";
  String point_kpa = "0";
  String degre = "0";
  
  carto = getValue(inputString, ';', 1) ; // 1er parametre
  point_rpm = getValue(inputString, ';', 2) ; // 2er parametre
  point_kpa = getValue(inputString, ';', 3) ; // 3er parametre
  degre = getValue(inputString, ';', 4) ; // 4er parametre
  
  if ( (carto.toInt() >0) && (carto.toInt() <= nombre_carto_max) && (point_rpm.toInt() <= nombre_point_RPM -1) && (point_kpa.toInt() <= nombre_point_DEP -1) && (degre.toInt() >= 0 ) ){
   ignition_map[(nombre_point_DEP*( carto.toInt() - 1 )) + point_kpa.toInt()] [point_rpm.toInt()] = degre.toInt();
    debug("chgt point kpa " + String((nombre_point_DEP* (carto.toInt() -1) ) + point_kpa.toInt() ) + "|rpm " + String(point_rpm.toInt() ) + "degre : " + degre );
  }else{
    debug("ordre invalid");
  }
}


//-----------------------------------------------------
//------------Envoi de donnÃ©e dans le port serie / BT
//-----------------------------------------------------
void gestionsortieECU(){
String SortieBT;
// on envoie au port serie pour debug
if (debugging == true && 1==2 ){
//if (debugging == true  ){
  debug("RPM :" +String(engine_rpm_average)+ " MAP :" + String(map_pressure_kpa) + " DEG :"+ String(Degree_Avance_calcul) + " MAP_US :"+String(map_value_us) ) ;
}

// on envoie au port BT
if (output == true){
  // parm 1 RPM
  // parm 2 depression actuel
  // parm 3 degre actuel
 // ECU;9999;100;032 // RPM KPA Degre
   SortieBT = "ECU;" + String(engine_rpm_average) + ";" + String(map_pressure_kpa) + ";"+ String(Degree_Avance_calcul) ; 
   Send_to_BT(SortieBT); 
  }
 
 } 
 
void gestionsortieEC1(){
String SortieBT;
// on envoie au port serie pour debug

// on envoie au port BT
if (output == true){
  // parm 1 carto actuelle
  // parm 2 correction actuel
  // parm 3 libre knock KNOCK MOYEN
  // parm 4 libre knock DIFFERENCE actuel - Moyen
 // var1=map_value_us;
// var1 = AFR_actuel;
   SortieBT = "EC1;" + String(carto_actuel) + ";" + String(correction_degre) +";"+String(var1)+";" + String(var2) ; 
 
 
   Send_to_BT(SortieBT); 
  }
 
 } 

