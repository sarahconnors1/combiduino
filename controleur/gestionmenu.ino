
 //----------------------------------------------------------
 //             ROUTINE
 //-----------------------------------------------------------

void lectureecran(){
 x = 0;
 y = 0;
 
  if (myTouch.dataAvailable() == true){
  // while (myTouch.dataAvailable()){
    myTouch.read();
    x=myTouch.getX();
    y=myTouch.getY();
    screen_pressed = true;
    debug(" position" + String(x) + "," + String(y) );

    // check bouton hit
    if (ZonePressed(x, y , 0, 165, 70, 230)) {
      if (event_requested == false){
      event_requested = true;
      drawbutton(0, 165, 70, 230, "HIT",10,190, true); 
      debug(" bouton Hit presse");
      }
    }
    // check bouton Principal
    else if (ZonePressed(x, y , 80, 165, 150, 230)) {
      debug(" bouton PRI presse");
      if (menu_actuel != menu_principal){
        menu_actuel = menu_principal;
        rebuild_menu = true;
        refresh_menu = false;
      } 
    }
    // check bouton ecran 1
    else if (ZonePressed(x, y ,160, 165, 230, 230)) {
      debug(" bouton EC1 presse");
      if (menu_actuel != menu_1){
        menu_actuel = menu_1;
        rebuild_menu = true;
        refresh_menu = false;
      }
    }
    // check bouton ecran 2
    else if (ZonePressed(x, y ,240, 165, 310, 230)) {
      debug(" bouton EC1 presse");
      if (menu_actuel != menu_2){
        menu_actuel = menu_2;
        rebuild_menu = true;
        refresh_menu = false;
      }
    }
  // Gestion des boutons du 2 eme écrans
  if (menu_actuel == menu_2){
    // Bouton fuel
    if (ZonePressed(x, y ,0, 100, 60, 160)) {debug(" bouton -2 presse");MCEchange_requested=true; MCE_requested=MCE1;}
    else if (ZonePressed(x, y ,70, 100, 130, 160)) {debug(" bouton -1 presse");MCEchange_requested=true;MCE_requested=MCE2;}
    else if (ZonePressed(x, y ,140, 100, 200, 160)) {debug(" bouton 0 presse");MCEchange_requested=true;MCE_requested=MCE3;}
    else if (ZonePressed(x, y ,210, 100, 270, 160)) {debug(" bouton 1 presse");MCEchange_requested=true;MCE_requested=MCE4;}
    else if (ZonePressed(x, y ,280, 100, 320, 160)) {debug(" bouton 2 presse");MCEchange_requested=true; MCE_requested=MCE5;}

    // Bouton allumage
    if      (ZonePressed(x, y ,0, 20, 60, 80))    {debug(" carto 1 presse");carto_change=true;carto_demande=1;}
    else if (ZonePressed(x, y ,70, 20, 130, 80))  {debug(" carto 2 presse");carto_change=true;carto_demande=2;}
    else if (ZonePressed(x, y ,140, 20, 200, 80)) {debug(" carto 3 presse");carto_change=true;carto_demande=3;}
    else if (ZonePressed(x, y ,210, 20, 270, 80)) {debug(" carto 4 presse");carto_change=true;carto_demande=4;}
    else if (ZonePressed(x, y ,280, 20, 320, 80)) {debug(" carto 5 presse");carto_change=true;carto_demande=5;}
    
  
  }  
  
  }

 
}



//------------------------------------
// Gestion des changements de menus
//------------------------------------
void gestion_affichage_des_menus(){
  
// gestion d'afficage nouvel ecran
  if (rebuild_menu == true){ // gestion de l'affichage complet du menu
     RAZ_affichage(); // on efface tout
   if (menu_actuel == menu_principal){affichage_menu_principal();}
   if (menu_actuel == menu_1){affichage_menu_1() ;} 
   if (menu_actuel == menu_2){affichage_menu_2() ;} 
   rebuild_menu = false;
  }

// Gestion du refresh de l'ecran actuel  
  if (refresh_menu == true){ // gestion de l'affichage des valeurs uniquement du menu
    if (menu_actuel == menu_principal){refresh_menu_principal();}
    if (menu_actuel == menu_1){refresh_menu_1() ;} 
   if (menu_actuel == menu_2){refresh_menu_2() ;} 
    refresh_menu = false;
  }
  
}
//---------------------------------------
// affichage des titre du menu principal
//---------------------------------------
void affichage_menu_principal(){
//  debug("aff menu princ"); 
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(BigFont);
// Ligne 1
  myGLCD.drawRect(0, 0, 319, 158);
  LcdPrint(205,60, "RPM");
  LcdPrint(155,135, "Kpa");
printbigsevendigit(RightNum("0000",4), 7 ,  3, 255, 255, 255);

// nbr de log
  LcdPrint(210,10, "Log");
// time 
  myGLCD.setFont(SmallFont);
  LcdPrint(210,30, "Time");

// time 
  LcdPrint(210,40, "Err");

  
// Carte SD OK
myGLCD.setFont(BigFont);
  if (SDinitfailed == false){
    LcdPrint(220,110, "REC");
    LcdPrint(220,135, (String) num_file);
  }else{
    LcdPrint(220,110, "NOK");
  }


 
creationbouton();
refresh_menu = true;
  }
//---------------------------------------
// affichage des valeurs du menu principal
//---------------------------------------
void refresh_menu_principal(){ 
unsigned long temp=micros(); 
count_menu++;
if (count_menu >= count_menu_max){count_menu = 0;}

if (count_menu == 0){
  // ******************* refresh lent **********************************
  // ***** Les Log recu
    myGLCD.setColor(255, 255, 255);
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setFont(SmallFont);
    LcdPrint(260,30, String(time_loop));
    LcdPrint(260,40, String(erreur_reception));

    myGLCD.setFont(BigFont);
    LcdPrint(260,10, String(nbr_log));
  }else{
    // *************************refresh rapide********************************  
    myGLCD.setColor(0, 255, 0);
    myGLCD.setBackColor(0, 0, 0);
    printbigsevendigit(RightNum(String(RPM),3), 7 ,  3, 255, 255, 255);
    printbigsevendigit(RightNum(String(KPA),2), 55 ,  80, 255, 255, 255);
    myGLCD.setColor(255, 255, 255);
    myGLCD.setBackColor(0, 0, 0);
    myGLCD.setFont(SmallFont);
    //  debug("****refersh rapide " + String((long)( micros() -temp)) ); 
  }
}

//---------------------------------------
// affichage des titre du menu 1
//---------------------------------------
void affichage_menu_1(){ 
//  debug("aff menu 1");
  creationbouton();
   myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(BigFont);
// colonne 1
  myGLCD.drawRect(0, 0, 319, 158);
  LcdPrint(10,10, "IAE");
  LcdPrint(10,30, "CLT");
  LcdPrint(10,50, "VLT");
  LcdPrint(10,70, "EGO");
  LcdPrint(10,90, "TPS");
  LcdPrint(10,110, "PID");
  LcdPrint(10,130, "IOT"); // temps d'ouverture injecteur

// colonne 2
ENGINE_IDLE_OLD = 0;
ENGINE_WARMUP_OLD = 0;
ENGINE_DCC_OLD = 0;
BIT_EGO_ACTIVE_OLD = 0;
drawbutton(120, 0, 190, 30, "IDL",130,10, false);
drawbutton(120, 32, 190, 60, "WRM",130,40, false);
drawbutton(120, 62, 190, 90, "DCC",130,70, false);
drawbutton(120, 92, 190, 120, "EGO",130,100, false);
 
}

//---------------------------------------
// affichage des valeurs du menu 1
//---------------------------------------
void refresh_menu_1(){ 
//  debug("refresh menu 1");
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(BigFont);

// colonne 1
  LcdPrint(70,10, (String) IAE);
  LcdPrint(80,30, (String) CLT);
  LcdPrint(70,50,(String) VLT);
  LcdPrint(70,70, (String) CorL);
  LcdPrint(70,90, (String) TPS);
  LcdPrint(70,110, (String)(int) IDL);
  LcdPrint(70,130, (String) IOT);

// gestion du mode idle
if  (bitRead(ECUdata.value.running_mode,BIT_ENGINE_IDLE ) != ENGINE_IDLE_OLD) {
  ENGINE_IDLE_OLD = bitRead(ECUdata.value.running_mode,BIT_ENGINE_IDLE );
  if  (bitRead(ECUdata.value.running_mode,BIT_ENGINE_IDLE ) == 1) {
     drawbutton(120, 2, 190, 30, "IDL",130,10, true);
  }else{
     drawbutton(120, 2, 190, 30, "IDL",130,10, false);
  }
}
// gestion du mode WARM UP
if  (bitRead(ECUdata.value.running_mode,BIT_ENGINE_WARMUP ) != ENGINE_WARMUP_OLD) {
  ENGINE_WARMUP_OLD = bitRead(ECUdata.value.running_mode,BIT_ENGINE_WARMUP );
  if  (bitRead(ECUdata.value.running_mode,BIT_ENGINE_WARMUP ) == 1) {
     drawbutton(120, 32, 190, 60, "WRM",130,40, true);
  }else{
     drawbutton(120, 32, 190, 60, "WRM",130,40, false);
  }
}
// gestion du mode DECEL
if  (bitRead(ECUdata.value.running_mode,BIT_ENGINE_DCC ) != ENGINE_DCC_OLD) {
  ENGINE_DCC_OLD = bitRead(ECUdata.value.running_mode,BIT_ENGINE_DCC );
  if  (bitRead(ECUdata.value.running_mode,BIT_ENGINE_DCC ) == 1) {
     drawbutton(120, 62, 190, 90, "DCC",130,70, true);
  }else{
     drawbutton(120, 62, 190, 90, "DCC",130,70, false);
  }
} 
// gestion du mode DECEL
if  (bitRead(ECUdata.value.running_mode,BIT_EGO_ACTIVE ) != BIT_EGO_ACTIVE_OLD) {
  BIT_EGO_ACTIVE_OLD = bitRead(ECUdata.value.running_mode,BIT_EGO_ACTIVE );
  if  (bitRead(ECUdata.value.running_mode,BIT_EGO_ACTIVE ) == 1) {
     drawbutton(120, 92, 190, 120, "EGO",130,100, true);
  }else{
     drawbutton(120, 92, 190, 120, "EGO",130,100, false);
  }
} 

}

void affichage_menu_2(){ 
  creationbouton();

  // Pour l'injection
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(BigFont);
  LcdPrint(70,80, "Carto injection ");
  drawbutton(0, 100, 60, 160, "-2",10,120, false); 
  drawbutton(70, 100, 130, 160, "-1",80,120, false); 
  drawbutton(140, 100, 200, 160, "0",150,120, false);
  drawbutton(210, 100, 270, 160, "1",220,120, false);  
  drawbutton(280, 100, 320, 160, "2",290,120, false);  
  MCE_OLD=0;
  
// Pour l' allumage
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(BigFont);
  LcdPrint(70,0, "Carto allumage ");
  drawbutton(0, 20, 60, 80, "1",10,40, false); 
  drawbutton(70, 20, 130, 80, "2",80,40, false); 
  drawbutton(140, 20, 200, 80, "3",150,40, false);
  drawbutton(210, 20, 270, 80, "4",220,40, false);  
  drawbutton(280, 20, 320, 80, "5",290,40, false);  
  carto_OLD = 99;
  
}

void refresh_menu_2(){

// Pour le Fuel
 if (ECUdata.value.MCE_actuel != MCE_OLD){
  // Eteindre le bouton
  if      (MCE_OLD == MCE1){drawbutton(0, 100, 60, 160, "-2",10,120, false);   }
  else if (MCE_OLD == MCE2){drawbutton(70, 100, 130, 160, "-1",80,120, false); }
  else if (MCE_OLD == MCE3){drawbutton(140, 100, 200, 160, "0",150,120, false);}
  else if (MCE_OLD == MCE4){drawbutton(210, 100, 270, 160, "1",220,120, false);}  
  else if (MCE_OLD == MCE5){drawbutton(280, 100, 320, 160, "2",290,120, false);}  

  MCE_OLD=ECUdata.value.MCE_actuel;

  if      (ECUdata.value.MCE_actuel == MCE1){drawbutton(0, 100, 60, 160, "-2",10,120, true); }
  else if (ECUdata.value.MCE_actuel == MCE2){drawbutton(70, 100, 130, 160, "-1",80,120, true);} 
  else if (ECUdata.value.MCE_actuel == MCE3){drawbutton(140, 100, 200, 160, "0",150,120, true);}
  else if (ECUdata.value.MCE_actuel == MCE4){drawbutton(210, 100, 270, 160, "1",220,120, true);}  
  else if (ECUdata.value.MCE_actuel == MCE5){drawbutton(280, 100, 320, 160, "2",290,120, true);}  
  
 }

// Pour la carto allumage
 
 if (ECUdata.value.carto_actuel != carto_OLD){
  // Eteindre le bouton
  if      (carto_OLD == 1){ drawbutton(0, 20, 60, 80, "1",10,40, false); }
  else if (carto_OLD == 2){drawbutton(70, 20, 130, 80, "2",80,40, false);}  
  else if (carto_OLD == 3){drawbutton(140, 20, 200, 80, "3",150,40, false);}
  else if (carto_OLD == 4){drawbutton(210, 20, 270, 80, "4",220,40, false); } 
  else if (carto_OLD == 5){drawbutton(280, 20, 320, 80, "5",290,40, false);  }
  
  carto_OLD=ECUdata.value.carto_actuel;
  // allumer le bouton
   if      (ECUdata.value.MCE_actuel == 1){drawbutton(0, 20, 60, 80, "1",10,40, true); }
   else if (ECUdata.value.MCE_actuel == 2){drawbutton(70, 20, 130, 80, "2",80,40, true);}  
   else if (ECUdata.value.MCE_actuel == 3){drawbutton(140, 20, 200, 80, "3",150,40, true);}
   else if (ECUdata.value.MCE_actuel == 4){drawbutton(210, 20, 270, 80, "4",220,40, true); } 
   else if (ECUdata.value.MCE_actuel == 5){drawbutton(280, 20, 320, 80, "5",290,40, true);  }
   
 }



}
void creationbouton(){
  // Hit
drawbutton(0, 165, 70, 230, "HIT",10,190, false); 
drawbutton(80, 165, 150, 230, "PRI",90,190, false); 
drawbutton(160, 165, 230, 230, "EC1",170,190, false);
drawbutton(240, 165, 310, 230, "EC2",250,190, false);   
}


boolean ZonePressed (int x, int y, int xhaut, int yhaut, int xbas, int ybas){
  if ( (x>xhaut  && x<xbas) && (y>yhaut  && y<ybas) ){
    return true;
  }else{
    return false;
  }
 
}
//---------------------------------------
// RAZ des 2 lignes d' affichage 
//---------------------------------------
void RAZ_affichage(){ 
   debug("RAZ affichage");
myGLCD.clrScr();

}


