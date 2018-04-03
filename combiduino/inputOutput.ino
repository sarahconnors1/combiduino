//---------------------------------------------------------------
//  GESTION DES ENTREE SORTIE VERS DEBUG OU IPHONE
//---------------------------------------------------------------

//-------------Check des donnÃƒÂ©e entrante par le port serie-----------------------------
void checkdesordres(){
 
  Read_BT(); // lecture du BT
 
  if (stringComplete){   
    inputString.trim(); 
  if(inputString == "fixed") {
        debug("Fixed Advance Selected");
        sbi(ECU.running_option,BIT_FIXED_ADV);
   }
   else if (inputString == "map") {
        debug("Ignition Advance Selected");
        cbi(ECU.running_option,BIT_FIXED_ADV);
   }
   else if (inputString == "dbg on") {
        debug("debug ON");
        sbi(ECU.running_option,BIT_DEBUG);
   }
   else if (inputString == "dbg off") {
        debug("debug OFF");
       cbi(ECU.running_option,BIT_DEBUG);
   }
   
   else if (inputString == "ms on") {
         debug("Multispark Enabled");
         sbi(ECU.running_option,BIT_MS);
       //  first_multispark = true;
   }
   else if (inputString == "ms off") {
         debug("Multispark Disabled");
         cbi(ECU.running_option,BIT_MS);
   }
   else if (inputString == "output on"){
      sbi(ECU.running_option,BIT_OUTPUT_BT);
    }
    else if (inputString == "output off"){
      cbi(ECU.running_option,BIT_OUTPUT_BT);
   }
    else if (inputString == "plus"){
      correction_degre = correction_degre + 3;
   }
    else if (inputString == "moins"){
      correction_degre = correction_degre - 3;
   }
    else if (inputString.startsWith("map;") ) {// change la carto actuelle
    //parm 1 nr de carto 
      changement_carto(); // REVU
    }
    else if (inputString.startsWith("chg;") ) {// change la 1 point de la carto x
    //parm 1 nr de carto RAM
    //parm 2 point RPM 0 -> 22
    //parm 3 point KPA 0 -> 16
    //parm 4 degre
      changement_point_carto(); // REVU
    }
    else if (inputString.startsWith("sndcarto;") ) {//envoie la carto eeprom => IPHONE
      //parm 1 nr de carto EEPROM
      send_carto_iphone(); // REVU
    }
    else if (inputString.startsWith("sndkpa;") ) {//envoie l axe KPA eeprom => IPHONE
      //parm 1 nr de carto EEPROM
      send_kpa_iphone(); // REVU
    }
    else if (inputString.startsWith("gtkpa;") ) {//envoie l axe KPA iphone => ECU
      //parm 1 nr de carto EEPROM
      // parm 2 point kpa
      // parm 3 valeur
      get_kpa_iphone(); // REVU
    }
    else if (inputString.startsWith("sndrpm;") ) {//envoie l axe RPM eeprom => I PHONE
      //parm 1 nr de carto EEPROM
      send_rpm_iphone(); // REVU
    }
    else if (inputString.startsWith("gtrpm;") ) {//envoie l axe RPM iphone => ECU
      //parm 1 nr de carto EEPROM
      // parm 2 point rpm
      // parm 3 valeur
      get_rpm_iphone(); // REVU
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
     #if KNOCK_USED == 1 
      knock_active = false;
     #endif
      debug("knock off");
    }
    else if (inputString.startsWith("knk on") ) {//stop gestion du cliquetis 
      #if KNOCK_USED == 1 
      knock_active = on;
      #endif
      debug("knock off");
    }
    else if (inputString.startsWith("afr on") ) {//regulation lambda 
      correction_lambda_used = true;
      debug("AFR ON");
    }
    else if (inputString.startsWith("afr off") ) {//regulation lambda 
      correction_lambda_used = false;
      debug("AFR OFF");
    }
    else if (inputString.equals("p") ) {//ajustement pour reqfuel
      Req_Fuel_us += 500;
      debug("plusreq");
    }
    else if (inputString.equals("m") ) {//ajustement pour reqfuel 
      Req_Fuel_us -= 500;
      debug("moinsreq");
    }
    else if (inputString.equals("ego w") ) {//write dans eeprom des corrections 
      writeego_ram_eeprom();
      debug("write ego");
    }
    else if (inputString.equals("ego p") ) {//imprime les corrections actuels 
      printego_ram();
      debug("print ego");
    }
    else if (inputString.equals("idle on") ) {//imprime les corrections actuels 
      Idle_management = true;
      debug("idle on");
    }
    else if (inputString.equals("idle off") ) {//imprime les corrections actuels 
      Idle_management = false;
      debug("idle off");
    }
     else if (inputString.equals("init log") ) {//imprime les corrections actuels 
      initlog();
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
  
   if ( (carto.toInt() > 0) and (carto.toInt() <= nombre_carto_max) ) {
     EEPROM.write(eprom_carto_actuel, carto.toInt()); // MAP en cours = 1
     ECU.carto_actuel = carto.toInt();
   }   
   if ( (debug.toInt() >= 0) and (debug.toInt() <= 1) ) {
     EEPROM.write(eprom_debug, debug.toInt()); // debug 

     if (debug.toInt() == 0){
      cbi(ECU.running_option,BIT_DEBUG);
     }else{
      sbi(ECU.running_option,BIT_DEBUG);
     }
     
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
    
    if (ms.toInt() == 0){
      cbi(ECU.running_option,BIT_MS);
    }else{
      sbi(ECU.running_option,BIT_MS);
    }

     
   }
// correction avance   
   if (avance.toInt() < 25) {
     EEPROMWriteInt(eprom_avance, avance.toInt() ); // rev min
     correction_degre = avance.toInt();
   }
   // knock actif
  #if KNOCK_USED == 1 
   if ( (knck.toInt() >= 0) || (knck.toInt() <= 1) ) {
     EEPROM.write(eprom_adresseknock, knck.toInt()); // debug 
     knock_active = knck.toInt() == 0 ? false : true;
   }
  #endif
}


//-----------------------------------------------------------------
//       ENVOI DES SETTINGS ECU A L'IPHONE
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

//------------------------------------------------------------------
// changement de carto actuelle en carto X
//------------------------------------------------------------------
void changement_carto(){
  String carto="1";
  carto = getValue(inputString, ';', 1) ; // 1er parametre
  if ( (carto.toInt() >0) && (carto.toInt() <= nombre_carto_max) ){
    ECU.carto_actuel = carto.toInt();
   // EEPROM.write(eprom_carto_actuel,carto_actuel); // SAV de la derniere carto en cours
    writecarto_eeprom_ram(ECU.carto_actuel); // on ecrit la carto en RAM + axe KPA/RPM
  debug("carto change" + carto);
  }else{
    debug("carto invalide" + carto);
  }
}



//*******************************************************
//  ENVOI DONNEE EEPROM VERS IPHONE
//*******************************************************

//----------------------------------
// point carto EEPROM ----> IPHONE
//----------------------------------
// format "cep,nrcarto,nrligne, point RPM,degre"
void send_carto_iphone(){
    String cartoeeprom="1";
    int carto_x = 1;
    int value = 0;
    OutputString = "";
  cartoeeprom = getValue(inputString, ';', 1) ; // 2eme parametre
  carto_x = cartoeeprom.toInt() ; 
  if (  (carto_x >0) && (carto_x <=nombre_carto_max)   ){   
  carto_x--;
    for (int nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++){ // on parcout les ligne de la carto EEPROM
      for (int nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++){ // on parcout les colonnes de la carto
        value = read_eeprom_point_carto (carto_x , nr_ligne , nr_RPM );
        OutputString = "cep;" + String(cartoeeprom.toInt()) + ";" + String(nr_ligne) + ";" + String(nr_RPM) + ";"  + String(value)  ; // on rempli la ligne carto   
        Send_to_BT(OutputString);  
      }
     OutputString ="";
    }
  }else{
    debug("ordre invalide");
  }
}
//-----------------------------------
// axe KPA EEPROM ----> IPHONE
//-----------------------------------
// format "cek,nrcarto,nrligne, kpa"
// REVU
void send_kpa_iphone(){
   String cartoeeprom="1";
  int carto_x = 1;
  int value = 0;
  OutputString = "";
  cartoeeprom = getValue(inputString, ';', 1) ; // 1eme parametre
  carto_x = cartoeeprom.toInt() ; 
  if (  (carto_x >0) && (carto_x <=nombre_carto_max)   ){
   carto_x--;
    for (int nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++){ // on parcout les colonnes de la carto
        value = read_eeprom_point_KPA (carto_x , nr_ligne );
        OutputString = "cek;" + String(cartoeeprom.toInt()) + ";" + String(nr_ligne) + ";"  + String(value)  ; // on rempli la ligne carto   
        Send_to_BT(OutputString);  
      }
     OutputString ="";
  }else{
    debug("ordre invalide");
  }
}
//---------------------------------
// axe RPM EEPROM ----> IPHONE
//---------------------------------
// format "cer,nrcarto,nrligne, kpa"
void send_rpm_iphone(){
  int carto_x = 1;
  int value = 0;
  String cartoeeprom="1";
  OutputString = "";
  cartoeeprom = getValue(inputString, ';', 1) ; // 1eme parametre
  carto_x = cartoeeprom.toInt() ; 
  if (  (carto_x >0) && (carto_x <=nombre_carto_max)   ){
   carto_x--;
    for (int nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++){ // on parcout les colonnes de la carto
        value = read_eeprom_point_RPM (carto_x , nr_RPM );
        OutputString = "cer;" + String(cartoeeprom.toInt()) + ";" + String(nr_RPM) + ";"  + String(value)  ; // on rempli la ligne carto   
        Send_to_BT(OutputString);  
    }
     OutputString ="";
  }else{
    debug("ordre invalide");
  }
}

//**********************************************
// RECEPTION DONNEE    IPHONE ===>> ECU
//**********************************************

//---------------------------------
//  point KPA IPHONE => ECU
//---------------------------------
void get_kpa_iphone(){
  int nr_carto = 0;
  String carto="1";
  String point_kpa = "0";
  String kpa = "0";
  
  carto = getValue(inputString, ';', 1) ; // 1er parametre
  point_kpa = getValue(inputString, ';', 2) ; // 3er parametre
  kpa = getValue(inputString, ';', 3) ; // 4er parametre
  
  if ( (carto.toInt() >0) && (carto.toInt() <= nombre_carto_max)  && (point_kpa.toInt() <= nombre_point_DEP -1) && (kpa.toInt() >= 0 ) ){
    nr_carto = carto.toInt(); 
    nr_carto-- ; // car la carto 1 -> pas de dÃƒÂ©calage
    write_eeprom_point_KPA (nr_carto , point_kpa.toInt() , kpa.toInt() );
  }
}
//---------------------------------
// point RPM IPHONE => ECU
//---------------------------------
void get_rpm_iphone(){
  int nr_carto = 0;
  String carto="1";
  String point_rpm = "0";
  String rpm = "0";
  
  carto = getValue(inputString, ';', 1) ; // 1er parametre
  point_rpm = getValue(inputString, ';', 2) ; // 3er parametre
  rpm = getValue(inputString, ';', 3) ; // 4er parametre
  
  if ( (carto.toInt() >0) && (carto.toInt() <= nombre_carto_max)  && (point_rpm.toInt() <= nombre_point_RPM -1) && (rpm.toInt() >= 0 ) ){
    nr_carto = carto.toInt(); 
    nr_carto-- ; // car la carto 1 -> pas de dÃƒÂ©calage
    write_eeprom_point_RPM (nr_carto , point_rpm.toInt() , rpm.toInt() );
  }
}
//--------------------------------
//  point de carto IPHONE => ECU 
//--------------------------------
void changement_point_carto(){
  String carto="1";
  String point_rpm = "0";
  String point_kpa = "0";
  String degre = "0";
  
  carto = getValue(inputString, ';', 1) ; // 1er parametre
  point_rpm = getValue(inputString, ';', 2) ; // 2er parametre
  point_kpa = getValue(inputString, ';', 3) ; // 3er parametre
  degre = getValue(inputString, ';', 4) ; // 4er parametre
  
  if ( (carto.toInt() >0) && (carto.toInt() <= nombre_carto_max) && (point_rpm.toInt() <= nombre_point_RPM -1) && (point_kpa.toInt() <= nombre_point_DEP -1) && (degre.toInt() >= 0 ) ){
    write_eeprom_point_carto (carto.toInt() , point_kpa.toInt() , point_rpm.toInt() , degre.toInt() );
  }else{
    debug("ordre invalid");
  }
}


//-----------------------------------------------------
//------------Envoi de donnee dans le port serie / BT
//-----------------------------------------------------
void gestionsortieECU(){
# if BLUETOOTH_USED == 1
String SortieBT;

// on envoie au port BT
if (BIT_CHECK(ECU.running_option,BIT_OUTPUT_BT)){
  // parm 1 RPM
  // parm 2 depression actuel
  // parm 3 degre actuel
 // ECU;9999;100;032 // RPM KPA Degre
   SortieBT = "ECU;" + String(ECU.engine_rpm_average) + ";" + String((int)(ECU.map_pressure_kpa)) + ";"+ String((int)(ECU.Degree_Avance_calcul )) ; 
   Send_to_BT(SortieBT); 
  }

 #endif
 } 
 
void gestionsortieEC1(){
# if BLUETOOTH_USED == 1
String SortieBT;
// on envoie au port serie pour debug

// on envoie au port BT
if (BIT_CHECK(ECU.running_option,BIT_OUTPUT_BT)){
  // parm 1 carto actuelle
  // parm 2 correction actuel
  // parm 3 libre knock KNOCK MOYEN
  // parm 4 libre knock DIFFERENCE actuel - Moyen
 // var1=map_value_us;
var1 = ECU.TPS_actuel;
var2 = ECU.AFR_actuel;
//var2 = acceleration_actuel;
   SortieBT = "EC1;" + String(ECU.carto_actuel) + ";" + String(correction_degre) +";"+String(var1)+";" + String(var2) ; 
 
 
   Send_to_BT(SortieBT); 
  }

#endif

} 

void printego_ram(){
// ecrit sur l'eeprom la carto actuelle
// gestion point de carto
String txt="";
  for (int nr_ligne = 0; nr_ligne < nombre_point_DEP; nr_ligne++) { // on parcout les ligne de la carto EEPROM
    txt = "{";
    for (int nr_RPM = 0; nr_RPM < nombre_point_RPM; nr_RPM++) { // on parcout les colonnes de la carto
      if (nr_RPM == nombre_point_RPM - 1){
        txt = txt +  String(Ego_map [nr_ligne][nr_RPM]); // cas du dernier 
      }else{
        txt = txt +  String(Ego_map [nr_ligne][nr_RPM]) + ",";
      }
    }
    txt = txt + "},";
    sndlog(txt);
  }
}


