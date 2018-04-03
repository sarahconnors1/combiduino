
void getknock() {
  #if KNOCK_USED == 1 
if ((stringComplete3 == true) and (knock_active == true) ){
// on check bin 20/21/22
String k1= getValue(inputString3,';', 20);
String k2= getValue(inputString3,';', 21);
String k3= getValue(inputString3,';', 22);

int vol1 = k1.toInt();
int vol2 = k2.toInt();
int vol3 = k3.toInt();


   count_knock++;
   total_knock = total_knock + vol1 + vol2 + vol3;
   // on fait la moyenne
   if (count_knock >= count_knock_max ){
     int binknock = decode_rpm(ECU.engine_rpm_average);
     if (knock_record == true){ // si on est en cours d'enregistrement du knock
       knockvalue[binknock] = knockvalue[binknock] + total_knock;
       knockcount[binknock] = knockcount[binknock] +  count_knock_max;
     }else{ // sinon on calcul le knock 
       knock_moyen = (total_knock / count_knock_max);  
       delta_knock = knock_moyen -  knockmoyen[binknock];
       var1 = knockmoyen[binknock] ;
       var2 = knock_moyen;
     }
     
     total_knock = 0;
     count_knock = 0;
   
   }
 stringComplete3 = false;
 inputString3 = "";
}
#endif
}
//---------------------------------------------
// gestion enregistrement du knock
//--------------------------------------------- 
void recordknock(){
#if KNOCK_USED == 1 
 knock_record = true;
  for (int n= 0; n < nombre_point_RPM; n++) {
   knockvalue[n] = 0;
   knockcount[n] = 0;
  } 
#endif  
} 

// SAV des moyennes en EEPROM
void saveknock(){
#if KNOCK_USED == 1 
  knock_record = false;
  int value = 0;
  for (int n= 0; n < nombre_point_RPM; n++) {
    if (knockcount[n] > 0){
       value = knockvalue[n] / knockcount[n] ;
    }else{
       value = 0;
    }
  EEPROM.write(eprom_adresseknock + n,  value );
  }
  initknock();
#endif
}

// charge les valeurs moyenne au demarrage
void initknock(){
#if KNOCK_USED == 1 
  for (int n= 0; n < nombre_point_RPM; n++) {
    knockmoyen[n] = EEPROM.read(eprom_adresseknock + n);
  }
#endif
}

void RAZknock(){
#if KNOCK_USED == 1 
  for (int n= 0; n < nombre_point_RPM; n++) {
    EEPROM.write(eprom_adresseknock + n,  0 );
  }
#endif
}

   // si on divise par 32
    // 16000000 Hz / 32 = 500 KHz, inside the desired 50-200 KHz range.
   // Each conversion in AVR takes 13 ADC clocks so 500 KHz /13 = 38500 Hz.
   
   // pour prendre 128 mesures il faut donc 1000000 (Âµs ) X 128 / 38000 -> 3300 Âµs 3,3ms
 

// freq = 38000  N = 128
//fht_lin_out8[0] = first bin magnitude (0hz -> 38000/128) = 0 -> 300
//fht_lin_out8[1] = second bin magnitude (38000/N -> 2 X 38000/128) = 300 -> 600
//fht_lin_out8[1] = second bin magnitude (38000/N -> 2 X 38000/128) = 300 -> 600

//fht_lin_out8[19] = second bin magnitude (19 *38000/N -> 20 X 38000/128) = 300 -> 600

// 6000 = 38000 x bin / 128 -> 6000 X 128 / 38000 = bin -> Bin 19 / 20 / 21 a verifier
/*
Bin     De HZ  a HZ
0		
1	296	593
2	593	890
3	890	1187
4	1187	1484
5	1484	1781
6	1781	2078
7	2078	2375
8	2375	2671
9	2671	2968
10	2968	3265
11	3265	3562
12	3562	3859
13	3859	4156
14	4156	4453
15	4453	4750
16	4750	5046
17	5046	5343
18	5343	5640
19	5640	5937
20	5937	6234
21	6234	6531
22	6531	6828
23	6828	7125
24	7125	7421
25	7421	7718
26	7718	8015
*/








