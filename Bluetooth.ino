// ------------------------------------------------------
//    GESTION DES FONCTIONS BLUETOOTH
// -----------------------------------------------------

void Send_to_BT(String str) {
#if BLUETOOTH_USED == 1  
  // envoie au debug sauf si info ECU RPM
  if (ble_connected() and !ble_busy() ){
    
   if (!str.startsWith("EC") ){
    debug("BTSend:"+str);
   } 
   for (unsigned int nr_car = 0; nr_car <= str.length()+1; nr_car++){ble_write( str[nr_car] );}
   ble_do_events();
  }
#endif  
}
void Read_BT(){
#if BLUETOOTH_USED == 1 
    ble_do_events();   
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
#endif   
 }

void checkBLE(){
#if BLUETOOTH_USED == 1  
  // si deconnection on reset la carte BLE
if (!ble_connected()){
  ble_reset(BLE_reset_pin);
  }
#endif  
}  

