
 //----------------------------------------------------------
 //             ROUTINE
 //-----------------------------------------------------------

//----------------------------------
// gestion des actions utilisateurs
//----------------------------------
void gestion_des_actions(){
  // debug(String(menu_actuel) + "|" + rebuild_menu + "|" + refresh_menu);
 lcd_key = read_LCD_buttons();  // lecture des boutons
 
 // action du menu principal
 if (menu_actuel == menu_principal) {
// gestion du changement de menu
  if (lcd_key == btnMENU) {
    debug("principal bouton menu");
     menu_actuel++;
     if (menu_actuel > nombre_de_menu ){menu_actuel = menu_principal;}
     rebuild_menu = true;
     refresh_menu = true;
     gestion_affichage_des_menus();
     return;
     }
 }
 
  // action du menu 1
 if (menu_actuel == menu_1) {
    // gestion du changement de menu
  if (lcd_key == btnMENU) {
    debug("menu1 bouton menu");
     menu_actuel++;
     if (menu_actuel > nombre_de_menu ){menu_actuel = menu_principal;}
     rebuild_menu = true;
     refresh_menu = true;
     gestion_affichage_des_menus();
     return;
     }
   //gestion du sous menu CARTO <==> MS
   if (lcd_key == btnLEFT) {
     debug("LEFT");
     sous_menu_1++;
     if (sous_menu_1 > nombre_de_sous_menu_1 ){sous_menu_1 = 1;}
     refresh_menu = true;
     gestion_affichage_des_menus();
   }  
    // gestion de la carto
   if (sous_menu_1 == sous_menu_1_carto){
     if (lcd_key == btnDOWN) {
       debug("CARTO DOWN");
       carto_demande++;
       if (carto_demande > carto_max ){carto_demande = 1;}
       refresh_menu = true;
       gestion_affichage_des_menus();
     }  
     if (lcd_key == btnUP) {
       debug("CARTO UP");
       carto_demande--;
       if (carto_demande < 1 ){carto_demande = carto_max;}
       refresh_menu = true;
       gestion_affichage_des_menus();
     }  
     if (lcd_key == btnRIGHT) { // validation du choix
      debug("CARTO RIGHT");
      carto_change = true;
      rebuild_menu = true;
      refresh_menu = true;
      envoiECU();
     }  
   }
      // gestion du multispark
   if (sous_menu_1 == sous_menu_1_MS){
     if ((lcd_key == btnDOWN) || (lcd_key== btnUP) ) {
       debug("MS DOWN UP");
       if (multispark_demande == true){multispark_demande = false;}else{multispark_demande = true;}
       refresh_menu = true;
       gestion_affichage_des_menus();
     }  
     if (lcd_key == btnRIGHT) { // validation du choix
      debug("MS RIGHT");
      multispark_change = true;
      rebuild_menu = true;
      refresh_menu = true;
      envoiECU();
     }  
   }
   
   
 } // de action menu 1
 
}


//------------------------------------
// Gestion des changements de menus
//------------------------------------
void gestion_affichage_des_menus(){
  if (rebuild_menu == true){ // gestion de l'affichage complet du menu
    RAZ_affichage(); // on efface tout
   if (menu_actuel == menu_principal){affichage_menu_principal();}
   if (menu_actuel == menu_1){affichage_menu_1() ;} 
   if (menu_actuel == menu_2){affichage_menu_2() ;} 
   rebuild_menu = false;
  }
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
//0123456789012345
//RPM Kpa AFR Deg
//1000 40 190 32 
void affichage_menu_principal(){ 
 LcdPrint(0,0, "RPM");
 LcdPrint(4,0, "Kpa");
 LcdPrint(8,0, "AFR");
 LcdPrint(12,0, "Deg");
 refresh_menu = true;
  }
//---------------------------------------
// affichage des valeurs du menu principal
//---------------------------------------
void refresh_menu_principal(){ 
 LcdPrint(0,1, Right(String(RPM),4) );
 LcdPrint(4,1, Right(String(KPA),3) );
 LcdPrint(8,1, Right(String(AFR), 3));
 LcdPrint(12,1, Right(String(avance), 2));
 
 if (multispark_actuel == true) {
 LcdPrint(14,1, "*");
 }else{
  LcdPrint(14,1, " ");
 }   
}

//---------------------------------------
// affichage des titre du menu 1
//---------------------------------------
void affichage_menu_1(){ 
  debug("aff menu 1");
   lcd.setCursor(0,0);
   lcd.print(" CARTO"); 
   lcd.setCursor(7,0);
   lcd.print(" MS"); 
   sous_menu_1 = sous_menu_1_carto ; // choix carto par défaut
   carto_demande = carto_actuel;
   multispark_demande = multispark_actuel;
 //  refresh_menu = true;
}

//---------------------------------------
// affichage des valeurs du menu 1
//---------------------------------------
void refresh_menu_1(){ 
  debug("refresh menu 1");
  lcd.setCursor(11,0);
  lcd.print(Serial.available());
  
 // gestion du * pour selection du menu
if (sous_menu_1 == sous_menu_1_carto){
  lcd.setCursor(0,0);
   lcd.print("*"); 
   lcd.setCursor(7,0);
   lcd.print(" "); 
} 
if (sous_menu_1 == sous_menu_1_MS){
  lcd.setCursor(0,0);
   lcd.print(" "); 
   lcd.setCursor(7,0);
   lcd.print("*"); 
} 
 // gestion des valeurs demandés
  lcd.setCursor(3,1);
  lcd.print(carto_demande); 
  
  if (multispark_demande == true){
    lcd.setCursor(8,1);
    lcd.print("ON "); 
  }else{
    lcd.setCursor(8,1);
    lcd.print("OFF"); 
  }
  
}

void affichage_menu_2(){ 
}

void refresh_menu_2(){
}
