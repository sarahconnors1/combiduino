//-------------------------------------------------------------------------------
//                ROUTINE X-TAU
//-------------------------------------------------------------------------------
/*
 * Byte ECU.Tau_evap           pourcentage d'évaporation du film par seconde
 * unsigned int  Tau_dt    pourcentage d evaporation sur un cycle * 100 pour stockage en entier  -> Tau_evap / 2*(60 * RPM)       
 * Byte X_adher            pourcentage de carburant qui colle a la paroi a chaque injection
 *  int qte_paroi  Quantite actuellement sur la paroi 
 *  = quantite precedente sur la paroi
 *  + nouvelle quantite adherente sur la paroi (PW_precedent * X_adher/100) 
 *  - quantité evaporée de la paroi sur la periode (qte_paroi_previous * 100/Tau_dt)
 * 
 *  int qte_paroi_previuos  Quantite precedente sur la paroi
 * 
 * unsigned int PW_actuel corrigé par le xtau
 *  =( PW de la MAP                          ( (injection_time_us)
 * - quantit evaporé disponible pour le cylindre    (qte_paroi * 100/Tau_dt) )
 * ) * facteur de non adherence                      * 100/ (100- X_adher)
 * + acceleration de la pompe de reprise


*/

void Calcul_PW_actuel_corrige_XTAU(){
// routine principale qui calcul le Pulse width corrigé 
// basé sur le PW demandé par la carto (injection_time_us)  sans tenir compte de l'enrichissement de la pompe de reprise
// A la fin du calcul PW_actuel tient compte aussi de l'enrichissement ( PW corrige + PW accel)
  Calcul_variable_XTAU(); 

  if (BIT_CHECK(ECU.running_option,BIT_XTAU_USED) // xtau utilise
  and BIT_CHECK(ECU.running_mode, BIT_ENGINE_RUN) // moteur tournant
  and  !BIT_CHECK(ECU.running_mode, BIT_ENGINE_CRANK) // pas en cours de demarrage
  and (!BIT_CHECK(ECU.running_mode,BIT_ENGINE_IDLE ) ) ){ // pas au ralenti
    
       ECU.PW_actuel = ( ( ECU.injection_time_us - (  (ECU.qte_paroi/Tau_dt) * float(100) ) ) 
      / (100 - ECU.X_adher) 
      )* 100;
    
      if (ECU.PW_actuel < 0){ECU.PW_actuel = 0;}  // sanity check
    
      if (    ECU.PW_actuel * Xtau_max_decel  < ECU.injection_time_us  ){
        ECU.PW_actuel = ECU.injection_time_us /  Xtau_max_decel ;
      }
  
  }else{  // pas de Xtau on renvoie la valeur de la cartographie
    ECU.PW_actuel = ECU.injection_time_us;
  }

  ECU.PW_actuel += ECU. PW_accel_actuel_us;  // On rajoute la pompe de reprise

}
void Calcul_qte_paroi(){
// calcul la quantité restante sur la paroi (unite = PW pulse width)
// cette routine doit être appelé à chaque cycle aka injection du cylindre 1
// elle tient compte de l'ajout sur la paroi de la derniere injection ( injection carto + pompe de reprise) 
if (recalcul_paroi == true){
ECU.qte_paroi = qte_paroi_previous
 +  ( float(PW_previous/float(100) )  * ECU.X_adher )  
 - ( (qte_paroi_previous /Tau_dt) * float(100) );
 
  if (ECU.qte_paroi < 0){
    ECU.qte_paroi=0;
  }
  qte_paroi_previous = ECU.qte_paroi;
  PW_previous = ECU.PW_actuel; 
  recalcul_paroi = false;
}

}



void Calcul_variable_XTAU(){
/*
// objectif Calcul de :
- ECU.X_adher  TAUX ADHERENCE
- ECU.Tau_evap TAUX EVAPORATION
- Tau_dt Temps pour  1 cycle moteur 2 tour


Facteur adherence
 BAWC   taux d'adherence en % en fonction de la depression (augmente avec KPA)
 AWN facteur de correction d adherence en fonction RPM (100 = pas de correction) (Baisse avec RPM)
 AWW facteur de correction d adherence en fonction TEMPERATURE (100 = pas de correction) (baisse avec la temperature)

Facteur evaporation
 BSOC  taux evaporation en % en fonction de la depression (augmente avec KPA)
 SON facteur de correction evaporation en fonction RPM (100 = pas de correction) (monte avec les RPM)
 SOW facteur de correction evaporation en fonction RPM (100 = pas de correction) (augmente avec la temperature )


*/ 

/* correctif pour passer en float et fournir les RPM et KPA reel
byte BAWC = 0;
byte BSOC = 0;
byte AWN = 100;
byte SON = 100;
byte AWW = 100;
byte SOW = 100;

// interpolation lineaire basee sur la depression actuelle
BAWC = map(pressure_axis[ECU.point_KPA], pressure_axis[nombre_point_DEP-1] ,pressure_axis[0], BAWC_min, BAWC_max);
BSOC = map(pressure_axis[ECU.point_KPA], pressure_axis[nombre_point_DEP-1] ,pressure_axis[0], BSOC_min, BSOC_max);

// facteur de corection base sur les RPM
AWN = map(rpm_axis[ECU.point_RPM] ,rpm_axis[0],rpm_axis[nombre_point_RPM - 1], AWN_min, AWN_max);
SON = map(rpm_axis[ECU.point_RPM] ,rpm_axis[0],rpm_axis[nombre_point_RPM - 1], SON_min, SON_max);
*/

byte BAWC = 0;
byte BSOC = 0;
byte AWN = 100;
byte SON = 100;
byte AWW = 100;
byte SOW = 100;

// interpolation lineaire basee sur la depression actuelle
BAWC = map(ECU.map_pressure_kpa , pressure_axis[nombre_point_DEP-1] ,pressure_axis[0], BAWC_min, BAWC_max);
BSOC = map(ECU.map_pressure_kpa, pressure_axis[nombre_point_DEP-1] ,pressure_axis[0], BSOC_min, BSOC_max);

// facteur de corection base sur les RPM
AWN = map(ECU.engine_rpm_average ,rpm_axis[0],rpm_axis[nombre_point_RPM - 1], AWN_min, AWN_max);
SON = map(ECU.engine_rpm_average ,rpm_axis[0],rpm_axis[nombre_point_RPM - 1], SON_min, SON_max);

// facteur de correction basé sur la temperature CLT
#if CLT_USED == 1
if (ECU.CLT <  lowtemp){
 AWW = AWW_min; SOW = SOW_min; 
}else if (ECU.CLT > hightemp){
 AWW = AWW_max;SOW = SOW_max; 
}else{
AWW = map(ECU.CLT ,lowtemp ,hightemp , AWW_min, AWW_max);
SOW = map(ECU.CLT ,lowtemp ,hightemp , SOW_min, SOW_max);
}
#endif
  
// calcul du taux d adherence final
ECU.X_adher = BAWC * (AWN / float(100) ) * (AWW / float(100) );
// calcul du taux evaporation final
ECU.Tau_evap = BSOC * (SON / float(100) ) * (SOW / float(100) );

// calcul le temps nécessaire entre 2 injection du meme cylindre (sur 1 cycle = 2 tours
  if ( ECU.engine_rpm_average> 0){
  Tau_dt =  ECU.engine_rpm_average /120;
   Tau_dt = ECU.Tau_evap  * Tau_dt   ;
  }else{
    Tau_dt = 10; // valeur arbitraire pour eviter les div par 0
  }  
}


