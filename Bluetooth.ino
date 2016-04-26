// ------------------------------------------------------
//    GESTION DES FONCTIONS BLUETOOTH
// -----------------------------------------------------

void Send_to_BT(String str) {
  // envoie au debug sauf si info ECU RPM
  if (ble_connected() and !ble_busy() ){
    
   if (!str.startsWith("EC") ){
    debug("BTSend:"+str);
   } 
   for (unsigned int nr_car = 0; nr_car <= str.length()+1; nr_car++){ble_write( str[nr_car] );}
   ble_do_events();
  }
  
}
void Read_BT(){
   if ( ble_available() ){ 
     inputString="";
     while ( ble_available() ){
       char car = char(ble_read());
       if ((car != 0) and (car !=12)) { // non null
       inputString = inputString + car;  
       }
     }
     stringComplete = true;
     debug("BTRec:"+inputString);    
      
  }
   
 }

void checkBLE(){
  // si deconnection on reset la carte BLE
if (!ble_connected()){
  ble_reset(BLE_reset_pin);
  }
}  

