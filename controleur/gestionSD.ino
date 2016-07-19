
// recupere le nouveau nom du fichier log
void init_SD_file(){
  byte num = 0;
  num = EEPROM.read(EEPROM_file_number);

  if (num > 250){
    num = 1;
  }
  num++;
  EEPROM.write(EEPROM_file_number ,  num );

filename = filename + num + ".csv";

     LcdPrint(0,1, "file " + filename);
}

void write_SD_log(){
  debug("write_SD_log " + filename);
   File dataFile = SD.open(filename, FILE_WRITE);
  
  if (dataFile) {
    dataFile.print(inputString);
    dataFile.close();
    debug("fait");
  } else {
    Serial.println("error opening datalog.txt");
  }
 
}

