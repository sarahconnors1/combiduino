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
 str = "             " + str;
    if (n <= 0)
       return "";
    else if (n > str.length()   )
       return str;
    else {
       iLen = str.length();
       return str.substring(iLen - n, iLen);
    }
}

// ---------------------------------
// cadrage a droite sur n caractere
// ---------------------------------
String RightNum(String str, int n){
 int iLen= 0;
 str = "000000000000000" + str;
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
   delay(2000);
}

//---------------------------------
//affichage sur l'ecran
//---------------------------------
void LcdPrint(int x,int y,String str){
  myGLCD.print(str, x, y);
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


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

/*
int n - The number to be displayed
int xLoc = The x location of the upper left corner of the number
int yLoc = The y location of the upper left corner of the number
int cSe = The size of the number. Range 1 to 10 uses Large Shaped Segments.
fC is the foreground color of the number
bC is the background color of the number (prevents having to clear previous space)
nD is the number of digit spaces to occupy (must include space for minus sign for numbers < 0)
nD < 0 Suppresses leading zero

Sample Use: Fill the screen with a 2-digit number showing current second suppressing leading zero 

draw7Number(second(),20,40,10,WHITE, BLACK, -2);

**********************************************************************************/

/*

void draw7Number(int n, unsigned int xLoc, unsigned int yLoc, char cS, unsigned int fC, unsigned int bC, char nD) {
unsigned int num=abs(n),i,s,t,w,col,h,a,b,si=0,j=1,d=0,S1=cS,S2=5*cS,S3=2*cS,S4=7*cS,x1=(S3/2)+1,x2=(2*S1)+S2+1,y1=yLoc+x1,y3=yLoc+(2*S1)+S4+1;
unsigned int seg[7][3]={{(S3/2)+1,yLoc,1},{x2,y1,0},{x2,y3+x1,0},{x1,(2*y3)-yLoc,1},{0,y3+x1,0},{0,y1,0},{x1,y3,1}};
unsigned char nums[12]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67,0x00,0x40},c=(c=abs(cS))>10?10:(c<1)?1:c,cnt=(cnt=abs(nD))>10?10:(cnt<1)?1:cnt;
for (xLoc+=cnt*(d=(2*S1)+S2+(2*S3)+2);cnt>0;cnt--){
for (i=(num>9)?num%10:((!cnt)&&(n<0))?11:((nD<0)&&(!num))?10:num,xLoc-=d,num/=10,j=0;j<7;++j){
col=(nums[i]&(1<<j))?fC:bC;s=(2*S1)/S3;
if (seg[j][2])for(w=S2,t=seg[j][1]+S3,h=seg[j][1]+(S3/2),a=xLoc+seg[j][0]+S1,b=seg[j][1];b<h;b++,a-=s,w+=(2*s))myGLCD.drawHLine(a,b,w,col);
else for(w=S4,t=xLoc+seg[j][0]+S3,h=xLoc+seg[j][0]+S3/2,b=xLoc+seg[j][0],a=seg[j][1]+S1;b<h;b++,a-=s,w+=(2*s)myGLCD.VLine(b,a,w,col);
for (;b<t;b++,a+=s,w-=(2*s))seg[j][2]?myGLCD.drawHLine(a,b,w,col):myGLCD.VLine(b,a,w,col);
}
}
}
*/

void printsevendigit(String nbre, int x , int y){
  // font 32 x 50
  /*
  a _____ b
    |   |
    |   |
  c _____ d
    |   |
    |   |       
  e_____ f

  */
int ax =2;
int ay =2;
int bx =30;
int by =2;
int cx =2;
int cy =25;
int dx =30;
int dy =25;
int ex =2;
int ey =48;
int fx =30;
int fy =48;
int z = 0 ;// decalage entre caractere
char nbr;
myGLCD.setColor(0, 255, 0);

for (int n=0; n < nbre.length() ; n++){ 
nbr = nbre.charAt(n)  ;
if (nbr == '0'){  
myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawHLine(ex + x +z, ey + y, 28); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawHLine(cx + x +z, cy + y, 28); // milieu horiz
}else if (nbr == '1'){
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawHLine(cx + x+z, cy + y, 28); // milieu horiz
myGLCD.drawHLine(ex + x+z, ey + y, 28); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche
  
}else if (nbr == '2'){
myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawHLine(cx + x+z, cy + y, 28); // milieu horiz
myGLCD.drawHLine(ex + x+z, ey + y, 28); // bas horiz
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche
myGLCD.setColor(0, 0, 0);

myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite
  
}else if (nbr == '3'){
myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawHLine(cx + x+z, cy + y, 28); // milieu horiz
myGLCD.drawHLine(ex + x+z, ey + y, 28); // bas horiz
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite
myGLCD.setColor(0, 0, 0);

myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche

}else if (nbr == '4'){

myGLCD.drawHLine(cx + x+z, cy + y, 28); // milieu horiz
myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite 
myGLCD.setColor(0, 0, 0);

myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawHLine(ex + x+z, ey + y, 28); // bas horiz
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche

}else if (nbr == '5'){
  myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y, 28); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 28); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
}else if (nbr == '6'){
  myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y, 28); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 28); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
}else if (nbr == '7'){
  myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawHLine(cx + x +z, cy + y, 28); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 28); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche

}else if (nbr == '8'){
  myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y, 28); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 28); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite
}else if (nbr == '9'){
myGLCD.drawHLine(ax + x +z , ay +y , 28); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y, 28); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 28); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 23); // vert haut gauche
myGLCD.drawVLine(bx + x+z, by + y, 23); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 23); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawVLine(cx + x+z, cy + y, 23); // vert bas gauche

}
myGLCD.setColor(0, 255, 0);
z = z + 32;
} // du for

myGLCD.setColor(0, 255, 0);

}


void printbigsevendigit(String nbre, int x , int y,byte R, byte G, byte B){
  
  // font 48 x 75
  /*
  a _____ b
    |   |
    |   |
  c _____ d
    |   |
    |   |       
  e_____ f

  */
int ax =2;
int ay =2;
int bx =46;
int by =2;
int cx =2;
int cy =37;
int dx =46;
int dy =37;
int ex =2;
int ey =73;
int fx =46;
int fy =73;
int z = 0 ;// decalage entre caractere
char nbr;
myGLCD.setColor(R, G, B);

for (int n=0; n < nbre.length() ; n++){ 
nbr = nbre.charAt(n)  ;
if ((nbr == '0') and n==0  ){ // 0 non significatif
myGLCD.setColor(0, 0, 0);
myGLCD.drawHLine(ax + x +z , ay +y, 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 44); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
//Gras
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
   


}else if (nbr == '0'){  
myGLCD.drawHLine(ax + x +z , ay +y , 44); //haut horiz
myGLCD.drawHLine(ex + x +z, ey + y, 44); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
//Gras
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz

myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawHLine(cx + x +z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
}else if (nbr == '1'){
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
//Gras
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawHLine(ax + x +z , ay +y , 44); //haut horiz
myGLCD.drawHLine(cx + x+z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(ex + x+z, ey + y, 44); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche
  
}else if (nbr == '2'){
myGLCD.drawHLine(ax + x +z , ay +y , 44); //haut horiz
myGLCD.drawHLine(cx + x+z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(ex + x+z, ey + y, 44); // bas horiz
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
//Gras
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche
myGLCD.setColor(0, 0, 0);

myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite

  
}else if (nbr == '3'){
myGLCD.drawHLine(ax + x +z , ay +y , 44); //haut horiz
myGLCD.drawHLine(cx + x+z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(ex + x+z, ey + y, 44); // bas horiz
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
//Gras
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
myGLCD.setColor(0, 0, 0);

myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche

}else if (nbr == '4'){

myGLCD.drawHLine(cx + x+z, cy + y, 44); // milieu horiz
myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite 
//Gras
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
myGLCD.setColor(0, 0, 0);

myGLCD.drawHLine(ax + x +z , ay +y , 44); //haut horiz
myGLCD.drawHLine(ex + x+z, ey + y, 44); // bas horiz
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche


}else if (nbr == '5'){
  myGLCD.drawHLine(ax + x +z , ay +y , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 44); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
//Gras
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche


}else if (nbr == '6'){
  myGLCD.drawHLine(ax + x +z , ay +y , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 44); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
//Gras
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite


}else if (nbr == '7'){
  myGLCD.drawHLine(ax + x +z , ay +y , 44); //haut horiz
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
//Gras
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawHLine(cx + x +z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 44); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche


}else if (nbr == '8'){
  myGLCD.drawHLine(ax + x +z , ay +y, 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 44); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
//Gras
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
}else if (nbr == '9'){
myGLCD.drawHLine(ax + x +z , ay +y , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y, 44); // bas horiz
myGLCD.drawVLine(ax + x+z, ay + y, 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z, by + y, 35); // vert haut droite
myGLCD.drawVLine(dx + x+z, dy + y, 35); // vert bas droite
//Gras
 myGLCD.drawHLine(ax + x +z , ay +y +1 , 44); //haut horiz
myGLCD.drawHLine(cx + x +z, cy + y +1, 44); // milieu horiz
myGLCD.drawHLine(ex + x +z, ey + y -1, 44); // bas horiz
myGLCD.drawVLine(ax + x+z +1, ay + y , 35); // vert haut gauche
myGLCD.drawVLine(bx + x+z -1, by + y , 35); // vert haut droite
myGLCD.drawVLine(dx + x+z -1, dy + y , 35); // vert bas droite
myGLCD.setColor(0, 0, 0);
myGLCD.drawVLine(cx + x+z, cy + y, 35); // vert bas gauche
myGLCD.drawVLine(cx + x+z +1, cy + y , 35); // vert bas gauche
}
myGLCD.setColor(R, G, B);
z = z + 48;
} // du for

myGLCD.setColor(R, G, B);

}  

//----------------------
// creation bouton
//----------------------
void drawbutton(int x1, int y1, int x2, int y2, String titre,int x3,int y3, boolean touch)
{
  if (touch){
    myGLCD.setColor(255, 0, 0);
    myGLCD.setBackColor (255, 0, 0);
  }else{
    myGLCD.setColor(0, 0, 255);
    myGLCD.setBackColor (0, 0, 255);
  }
  myGLCD.fillRoundRect (x1, y1, x2, y2);
  
  myGLCD.setColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  LcdPrint(x3 ,y3, titre);
}

void ecrireverticale(int posx, int posy,String a,String b,String c,String d,String e,int R, int G, int B){

  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(R, G, B);
  myGLCD.setFont(SmallFont);
  LcdPrint(posx ,posy, a);
  LcdPrint(posx ,posy + 10 , b);
  LcdPrint(posx ,posy + 20 , c);
  LcdPrint(posx ,posy + 30 , d);
  LcdPrint(posx ,posy + 40 , e);
 }



