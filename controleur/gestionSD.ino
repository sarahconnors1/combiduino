
// recupere le nouveau nom du fichier log
void init_SD_file(){
 
  num_file = EEPROM.read(EEPROM_file_number);
  if (num_file > 250){ num_file = 1;}
  num_file++;
  EEPROM.write(EEPROM_file_number ,  num_file );
  filename = filename + num_file + ".csv";
  LcdPrint(0,20, "file " + filename);
#if SD_USED == 1 
  dataFile = SD.open(filename, O_CREAT | O_WRITE);  
#endif
}

void write_SD_log(){
#if SD_USED == 1
unsigned long temp=micros(); 
  
  if (dataFile) {
    dataFile.println(inputString);    
  }

flush_cycle = flush_cycle + inputString.length() ;
if (flush_cycle >= 400){
  dataFile.flush();
  flush_cycle= 0;
 // debug("SD flush " + String((long)( micros() -temp)) );
}
// debug("SD write " + String((long)( micros() -temp)) + " " + String(flush_cycle) + " " + String(inputString.length() )  );
#endif
}

//********************************************************************************
//               GESTION DU PORT SERIE
//********************************************************************************

void serialEvent1() {
int ByteCount = -1;

 
if (Serial1.available()>40) {
  if (Serial1.read() == 0x06){
   if (Serial1.read() == 0x85){
    if (Serial1.read() == sizeof(ECUdata) ){
       ByteCount =  Serial1.readBytesUntil('\n',Buffer,sizeof(RECEIVE_DATA_STRUCTURE_t)); 
      if (ByteCount == sizeof(ECUdata)){
        if (Buffer[3] == 0x00){ // sanity check
          stringComplete = true;
        }else{erreur_reception++;}
      }else{erreur_reception++;}
   }else{erreur_reception++;}
  }else{erreur_reception++;}
 }else{erreur_reception++;}
}
}

void envoiECU(){

// envoi de la nouvelle carto a l'ECU
  if (carto_change == true ){
   carto_change = false;
   Serial.println("map;" + String(carto_demande) );
   message("  carto changé");
   rebuild_menu = true;
   refresh_menu = true;
   gestion_affichage_des_menus();
  }

// Enrichissement manuel dans l'ecran 3
  if (MCEchange_requested == true ){
    MCEchange_requested=false;
    Serial.println("MCE;" + String(MCE_requested) );
    message("  MCE changé");
    rebuild_menu = true;
    refresh_menu = true;
    gestion_affichage_des_menus();
  }
  
  
}

//---------------------------------
// Gestion ENTREES/SORTIES vers l'ECU
//---------------------------------
void decodeECU(){


if(stringComplete == true ){  
  for (int k=0; k < sizeof(RECEIVE_DATA_STRUCTURE_t); k++){ 
        ECUdata.packet[k] = Buffer[k];
  }

// log erreur de recpetion
//ECUdata.value.var2 = erreur_reception;

  unsigned long temp=micros(); 
  inputString = String( ECUdata.value.time_loop/ float(1000) ) +  ";" 
  + String(ECUdata.value.CLT)  + ";"
  + ECUdata.value.IAE_actuel + ";" 
  + ECUdata.value.engine_rpm_average + ";" 
  + ECUdata.value.map_pressure_kpa  + ";" 
//  + ECUdata.value.MAPdot  + ";"
  + ECUdata.value.injection_time_us  + ";" 
  + ECUdata.value.VE_actuel + ";"  
  + ECUdata.value.AFR_actuel + ";"   
  + ECUdata.value.correction_lambda_actuel + ";"  
  + ECUdata.value.Degree_Avance_calcul + ";"    
  + ECUdata.value.TPS_actuel + ";"  
//  + ECUdata.value.TPSdot + ";"  
  + ECUdata.value.carto_actuel  + ";" 
  + ECUdata.value.PID_idle_advance + ";"  
  + ECUdata.value.DEC_actuel   + ";"
  + ECUdata.value.PW_actuel + ";" 
  + ECUdata.value.qte_paroi  + ";"
  + ECUdata.value.X_adher  + ";"
  + ECUdata.value.Tau_evap  + ";"
  + ECUdata.value.AFR_objectif + ";"
  + ECUdata.value.point_KPA + ";"
  + ECUdata.value.point_RPM + ";" 
  +  bitRead(ECUdata.value.running_mode,BIT_ENGINE_DCC)+ ";"  // decel avec coupure
  +  bitRead(ECUdata.value.running_mode,BIT_ENGINE_ACC ) + ";" // accel declenché
  +  bitRead(ECUdata.value.running_mode,BIT_ENGINE_IDLE ) + ";" // iddle enclenché
  +  bitRead(ECUdata.value.running_mode,BIT_ENGINE_WARMUP ) + ";" // en chauffe
  +  bitRead(ECUdata.value.running_mode,BIT_EGO_ACTIVE   ) + ";" // activatio, EGO
//  + ECUdata.value.TPSMAPdot + ";" 
//  + ECUdata.value.PW_accel_actuel_us + ";" 
  + ECUdata.value.VLT + ";" 
  + ECUdata.value.tps_ouverture_injecteur_us + ";"
  + ECUdata.value.MCE_actuel + ";"
//  + ECUdata.value.var1 + ";" 
//  + ECUdata.value.var2 + ";" 
  ;

 //debug("decode ECUstring " + String((long)( micros() -temp)) );  

  if (event_requested == true){
    event_requested = false;
    inputString = inputString + "1;"; // on rajoute l'info evenement demandé
    drawbutton(0, 165, 70, 230, "HIT",10,190, false); 
  }else{
    inputString = inputString + "0;"; // on rajoute l'info evenement demandé
  }

// verification de la validité des données 

  if (
    ((ECUdata.value.engine_rpm_average >= 0) and (ECUdata.value.engine_rpm_average < 10000)  )
    and ( (ECUdata.value.AFR_actuel > 0) and ( ECUdata.value.AFR_actuel < 200) )
    and (( ECUdata.value.carto_actuel >0 ) and (ECUdata.value.carto_actuel < 20) )
    and ( (ECUdata.value.point_RPM >= 0) and (ECUdata.value.point_RPM < 20) ) 
     and ( (ECUdata.value.point_KPA >= 0) and (ECUdata.value.point_KPA < 20) )
    ){
    
      write_SD_log();
      nbr_log++; 
      RPM = ECUdata.value.engine_rpm_average ;
      KPA = ECUdata.value.map_pressure_kpa;
      AFR = ECUdata.value.AFR_actuel; 
      IAE = ECUdata.value.IAE_actuel; 
      CLT = ECUdata.value.CLT; 
      VLT = ECUdata.value.VLT; 
      TPS = ECUdata.value.TPS_actuel; 
      CorL = ECUdata.value.correction_lambda_actuel;
      IDL = ECUdata.value.PID_idle_advance;
      IOT = ECUdata.value.tps_ouverture_injecteur_us;
      avance = ECUdata.value.Degree_Avance_calcul; 
      multispark_actuel = BIT_CHECK(ECUdata.value.running_option,BIT_MS) ;

//pour affichage
      RPM = RPM/10;
      if (KPA >= 100){KPA=99;}
      
      //var1 = ECUdata.value.var1;
      //var2 = ECUdata.value.correction_lambda_actuel;
    }
 
  }

  stringComplete = false;
  inputString = ""; 
}



void initlog(){

# if SD_USED == 1
  inputString = "Time;CLT;IAE;RPM;Load;PW;VE;AFR;Gego;SparkAdv;TPS;carto;PIDidle;Dec;PW_act;qte_paroi;X_adher;Tau_evap;AFRobj;ptKPA;ptRPM;decel;accel;iddle;warmup;Egoactive;VLT;IOT;EnrichM;event"; 
write_SD_log();
# endif
}





