//-----------------------------------
//         UTILITAIRE
//----------------------------------
// gestion du mode debug avec renvoi vers la console
void debug(String str){if (debugging == true){ Serial.println(str);} }

void sndlog(String str){ Serial.println(str);} 

// gestion du parsing
String getValue(String data, char separator, int index){
    // spliting a string and return the part nr index
    // split by separator
    int stringData = 0;        //variable to count data part nr 
    String dataPart = "";      //variable to hole the return text
    for(unsigned int i = 0; i<data.length(); i++) {    //Walk through the text one letter at a time
      if(data[i]==separator) { stringData++; //Count the number of times separator character appears in the text
      }else if(stringData==index) {dataPart.concat(data[i]); //get the text when separator is the rignt one  
      }else if(stringData>index) {return dataPart;break;//return text and stop if the next separator appears - to save CPU-time
      }
    }
    return dataPart; //return text if this is the last part
}


//This function will write a 2 byte integer to the eeprom at the specified address and address + 1
void EEPROMWriteInt(int p_address, int p_value){
     byte lowByte = ((p_value >> 0) & 0xFF);
     byte highByte = ((p_value >> 8) & 0xFF);

     EEPROM.write(p_address, lowByte);
     EEPROM.write(p_address + 1, highByte);
}

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int EEPROMReadInt(int p_address){
     byte lowByte = EEPROM.read(p_address);
     byte highByte = EEPROM.read(p_address + 1);

     return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
 }
 
/* 
// free RAM check for debugging. SRAM for ATmega328p = 2048Kb.
int availableMemory() {
    // Use 1024 with ATmega168
    int size = 8192;
    byte *buf;
    while ((buf = (byte *) malloc(--size)) == NULL);
        free(buf);
    return size;
}
*/

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//-------------------------------------------- Triggered when serial input detected --------------------------------------------//

 # if KNOCK_USED == 1
 void serialEvent1() {
 while (Serial1.available()) {                    //whilst the serial port is available...
   inputString3 = Serial1.readStringUntil('\n');   //... read in the string until a new line is recieved
   stringComplete3 = true;                      
 }
 }
#endif

