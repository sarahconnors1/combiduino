void debug(String str){
 if (debugging == true){
 Serial.println(str);
 } 
}

// ---------------------------------
// cadrage a droite sur n caractere
// ---------------------------------
String Right(String str, int n){
 int iLen= 0;
 str = "              " + str;
    if (n <= 0)
       return "";
    else if (n > str.length()   )
       return str;
    else {
       iLen = str.length();
       return str.substring(iLen - n, iLen);
    }
}

//-----------------------------------
// envoi d'un message + pause de 2s
//-----------------------------------

void message(String str){
  RAZ_affichage();
   lcd.setCursor(0,0);
   lcd.print(str); 
   delay(2000);
}

//---------------------------------
//affichage sur l'ecran
//---------------------------------
void LcdPrint(int x,int y,String str){
   lcd.setCursor(x,y);
   lcd.print(str);  
}

//------------------------------------
// gestion du parsing
//------------------------------------
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

