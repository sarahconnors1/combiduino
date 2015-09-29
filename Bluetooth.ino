// ------------------------------------------------------
//    GESTION DES FONCTIONS BLUETOOTH
// -----------------------------------------------------
//
void Send_to_BT(String str) {
  
  // envoie au debug sauf si info ECU RPM
  if (!str.startsWith("ECU;") ){
  debug("BT:"+str);
  }
  
   for (int nr_car = 0; nr_car <= str.length(); nr_car++){ble_write( str[nr_car] );}
   ble_do_events();
  
}
void Read_BT(){
   if ( ble_available() ){ 
     while ( ble_available() )
      inputString = inputString + char(ble_read());      
    }
    stringComplete = true;  
 }
  
