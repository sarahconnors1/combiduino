
void Megalog(){

# if SD_LOG_USED == 0
String logsend = "";
// fin du message
  logsend = String( time_loop/ float(1000) )  + ';'
 +  ECU.CLT + ';'
 + ECU.IAE_actuel + ';'
 +  bitRead(ECU.running_option,BIT_MS) +  ';'
 +  ECU.engine_rpm_average +  ';' 
 +  ECU.map_pressure_kpa +  ';' 
 +  ECU.MAPdot +  ';' 
// +   bitRead(ECU.ECU.running_mode, BIT_ENGINE_ACC )  +  ';' 
// + ECU. PW_accel_actuel_us  +  ';'
 +  ECU.injection_time_us +  ';' 
// +  ECU.VE_actuel  + ';' 
 +  ECU.AFR_actuel/ float(10)  +  ';' 
 +  ECU.correction_lambda_actuel  +  ';'
 +  ECU.Degree_Avance_calcul  +  ';'  
 +  ECU.TPS_actuel  +  ';' 
// +  ECU.ECU.TPSdot  + ';'
 +   bitRead(ECU.running_mode, BIT_ENGINE_IDLE )  + ';'
 +  ECU.carto_actuel  + ';'
// + ECU.PID_idle_advance  + ';'
 + ECU.DEC_actuel  + ';'
 + ECU.PW_actuel + ';'
// + qte_paroi + ';' 
// + X_adher + ';'
// + ECU.Tau_evap + ';'
 + ECU.AFR_objectif/ float(10) + ';'
 + ECU.point_KPA + ';'
 + ECU.point_RPM + ';' 
// + pip_fault + ';'
// + debouncePIP + ';'
  ;
  
 // Serial.println(logsend);

#endif

# if SD_LOG_USED == 1
char buffer[150]; // make sure this is large enough for your string + 1 for NULL char

  sprintf(buffer, "%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;%i;" , 
  (int)(time_loop/100)
  , ECU.CLT 
  , ECU.IAE_actuel 
  ,  bitRead(ECU.running_option,BIT_MS) 
  ,  ECU.engine_rpm_average 
  ,  (int) ECU.map_pressure_kpa  
  ,  ECU.MAPdot 
  ,  ECU.injection_time_us  
  ,  (int)ECU.VE_actuel  
  ,  ECU.AFR_actuel   
  ,  ECU.correction_lambda_actuel  
  ,  (int)ECU.Degree_Avance_calcul    
  ,  ECU.TPS_actuel  
  ,  ECU.TPSdot  
  ,   ECU.running_mode  
  ,  ECU.carto_actuel  
  , (int)ECU.PID_idle_advance  
  , ECU.DEC_actuel  
  , ECU.PW_actuel 
  , ECU.qte_paroi 
  , ECU.X_adher 
  , ECU.Tau_evap 
  , ECU.AFR_objectif
  ,ECU.point_KPA
  ,ECU.point_RPM
  );
  sndlog(buffer);

# endif

# if SD_LOG_USED == 2
  ECU.time_loop = time_loop;
  ET.sendData();
  Serial.println("");
# endif
}


