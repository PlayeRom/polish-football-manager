#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include <process.h>
struct
{
 char newss[500];
 int num;
} meczs;
struct
{
 char imie_m[15],nazw_m[20],nick[20];
 int czy_nick,kolejka,jestM;
 int klub,kasa;//numer od 1 do 16 oznaczaj•cy klub
 int d,m,r,tydzien;//dzien,miesiac,rok
 int trening[28],treBOPN,inst[6],usta,asystent,lostmecz,gol1,gol2,zadyma;
 //co trenuj•,+/-BOPN,instrukcje,ustawienie
 float umie[5],finanse[14],finanse2[14],ileTrans,dane2_m[2];
 int rywal[4],rinst[6],rywalTN,ilekontrol,nie;//dla rywala
 int gol[480],golK[10],dane_m[12],sezon,mecz;//wiosna,gole dla kontrolnych
} Klub;
struct
{
 int numer,d,m,r;//nr kolejki dni‰ miesi•c i rok
 int nr[16];//numerowe oznaczenie druæyn dla àatwej identyfikacji
 char kol1[16][19];//16 druæyn po 19 znak¢w
 //char kol2[16][11];
} kolej;//kolejka
struct
{
 int num;//numer klubu
 int dane[9];
} tabela;
/*struct
{
 char newss[500];
 int num;
} news;  */
struct
{
 char imie[4],nazwisko[20];
 int dane[23];
 float waga[4];
} gracz;
//----------------FUNKCJE-----------
//void info();
//char MenuGlowne(int wiadomosc[5]);
void sklad(int usta,int tryb,int kto);//,int mecz
//void zamiana();
void przepisz();
void instrukcje(int a,int b,int c,int d,int e,int f,int belka);
void takty(int usta,int kto);
//void wykres(int usta,int kto);
int usalenie(int pilka,int PnaP,int OnaA,int AnaO,int sumaB,int sumaB2,int gdzie);
//void mecz();
#define UP 72
#define LEFT 75
#define RIGHT 77
#define DOWN 80
void main()
{
 FILE *f,*f2,*f3;
 char *krajMS[16]={"Amica Wronki","Garbarnia Szcz.","GKS Katowice",
 "G¢rnik Zabrze","Groclin Dyskobolia","KSZO Ostrowiec","Lech Pozna‰",
 "Legia Warszawa","Odra Wodzisàaw","Polonia Warszawa","Pogo‰ Szczecin",
 "Ruch Chorz¢w","Widzew ù¢d´","Wisàa Krak¢w","Wisàa Pàock","Zagà©bie Lubin"};
 char *tak[15]={"4-4-2","4-4-2 Obrona","4-4-2 Atak","4-4-2 Diamond",
 "3-4-3","3-5-2","3-5-2 Obrona","3-5-2 Atak","4-2-4","4-3-3","4-5-1","5-3-2",
 "5-3-2 Obrona","5-3-2 Atak","Bà•d"};
 char *speed[5]={"B.szybko","Szybko","órednio","Wolno","B.wolno"};
 char end='s',menu,pp_usta,pp_usta2,tn,zastep[16][20]/*,zastep2[2][20]*/,rzastep[16][20],*dlawiado[20];
 char *dlagol1[10],*dlagol2[10],menu2;
 int mingol1[10],mingol2[10],minuta2=0,juzzmienil[3]={0,0,0},taktyka2=0;//,str12[16]
 int koniec=0,gol1=0,gol2=0,pilka=0,ktoZacz=0,minuta=0,start=0,taktyka=1,k,czas=2;
 int sumaB=0,sumaO=0,sumaP=0,sumaN=0,sumaB2=0,sumaO2=0,sumaP2=0,sumaN2=0,dlazmiana=0;
 int sumaBx=0,sumaOx=0,sumaPx=0,sumaNx=0,OnaA,PnaP,AnaO,gdzie=1,belka=1;
 int wiado[10],ktop[10],i=0,los=0,co=0,x1=0,x2=0,x3=0,czyzmiana=1,pam,pos1=0,pos2=0,pos=0;
 int q=0,str[8]={0,0,0,0,0,0,0,0},str2[8]={0,0,0,0,0,0,0,0},dystans=0,zmiana=0;//pamzmiana[6];//zastrzeæone
 int v=0,pkt[16],pkt2[16],golr[16],gol[16],blokada[16],licz,kol[40],numer1=0,walkower=0;
 float h=0,strefa[6]={0,0,0,0,0,0},premia=0;
 struct  time t;
 randomize();
 f=fopen("Gra_mana.cfg","rb");
 while(fread(&gracz,sizeof(gracz),1,f)==1) walkower++;
 fclose(f);
 for(k=0;k<10;k++)
  {
   mingol1[k]=0;mingol2[k]=0;
  }
 f=fopen("Klub.cfg","rb");
 fread(&Klub,sizeof(Klub),1,f);
 fclose(f);
 if (Klub.jestM==1&&Klub.nie==0)
 {
 f=fopen("Lostmecz.cfg","w");
 fclose(f);
 f=fopen("wymiana.cfg","wb");
 fclose(f);
 f=fopen("Gra_mana.cfg","rb");
 while(fread(&gracz,sizeof(gracz),1,f)==1)
  {
   if (gracz.dane[0]<12)//zwi©kszam morale jedenastce za 1 skàad
    {
     gracz.dane[8]+=3;
     if (gracz.dane[8]>5)
      {
       gracz.dane[7]++;gracz.dane[8]=0;
       if (gracz.dane[7]>3) gracz.dane[7]=3;
      }}
   if (gracz.dane[0]>16)
    {
     gracz.dane[8]-=2;
     if (gracz.dane[8]<-5)
      {
       gracz.dane[7]--;gracz.dane[8]=0;
       if (gracz.dane[7]<-3) gracz.dane[7]=-3;
      }}
   gracz.dane[20]=0;gracz.dane[21]=0;
   f2=fopen("wymiana.cfg","ab");
   fwrite(&gracz,sizeof(gracz),1,f2);
   fclose(f2);
  }
 fclose(f);
 przepisz();
 f=fopen("wymiana.cfg","wb");//zerowanie
 fclose(f);
 f=fopen("Rywal.cfg","rb");
 while(fread(&gracz,sizeof(gracz),1,f)==1)
  {
   gracz.dane[20]=0;gracz.dane[21]=0;gracz.dane[12]=0;
   f2=fopen("wymiana.cfg","ab");
   fwrite(&gracz,sizeof(gracz),1,f2);
   fclose(f2);
  }
 fclose(f);
 f=fopen("Rywal.cfg","wb");
 fclose(f);//szyszczenie starej
 f=fopen("Wymiana.cfg","rb");
 while(fread(&gracz,sizeof(gracz),1,f)==1)
  {
   f2=fopen("Rywal.cfg","ab");
   fwrite(&gracz,sizeof(gracz),1,f2);
   fclose(f2);
  }
 fclose(f);
 do
 {
  //-------przypisanie skàadu *******
  if (czyzmiana==1)
   {
    czyzmiana=0; k=0;
    while(k!=16)
     {
     k++;
     f=fopen("Gra_mana.cfg","rb");
     while(fread(&gracz,sizeof(gracz),1,f)==1)
      {
       if (gracz.dane[0]==k)
	{
	 for(x1=0;x1<20;x1++)
	  zastep[k-1][x1]=gracz.nazwisko[x1];
	}}
     fclose(f);
     f=fopen("Rywal.cfg","rb");
     while(fread(&gracz,sizeof(gracz),1,f)==1)
      {
       if (gracz.dane[0]==k&&gracz.dane[22]==Klub.rywal[0])
	{
	 for(x1=0;x1<20;x1++)
	  rzastep[k-1][x1]=gracz.nazwisko[x1];
	}}
     fclose(f);
    }}
  //-------przypisanie skàadu end
  clrscr();textcolor(15);
  if (Klub.rywal[1]==0)//w domu
   {
    textbackground(9);cprintf(" %19s %d ",krajMS[Klub.klub-1],gol1);
    textbackground(4);cprintf(" %d %-19s ",gol2,krajMS[Klub.rywal[0]-1]);
   }
  else //wyjazd
   {
    textbackground(4);cprintf(" %19s %d ",krajMS[Klub.rywal[0]-1],gol2);
    textbackground(9);cprintf(" %d %-19s ",gol1,krajMS[Klub.klub-1]);
   }
 textbackground(0);cprintf(" Czas:%3d",minuta);
 textcolor(7);i=3;
 if (Klub.rywal[1]==0)//w domu
  {
   for(k=0;k<10;k++)
    {
     if (mingol1[k]!=0) {cprintf("\n\r%19s %2d ",dlagol1[k],mingol1[k]);i--;}
     if (mingol2[k]!=0&&mingol1[k]!=0) cprintf(" %2d %-19s",mingol2[k],dlagol2[k]);
     else if (mingol2[k]!=0&&mingol1[k]==0)
      {cprintf("\n\r                        %2d %-19s",mingol2[k],dlagol2[k]);i--;}
    }
   if (i==3) cputs("\n\r\n\r\n\r");
   else if (i==2) cputs("\n\r\n\r");
   else if (i==1) cputs("\n\r");
  }
 else
  {
   for(k=0;k<10;k++)
    {
     if (mingol2[k]!=0) {cprintf("\n\r%19s %2d ",dlagol2[k],mingol2[k]);i--;}
     if (mingol1[k]!=0&&mingol2[k]!=0) cprintf(" %2d %-19s",mingol1[k],dlagol1[k]);
     else if (mingol1[k]!=0&&mingol2[k]==0)
      {cprintf("\n\r                        %2d %-19s",mingol1[k],dlagol1[k]);i--;}
    }
   if (i==3) cputs("\n\r\n\r\n\r");
   else if (i==2) cputs("\n\r\n\r");
   else if (i==1) cputs("\n\r");
  }
 k=0;
 sumaB=0;sumaO=0;sumaP=0;sumaN=0;sumaB2=0;sumaO2=0;sumaP2=0;sumaN2=0;
 //sumaBx=0;sumaOx=0;sumaPx=0;sumaNx=0;
 while(k!=2)
  {
   k++;sumaBx=0;sumaOx=0;sumaPx=0;sumaNx=0;premia=0;
   if (k==1) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;x2=Klub.usta;}
   else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];x2=Klub.rywal[2];}
   while(fread(&gracz,sizeof(gracz),1,f)==1)
    {
     if (x1==gracz.dane[22]&&gracz.dane[12]<2)
     {
     if (gracz.dane[0]<12) premia+=gracz.waga[2];
     if (gracz.dane[0]==1)
      sumaBx=gracz.dane[3]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     if (gracz.dane[0]==2||gracz.dane[0]==3||gracz.dane[0]==4)
      sumaOx+=gracz.dane[4]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     if ((gracz.dane[0]==5)&&(x2==1||x2==2||x2==3||x2==4||x2==9||x2==10||x2==11||x2==12||x2==13||x2==14))
      sumaOx+=gracz.dane[4]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     else if (gracz.dane[0]==5) sumaPx+=gracz.dane[5]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     if ((gracz.dane[0]==6)&&(x2==12||x2==13||x2==14))
      sumaOx+=gracz.dane[4]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     else if (gracz.dane[0]==6) sumaPx+=gracz.dane[5]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     if (gracz.dane[0]==7) sumaPx+=gracz.dane[5]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     if ((gracz.dane[0]==8)&&(x2==9)) sumaNx+=gracz.dane[6]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     else if (gracz.dane[0]==8) sumaPx+=gracz.dane[5]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     if ((gracz.dane[0]==9)&&(x2==5||x2==10||x2==9))
      sumaNx+=gracz.dane[6]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     else if (gracz.dane[0]==9) sumaPx+=gracz.dane[5]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     if ((gracz.dane[0]==10)&&(x2==11)) sumaPx+=gracz.dane[5]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     else if (gracz.dane[0]==10) sumaNx+=gracz.dane[6]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
     if (gracz.dane[0]==11) sumaNx+=gracz.dane[6]+gracz.dane[7]+gracz.dane[9]+gracz.dane[20]+gracz.dane[11]/10;
    }}
   fclose(f);//end - sumowanie formacji
   if (k==1)
    {sumaB=sumaBx;sumaO=sumaOx;sumaP=sumaPx;sumaN=sumaNx;
     sumaN+=premia/1000;sumaP+=premia/1000;
     //if (Klub.rywal[1]==0) {sumaB+=2;sumaO+=5;sumaP+=5;sumaN+=5;}
     if (Klub.inst[1]==3) {sumaO+=5;sumaP+=5;sumaN+=5;}//obchodzenie
     if (Klub.inst[1]==2) {sumaO-=5;sumaP-=5;sumaN-=5;}//si© z rzwalem
     if (Klub.inst[5]==2) {sumaO+=10;sumaN-=10;}//nastawienie obronne
     if (Klub.inst[5]==3) {sumaO-=10;sumaN+=10;}//nastawienie atak
     //x1=Klub.umie[3];
     sumaO+=Klub.umie[3];sumaP+=Klub.umie[3];sumaN+=Klub.umie[3];//taktyka
     if (Klub.usta==2||Klub.usta==7||Klub.usta==13)
      {sumaO+=10;sumaN-=10;}//ustawienie obronne
     if (Klub.usta==3||Klub.usta==8||Klub.usta==14)
      {sumaO-=10;sumaN+=10;}//ustawienie atak
     if (Klub.usta==4) {sumaO+=10;sumaN+=10;sumaP-=15;}//ustawienie diamond
     sumaN+=Klub.umie[2]*2;
     //cprintf("pre: %.2f",premia);
    }
   else
    {sumaB2=sumaBx;sumaO2=sumaOx;sumaP2=sumaPx;sumaN2=sumaNx;
     sumaN2+=premia/1000;sumaP2+=premia/1000;
     if (Klub.rywal[1]==1) {sumaB2+=5;sumaO2+=10;sumaP2+=10;sumaN2+=10;}
     if (Klub.rinst[1]==3) {sumaO2+=5;sumaP2+=5;sumaN2+=5;}
     if (Klub.rinst[1]==2) {sumaO2-=5;sumaP2-=5;sumaN2-=5;}
     if (Klub.rinst[5]==2) {sumaO2+=10;sumaN2-=10;}//nastawienie obronne
     if (Klub.rinst[5]==3) {sumaO2-=10;sumaN2+=10;}//nastawienie atak
     if (Klub.rywal[2]==2||Klub.rywal[2]==7||Klub.rywal[2]==13)
      {sumaO2+=10;sumaN2-=10;}//ustawienie obronne
     if (Klub.rywal[2]==3||Klub.rywal[2]==8||Klub.rywal[2]==14)
      {sumaO2-=10;sumaN2+=10;}//ustawienie atak
     //cprintf(" pre: %.2f",premia);
    }
  }//dla whilek!=2
 //cprintf(" pre: %.2f",premia);
 OnaA=sumaO-sumaN2; PnaP=sumaP-sumaP2; AnaO=sumaN-sumaO2;
 if (dystans==1) {sumaB+=50;sumaB2+=50;dystans=0;}
 /*for(x1=0;x1<8;x1++)
  {
   if (Klub.rywal[1]==0) str12[x1]=str[x1];
   else str12[x1]=str2[x1];
  }
 for(x1=0;x1<8;x1++)
  {
   if (Klub.rywal[1]==0) str12[x1+8]=str2[x1];
   else str12[x1+8]=str[x1];
  } */
 if (Klub.rywal[1]==0)
 {
 textcolor(15);textbackground(2);
 cputs("\n\r⁄ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ¬ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒø");textbackground(0);cprintf("     %2d     Strzaày     %2d",str[0],str2[0]);textbackground(2);//cprintf("     %2d     Strzaày     %2d",str12[0],str12[8]);textbackground(2);
 cputs("\n\r≥");
 textcolor(9);cputs("   O");
 textcolor(15);cputs("/");
 textcolor(4);cputs("A   ");textcolor(15);
 cputs("|");
 textcolor(9);cputs("     P");
 textcolor(15);cputs("/");
 textcolor(4);cputs("P     ");textcolor(15);
 cputs("|");
 textcolor(9);cputs("   A");
 textcolor(15);cputs("/");
 textcolor(4);cputs("O   ");textcolor(15);
 cputs("≥");textbackground(0);cprintf("     %2d  Strzaày celne  %2d",str[1],str2[1]);textbackground(2);//cprintf("     %2d  Strzaày celne  %2d",str12[1],str12[9]);textbackground(2);
 cputs("\n\r√ƒƒƒƒø    |      ≥      |    ⁄ƒƒƒƒ¥");textbackground(0);cprintf("     %2d   Rzuty roæne   %2d",str[2],str2[2]);textbackground(2);//cprintf("     %2d   Rzuty roæne   %2d",str12[2],str12[10]);textbackground(2);
 cprintf("\n\r√ø %3.0f%%   |    %3.0f%%     |  %3.0f%%  ⁄¥",strefa[3],strefa[4],strefa[5]);
 textbackground(0);cprintf("     %2d     Spalone     %2d",str[3],str2[3]);textbackground(2);//cprintf("     %2d     Spalone     %2d",str12[3],str12[11]);textbackground(2);
 cputs("\n\r≥≥   ≥    |      ≈      |    ≥   ≥≥");textbackground(0);cprintf("     %2d      Faule      %2d",str[7],str2[7]);textbackground(2);//cprintf("     %2d      Faule      %2d",str12[7],str12[15]);textbackground(2);
 cputs("\n\r√Ÿ   ≥    |      ≥      |    ≥   ¿¥");textbackground(0);cprintf("     %2d   Rzuty karne   %2d",str[6],str2[6]);textbackground(2);//cprintf("     %2d   Rzuty karne   %2d",str12[6],str12[14]);textbackground(2);
 cputs("\n\r√ƒƒƒƒŸ    |      ≥      |    ¿ƒƒƒƒ¥");textbackground(0);textcolor(14);cprintf("     %2d   Ω¢àte kartki  %2d",str[4],str2[4]);textcolor(15);textbackground(2);//cprintf("     %2d   Ω¢àte kartki  %2d",str12[4],str12[12]);textcolor(15);textbackground(2);
 cputs("\n\r≥  ");
 if (OnaA<-10)
  {
   textcolor(4);cputs("˛˛˛˛");
  }
 if (OnaA>-11&&OnaA<0)
  {
   textcolor(9);cputs("˛");textcolor(4);cputs("˛˛˛");
  }
 if (OnaA>-1&&OnaA<21)
  {
   textcolor(9);cputs("˛˛");textcolor(4);cputs("˛˛");
  }
 if (OnaA>20&&OnaA<41)
  {
   textcolor(9);cputs("˛˛˛");textcolor(4);cputs("˛");
  }
 if (OnaA>40)
  {
   textcolor(9);cputs("˛˛˛˛");
  }
 textcolor(15);cputs("   |    ");
 if (PnaP<-20)
  {
   textcolor(4);cputs("˛˛˛˛");
  }
 if (PnaP<-10&&PnaP>-21)
  {
   textcolor(9);cputs("˛");textcolor(4);cputs("˛˛˛");
  }
 if (PnaP>-11&&PnaP<11)
  {
   textcolor(9);cputs("˛˛");textcolor(4);cputs("˛˛");
  }
 if (PnaP>10&&PnaP<21)
  {
   textcolor(9);cputs("˛˛˛");textcolor(4);cputs("˛");
  }
 if (PnaP>20)
  {
   textcolor(9);cputs("˛˛˛˛");
  }
 textcolor(15);cputs("     |   ");
 if (AnaO<-40)
  {
   textcolor(4);cputs("˛˛˛˛");
  }
 if (AnaO<-20&&AnaO>-41)
  {
   textcolor(9);cputs("˛");textcolor(4);cputs("˛˛˛");
  }
 if (AnaO>-21&&AnaO<1)
  {
   textcolor(9);cputs("˛˛");textcolor(4);cputs("˛˛");
  }
 if (AnaO>0&&AnaO<11)
  {
   textcolor(9);cputs("˛˛˛");textcolor(4);cputs("˛");
  }
 if (AnaO>10)
  {
   textcolor(9);cputs("˛˛˛˛");
  }
 textcolor(15);cputs("  ≥");textcolor(12);textbackground(0);cprintf("     %2d Czerwone kartki %2d",str[5],str2[5]);textcolor(15);textbackground(2);//cprintf("     %2d Czerwone kartki %2d",str12[5],str12[13]);textcolor(15);textbackground(2);
 //cputs("\n\r≥  1111   |      ≥      |         ≥");
 //cprintf("\n\r¿ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒŸ");
 cputs("\n\r¿ƒ");textcolor(0);
 cprintf("%4d",OnaA);textcolor(15);
 cputs("ƒƒƒƒƒƒƒƒƒ");textcolor(0);
 cprintf("%4d",PnaP);textcolor(15);
 cputs("ƒƒƒƒƒƒƒƒƒ");textcolor(0);
 cprintf("%4d",AnaO);textcolor(15);cputs("ƒƒŸ");
 textbackground(0);
 //if (Klub.rywal[1]==0)
 cprintf("   %3d%% Posiadanie piàki %d%%\n\r",pos1,pos2);
 //else cprintf("   %3d%% Posiadanie piàki %d%%\n\r",pos2,pos1);//textbackground(2);
 textbackground(0);textcolor(15);
 if (gdzie==1) cputs("                 ");
 else
  {
   if (pilka==1) cputs("                              ");
   else cputs("    ");
  }
 }
 else
 {
 textcolor(15);textbackground(2);
 cputs("\n\r⁄ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ¬ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒø");textbackground(0);cprintf("     %2d     Strzaày     %2d",str2[0],str[0]);textbackground(2);//cprintf("     %2d     Strzaày     %2d",str12[0],str12[8]);textbackground(2);
 cputs("\n\r≥");
 textcolor(4);cputs("   O");
 textcolor(15);cputs("/");
 textcolor(9);cputs("A   ");textcolor(15);
 cputs("|");
 textcolor(4);cputs("     P");
 textcolor(15);cputs("/");
 textcolor(9);cputs("P     ");textcolor(15);
 cputs("|");
 textcolor(4);cputs("   A");
 textcolor(15);cputs("/");
 textcolor(9);cputs("O   ");textcolor(15);
 cputs("≥");textbackground(0);cprintf("     %2d  Strzaày celne  %2d",str2[1],str[1]);textbackground(2);//cprintf("     %2d  Strzaày celne  %2d",str12[1],str12[9]);textbackground(2);
 cputs("\n\r√ƒƒƒƒø    |      ≥      |    ⁄ƒƒƒƒ¥");textbackground(0);cprintf("     %2d   Rzuty roæne   %2d",str2[2],str[2]);textbackground(2);//cprintf("     %2d   Rzuty roæne   %2d",str12[2],str12[10]);textbackground(2);
 cprintf("\n\r√ø %3.0f%%   |    %3.0f%%     |  %3.0f%%  ⁄¥",strefa[5],strefa[4],strefa[3]);
 textbackground(0);cprintf("     %2d     Spalone     %2d",str2[3],str[3]);textbackground(2);//cprintf("     %2d     Spalone     %2d",str12[3],str12[11]);textbackground(2);
 cputs("\n\r≥≥   ≥    |      ≈      |    ≥   ≥≥");textbackground(0);cprintf("     %2d      Faule      %2d",str2[7],str[7]);textbackground(2);//cprintf("     %2d      Faule      %2d",str12[7],str12[15]);textbackground(2);
 cputs("\n\r√Ÿ   ≥    |      ≥      |    ≥   ¿¥");textbackground(0);cprintf("     %2d   Rzuty karne   %2d",str2[6],str[6]);textbackground(2);//cprintf("     %2d   Rzuty karne   %2d",str12[6],str12[14]);textbackground(2);
 cputs("\n\r√ƒƒƒƒŸ    |      ≥      |    ¿ƒƒƒƒ¥");textbackground(0);textcolor(14);cprintf("     %2d   Ω¢àte kartki  %2d",str2[4],str[4]);textcolor(15);textbackground(2);//cprintf("     %2d   Ω¢àte kartki  %2d",str12[4],str12[12]);textcolor(15);textbackground(2);
 cputs("\n\r≥  ");
 if (AnaO<-40)
  {
   textcolor(4);cputs("˛˛˛˛");
  }
 if (AnaO<-20&&AnaO>-41)
  {
   textcolor(4);cputs("˛˛˛");textcolor(9);cputs("˛");
  }
 if (AnaO>-21&&AnaO<1)
  {
   textcolor(4);cputs("˛˛");textcolor(9);cputs("˛˛");
  }
 if (AnaO>0&&AnaO<11)
  {
   textcolor(4);cputs("˛");textcolor(9);cputs("˛˛˛");
  }
 if (AnaO>10)
  {
   textcolor(9);cputs("˛˛˛˛");
  }
 textcolor(15);cputs("   |    ");
 if (PnaP<-20)
  {
   textcolor(4);cputs("˛˛˛˛");
  }
 if (PnaP<-10&&PnaP>-21)
  {
   textcolor(4);cputs("˛˛˛");textcolor(9);cputs("˛");
  }
 if (PnaP>-11&&PnaP<11)
  {
   textcolor(4);cputs("˛˛");textcolor(9);cputs("˛˛");
  }
 if (PnaP>10&&PnaP<21)
  {
   textcolor(4);cputs("˛");textcolor(9);cputs("˛˛˛");
  }
 if (PnaP>20)
  {
   textcolor(9);cputs("˛˛˛˛");
  }
 textcolor(15);cputs("     |   ");
 if (OnaA<-10)
  {
   textcolor(4);cputs("˛˛˛˛");
  }
 if (OnaA>-11&&OnaA<0)
  {
   textcolor(4);cputs("˛˛˛");textcolor(9);cputs("˛");
  }
 if (OnaA>-1&&OnaA<21)
  {
   textcolor(4);cputs("˛˛");textcolor(9);cputs("˛˛");
  }
 if (OnaA>20&&OnaA<41)
  {
   textcolor(4);cputs("˛");textcolor(9);cputs("˛˛˛");
  }
 if (OnaA>40)
  {
   textcolor(9);cputs("˛˛˛˛");
  }
 textcolor(15);cputs("  ≥");textcolor(12);textbackground(0);cprintf("     %2d Czerwone kartki %2d",str2[5],str[5]);textcolor(15);textbackground(2);//cprintf("     %2d Czerwone kartki %2d",str12[5],str12[13]);textcolor(15);textbackground(2);
 //cputs("\n\r≥  1111   |      ≥      |         ≥");
 //cprintf("\n\r¿ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒŸ");
 cputs("\n\r¿ƒ");textcolor(0);
 cprintf("%4d",AnaO);textcolor(15);
 cputs("ƒƒƒƒƒƒƒƒƒ");textcolor(0);
 cprintf("%4d",PnaP);textcolor(15);
 cputs("ƒƒƒƒƒƒƒƒƒ");textcolor(0);
 cprintf("%4d",OnaA);textcolor(15);cputs("ƒƒŸ");
 textbackground(0);
 //if (Klub.rywal[1]==0) cprintf("   %3d%% Posiadanie piàki %d%%\n\r",pos1,pos2);
 //else
 cprintf("   %3d%% Posiadanie piàki %d%%\n\r",pos2,pos1);//textbackground(2);
 //cprintf("B %d, B2 %d",sumaB,sumaB2);
 textbackground(0);textcolor(15);
 if (gdzie==1) cputs("                 ");
 else
  {
   if (pilka==2) cputs("                              ");
   else cputs("    ");
  }}//dla meczu na wyje´dzie
 if (start==1&&taktyka2==0&&koniec!=4&&koniec!=1&&Klub.mecz==1)//||koniec==3||koniec==1)
  {
   textcolor(2);
   cprintf("\n\rA Taktyka-%s  P Taktyka-%s  C SzybkoòÜ-%s\n\r",krajMS[Klub.klub-1],krajMS[Klub.rywal[0]-1],speed[czas]);
  }
 if (start==1&&koniec<3)//mecz si© rozpocz•à
  {
   co=usalenie(pilka,PnaP,OnaA,AnaO,sumaB,sumaB2,gdzie);
   //cprintf("\n\rPilka = %d, gdzie = %d, co = %d\n\r",pilka,gdzie,co);
 //************ gdzie = 1 *******************
   if (co==9) dystans=1;
   if (gdzie==1)
    {
     if (pilka==1)
      {
       los=random(3)+2;
       dlawiado[0]=zastep[los-1];
       dlawiado[1]=zastep[6];//czyli 7
      }
     else //if (pilka==2)
      {
       los=random(3)+2;
       dlawiado[0]=rzastep[los-1];
       dlawiado[1]=rzastep[6];//czyli 7
      }
     //blokada=0;
     if (co==1)
      {
       wiado[0]=8;los=random(3);//blokada=1;
       ktop[0]=pilka;ktop[1]=pilka;
       if (pilka==1) {x1=Klub.usta; dlawiado[2]=zastep[6];}
       else {x1=Klub.rywal[2];dlawiado[2]=rzastep[6];}
       if (los==0)//na lewe skrzydào
	{
	 wiado[1]=9;gdzie=2;
	 if (x1==1||x1==2||x1==3||x1==4||x1==10||x1==11)
	  {if (pilka==1) dlawiado[3]=zastep[5];//czyli 6
	   else dlawiado[3]=rzastep[5];pam=5;}
	 if (x1==5||x1==6||x1==7||x1==8||x1==12||x1==13||x1==14)
	  {if (pilka==1)  dlawiado[3]=zastep[4];
	   else dlawiado[3]=rzastep[4];pam=4;}
	 if (x1==9)
	  {if (pilka==1) dlawiado[3]=zastep[7];
	   else dlawiado[3]=rzastep[7];pam=7;}
	}
       else //if (los==1)//podanie na prawe skrzydào
	{
	 wiado[1]=10;gdzie=2;
	 if (x1==1||x1==2||x1==3||x1==6||x1==7||x1==9)
	  {if (pilka==1)  dlawiado[3]=zastep[8];// 9
	   else dlawiado[3]=rzastep[8];pam=8;}
	 if (x1==4||x1==5||x1==8||x1==10)
	  {if (pilka==1)  dlawiado[3]=zastep[7];//8
	   else dlawiado[3]=rzastep[7];pam=7;}
	 if (x1==11)
	  {if (pilka==1) dlawiado[3]=zastep[9];//10
	   else dlawiado[3]=rzastep[9];pam=9;}
	 if (x1==12||x1==13||x1==14)
	  {if (pilka==1)  dlawiado[3]=zastep[5];// 6
	   else dlawiado[3]=rzastep[5];pam=5;}
	}
       if (los==2)
	{
	 wiado[1]=11;gdzie=3;
	 if (pilka==1) dlawiado[2]=zastep[6];
	 else dlawiado[2]=rzastep[6];
	}}//co=1
     else if (co==2)
      {
       wiado[0]=12;gdzie=3;ktop[0]=pilka;
      }//co=2
     else if (co==3||co==4)
      {
       wiado[0]=12;ktop[0]=pilka;
       if (co==4) wiado[0]=8;
       wiado[1]=13;
       if (co==4) wiado[1]=14;
       if (pilka==1) {ktop[1]=2;dlawiado[2]=rzastep[6];dlawiado[3]=krajMS[Klub.rywal[0]-1];pilka=2;}
       else {ktop[1]=1; dlawiado[2]=zastep[6]; dlawiado[3]=krajMS[Klub.klub-1];pilka=1;}
       gdzie=1;
      }//co=3||co=4
     else if (co==5)
      {
       wiado[0]=8;ktop[0]=pilka;
       wiado[1]=15;ktop[1]=pilka;
       wiado[2]=16;
       if (pilka==1) {ktop[2]=2;dlawiado[4]=rzastep[6];pilka=2;}
       else {ktop[2]=1;dlawiado[4]=zastep[6];pilka=1;}
       gdzie=1;
      }//co=5
     else if (co==6)
      {
       wiado[0]=8;ktop[0]=pilka;
       wiado[1]=17;wiado[2]=18;
       if (pilka==1) {ktop[1]=2;ktop[2]=2;dlawiado[2]=rzastep[6];dlawiado[3]=dlawiado[1];dlawiado[4]=rzastep[6];pilka=2;}
       else {ktop[1]=1;ktop[2]=1;dlawiado[2]=zastep[6];dlawiado[3]=dlawiado[1];dlawiado[4]=zastep[6];pilka=1;}
       gdzie=1;
      }//co=6
     else if (co==7)//faul przeciwnika
      {
       wiado[0]=8;ktop[0]=pilka;taktyka=1;
       wiado[1]=14;wiado[2]=19;
       if (pilka==1) {x1=Klub.rinst[1];x2=Klub.rywal[2];str2[7]++;}
       else {x1=Klub.inst[1];x2=Klub.usta;str[7]++;}
       los=random(6);
       if (x1==2)//delikatne
	{
	 if (los==0||los==1||los==2) co=1;//nie ma kartki
	 else if (los==3||los==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       else if (x1==1)//normalne
	{
	 if (los==0||los==1) co=1;//nie ma kartki
	 else if (los==3||los==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartkaif (los==5||los==2)
	}
       else if (x1==3)//twarde
	{
	 if (los==0) co=1;//nie ma kartki
	 else if (los==3||los==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartkaif (los==5||los==2||los==1)
	}
       if (x2==1||x2==2||x2==3||x2==4) los=random(4)+6;
       else if (x2==5) los=random(4)+5;
       else if (x2==6||x2==7||x2==8) los=random(5)+5;
       else if (x2==9) los=random(2)+6;
       else if (x2==10) los=random(3)+6;
       else if (x2==11) los=random(5)+6;
       else if (x2==12||x2==13||x2==14) los=random(3)+7;
       if (pilka==1) {dlawiado[2]=rzastep[los-1];ktop[1]=2;ktop[2]=2;}
       else {dlawiado[2]=zastep[los-1]; ktop[1]=1;ktop[2]=1;}
       if (co==3)//jest æ¢àta
	{
	 f=fopen("wymiana.cfg","wb");
	 fclose(f);
	 if (pilka==2) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22])
	    {gracz.dane[12]++;gracz.dane[7]--;
	     if (gracz.dane[7]<-3) gracz.dane[7]=-3;}
	   f2=fopen("wymiana.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	 if (pilka==2) przepisz();
	 else
	  {
	   f=fopen("Rywal.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Rywal.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	  }
	 k=0;
	 if (pilka==2) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22]&&gracz.dane[12]>=2)
	    k=1;
	  }
	 fclose(f);
	 if (k==1)
	  {wiado[3]=60;
	   if (pilka==1) {str2[5]++;str2[4]++;}
	   else {str[5]++;str[4]++;}
	  }
	 else
	  {wiado[3]=20;
	   if (pilka==1) str2[4]++;
	   else str[4]++;
	  }
	 if (pilka==1) {dlawiado[6]=rzastep[los-1];ktop[3]=2;}
	 else {dlawiado[6]=zastep[los-1];ktop[3]=1;}
	}// jest æ¢àta end
       else if (co==2) //sàownie
	{
	 wiado[3]=21;
	 if (pilka==1) {dlawiado[6]=rzastep[los-1];ktop[3]=2;}
	 else {dlawiado[6]=zastep[los-1];ktop[3]=1;}
	}//sàownie end
       wiado[4]=22;ktop[4]=pilka;ktop[5]=pilka;
       los=random(3);
       if (pilka==1) {x1=Klub.usta; dlawiado[8]=zastep[6];dlawiado[10]=zastep[6];}
       else {x1=Klub.rywal[2];dlawiado[8]=rzastep[6];dlawiado[10]=rzastep[6];}
       if (los==0)//na lewe skrzydào
	{
	 wiado[5]=9;gdzie=2;
	 if (x1==1||x1==2||x1==3||x1==4||x1==10||x1==11)
	  {if (pilka==1) dlawiado[11]=zastep[5];//czyli 6
	   else dlawiado[11]=rzastep[5];pam=5;}
	 if (x1==5||x1==6||x1==7||x1==8||x1==12||x1==13||x1==14)
	  {if (pilka==1)  dlawiado[11]=zastep[4];
	   else dlawiado[11]=rzastep[4];pam=4;}
	 if (x1==9)
	  {if (pilka==1) dlawiado[11]=zastep[7];
	   else dlawiado[11]=rzastep[7];pam=7;}
	}
       if (los==1)//podanie na prawe skrzydào
	{
	 wiado[5]=10;gdzie=2;
	 if (x1==1||x1==2||x1==3||x1==6||x1==7||x1==9)
	  {if (pilka==1)  dlawiado[11]=zastep[8];// 9
	   else dlawiado[11]=rzastep[8];pam=8;}
	 if (x1==4||x1==5||x1==8||x1==10)
	  {if (pilka==1)  dlawiado[11]=zastep[7];//8
	   else dlawiado[11]=rzastep[7];pam=7;}
	 if (x1==11)
	  {if (pilka==1) dlawiado[11]=zastep[9];//10
	   else dlawiado[11]=rzastep[9];pam=9;}
	 if (x1==12||x1==13||x1==14)
	  {if (pilka==1)  dlawiado[11]=zastep[5];// 6
	   else dlawiado[11]=rzastep[5];pam=5;}
	}
       if (los==2)
	{
	 wiado[5]=11;gdzie=3;
	 //if (pilka==1) dlawiado[10]=zastep[6];
	 //else dlawiado[10]=rzastep[6];
	}
      }//co=7
     else if (co==8)//aut dla przeciwnika
      {
       wiado[0]=8;ktop[0]=pilka;taktyka=1;
       wiado[1]=23;ktop[1]=pilka;los=random(2);
       if (los==0) wiado[2]=24;
       else wiado[2]=25;
       los=random(3)+2;
       if (pilka==1) {dlawiado[4]=rzastep[los-1];pilka=2;}
       else {dlawiado[4]=zastep[los-1];pilka=1;}
       gdzie=1;ktop[2]=pilka;
      }//co=8
     else if (co==9)//strzaà z dystansu
      {
       wiado[0]=8;ktop[0]=pilka;ktop[2]=pilka;
       wiado[1]=26;ktop[1]=pilka;los=random(2);
       if (los==0) wiado[2]=27;
       else wiado[2]=28;
       if (pilka==1) {str[0]++;x1=Klub.usta;}
       else {str2[0]++;x1=Klub.rywal[2];}
       if (x1==6||x1==7||x1==8) los=random(5)+5;
       else if (x1==11) los=random(5)+6;
       else if (x1==12||x1==13||x1==14) los=random(3)+7;
       else if (x1==5) los=random(4)+5;
       else if (x1==10) los=random(3)+6;
       else los=random(4)+6;
       if (pilka==1) dlawiado[1]=zastep[los-1];
       else dlawiado[1]=rzastep[los-1];
       dlawiado[2]=dlawiado[1];
       dlawiado[4]=dlawiado[2];
       gdzie=4;pam=los-1;
      }//co=9
    }////dla blokada==1 gdzie==1
//************** Skrzydàowy przy piàce ******************
   else if (gdzie==2)
    {
     wiado[0]=29;ktop[0]=pilka;
     if (pilka==1) dlawiado[0]=zastep[pam];//x1=Klub.usta;
     else dlawiado[0]=rzastep[pam];//x1=Klub.rywal[2];
     if (co==10)//udane doòrodkowanie
      {
       wiado[1]=30;ktop[1]=pilka;dlawiado[2]=dlawiado[0];
       f=fopen("wymiana.cfg","wb");
       fclose(f);
       if (pilka==1) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;dlawiado[3]=rzastep[3];}
       else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];dlawiado[3]=zastep[3];}
       while(fread(&gracz,sizeof(gracz),1,f)==1)
	{
	 if (gracz.dane[0]==pam+1&&gracz.dane[22]==x1) gracz.dane[20]++;
	 f2=fopen("wymiana.cfg","ab");
	 fwrite(&gracz,sizeof(gracz),1,f2);
	 fclose(f2);
	}
       fclose(f);
       if (pilka==1) przepisz();
       else
	{
	 f=fopen("Rywal.cfg","wb");
	 fclose(f);
	 f=fopen("wymiana.cfg","rb");
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   f2=fopen("Rywal.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	}
       los=random(2);
       if (los==0) wiado[2]=31;
       else wiado[2]=32;
       ktop[2]=pilka;gdzie=3;
      }
     else if (co==11)//faul
      {
       f=fopen("wymiana.cfg","wb");
       fclose(f);
       if (pilka==1) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;dlawiado[3]=rzastep[3];}
       else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];dlawiado[3]=zastep[3];}
       while(fread(&gracz,sizeof(gracz),1,f)==1)
	{
	 if (gracz.dane[0]==pam+1&&gracz.dane[22]==x1) gracz.dane[20]++;
	 f2=fopen("wymiana.cfg","ab");
	 fwrite(&gracz,sizeof(gracz),1,f2);
	 fclose(f2);
	}
       fclose(f);
       if (pilka==1) przepisz();
       else
	{
	 f=fopen("Rywal.cfg","wb");
	 fclose(f);
	 f=fopen("wymiana.cfg","rb");
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   f2=fopen("Rywal.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	}
       wiado[1]=30;ktop[1]=pilka;dlawiado[2]=dlawiado[0];taktyka=1;
       if (pilka==1) {x1=Klub.rywal[2];str2[7]++;}
       else {x1=Klub.usta;str[7]++;}
       if (x1==5||x1==6||x1==7||x1==8) los=random(3)+2;
       else if (x1==12||x1==13||x1==14) los=random(5)+2;
       else los=random(4)+2;//if (x1==1||x1==2||x1==3||x1==4||x1==9||x1==10||x1==11)
       if (pilka==1) dlawiado[3]=rzastep[los-1];
       else dlawiado[3]=zastep[los-1];
       wiado[2]=33;dlawiado[4]=dlawiado[3];dlawiado[5]=dlawiado[2];
       x2=random(6);
       if (pilka==1) {ktop[2]=2;ktop[3]=2;x1=Klub.rinst[1];}
       else {ktop[2]=1;ktop[3]=1;x1=Klub.inst[1];}
       if (x1==2)//delikatne
	{
	 if (x2==0||x2==1||x2==2) co=1;//nie ma kartki
	 else if (x2==3||x2==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       else if (x1==1)//normalne
	{
	 if (x2==0||x2==1) co=1;//nie ma kartki
	 else if (x2==3||x2==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       else if (x1==3)//twarde
	{
	 if (x2==0) co=1;//nie ma kartki
	 else if (x2==3||x2==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       if (co==3)//jest æ¢àta
	{
	 f=fopen("wymiana.cfg","wb");
	 fclose(f);
	 if (pilka==2) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22])
	    {gracz.dane[12]++;gracz.dane[7]--;
	     if (gracz.dane[7]<-3) gracz.dane[7]=-3;}
	   f2=fopen("wymiana.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	 if (pilka==2) przepisz();
	 else
	  {
	   f=fopen("Rywal.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Rywal.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	  }
	 k=0;
	 if (pilka==2) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22]&&gracz.dane[12]>=2)
	    k=1;
	  }
	 fclose(f);
	 dlawiado[6]=dlawiado[3];
	 if (k==0)
	  {wiado[3]=20;
	   if (pilka==1) str2[4]++;
	   else str[4]++;
	  }
	 else if (k==1)
	  { wiado[3]=60;
	   if (pilka==1) {str2[5]++;str2[4]++;}
	   else {str[5]++;str[4]++;}
	  }
	}// jest æ¢àta end
       else if (co==2) //sàownie
	{
	 wiado[3]=21;dlawiado[6]=dlawiado[3];
	 if (pilka==1) ktop[3]=2;
	 else ktop[3]=1;
	}//sàownie end
       los=random(10)+2;
       wiado[4]=85;ktop[4]=pilka;
       if (pilka==1) {dlawiado[8]=zastep[los-1];str[0]++;}
       else {dlawiado[8]=rzastep[los-1];str2[0]++;}
       pam=los-1;
       wiado[5]=63;ktop[5]=pilka;
       wiado[6]=64;ktop[6]=pilka;
       dlawiado[12]=dlawiado[8];
       gdzie=6;
      }//end faul
     else if (co==12)//aut
      {
       wiado[1]=34;ktop[1]=pilka;taktyka=1;
       los=random(2);
       if (los==0) wiado[2]=24;
       else wiado[2]=25;
       los=random(3)+2;
       if (pilka==1) {dlawiado[4]=rzastep[los-1];pilka=2;ktop[2]=2;}
       else {dlawiado[4]=zastep[los-1];pilka=1;ktop[2]=1;}
       gdzie=1;
      }
     else if (co==13)//nie doòrodkowaà
      {
       wiado[1]=35;los=random(3)+2;
       if (pilka==1) {pilka=2;dlawiado[2]=rzastep[los-1];ktop[1]=2;ktop[2]=2;x1=Klub.rinst[0];}
       else {pilka=1;dlawiado[2]=zastep[los-1];ktop[1]=1;ktop[2]=1;x1=Klub.inst[0];}
       f=fopen("wymiana.cfg","wb");
       fclose(f);
       if (pilka==2) {f=fopen("Gra_mana.cfg","rb");x2=Klub.klub;}
       else {f=fopen("Rywal.cfg","rb");x2=Klub.rywal[0];}
       while(fread(&gracz,sizeof(gracz),1,f)==1)
	{
	 if (gracz.dane[0]==pam+1&&gracz.dane[22]==x2) gracz.dane[20]--;
	 f2=fopen("wymiana.cfg","ab");
	 fwrite(&gracz,sizeof(gracz),1,f2);
	 fclose(f2);
	}
       fclose(f);
       if (pilka==2) przepisz();
       else
	{
	 f=fopen("Rywal.cfg","wb");
	 fclose(f);
	 f=fopen("wymiana.cfg","rb");
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   f2=fopen("Rywal.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	}
       dlawiado[4]=dlawiado[2];
       los=random(6);
       if (x1==4) //dàugie podania
	{
	 if (los==1||los==2||los==3||los==4||los==5) wiado[2]=36;//gdzie=3;}
	 else wiado[2]=11;//gdzie=1;}
	}
       else if (x1==1||x1==3)//mieszane, òrednie
	{
	 if (los==1||los==2||los==5) wiado[2]=36;//gdzie=3;}
	 else wiado[2]=11;//gdzie=1;}
	}
       else //if (x1==2) //kt¢tkie podania
	{
	 if (los==1||los==2||los==3||los==4||los==5) wiado[2]=11;//gdzie=1;}
	 else wiado[2]=36;//gdzie=3;}
	}
       if (wiado[2]==11) gdzie=1;
       else gdzie=3;
      }//nie doòrodkowaà
    }//dla skrzydàowy przy piàce
   else if (gdzie==3)//****************** Pole karne ********
    {
     if (co==14)//spalony
      {
       wiado[0]=37;ktop[0]=pilka;taktyka=1;
       if (pilka==1) dlawiado[0]=zastep[10];
       else dlawiado[0]=rzastep[10];
       los=random(3);
       if (los==0) wiado[1]=38;
       else if (los==1) wiado[1]=39;
       else wiado[1]=40;
       dlawiado[2]=dlawiado[0];ktop[1]=pilka;
       wiado[2]=41;
       if (pilka==1) {pilka=2;dlawiado[4]=rzastep[0];ktop[2]=2;str[3]++;}
       else {pilka=1;dlawiado[4]=zastep[0];ktop[2]=1;str2[3]++;}
       gdzie=1;
      }//spalony
     else if (co==15)//spalony nieudany
      {
       wiado[0]=37;ktop[0]=pilka;
       wiado[1]=42;ktop[1]=pilka;
       wiado[2]=43;ktop[2]=pilka;
       if (pilka==1) x1=Klub.usta;
       else x1=Klub.rywal[2];
       if (x1==11) los=11;
       else if (x1==5||x1==10) los=random(3)+9;
       else los=random(2)+10;
       if (pilka==1) {dlawiado[0]=zastep[los-1];str[0]++;}
       else {dlawiado[0]=rzastep[los-1];str2[0]++;}
       pam=los-1;
       dlawiado[2]=dlawiado[0];dlawiado[4]=dlawiado[0];
       gdzie=5;
      }//sam na sam
     else if (co==16)//obrona wykopuje
      {
       if (pilka==1) x1=Klub.rinst[0];
       else x1=Klub.inst[0];
       los=random(6);x2=random(2);
       if (x1==4)//dàugie
	{
	 if (los==0)
	  {
	   if (x2==0) wiado[0]=44;
	   else wiado[0]=45;
	   gdzie=1;
	  }
	 else
	  {
	   gdzie=3;
	   if (x2==0) wiado[0]=46;
	   else wiado[0]=47;
	  }}
       else if (x1==2)//kr¢tkie
	{
	 if (los==0)
	  {
	   gdzie=3;
	   if (x2==0) wiado[0]=46;
	   else wiado[0]=47;
	  }
	 else
	  {
	   gdzie=1;
	   if (x2==0) wiado[0]=44;
	   else wiado[0]=45;
	  }}
       else//pozostaàe
	{
	 if (los==0||los==1||los==2)
	  {
	   gdzie=3;
	   if (x2==0) wiado[0]=46;
	   else wiado[0]=47;
	  }
	 else
	  {
	   gdzie=1;
	   if (x2==0) wiado[0]=44;
	   else wiado[0]=45;
	  }}
       los=random(3)+2;
       if (pilka==1) {pilka=2;dlawiado[0]=rzastep[los-1];ktop[0]=2;}
       else {pilka=1;dlawiado[0]=zastep[los-1];ktop[0]=1;}
       //gdzie=1;
      }//obrona wykopuje
     else if (co==17)//B àapie
      {
       wiado[0]=48;wiado[1]=49;
       if (pilka==1) {pilka=2;dlawiado[0]=rzastep[0];ktop[0]=2;ktop[1]=2;}
       else {pilka=1;dlawiado[0]=zastep[0];ktop[0]=1;ktop[1]=1;}
       gdzie=1;
      }//B àapie
     else if (co==18)//karny
      {
       wiado[0]=37;ktop[0]=pilka;wiado[1]=50;
       if (pilka==1) {dlawiado[0]=zastep[10];ktop[1]=2;x1=Klub.rywal[2];str2[7]++;str[6]++;}
       else {dlawiado[0]=rzastep[10];ktop[1]=1;x1=Klub.usta;str[7]++;str2[6]++;}
       if (x1==1||x1==2||x1==3||x1==4||x1==9||x1==10||x1==11) los=random(4)+2;
       else if (x1==5||x1==6||x1==7||x1==8) los=random(3)+2;
       else los=random(5)+2;
       dlawiado[3]=dlawiado[0];
       if (pilka==1) dlawiado[2]=rzastep[los-1];
       else dlawiado[2]=zastep[los-1];
       x1=random(2);
       if (x1==0) wiado[2]=51;
       else wiado[2]=52;
       x2=random(6);
       if (pilka==1) {ktop[2]=2;ktop[3]=2;x1=Klub.rinst[1];str[0]++;}
       else {ktop[2]=1;ktop[3]=1;x1=Klub.inst[1];str2[0]++;}
       if (x1==2)//delikatne
	{
	 if (x2==0||x2==1||x2==2) co=1;//nie ma kartki
	 else if (x2==3||x2==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       else if (x1==1)//normalne
	{
	 if (x2==0||x2==1) co=1;//nie ma kartki
	 else if (x2==3||x2==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       else if (x1==3)//twarde
	{
	 if (x2==0) co=1;//nie ma kartki
	 else if (x2==3||x2==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       if (co==3)//jest æ¢àta
	{
	 f=fopen("wymiana.cfg","wb");
	 fclose(f);
	 if (pilka==2) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22])
	    {gracz.dane[12]++;gracz.dane[7]--;
	     if (gracz.dane[7]<-3) gracz.dane[7]=-3;}
	   f2=fopen("wymiana.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	 if (pilka==2) przepisz();
	 else
	  {
	   f=fopen("Rywal.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Rywal.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	  }
	 k=0;
	 if (pilka==2) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22]&&gracz.dane[12]>=2)
	    k=1;
	  }
	 fclose(f);
	 dlawiado[6]=dlawiado[2];
	 if (k==0)
	  {wiado[3]=20;
	   if (pilka==1) str2[4]++;
	   else str[4]++;
	  }
	 else if (k==1)
	  {wiado[3]=60;
	   if (pilka==1) {str2[5]++;str2[4]++;}
	   else {str[5]++;str[4]++;}
	  }
	}// jest æ¢àta end
       else if (co==2) //sàownie
	{
	 wiado[3]=21;dlawiado[6]=dlawiado[2];
	 if (pilka==1) ktop[3]=2;
	 else ktop[3]=1;
	}//sàownie end
       x1=random(2);
       if (x1==0) wiado[4]=53;
       else wiado[4]=54;
       los=random(4)+8;
       if (pilka==1) dlawiado[8]=zastep[los-1];
       else dlawiado[8]=rzastep[los-1];
       pam=los-1;gdzie=5;ktop[4]=pilka;
      }//karny
     else if (co==19)//symulowany
      {
       wiado[0]=37;ktop[0]=pilka;ktop[2]=pilka;
       if (pilka==1) {dlawiado[0]=zastep[10];ktop[1]=2;ktop[3]=2;x1=Klub.rywal[2];}
       else {dlawiado[0]=rzastep[10];ktop[1]=1;ktop[3]=1;x1=Klub.usta;}
       if (x1==1||x1==2||x1==3||x1==4||x1==9||x1==10||x1==11) los=random(4)+2;
       else if (x1==5||x1==6||x1==7||x1==8) los=random(3)+2;
       else los=random(5)+2;
       dlawiado[3]=dlawiado[0];
       if (pilka==1) dlawiado[2]=rzastep[los-1];
       else dlawiado[2]=zastep[los-1];
       wiado[1]=50;wiado[2]=55;ktop[2]=pilka;dlawiado[4]=dlawiado[0];
       los=random(2);
       if (los==0) wiado[3]=44;
       else wiado[3]=45;
       if (pilka==1) pilka=2;
       else pilka=1;
       dlawiado[6]=dlawiado[2];
       ktop[3]=pilka;gdzie=1;
      }//faul symulowany
     else if (co==20) //r¢g
      {
       wiado[0]=37;ktop[0]=pilka;ktop[2]=pilka;ktop[3]=pilka;taktyka=1;
       if (pilka==1) {dlawiado[0]=zastep[10];ktop[1]=2;x1=Klub.rywal[2];str[2]++;}
       else {dlawiado[0]=rzastep[10];ktop[1]=1;x1=Klub.usta;str2[2]++;}
       if (x1==1||x1==2||x1==3||x1==4||x1==9||x1==10||x1==11) los=random(4)+2;
       else if (x1==5||x1==6||x1==7||x1==8) los=random(3)+2;
       else los=random(5)+2;
       dlawiado[3]=dlawiado[0];
       if (pilka==1) dlawiado[2]=rzastep[los-1];
       else dlawiado[2]=zastep[los-1];
       wiado[1]=50;wiado[2]=56;wiado[3]=78;
       if (pilka==1) dlawiado[6]=zastep[6];
       else dlawiado[6]=rzastep[6];
       gdzie=3;
      }//r¢g
     else if (co==21)//strzaà
      {
       wiado[0]=37;ktop[0]=pilka;ktop[1]=pilka;
       los=random(2);
       if (los==0) wiado[1]=43;
       else wiado[1]=57;
       if (pilka==1) x1=Klub.usta;
       else x1=Klub.rywal[2];
       if (x1==5||x1==10) los=random(3)+9;
       else if (x1==11) los=11;
       else los=random(2)+10;
       if (pilka==1) {dlawiado[0]=zastep[los-1];str[0]++;}
       else {dlawiado[0]=rzastep[los-1];str2[0]++;}
       dlawiado[2]=dlawiado[0];
       pam=los-1;gdzie=4;
      }//strzaà
     else if (co==22)//podanie i strzaà
      {
       wiado[0]=37;ktop[0]=pilka;
       wiado[1]=58;ktop[1]=pilka;ktop[2]=pilka;
       los=random(2);
       if (los==0) wiado[2]=59;
       else wiado[2]=57;
       if (pilka==1) dlawiado[0]=zastep[10];
       else dlawiado[0]=rzastep[10];
       los=random(4)+7;
       if (pilka==1) {dlawiado[2]=zastep[los-1];str[0]++;}
       else {dlawiado[2]=rzastep[los-1];str2[0]++;}
       dlawiado[4]=dlawiado[2];
       pam=los-1;gdzie=4;
      }//podanie i strzaà
     else if (co==23) //faul napastnika
      {
       wiado[0]=37;ktop[0]=pilka;taktyka=1;
       if (pilka==1) {str[7]++;dlawiado[2]=rzastep[1];ktop[1]=2;ktop[2]=2;x1=Klub.inst[1];x2=Klub.usta;}
       else {str2[7]++;dlawiado[2]=zastep[1];ktop[1]=1;ktop[2]=1;x1=Klub.rinst[1];x2=Klub.rywal[2];}
       if (x2==5||x2==10) los=random(3)+9;
       else if (x2==11) los=11;
       else los=random(2)+10;
       if (pilka==1) dlawiado[0]=zastep[los-1];
       else dlawiado[0]=rzastep[los-1];
       dlawiado[3]=dlawiado[0];
       wiado[1]=50;wiado[2]=61;wiado[3]=62;ktop[3]=pilka;
       dlawiado[6]=dlawiado[0];wiado[4]=19;ktop[4]=pilka;
       x2=random(6);
       if (x1==2)//delikatne
	{
	 if (x2==0||x2==1||x2==2) co=1;//nie ma kartki
	 else if (x2==3||x2==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       else if (x1==1)//normalne
	{
	 if (x2==0||x2==1) co=1;//nie ma kartki
	 else if (x2==3||x2==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       else if (x1==3)//twarde
	{
	 if (x2==0) co=1;//nie ma kartki
	 else if (x2==3||x2==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       if (co==3)//jest æ¢àta
	{
	 f=fopen("wymiana.cfg","wb");
	 fclose(f);
	 if (pilka==1) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22])
	    {gracz.dane[12]++;gracz.dane[7]--;
	     if (gracz.dane[7]<-3) gracz.dane[7]=-3;}
	   f2=fopen("wymiana.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	 if (pilka==1) przepisz();
	 else
	  {
	   f=fopen("Rywal.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Rywal.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	  }
	 k=0;
	 if (pilka==1) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22]&&gracz.dane[12]>=2)
	    k=1;
	  }
	 fclose(f);
	 dlawiado[10]=dlawiado[0];ktop[5]=pilka;
	 if (k==0)
	  {wiado[5]=20;
	   if (pilka==1) str[4]++;
	   else str2[4]++;
	  }
	 else if (k==1)
	  {wiado[5]=60;
	   if (pilka==1) {str[4]++;str[5]++;}
	   else {str2[4]++;str2[5]++;}
	  }
	}// jest æ¢àta end
       else if (co==2) //sàownie
	{
	 wiado[5]=21;dlawiado[10]=dlawiado[3];
	 ktop[5]=pilka;
	}//sàownie end
       wiado[6]=41;
       if (pilka==1) {pilka=2;dlawiado[12]=rzastep[0];}
       else {pilka=1;dlawiado[12]=zastep[0];}
       ktop[6]=pilka;gdzie=1;
      }//faul napastnika
    }//gdzie=3,blokada=1,pole karne
  //*********************** obron B ***********************************
   else if (gdzie==4||gdzie==6||gdzie==5)//obrona B
    {
     if (co==24)//udana
      {
       x1=random(2);
       if (x1==0) wiado[0]=65;
       else wiado[0]=66;
       wiado[1]=67;wiado[2]=49;
       if (pilka==1) {dlawiado[0]=rzastep[0];ktop[0]=2;ktop[1]=2;pilka=2;str[1]++;}
       else {dlawiado[0]=zastep[0];ktop[0]=1;ktop[1]=1;pilka=1;str2[1]++;}
       dlawiado[2]=dlawiado[0];ktop[2]=pilka;
       gdzie=1;
	f=fopen("wymiana.cfg","wb");
	 fclose(f);
	 if (pilka==1) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==1&&x1==gracz.dane[22])
	    {gracz.dane[20]++;gracz.dane[7]++;
	     if (gracz.dane[7]>3) gracz.dane[7]=3;}
	   f2=fopen("wymiana.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	 if (pilka==1) przepisz();
	 else
	  {
	   f=fopen("Rywal.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Rywal.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	  }
      }//udana
     else if (co==25)//no problem
      {
       los=random(2);
       if (los==0) wiado[0]=68;
       else wiado[0]=69;
       wiado[1]=49;
       if (pilka==1) {dlawiado[0]=rzastep[0];ktop[0]=2;pilka=2;str[1]++;}
       else {dlawiado[0]=zastep[0];ktop[0]=1;pilka=1;str2[1]++;}
       ktop[1]=pilka;gdzie=1;
	 f=fopen("wymiana.cfg","wb");
	 fclose(f);
	 if (pilka==1) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==1&&x1==gracz.dane[22])
	    {gracz.dane[20]++;gracz.dane[7]++;
	     if (gracz.dane[7]>3) gracz.dane[7]=3;}
	   f2=fopen("wymiana.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	 if (pilka==1) przepisz();
	 else
	  {
	   f=fopen("Rywal.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Rywal.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	  }
      }//no problem
     else if (co==26) //niepewnie
      {
       los=random(4);
       if (los==0) {wiado[0]=70;ktop[0]=1;if (pilka==1) {ktop[0]=2;str[1]++;}
       else str2[1]++;}
       else if (los==1) {wiado[0]=71;ktop[0]=pilka;}
       else if (los==2) {wiado[0]=72;ktop[0]=pilka;}
       else {wiado[0]=73;ktop[0]=1;if (pilka==1) ktop[0]=2;}
       if (pilka==1) dlawiado[0]=rzastep[0];
       else dlawiado[0]=zastep[0];
       gdzie=3;
      }//niepewnie
     else if (co==27)//r¢g
      {
       los=random(2);taktyka=1;
       if (los==0)
	{wiado[0]=70;
	if (pilka==1) str[1]++;
	else str2[1]++;}
       else wiado[0]=73;
       if (pilka==1) {str[2]++;ktop[0]=2;dlawiado[0]=rzastep[0];dlawiado[4]=zastep[6];}
       else {str2[2]++;ktop[0]=1;dlawiado[0]=zastep[0];dlawiado[4]=rzastep[6];}
       wiado[1]=56;ktop[1]=pilka;
       wiado[2]=78;ktop[2]=pilka;
       gdzie=3;
      }//r¢g
     else if (co==28)//GOOL
      {
       los=random(2);taktyka=1;
       if (los==0) wiado[0]=65;
       else wiado[0]=66;
       wiado[1]=74;ktop[1]=pilka;ktop[2]=pilka;
       los=random(2);
       if (los==0) wiado[2]=75;
       else wiado[2]=76;
       wiado[3]=22;los=random(11)+1;
	 f=fopen("wymiana.cfg","wb");
	 fclose(f);
	 if (pilka==1) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==pam+1&&x1==gracz.dane[22])
	    {gracz.dane[21]++;gracz.dane[20]++;gracz.dane[8]+=3;
	     if (gracz.dane[8]>=5) {gracz.dane[8]=0;gracz.dane[7]++;}
	     if (gracz.dane[7]>3) gracz.dane[7]=3;
	     if (pilka==1) Klub.finanse[7]+=gracz.waga[2];
	    }
	   if (gracz.dane[0]==los&&x1==gracz.dane[22]) gracz.dane[20]++;
	   f2=fopen("wymiana.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	 if (pilka==1) przepisz();
	 else
	  {
	   f=fopen("Rywal.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Rywal.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	  }
       if (pilka==1)
	{dlawiado[0]=rzastep[0];dlawiado[2]=krajMS[Klub.klub-1];
	 dlawiado[4]=zastep[pam];dlawiado[6]=krajMS[Klub.rywal[0]-1];
	 pilka=2;gol1++;ktop[3]=2;ktop[0]=2;str[1]++;
	 dlagol1[gol1-1]=zastep[pam];
	 x1=minuta+2;
	 if (x1>45&&(koniec==0||koniec==1)) x1=45;
	 if (x1>90) x1=90;
	 mingol1[gol1-1]=x1;
	}
       else
	{dlawiado[0]=zastep[0];dlawiado[2]=krajMS[Klub.rywal[0]-1];
	 dlawiado[4]=rzastep[pam];dlawiado[6]=krajMS[Klub.klub-1];
	 pilka=1;gol2++;ktop[3]=1;ktop[0]=1;str2[1]++;
	 dlagol2[gol2-1]=rzastep[pam];
	 x1=minuta+2;
	 if (x1>45&&(koniec==0||koniec==1)) x1=45;
	 if (x1>90) x1=90;
	 mingol2[gol2-1]=x1;
	}
       gdzie=1;
       los=random(11)+1;
	 f=fopen("wymiana.cfg","wb");
	 fclose(f);
	 if (pilka==1) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==1&&x1==gracz.dane[22])
	    {gracz.dane[20]--;gracz.dane[8]-=3;
	     if (gracz.dane[8]<-5) {gracz.dane[8]=0;gracz.dane[7]--;}
	     if (gracz.dane[7]<-3) gracz.dane[7]=-3;}
	   if (gracz.dane[0]==los&&x1==gracz.dane[22]) gracz.dane[20]--;
	   f2=fopen("wymiana.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	 if (pilka==1) przepisz();
	 else
	  {
	   f=fopen("Rywal.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Rywal.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	  }
      }//gool
     else if (co==29)//nieuznany
      {
       los=random(2);taktyka=1;
       if (los==0) wiado[0]=65;
       else wiado[0]=66;
       wiado[1]=74;ktop[1]=pilka;ktop[2]=pilka;
       los=random(2);
       if (los==0) wiado[2]=75;
       else wiado[2]=76;
       wiado[3]=77;ktop[3]=pilka;
       wiado[4]=41;
       if (pilka==1)
	{dlawiado[0]=rzastep[0];dlawiado[2]=krajMS[Klub.klub-1];
	 dlawiado[4]=zastep[pam];dlawiado[8]=rzastep[0];
	 pilka=2;ktop[4]=2;ktop[0]=2;str[0]--;}
       else
	{dlawiado[0]=zastep[0];dlawiado[2]=krajMS[Klub.rywal[0]-1];
	 dlawiado[4]=rzastep[pam];dlawiado[8]=zastep[0];
	 pilka=1;ktop[4]=1;ktop[0]=1;str2[0]--;}
       gdzie=1;
      }//gool
     else if (co==30)//niecelnie
      {
       los=random(4);taktyka=1;
       if (los==0) wiado[0]=79;
       else if (los==1) wiado[0]=80;
       else if (los==2) wiado[0]=81;
       else wiado[0]=82;
       ktop[0]=pilka;
       if (pilka==1) {dlawiado[0]=zastep[pam];pilka=2;dlawiado[2]=rzastep[0];}
       else {dlawiado[0]=rzastep[pam];pilka=1;dlawiado[2]=zastep[0];}
       wiado[1]=41;ktop[1]=pilka;
       gdzie=1;
      }//niecelnie
    }//obrona B gdzie=4;
   textcolor(7);//blokada=0;//wiado[0]=10;wiado[1]=11;wiado[2]=12;wiado[3]=13;
   if (minuta==0&&koniec==0)//pocz•tek meczu
    {
     ktoZacz=random(2)+1;
     wiado[0]=1;//mecz si© rozpocz•à
     if (ktoZacz==1) {dlawiado[0]=krajMS[Klub.klub-1];pilka=1;}//zaczyna gracz
     else {dlawiado[0]=krajMS[Klub.rywal[0]-1];pilka=2;}
     gdzie=1;ktop[0]=pilka;
     for (i=1;i<10;i++) wiado[i]=0;
    }//dla minuta=0 pocz•tek meczu
   else if (minuta==45&&koniec==2)//pocz•tek 2 poàowy
    {
     if (ktoZacz==1) {dlawiado[0]=krajMS[Klub.rywal[0]-1];pilka=2;}//zaczyna rywal
     else {dlawiado[0]=krajMS[Klub.klub-1];pilka=1;}
     wiado[0]=2;gdzie=1;ktop[0]=pilka;
     for (i=1;i<10;i++) wiado[i]=0;
    }//dla pocz•tek 2 poàowy
   else if (koniec==1)//koniec 1 poàowy
    {
     taktyka=1;minuta=43;wiado[0]=3;koniec=2;start=0;gdzie=1;
     for (i=1;i<10;i++) wiado[i]=0;
    }
   /*else if (koniec==3&&blokada==0)//koniec meczu
    {
     taktyka=1;blokada=1;wiado[0]=4;start=0;koniec=3;
     for (i=1;i<10;i++) wiado[i]=0;
    }  */
   //************************* kartki dla nieposiadacza piàki *************************
   /*if (kartki==1)
    {kartki=0;
       if (pilka==1) {x1=Klub.rinst[1];x2=Klub.rywal[2];str2[7]++;}
       else {x1=Klub.inst[1];x2=Klub.usta;str[7]++;}
       los=random(6);
       if (x1==2)//delikatne
	{
	 if (los==0||los==1||los==2) co=1;//nie ma kartki
	 else if (los==3||los==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartka
	}
       else if (x1==1)//normalne
	{
	 if (los==0||los==1) co=1;//nie ma kartki
	 else if (los==3||los==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartkaif (los==5||los==2)
	}
       else if (x1==3)//twarde
	{
	 if (los==0) co=1;//nie ma kartki
	 else if (los==3||los==4) co=2;//sàowne upomienie
	 else co=3;//æ¢lta kartkaif (los==5||los==2||los==1)
	}
       if (x2==1||x2==2||x2==3||x2==4) los=random(4)+6;
       else if (x2==5) los=random(4)+5;
       else if (x2==6||x2==7||x2==8) los=random(5)+5;
       else if (x2==9) los=random(2)+6;
       else if (x2==10) los=random(3)+6;
       else if (x2==11) los=random(5)+6;
       else if (x2==12||x2==13||x2==14) los=random(3)+7;
       if (pilka==1) {dlawiado[2]=rzastep[los-1];ktop[1]=2;ktop[2]=2;}
       else {dlawiado[2]=zastep[los-1]; ktop[1]=1;ktop[2]=1;}
       if (co==3)//jest æ¢àta
	{
	 f=fopen("wymiana.cfg","wb");
	 fclose(f);
	 if (pilka==2) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22])
	    {gracz.dane[12]++;gracz.dane[7]--;
	     if (gracz.dane[7]<-3) gracz.dane[7]=-3;}
	   f2=fopen("wymiana.cfg","ab");
	   fwrite(&gracz,sizeof(gracz),1,f2);
	   fclose(f2);
	  }
	 fclose(f);
	 if (pilka==2) przepisz();
	 else
	  {
	   f=fopen("Rywal.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Rywal.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	  }
	 k=0;
	 if (pilka==2) {f=fopen("Gra_mana.cfg","rb");x1=Klub.klub;}
	 else {f=fopen("Rywal.cfg","rb");x1=Klub.rywal[0];}
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[0]==los&&x1==gracz.dane[22]&&gracz.dane[12]>=2)
	    k=1;
	  }
	 fclose(f);
	 if (k==1)
	  {wiado[3]=60;
	   if (pilka==1) {str2[5]++;str2[4]++;}
	   else {str[5]++;str[4]++;}
	  }
	 else
	  {wiado[3]=20;
	   if (pilka==1) str2[4]++;
	   else str[4]++;
	  }
	 if (pilka==1) {dlawiado[6]=rzastep[los-1];ktop[3]=2;}
	 else {dlawiado[6]=zastep[los-1];ktop[3]=1;}
	}// jest æ¢àta end
       else if (co==2) //sàownie
	{
	 wiado[3]=21;
	 if (pilka==1) {dlawiado[6]=rzastep[los-1];ktop[3]=2;}
	 else {dlawiado[6]=zastep[los-1];ktop[3]=1;}
	}//sàownie end
    }        */
   //************************ kontuzja *************************
   los=random(100);
   if (los==0)
    {
     x1=random(2);
     los=random(11)+1;
     f=fopen("wymiana.cfg","wb");
     fclose(f);
     if (x1==0) {f=fopen("Gra_mana.cfg","rb");x2=Klub.klub;}
     else {f=fopen("Rywal.cfg","rb");x2=Klub.rywal[0];}
     while(fread(&gracz,sizeof(gracz),1,f)==1)
      {
       if (gracz.dane[0]==los&&gracz.dane[22]==x2&&gracz.dane[12]<2)
	{gracz.dane[11]=0;gracz.dane[19]=1;
	 for (i=10;i>0;i--)
	  if (wiado[i]==0) k=i;//break;}}
	 if (x1==0) wiado[k]=83;
	 else wiado[k]=84;
	 if (x1==0) {ktop[k]=1;dlawiado[k*2]=zastep[los-1];}
	 else {ktop[k]=2;dlawiado[k*2]=rzastep[los-1];}
	}
       f2=fopen("wymiana.cfg","ab");
       fwrite(&gracz,sizeof(gracz),1,f2);
       fclose(f2);
      }
     fclose(f);
     if (x1==0) przepisz();
     else
      {
       f=fopen("Rywal.cfg","wb");
       fclose(f);
       f=fopen("wymiana.cfg","rb");
       while(fread(&gracz,sizeof(gracz),1,f)==1)
	{
	 f2=fopen("Rywal.cfg","ab");
	 fwrite(&gracz,sizeof(gracz),1,f2);
	 fclose(f2);
	}
       fclose(f);
      }
    }
   //************************ kontuzja end *************************
   //******************** zamina zawonika kontuzjowanego ***********
   if (taktyka==1)
   {
   x1=0;x3=0;
   f=fopen("Rywal.cfg","rb");
   while(fread(&gracz,sizeof(gracz),1,f)==1)
    {
     if (gracz.dane[11]==0&&Klub.rywal[0]==gracz.dane[22]&&gracz.dane[0]<12)
      {x1=gracz.dane[0];x2=gracz.dane[2];}//x2=pozycja
    }
   fclose(f);
   for (i=0;i<3;i++)
    {if (juzzmienil[i]==0){juzzmienil[i]=x1;break;}
    }
   for (i=0;i<3;i++)
    {if (juzzmienil[i]==x1) x3++;
    }
   if (x3>1)
    {
     for (i=2;i>=0;i--)
      {
       if (juzzmienil[i]==x1) {juzzmienil[i]=0;break;}
      }
     x1=0;
    }
   if (x1>0)
    {
     f=fopen("Rywal.cfg","rb");
     while(fread(&gracz,sizeof(gracz),1,f)==1)
      {
       if (gracz.dane[0]>11&&gracz.dane[0]<17&&Klub.rywal[0]==gracz.dane[22]&&x2==gracz.dane[2])
	x3=gracz.dane[0];
       f2=fopen("wymiana.cfg","ab");
       fwrite(&gracz,sizeof(gracz),1,f2);
       fclose(f2);
      }
     fclose(f);
     f=fopen("wymiana.cfg","wb");
     fclose(f);
     f=fopen("Rywal.cfg","rb");
     while(fread(&gracz,sizeof(gracz),1,f)==1)
      {
       if (gracz.dane[0]==x1&&Klub.rywal[0]==gracz.dane[22]) gracz.dane[0]=x3;
       else if (gracz.dane[0]==x3&&Klub.rywal[0]==gracz.dane[22]) gracz.dane[0]=x1;
       f2=fopen("wymiana.cfg","ab");
       fwrite(&gracz,sizeof(gracz),1,f2);
       fclose(f2);
      }
     fclose(f);
     f=fopen("Rywal.cfg","wb");
     fclose(f);
     f=fopen("wymiana.cfg","rb");
     while(fread(&gracz,sizeof(gracz),1,f)==1)
      {
       f2=fopen("Rywal.cfg","ab");
       fwrite(&gracz,sizeof(gracz),1,f2);
       fclose(f2);
      }
     fclose(f);
     dlazmiana=2;czyzmiana=1;
    }}//dla if taktyka==1
   //************************************************
   if (dlazmiana>0)//zmiana zawodnika**************
    {
     for (i=10;i>0;i--)
      if (wiado[i]==0) k=i;
     wiado[k]=5;
     if (dlazmiana==1) {ktop[k]=1;dlawiado[k*2]=krajMS[Klub.klub-1];}
     else {ktop[k]=2;dlawiado[k*2]=krajMS[Klub.rywal[0]-1];
	   wiado[k+1]=7;ktop[k+1]=2;
	   dlawiado[(k+1)*2]=rzastep[x3-1];
	   dlawiado[(k+1)*2+1]=rzastep[x1-1];}
     dlazmiana=0;
    }
   //*************** zadyma **********************
   x1=random(100);
   if (x1==1&&gdzie==1)
    {
     for (i=10;i>0;i--)
      if (wiado[i]==0) k=i;
     wiado[k]=86;wiado[k+1]=87;wiado[k+2]=88;wiado[k+3]=89;
     if (Klub.rywal[1]==0)//u mnie
      {
       ktop[k]=1;ktop[k+1]=1;ktop[k+2]=1;ktop[k+3]=1;
       Klub.zadyma=1;
       f=fopen("Klub.cfg","wb");
       fwrite(&Klub,sizeof(Klub),1,f);
       fclose(f);
      }
     else
      {
       ktop[k]=2;ktop[k+1]=2;ktop[k+2]=2;ktop[k+3]=2;
      }
    }
   //*********************************
   for (i=0;i<10;i++)
    {
     k=i*2;
     if (wiado[i]!=0) cputs("\n\r");
     f=fopen("Mecz.cfg","rb");
     while(fread(&meczs,sizeof(meczs),1,f)==1)
      {
       if (wiado[i]==meczs.num)
	{
	 if (wiado[i]==74) textcolor(143);
	 else if (wiado[i]==20) textcolor(14);
	 else if (wiado[i]==60) textcolor(12);
	 else if (wiado[i]==83||wiado[i]==84||wiado[i]==86||wiado[i]==87||wiado[i]==88||wiado[i]==89) textcolor(10);
	 else textcolor(7);
	 if (ktop[i]==1) textbackground(9);
	 else textbackground(4);
	 cprintf(meczs.newss,dlawiado[k],dlawiado[k+1]);
	 f2=fopen("Lostmecz.cfg","aw");
	 fprintf(f2,"\n");
	 fprintf(f2,"%02d min. ",minuta+2);
	 fprintf(f2,meczs.newss,dlawiado[k],dlawiado[k+1]);
	 fclose(f2);
	}}
     fclose(f);
     if (wiado[i]!=0)//&&wiado[i+1]!=0)
      {
       if (Klub.mecz==1)
       {
       gettime(&t);
       k=t.ti_sec;
       if (czas==1)
	{
	 if (k==59) k=0;
	 else k+=czas;
	}
       else if (czas==2)
	{
	 if (k==58) k=0;
	 else if (k==59) k=1;
	 else k+=czas;
	}
       else if (czas==3)
	{
	 if (k==57) k=0;
	 else if (k==58) k=1;
	 else if (k==59) k=2;
	 else k+=czas;
	}
       else if (czas==4)
	{
	 if (k==56) k=0;
	 else if (k==57) k=1;
	 else if (k==58) k=2;
	 else if (k==59) k=3;
	 else k+=czas;
	}
       while(t.ti_sec!=k)
	{
	 gettime(&t);
	 if (kbhit()&&taktyka==1) taktyka2=1;
	}//menu=getch();
	}//dla Klub.mecz==1
       else getch();
      }
     //if (wiado[i]!=0&&wiado[i+1]!=0) getch();
    }
   //getch();
   textbackground(0);
   for (i=0;i<10;i++)
    wiado[i]=0;
   minuta+=2;
   minuta2+=2;
   //****************** posiadanie piàki i strefy **********************
   if (pilka==1) pos++;
   pos1=(pos*100)/(minuta2/2);
   pos2=100-pos1;
   //if (minuta>45&&(koniec==0||koniec==1)) minuta2++;
   if (gdzie==1) strefa[1]++;
   else
    {
     if (pilka==1) strefa[2]++;
     else strefa[0]++;
    }
   strefa[3]=(strefa[0]*100)/(minuta2/2);
   strefa[4]=(strefa[1]*100)/(minuta2/2);
   strefa[5]=(strefa[2]*100)/(minuta2/2);
   //***************** posiadanie i strefy end *********************
   //***************** kondycja ***************************
     f=fopen("wymiana.cfg","wb");
     fclose(f);
     if (Klub.inst[2]==1) k=2;
     else k=3;
     f=fopen("Gra_mana.cfg","rb");
     while(fread(&gracz,sizeof(gracz),1,f)==1)
      {
       los=random(k)+1; if (los>1) los=0;
       if (Klub.klub==gracz.dane[22]&&gracz.dane[0]<12) gracz.dane[11]-=los;
       if (gracz.dane[11]<0) gracz.dane[11]=0;
       f2=fopen("wymiana.cfg","ab");
       fwrite(&gracz,sizeof(gracz),1,f2);
       fclose(f2);
      }
     fclose(f);
     przepisz();
     f=fopen("wymiana.cfg","wb");
     fclose(f);
     if (Klub.rinst[2]==1) k=2;
     else k=3;
     f=fopen("Rywal.cfg","rb");
     while(fread(&gracz,sizeof(gracz),1,f)==1)
      {
       los=random(k)+1; if (los>1) los=0;
       if (Klub.rywal[0]==gracz.dane[22]&&gracz.dane[0]<12) gracz.dane[11]-=los;
       if (gracz.dane[11]<0) gracz.dane[11]=0;
       f2=fopen("wymiana.cfg","ab");
       fwrite(&gracz,sizeof(gracz),1,f2);
       fclose(f2);
      }
     fclose(f);
       f=fopen("Rywal.cfg","wb");
       fclose(f);
       f=fopen("wymiana.cfg","rb");
       while(fread(&gracz,sizeof(gracz),1,f)==1)
	{
	 f2=fopen("Rywal.cfg","ab");
	 fwrite(&gracz,sizeof(gracz),1,f2);
	 fclose(f2);
	}
       fclose(f);
  }//dla start=1
  //getch();
 //************ wypisz wiadomoòÜ *************
 //if (gdzie==1) taktyka=1;
 //else taktyka=0;
 if (minuta>45&&gdzie==1&&koniec==0&&q==0)
  {koniec=1;minuta=45;q=1;gdzie=1;}//koniec1 poàowy
 if (minuta>90&&gdzie==1&&koniec==2) koniec=3;
 if (koniec==4) //cputs("\n\rKONIEC MECZU");
  { cputs("\n\rKONIEC MECZU");
    f=fopen("001.cfg","w");
    fprintf(f,"%d %d %d",OnaA,PnaP,AnaO);
    fclose(f);
    /*f=fopen("Form.cfg","w");
    fprintf(f,"%d %d %d",OnaA,PnaP,AnaO);
    fclose(f); */
  }
 textcolor(2);
 cputs("\n\r");
 if (start==0) cputs("\n\rS Start mecz");
 if (Klub.mecz==1&&(koniec==0||koniec==2||koniec==1||koniec==4))
 cprintf("\n\rA Taktyka-%s  P Taktyka-%s  C SzybkoòÜ-%s",krajMS[Klub.klub-1],krajMS[Klub.rywal[0]-1],speed[czas]);
 if (Klub.mecz==2)
 cprintf("\n\rA Taktyka - %s    P Taktyka - %s",krajMS[Klub.klub-1],krajMS[Klub.rywal[0]-1]);
 //cprintf("  C SzybkoòÜ-%s",speed[czas]);
 if (koniec==4) {textcolor(4); cputs("\n\rQ Wyjòcie");}
 //menu='y';
 if (Klub.mecz==1&&(start==0||taktyka2==1||koniec==4)) menu=getch();//koniec==1
 else if (Klub.mecz==2&&(start==0||taktyka==1||koniec==1||koniec==4)) menu=getch();
 else menu='y';
 if (koniec==3) koniec=4;
 taktyka=0;taktyka2=0;k=0;
 //if (koniec==3||koniec==1) menu=getch();
 if (walkower<11)
  {
   end='Q';
   textcolor(12);
   cputs("\n\rPrzegrywasz walkowerem 0-3");
   gol1=0;gol2=3;
  }
 switch(toupper(menu))
  {
   case 'C':
    //taktyka=1;
    clrscr();textcolor(2);
    cputs("\n\rPodaj szybkoòÜ wyòwietlania napis¢w:\n\r0. Bardzo szybko\n\r1. Szybko\n\r2. órednio\n\r3. Wolno\n\r4. Bardzo wolno\n\r");
    scanf("%d",&czas);
    if (czas<0||czas>4) czas=2;
    break;
   case 'Q':
    if (koniec==4) end='Q';
    else end='s';
   case 'S':
    if (start==0) start=1;
    break;
   case 'A':
   //if (taktyka==1)
    //{
     if (koniec<3) start=0;//if (koniec!=3)
     //taktyka=1;
     do
      {
	   clrscr();textcolor(15);cprintf(" %s",krajMS[Klub.klub-1]);
	   takty(Klub.usta,0);
	   //wykres(Klub.usta,0);
	   //tryb=11;//cena=0;
	   sklad(Klub.usta,11,0);
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case 'U':
	      k=0;
	      while(!k)
	      {
	      clrscr();textbackground(0);textcolor(12);
	      cprintf("Obecne ustawienie: %s",tak[Klub.usta-1]);textcolor(7);
	      cputs("\n\rWybierz ustawienie zespoàu:\n\r 1. 4-4-2\n\r 2. 4-4-2 Obrona\n\r 3. 4-4-2 Atak\n\r 4. 4-4-2 Diamond\n\r 5. 3-4-3\n\r 6. 3-5-2\n\r 7. 3-5-2 Obrona\n\r 8. 3-5-2 Atak\n\r 9. 4-2-4\n\r10. 4-3-3\n\r11. 4-5-1\n\r12. 5-3-2\n\r13. 5-3-2 Obrona\n\r14. 5-3-2 Atak\n\r\n\rWpisz odpowiedni• cyfr©: ");
	      scanf("%d",&Klub.usta);
	      if (Klub.usta<15&&Klub.usta>0) k=1;
	      else Klub.usta=15;
	      f=fopen("Klub.cfg","wb");
	      fwrite(&Klub,sizeof(Klub),1,f);
	      fclose(f);
	      }
	      break;
	     case 'P':
	      //zamiana();
	      textcolor(7);cputs("\n\rPodaj numer, spacja, drugi numer i Enter: ");
	      scanf("%d %d",&k,&i);
	      f=fopen("Gra_mana.cfg","rb");
	      while(fread(&gracz,sizeof(gracz),1,f)==1)
	       {
		if ((gracz.dane[0]==k&&gracz.dane[12]>1)||(gracz.dane[0]==i&&gracz.dane[12]>1)||(gracz.dane[0]==k&&gracz.dane[13]>1)||(gracz.dane[0]==i&&gracz.dane[13]>1)||(gracz.dane[0]==k&&gracz.dane[15]>7)||(gracz.dane[0]==i&&gracz.dane[15]>5)||(gracz.dane[0]==k&&gracz.dane[14]==1)||(gracz.dane[0]==i&&gracz.dane[14]==1))
		 {k=1;i=1;}
	       }
	      fclose(f);
	      if (k<1||k>16||i<1||i>16||zmiana>=3) {k=1;i=1;}
	      //if (((k<1&&i>11)||(i<1&&k>11))&&k!=i) zmiana++;
	      if (k!=i)
	       {
		if ((k<12&&i>11)||(i<12&&k>11))
		 { zmiana++;ktop[9]=1;dlazmiana=1;wiado[9]=6;
		   if (k<12&&i>11)
		   {dlawiado[18]=zastep[k-1];dlawiado[19]=zastep[i-1];}
		   if (i<12&&k>11)
		   {dlawiado[18]=zastep[i-1];dlawiado[19]=zastep[k-1];}
		 }
		f=fopen("wymiana.cfg","wb");
		fclose(f);//szyszczenie starej
		f=fopen("Gra_mana.cfg","rb");
		while(fread(&gracz,sizeof(gracz),1,f)==1)
		 {
		  if (gracz.dane[0]==k) gracz.dane[0]=i;
		  else if (gracz.dane[0]==i) gracz.dane[0]=k;
		  f2=fopen("wymiana.cfg","ab");
		  fwrite(&gracz,sizeof(gracz),1,f2);
		  fclose(f2);
		 }
		fclose(f);
		przepisz();
		}//dla if
	      licz=-1;
	      f=fopen("Gra_mana.cfg","rb");
	      while(fread(&gracz,sizeof(gracz),1,f)==1)
	       {
		licz++;
		kol[licz]=gracz.dane[0];
	       }
	      fclose(f);
	      for(i=0;i<licz+1;i++)//sortowanie wg. pkt
	       {
		//blokada[i]=0;
		for(k=i+1;k<licz+1;k++)
		 {
		  if (kol[i]>kol[k])
		   {
		   numer1=kol[i];//pkt2 numery klub¢w, zamieniam wg. kolejnoòci
		   kol[i]=kol[k];
		   kol[k]=numer1;
		  }}}
	      f=fopen("wymiana.cfg","wb");
	      fclose(f);//szyszczenie starej
	      for(i=0;i<licz+1;i++)
	       {
		f=fopen("Gra_mana.cfg","rb");
		while(fread(&gracz,sizeof(gracz),1,f)==1)
		 {
		  if (kol[i]==gracz.dane[0])
		   {
		    f2=fopen("wymiana.cfg","ab");
		    fwrite(&gracz,sizeof(gracz),1,f2);
		    fclose(f2);
		   }}
		fclose(f);
	       }
	      przepisz();
	      czyzmiana=1;
	      break;
	     case 'R':
	      clrscr();textcolor(15);cprintf("REZERWOWI - %s",krajMS[Klub.klub-1]);
	      //tryb=16;cena=0;
	      sklad(Klub.usta,16,0);
	      textcolor(7);cputs("\n\r\n\rNaciònij dowolny klawisz...");
	      getch();
	      break;
	     case 'I':
	      belka=1;
	      do
	      {
	      clrscr();textcolor(15);cprintf("INSTRUKCJE DLA DRUΩYNY - %s",krajMS[Klub.klub-1]);
	      instrukcje(Klub.inst[0],Klub.inst[1],Klub.inst[2],Klub.inst[3],Klub.inst[4],Klub.inst[5],belka);
	      textcolor(4);cputs("\n\r Q Powr¢t\n\r");textcolor(2);
	      pp_usta2=getch();
	      switch(toupper(pp_usta2))
	       {
		case UP:
		 belka--;
		 if (belka==0) belka=6;
		 break;
		case DOWN:
		 belka++;
		 if (belka==7) belka=1;
		 break;
		case RIGHT:
		 if (belka==1)
		  {
		   Klub.inst[5]++;
		   if (Klub.inst[5]==4) Klub.inst[5]=1;
		  }
		 else if (belka==2)
		  {
		   Klub.inst[0]++;
		   if (Klub.inst[0]==5) Klub.inst[0]=1;
		  }
		 else if (belka==3)
		  {
		   Klub.inst[1]++;
		   if (Klub.inst[1]==4) Klub.inst[1]=1;
		  }
		 else if (belka==4)
		  {
		   Klub.inst[2]++;
		   if (Klub.inst[2]==2) Klub.inst[2]=0;
		  }
		 else if (belka==5)
		  {
		   Klub.inst[3]++;
		   if (Klub.inst[3]==2) Klub.inst[3]=0;
		  }
		 else if (belka==6)
		  {
		   Klub.inst[4]++;
		   if (Klub.inst[4]==2) Klub.inst[4]=0;
		  }
		 break;
		case LEFT:
		 if (belka==1)
		  {
		   Klub.inst[5]--;
		   if (Klub.inst[5]==0) Klub.inst[5]=3;
		  }
		 else if (belka==2)
		  {
		   Klub.inst[0]--;
		   if (Klub.inst[0]==0) Klub.inst[0]=4;
		  }
		 else if (belka==3)
		  {
		   Klub.inst[1]--;
		   if (Klub.inst[1]==0) Klub.inst[1]=3;
		  }
		 else if (belka==4)
		  {
		   Klub.inst[2]--;
		   if (Klub.inst[2]==-1) Klub.inst[2]=1;
		  }
		 else if (belka==5)
		  {
		   Klub.inst[3]--;
		   if (Klub.inst[3]==-1) Klub.inst[3]=1;
		  }
		 else if (belka==6)
		  {
		   Klub.inst[4]--;
		   if (Klub.inst[4]==-1) Klub.inst[4]=1;
		  }
		 break;
	       }
	      f=fopen("Klub.cfg","wb");
	      fwrite(&Klub,sizeof(Klub),1,f);
	      fclose(f);
	       }
	      while(toupper(pp_usta2)!='Q');
	      break;
	    }}
	   while(toupper(pp_usta)!='Q');
    //}//dla taktyka
    break;
   case 'P':
   //if (taktyka==1)
    //{
    //taktyka=1;
    if (koniec<3) start=0;//if (koniec!=3) start=0;
       do
       {
       clrscr();textcolor(15);cprintf(" %s",krajMS[Klub.rywal[0]-1]);
	   takty(Klub.rywal[2],Klub.rywal[0]);
	   //wykres(Klub.usta,0);
	   //tryb=11;//cena=0;
	   sklad(Klub.rywal[2],11,Klub.rywal[0]);
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case 'R':
	      clrscr();textcolor(15);cprintf("REZERWOWI - %s",krajMS[Klub.rywal[0]-1]);
	      //tryb=16;cena=0;
	      sklad(Klub.rywal[2],16,Klub.rywal[0]);
	      textcolor(7);cputs("\n\r\n\rNaciònij dowolny klawisz...");
	      getch();
	      break;
	     case 'I':
	      clrscr();textcolor(15);cprintf("INSTRUKCJE DLA DRUΩYNY - %s",krajMS[Klub.rywal[0]-1]);
	      instrukcje(Klub.rinst[0],Klub.rinst[1],Klub.rinst[2],Klub.inst[3],Klub.rinst[4],Klub.rinst[5],0);
	      textcolor(7);cputs("\n\r\n\rNaciònij dowolny klawisz...");
	      getch();
	      break;
	    }}
	   while(toupper(pp_usta)!='Q');
    //}//dla taktyka
    break;
 }}
 while(end!='Q');
 textcolor(7);
		 cputs("\n\rProsz© czekaÜ...");
		 if (Klub.rywal[1]==0)
		  {
		   x1=random(30)+30;
		   h=x1*5000.0;
		   Klub.finanse[0]+=h;//bilety
		   x2=random(30)+30;
		   h=x2*5000.0;
		   Klub.finanse[1]+=h;//Tv
		   k=random(10)+1;
		   Klub.finanse[2]+=k*1000;//handel
		   Klub.finanse[4]+=300000;
		   //getch();
		  }
		 else Klub.finanse[4]+=50000;
		 Klub.finanse[5]=Klub.finanse[0]+Klub.finanse[1]+Klub.finanse[2]+Klub.finanse[3]+Klub.finanse[4];
		 Klub.finanse[10]=Klub.finanse[6]+Klub.finanse[7]+Klub.finanse[8]+Klub.finanse[9];
		 Klub.finanse[11]=Klub.finanse[5]-Klub.finanse[10];
		 Klub.jestM=0;//bylM=1;
		 Klub.gol1=gol1;Klub.gol2=gol2;Klub.asystent=1;
		 //******** manager stats ************
		 Klub.dane_m[4]++;
		 if (gol1>gol2) Klub.dane_m[5]++;
		 else if (gol1==gol2) Klub.dane_m[6]++;
		 else Klub.dane_m[7]++;
		 Klub.dane_m[8]+=gol1;Klub.dane_m[9]+=gol2;
		 Klub.dane_m[3]=(Klub.dane_m[0]*100)+(Klub.dane_m[2]*10)+(Klub.dane_m[4]*10)+(Klub.dane_m[5]*3)+Klub.dane_m[6]-Klub.dane_m[7]+Klub.dane_m[8]-Klub.dane_m[9];
		 //*******************************
		 f=fopen("Klub.cfg","wb");
		 fwrite(&Klub,sizeof(Klub),1,f);
		 fclose(f);
		 if (Klub.kolejka<=0)//mecze kontrolne
		  {
		   k=Klub.kolejka+4;
		   /*f=fopen("001.cfg","r");
		   fscanf(f,"%d %d",&sumaO,&sumaN);
		   fclose(f); */
		   Klub.golK[k*2]=gol1;
		   Klub.golK[k*2+1]=gol2;
		   f=fopen("Klub.cfg","wb");
		   fwrite(&Klub,sizeof(Klub),1,f);
		   fclose(f);
		  }
		 else if (Klub.kolejka>0)
		 {
		 k=-2;
		 while(k!=14)//wszystkie mecze kolejki
		  {
		   k+=2;
		   sumaO=Klub.kolejka*16;
		   sumaP=random(5);sumaN=random(4);
		   Klub.gol[sumaO-16+k]=sumaP;
		   Klub.gol[sumaO-16+k+1]=sumaN;
		   //for(i=sumaO;i<120;i++) gol[sumaO+i]=0;
		  }
		 //for(i=sumaO;i<120;i++) Klub.gol[i]=0;
		 f=fopen("Kolejka2.cfg","rb");
		 while(fread(&kolej,sizeof(kolej),1,f)==1)
		  {
		   if (Klub.kolejka==kolej.numer)
		    {
		     for (i=0;i<16;i++)
		      if (Klub.klub==kolej.nr[i]) sumaO=i;
		  }}
		 fclose(f);
		 sumaP=Klub.kolejka*16;
		 if (sumaO%2==0)
		  {
		   /*f=fopen("001.cfg","r");
		   //fscanf(f,"%d %d",&Klub.gol[sumaP-16+sumaO],&Klub.gol[sumaP-16+sumaO+1]);
		   fscanf(f,"%d %d",&sumaN,&k);
		   fclose(f);*/
		   Klub.gol[sumaP-16+sumaO]=gol1;
		   Klub.gol[sumaP-16+sumaO+1]=gol2;
		  }
		 else
		  {
		   /*f=fopen("001.cfg","r");
		   //fscanf(f,"%d %d",&Klub.gol[sumaP-16+sumaO],&Klub.gol[sumaP-16+sumaO-1]);
		   fscanf(f,"%d %d",&sumaN,&k);
		   fclose(f);  */
		   Klub.gol[sumaP-16+sumaO]=gol1;
		   Klub.gol[sumaP-16+sumaO-1]=gol2;
		  }
		 //Klub.asystent=1;
		 f=fopen("Klub.cfg","wb");
		 fwrite(&Klub,sizeof(Klub),1,f);
		 fclose(f);
		 //********* przypisanie do tabeli ***************
		 f=fopen("wymiana.cfg","wb");
		 fclose(f);
		 f=fopen("Kolejka2.cfg","rb");
		 while(fread(&kolej,sizeof(kolej),1,f)==1)
		  {
		   if (Klub.kolejka==kolej.numer)
		    {
		     for (i=0;i<16;i++)
		      {
		       f2=fopen("Tabela.cfg","rb");
		       while(fread(&tabela,sizeof(tabela),1,f2)==1)
			{
			 if (kolej.nr[i]==tabela.num)
			 {
			 sumaO=i;
			 tabela.dane[0]++;
			 if (sumaO%2==0)
			  {
			   tabela.dane[4]+=Klub.gol[sumaP-16+sumaO];
			   tabela.dane[5]+=Klub.gol[sumaP-16+sumaO+1];
			   if (Klub.gol[sumaP-16+sumaO]>Klub.gol[sumaP-16+sumaO+1])
			    {
			     tabela.dane[1]++;tabela.dane[7]+=3;
			    }
			   else if (Klub.gol[sumaP-16+sumaO]==Klub.gol[sumaP-16+sumaO+1])
			    {
			     tabela.dane[2]++;tabela.dane[7]++;
			    }
			   else tabela.dane[3]++; //przegrana
			  }
			 else //nieparzysta
			  {
			   tabela.dane[4]+=Klub.gol[sumaP-16+sumaO];
			   tabela.dane[5]+=Klub.gol[sumaP-16+sumaO-1];
			   if (Klub.gol[sumaP-16+sumaO]>Klub.gol[sumaP-16+sumaO-1])
			    {
			     tabela.dane[1]++;tabela.dane[7]+=3;
			    }
			   else if (Klub.gol[sumaP-16+sumaO]==Klub.gol[sumaP-16+sumaO-1])
			    {
			     tabela.dane[2]++;tabela.dane[7]++;
			    }
			   else tabela.dane[3]++; //przegrana
			  }
			 tabela.dane[6]=tabela.dane[4]-tabela.dane[5];
			 f3=fopen("wymiana.cfg","ab");
			 fwrite(&tabela,sizeof(tabela),1,f3);
			 fclose(f3);
			 }}
		       fclose(f2);
		      }}}
		 fclose(f);
		 f=fopen("Tabela.cfg","wb");
		 fclose(f);
		 f=fopen("wymiana.cfg","rb");
		 while(fread(&tabela,sizeof(tabela),1,f)==1)
		  {
		   f2=fopen("Tabela.cfg","ab");
		   fwrite(&tabela,sizeof(tabela),1,f2);
		   fclose(f2);
		  }
		 fclose(f);
		 }//else kolejka ligowa
		 //**************
		 f=fopen("wymiana.cfg","wb");
		 fclose(f);
		 f=fopen("Gra_mana.cfg","rb");
		 while(fread(&gracz,sizeof(gracz),1,f)==1)
		  {
		   if (gracz.dane[13]>=2) gracz.dane[13]=0;
		   if (gracz.dane[14]==1) gracz.dane[14]=0;
		   if (gracz.dane[21]>0) {gracz.dane[16]+=gracz.dane[21];gracz.dane[21]=0;}
		   if (gracz.dane[12]>=2) {gracz.dane[14]=1;gracz.dane[12]=0;gracz.dane[13]=0;}
		   if (gracz.dane[12]>0) gracz.dane[13]+=gracz.dane[12];
		   gracz.dane[9]+=gracz.dane[20];gracz.dane[12]=0;
		   if (gracz.dane[9]>10) gracz.dane[9]=10;
		   if (gracz.dane[9]<1) gracz.dane[9]=1;
		   f2=fopen("wymiana.cfg","ab");
		   fwrite(&gracz,sizeof(gracz),1,f2);
		   fclose(f2);
		  }
		 fclose(f);
		 przepisz();
  //******************** tabela ***********************************
	   sumaN=0;
	   while(sumaN!=3)
	   {
	   sumaN++;
	   k=-1;
	   f=fopen("Tabela.cfg","rb");
	   while(fread(&tabela,sizeof(tabela),1,f)==1)
	    {
	     k++;
	     pkt[k]=tabela.dane[7];//zapisuje punkty do pkt
	     pkt2[k]=tabela.num;
	     golr[k]=tabela.dane[6];
	     gol[k]=tabela.dane[4];//gole zdobyte
	     blokada[k]=0;
	    }
	   fclose(f);
	   for(i=0;i<16;i++)//sortowanie wg. pkt
	    {
	     blokada[i]=0;
	     for(k=i+1;k<16;k++)
	      {
	       if (pkt[i]<pkt[k])
		{
		 sumaO=pkt2[i];//pkt2 numery klub¢w, zamieniam wg. kolejnoòci
		 pkt2[i]=pkt2[k];
		 pkt2[k]=sumaO;
		 //**********
		 sumaO=pkt[i];//punkty klub¢w
		 pkt[i]=pkt[k];
		 pkt[k]=sumaO;
		}}}
	   for(i=0;i<16;i++)//sortowanie wg. +/-
	    {
	     for(k=i+1;k<16;k++)
	      {
	       if (golr[i]<golr[k]&&pkt[i]==pkt[k])
		{
		 sumaO=pkt2[i];//pkt2 numery klub¢w, zamieniam wg. kolejnoòci
		 pkt2[i]=pkt2[k];
		 pkt2[k]=sumaO;
		 //**********
		 sumaO=pkt[i];//punkty klub¢w
		 pkt[i]=pkt[k];
		 pkt[k]=sumaO;
		 //**********
		 sumaO=golr[i];//r¢ænica goli
		 golr[i]=golr[k];
		 golr[k]=sumaO;
		}}}
	   for(i=0;i<16;i++)//sortowanie wg. goli zdobytych
	    {
	     for(k=i+1;k<16;k++)
	      {
	       if (gol[i]<gol[k]&&golr[i]==golr[k]&&pkt[i]==pkt[k])
		{
		 sumaO=pkt2[i];//pkt2 numery klub¢w, zamieniam wg. kolejnoòci
		 pkt2[i]=pkt2[k];
		 pkt2[k]=sumaO;
		 //**********
		 sumaO=pkt[i];//punkty klub¢w
		 pkt[i]=pkt[k];
		 pkt[k]=sumaO;
		 //**********
		 sumaO=golr[i];//r¢ænica goli
		 golr[i]=golr[k];
		 golr[k]=sumaO;
		 //***********
		 sumaO=gol[i];//gole zdobyte
		 gol[i]=gol[k];
		 gol[k]=sumaO;
		}}}
	   f=fopen("wymiana.cfg","wb");
	   fclose(f);
	   for(i=0;i<16;i++)
	    {
	     for(k=0;k<16;k++)
	     {
	     for(v=0;v<16;v++)
	     {
	     f=fopen("Tabela.cfg","rb");
	     while(fread(&tabela,sizeof(tabela),1,f)==1)
	      {
	       if (pkt2[i]==tabela.num&&pkt[i]==tabela.dane[7]&&golr[k]==tabela.dane[6]&&gol[v]==tabela.dane[4]&&blokada[i]==0)
		{
		 tabela.dane[8]=i+1;blokada[i]=tabela.num;
		 f2=fopen("wymiana.cfg","ab");
		 fwrite(&tabela,sizeof(tabela),1,f2);
		 fclose(f2);
		}}
	     fclose(f);
	    }}}
	   f=fopen("Tabela.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&tabela,sizeof(tabela),1,f)==1)
	    {
	     f2=fopen("Tabela.cfg","ab");
	     fwrite(&tabela,sizeof(tabela),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	   }//dla while!=2
  textcolor(10);
  cputs("\n\rAby kontynuowaÜ gr©, uruchom plik Menager.exe");
  execl("manager.exe",NULL, NULL);
 }//dla Klub.jestM==1
 else //if (Klub.jestM==0)lub Klub.nie==1
  {
   clrscr();
   textcolor(12);cputs("\n\rTeraz nie moæna rozegraÜ meczu!\n\r\n\rUruchom plik Manager.exe!");
  }
}

void przepisz()
{
 FILE *f,*f2;
 f=fopen("Gra_mana.cfg","wb");
 fclose(f);//szyszczenie starej
 f=fopen("wymiana.cfg","rb");
 while(fread(&gracz,sizeof(gracz),1,f)==1)
  {
   f2=fopen("Gra_mana.cfg","ab");
   fwrite(&gracz,sizeof(gracz),1,f2);
   fclose(f2);
  }
 fclose(f);
}

void sklad(int usta,int tryb,int kto)//,int mecz
{
 FILE *f;
 int i=0,c=0,x;
 char tn,*morale;
 textcolor(2);
 cprintf("\n\rLp.   Zawodnik         Po.  ");
 textcolor(9);
 cputs("B  ");textcolor(5);
 cputs("O  ");textcolor(11);
 cputs("P  ");textcolor(10);
 cputs("N  ");textcolor(2);
 //if (kto==-1) {cputs("Trening");textcolor(7);c=7;}//jeæeli e treningu
 //else
  //{if (mecz==0) cputs("Morale  For. Kon. Gole");
   //else if (mecz==1)
   cputs("Morale  For. ForM Kon. Gole");
   textcolor(9);c=9;//}//normalnie
 if (tryb==16) i=11;//dla pokazania rezerwowych w taktyce
 if (tryb==40) i=20;//dla dalej w Skàadzie
 //if (usta==0) i=16;
 //if (kto>0) i=kto*16-16;//dla pokazania przeciwnika 11-nastki
 //if (kto>0&&tryb==16) i=kto*16-5;//dla rezerwowych przeciwnika
 while(i!=tryb)
  {
   i++;
   if (tryb==16||i==12) {c=6;textcolor(6);}
   if (tryb==40) {c=7;textcolor(7);}
   if (kto>0) {f=fopen("Rywal.cfg","rb");x=kto;}
   else {f=fopen("Gra_mana.cfg","rb");x=Klub.klub;}
   while(fread(&gracz,sizeof(gracz),1,f)==1)
    {
     if (gracz.dane[2]==1) tn='B';
     if (gracz.dane[2]==2) tn='O';
     if (gracz.dane[2]==3) tn='P';
     if (gracz.dane[2]==4) tn='N';
     if (gracz.dane[7]==-3) morale="Fatalne";
     if (gracz.dane[7]==-2) morale="Zàe";
     if (gracz.dane[7]==-1) morale="Niskie";
     if (gracz.dane[7]==0) morale="órednie";
     if (gracz.dane[7]==1) morale="Dobre";
     if (gracz.dane[7]==2) morale="B.dobre";
     if (gracz.dane[7]==3) morale="Super";
     /*if (kto==-1)//dla treningu
      {
       if (gracz.dane[1]==0) {morale="Nie trenuje";textcolor(7);c=7;}
       if (gracz.dane[1]==1) {morale="Bramkarze";textcolor(9);c=9;}
       if (gracz.dane[1]==2) {morale="Obrona";textcolor(5);c=5;}
       if (gracz.dane[1]==3) {morale="Pomoc";textcolor(11);c=11;}
       if (gracz.dane[1]==4) {morale="Atak";textcolor(10);c=10;}
      } */
     if (i==gracz.dane[0]&&x==gracz.dane[22])
      {
       cprintf("\n\r%2d.",gracz.dane[0]);
       /*if (mecz==0)
	{
	 if (gracz.dane[15]>0) {textbackground(4);textcolor(7);cputs("Ko");textbackground(0);textcolor(c);}
	 else if (gracz.dane[14]==1) {textcolor(4);cputs("˛ ");textcolor(c);}
	 else if (gracz.dane[13]==1) {textcolor(14);cputs("˛ ");textcolor(c);}
	 else if (gracz.dane[13]==2) {textcolor(14);cputs("˛˛");textcolor(c);}
	 else if (gracz.dane[17]==1) {textcolor(13);cputs("T ");textcolor(c);}
	 else cputs("  ");
	}  */
       //else if (mecz==1)
	//{
	 if (gracz.dane[15]>0) {textbackground(4);textcolor(7);cputs("Ko");textbackground(0);/*cputs(" ");*/textcolor(c);}
	 //else if (gracz.dane[14]==1) {textcolor(4);cputs("˛ ");textcolor(c);}
	 else if (gracz.dane[12]==1) {textcolor(14);cputs("˛ ");textcolor(c);}
	 else if (gracz.dane[12]>=2) {textcolor(4);cputs("˛ ");textcolor(c);}
	 //else if (gracz.dane[17]==1) {textcolor(13);cputs("T ");textcolor(c);}
	 else cputs("  ");
	//}
       //cprintf("%-15s %-10s %d %d %2d  %c  %2d %2d %2d %2d  %-7s %2d   %3d%%  %2d",gracz.nazwisko,gracz.imie,gracz.dane[13],gracz.dane[14],gracz.dane[15],tn,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],morale,gracz.dane[9],gracz.dane[11],gracz.dane[16]);
       /*if (mecz==0)
	{
	 if (kto==-1) cprintf("%3s%-15s %c  %2d %2d %2d %2d  %-7s",gracz.imie,gracz.nazwisko,tn,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],morale);
	 else cprintf("%3s%-15s %c  %2d %2d %2d %2d  %-7s  %2d  %3d%%  %2d",gracz.imie,gracz.nazwisko,tn,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],morale,gracz.dane[9],gracz.dane[11],gracz.dane[16]);
	} */
       //else if (mecz==1)
	//{
	 //cprintf("%3s%-15s %c  %2d %2d %2d %2d  %-7s  %2d   %2d  %3d%%  %2d",gracz.imie,gracz.nazwisko,tn,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],morale,gracz.dane[9],gracz.dane[20],gracz.dane[11],gracz.dane[21]);
	 cprintf("%3s%-15s %c  %2d %2d %2d %2d  %-7s  %2d",gracz.imie,gracz.nazwisko,tn,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],morale,gracz.dane[9]);
	 if (gracz.dane[20]>0) cprintf("   +%d  %3d%%  %2d",gracz.dane[20],gracz.dane[11],gracz.dane[21]);
	 else cprintf("   %2d  %3d%%  %2d",gracz.dane[20],gracz.dane[11],gracz.dane[21]);
      }}//}
   fclose(f);
   if (kto>-1)
   {
   if (i==1) {c=5;textcolor(5);}
   if ((i==5)&&(usta==1||usta==2||usta==3||usta==4||usta==9||usta==10||usta==11)) {c=11;textcolor(11);}
   else if ((i==4)&&(usta==5||usta==6||usta==7||usta==8)) {c=11;textcolor(11);}
   else if ((i==6)&&(usta==12||usta==13||usta==14)) {c=11;textcolor(11);}
   if ((i==7)&&(usta==9)) {c=10;textcolor(10);}
   else if ((i==8)&&(usta==10||usta==5)) {c=10;textcolor(10);}
   else if ((i==10)&&(usta==11)) {c=10;textcolor(10);}
   else if ((i==9)&&(usta==1||usta==2||usta==3||usta==4||usta==6||usta==7||usta==8||usta==12||usta==13||usta==14)) {c=10;textcolor(10);}
   if (i==16) {c=7;textcolor(7);}
   //if (usta==0) {c=7;textcolor(7);}//dla listy transferowej
   }
   //if (tryb==11&&i==11) cputs("\n\r");
  }
}

/*void zamiana()
{
 FILE *f,*f2;
 int numer1,numer2,licz=0;
 f=fopen("Gra_mana.cfg","rb");
 while(fread(&gracz,sizeof(gracz),1,f)==1) licz++;
 fclose(f);
 textcolor(7);cputs("\n\rPodaj numer, spacja, drugi numer i Enter: ");
 scanf("%d %d",&numer1,&numer2);
 if (numer1<1||numer1>licz||numer2<1||numer2>licz) {numer1=1;numer2=1;}
 if (numer1!=numer2)
  {
   f=fopen("wymiana.cfg","wb");
   fclose(f);//szyszczenie starej
   f=fopen("Gra_mana.cfg","rb");
   while(fread(&gracz,sizeof(gracz),1,f)==1)
    {
     if (gracz.dane[0]==numer1) gracz.dane[0]=numer2;
     else if (gracz.dane[0]==numer2) gracz.dane[0]=numer1;
     f2=fopen("wymiana.cfg","ab");
     fwrite(&gracz,sizeof(gracz),1,f2);
     fclose(f2);
    }
   fclose(f);
   przepisz();
  }//dla if
} */

void instrukcje(int a,int b,int c,int d,int e,int f,int belka)
{
 if (belka==0) textbackground(0);
 if (belka==1) {textbackground(1);textcolor(7);}
 else {textbackground(0);textcolor(2);}
 cputs("\n\r Nastawienie        -");
 if (f==1) textcolor(2);
 else textcolor(8);
 if (belka==1&&f==1) textcolor(7);
 if (belka==1&&f!=1) textcolor(0);
 cputs(" Normalne");
 if (f==2) textcolor(2);
 else textcolor(8);
 if (belka==1&&f==2) textcolor(7);
 if (belka==1&&f!=2) textcolor(0);
 cputs(" Obronne");
 if (f==3) textcolor(2);
 else textcolor(8);
 if (belka==1&&f==3) textcolor(7);
 if (belka==1&&f!=3) textcolor(0);
 cputs(" Atak ");
 //
 if (belka==2) {textbackground(1);textcolor(7);}
 else {textbackground(0);textcolor(2);}
 cputs("\n\r Podania            -");
 if (a==1) textcolor(2);
 else textcolor(8);
 if (belka==2&&a==1) textcolor(7);
 if (belka==2&&a!=1) textcolor(0);
 cputs(" Mieszane");
 if (a==2) textcolor(2);
 else textcolor(8);
 if (belka==2&&a==2) textcolor(7);
 if (belka==2&&a!=2) textcolor(0);
 cputs(" Kr¢tkie");
 if (a==3) textcolor(2);
 else textcolor(8);
 if (belka==2&&a==3) textcolor(7);
 if (belka==2&&a!=3) textcolor(0);
 cputs(" órednie");
 if (a==4) textcolor(2);
 else textcolor(8);
 if (belka==2&&a==4) textcolor(7);
 if (belka==2&&a!=4) textcolor(0);
 cputs(" Dàugie ");
 textcolor(2);
 //
 if (belka==3) {textbackground(1);textcolor(7);}
 else {textbackground(0);textcolor(2);}
 cputs("\n\r Traktowanie rywala -");
 if (b==1) textcolor(2);
 else textcolor(8);
 if (belka==3&&b==1) textcolor(7);
 if (belka==3&&b!=1) textcolor(0);
 cputs(" Normalne");
 if (b==2) textcolor(2);
 else textcolor(8);
 if (belka==3&&b==2) textcolor(7);
 if (belka==3&&b!=2) textcolor(0);
 cputs(" Delikatne");
 if (b==3) textcolor(2);
 else textcolor(8);
 if (belka==3&&b==3) textcolor(7);
 if (belka==3&&b!=3) textcolor(0);
 cputs(" Twarde ");
 textcolor(2);
 //
 if (belka==4) {textbackground(1);textcolor(7);}
 else {textbackground(0);textcolor(2);}
 cputs("\n\r Pressing           -");
 if (c==0) textcolor(2);
 else textcolor(8);
 if (belka==4&&c==0) textcolor(7);
 if (belka==4&&c==1) textcolor(0);
 cputs(" Nie");
 if (c==1) textcolor(2);
 else textcolor(8);
 if (belka==4&&c==1) textcolor(7);
 if (belka==4&&c==0) textcolor(0);
 cputs(" Tak ");
 textcolor(2);
 //
 if (belka==5) {textbackground(1);textcolor(7);}
 else {textbackground(0);textcolor(2);}
 cputs("\n\r Puàapki ofsajdowe  -");
 if (d==0) textcolor(2);
 else textcolor(8);
 if (belka==5&&d==0) textcolor(7);
 if (belka==5&&d==1) textcolor(0);
 cputs(" Nie");
 if (d==1) textcolor(2);
 else textcolor(8);
 if (belka==5&&d==1) textcolor(7);
 if (belka==5&&d==0) textcolor(0);
 cputs(" Tak ");
 textcolor(2);
 //
 if (belka==6) {textbackground(1);textcolor(7);}
 else {textbackground(0);textcolor(2);}
 cputs("\n\r Gra z kontry       -");
 if (e==0) textcolor(2);
 else textcolor(8);
 if (belka==6&&e==0) textcolor(7);
 if (belka==6&&e==1) textcolor(0);
 cputs(" Nie");
 if (e==1) textcolor(2);
 else textcolor(8);
 if (belka==6&&e==1) textcolor(7);
 if (belka==6&&e==0) textcolor(0);
 cputs(" Tak ");
 textbackground(0);
}

void takty(int usta,int kto)
{
 FILE *f;
 //int sumaO=0,sumaP=0,sumaN=0,k=0,ile=0,nm=0;
 char *taktyka[15]={"4-4-2","4-4-2 Obrona","4-4-2 Atak","4-4-2 Diamond",
 "3-4-3","3-5-2","3-5-2 Obrona","3-5-2 Atak","4-2-4","4-3-3","4-5-1","5-3-2",
 "5-3-2 Obrona","5-3-2 Atak","Bà•d"};
 textbackground(2);textcolor(0);//usta,wypis taktyk
 cprintf("\n\r %-15s",taktyka[usta-1]);
 textcolor(10);
 //--------------------linia ataku
 if (usta==1||usta==2||usta==3||usta==4||usta==6||usta==7||usta==8||usta==9||usta==12||usta==13||usta==14) {cputs("\n\r     10  11     ");if (kto==0){textbackground(0);textcolor(2);cputs("     U Zmiana ustawienia         P Zamiana zawodnik¢w");textbackground(2);textcolor(10);}}
 else if (usta==5||usta==10) {cputs("\n\r    09 10 11    ");if (kto==0){textbackground(0);textcolor(2);cputs("     U Zmiana ustawienia         P Zamiana zawodnik¢w");textbackground(2);textcolor(10);}}
 else if (usta==11) {cputs("\n\r       11       ");if (kto==0){textbackground(0);textcolor(2);cputs("     U Zmiana ustawienia         P Zamiana zawodnik¢w");textbackground(2);textcolor(10);}}
 //------------------------------------
 if (usta==9) { cputs("\n\r 08          09 "); textbackground(0);textcolor(2);cputs("     I Instrukcje dla druæyny    R Pokaæ rezerwowych");textbackground(2);}//4 w ataku
 else {cputs("\n\r                ");textbackground(0);textcolor(2);cputs("     I Instrukcje dla druæyny    R Pokaæ rezerwowych");textbackground(2);}
 //------------------------------------------
 textcolor(11);
 if (usta==4||usta==8||usta==14) {cputs("\n\r       09       ");textbackground(0);textcolor(4);cputs("     Q Powr¢t");textbackground(2);textcolor(11);}
 else if (usta==3) {cputs("\n\r 06          09 ");textbackground(0);textcolor(4);cputs("     Q Powr¢t");textbackground(2);textcolor(11);}
 else {cputs("\n\r                ");textbackground(0);textcolor(4);cputs("     Q Powr¢t");textbackground(2);textcolor(11);}
 //--------------------------linia srodkowa
 if (usta==1) cputs("\n\r 06  07  08  09 ");
 else if (usta==5||usta==8) cputs("\n\r 05  06  07  08 ");
 else if (usta==2) cputs("\n\r 06          09 ");
 else if (usta==4) cputs("\n\r 06          08 ");
 else if (usta==13) cputs("\n\r     08  09     ");
 else if (usta==3||usta==14) cputs("\n\r     07  08     ");
 else if (usta==9) cputs("\n\r     06  07     ");
 else if (usta==10) cputs("\n\r    06 07 08    ");
 else if (usta==12) cputs("\n\r    07 08 09    ");
 else if (usta==7) cputs("\n\r 05    06    09 ");
 else if (usta==6) cputs("\n\r 05 06 07 08 09 ");
 else if (usta==11) cputs("\n\r 06 07 08 09 10 ");
 //--------------------------------------
 if (usta==2||usta==7) cputs("\n\r     07  08     ");
 else if (usta==4||usta==13) cputs("\n\r       07       ");
 else cputs("\n\r                ");
 //---------------------------------
 textcolor(5);
 if (usta==12||usta==13||usta==14) cputs("\n\r 05          06 ");
 else cputs("\n\r                ");
 //-----------------------------linia obrony
 if (usta==1||usta==2||usta==3||usta==4||usta==9||usta==10||usta==11) cputs("\n\r 02  03  04  05 ");
 else cputs("\n\r    02 03 04    ");
 //--------------------------------------
 textcolor(15);
 cputs("\n\r   ⁄ƒƒƒƒƒƒƒƒø   ");textcolor(15);
 cputs("\n\r   ≥");textcolor(9);cputs("   01");textcolor(15);cputs("   ≥   ");
 textbackground(0);
}

int usalenie(int pilka,int PnaP,int OnaA,int AnaO,int sumaB,int sumaB2,int gdzie)
 {
  int x1,x2,x3,los,co;
   if (gdzie==1)//P/P
    {
     if (pilka==1)
      {
       x1=Klub.inst[4];//gra z kontry twoja
       x2=Klub.umie[1];//podania trening
       x3=Klub.rinst[2];//pressing rywala
      }
     else //if (pilka==2)
      {
       x1=Klub.rinst[4];//gra z kontry rywala
       x2=2;//podania trening rywala
       x3=Klub.inst[2];//pressing tw¢j
      }
     //los=random(18)+1;
     los=random(27)+1;
     //if (los>9) co=los-9;
     if (los<10) co=los;
     else
     {
     if ((PnaP>20&&pilka==1)||(PnaP<-20&&pilka==2))//przewaga duæa
      {
       if (x1==0&&x2>=2&&x3==0) co=1;
       else if (x1==0&&x2>=2&&x3==1)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=6;
	 else co=1;
	}
       else if (x1==0&&x2<2&&x3==0)
	{
	 los=random(18)+1;
	 if (los==1||los==2) co=5;
	 else co=1;
	}
       else if (x1==0&&x2<2&&x3==1)
	{
	 los=random(18)+1;
	 if (los==1||los==2) co=6;
	 else if (los==3||los==4) co=5;
	 else co=1;
	}
       else if (x1==1&&x2>=2&&x3==0)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=1;//udana normal
	 else co=2;//udana z kontry
	}
       else if (x1==1&&x2>=2&&x3==1)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=1;//udana normal
	 else if (los==4||los==5||los==6) co=6;//pressing rywala
	 else co=2;//udana z kontry
	}
       else if (x1==1&&x2<2&&x3==0)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=1;//udana normal
	 else if (los==4||los==5) co=5;//sàabe podania
	 else co=2;//udana z kontry
	}
       else if (x1==1&&x2<2&&x3==1)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=1;//udana normal
	 else if (los==4||los==5) co=5;//sàabe podania
	 else if (los==6||los==7) co=6;//pressing rywala
	 else co=2;//udana z kontry
	}
      }
     else if ((PnaP>10&&PnaP<21&&pilka==1)||(PnaP<-10&&PnaP>-21&&pilka==2))//przewaga maàa
      {
       if (x1==0&&x2>=2&&x3==0)
	{
	 if (los==14||los==22) co=4;
	 else if (los==15||los==23||los==26) co=7;
	 else if (los==16||los==24||los==27) co=8;
	 else if (los==17||los==18) co=9;
	 else co=1;//if (los==10||los==11||los==12||los==13||los==19||los==20||los==21||los==25)
	}
       else if (x1==0&&x2>=2&&x3==1)
	{
	 if (los==13||los==21||los==22||los==26) co=6;//pressing przeciwnika
	 else if (los==14||los==23) co=4;
	 else if (los==15||los==24) co=7;
	 else if (los==16) co=8;
	 else if (los==17||los==18) co=9;
	 else co=1;//if (los==10||los==11||los==12||los==19||los==20||los==25||los==27)
	}
       else if (x1==0&&x2<2&&x3==0)
	{
	 if (los==13||los==23||los==24||los==27) co=5;//sàabe podania
	 else if (los==14) co=4;
	 else if (los==15||los==25||los==26) co=7;
	 else if (los==16) co=8;
	 else if (los==17||los==18) co=9;
	 else co=1;//if (los==10||los==11||los==12||los==19||los==20||los==21||los==22)
	}
       else if (x1==0&&x2<2&&x3==1)
	{
	 if (los==12||los==22||los==24) co=6;//pressing przeciwnika
	 else if (los==13||los==23||los==25) co=5;//sàabe podania
	 else if (los==14) co=4;
	 else if (los==15||los==26) co=7;
	 else if (los==16) co=8;
	 else if (los==17||los==18) co=9;
	 else co=1;//if (los==10||los==11||los==19||los==20||los==21||los==27)
	}
       else if (x1==1&&x2>=2&&x3==0)
	{
	 if (los==13||los==24||los==25||los==26) co=3;//nie udana z kontry
	 else if (los==14) co=4;
	 else if (los==15||los==27) co=7;
	 else if (los==16) co=8;
	 else if (los==17||los==18) co=9;
	 else co=2;//udana z kontryif (los==10||los==11||los==12||los==19||los==20||los==21||los==22||los==23)
	}
       else if (x1==1&&x2>=2&&x3==1)
	{
	 if (los==12||los==23||los==25) co=6;//pressing przeciwnika
	 else if (los==13||los==24||los==14) co=3;//nie udana z kontry
	 else if (los==15||los==26) co=7;
	 else if (los==16) co=8;
	 else if (los==17||los==18) co=9;
	 else co=2;//udana z kontry if (los==10||los==11||los==19||los==20||los==21||los==22||los==27)
	}
       else if (x1==1&&x2<2&&x3==0)
	{
	 if (los==12||los==23||los==24) co=5;//sàaby trening poda‰
	 else if (los==13||los==26||los==14) co=3;//nie udana z kontry
	 else if (los==15||los==27) co=7;
	 else if (los==16) co=8;
	 else if (los==17||los==18) co=9;
	 else co=2;//udana z kontry if (los==10||los==11||los==19||los==20||los==21||los==22||los==25)
	}
       else //if (x1==1&&x2<2&&x3==1)
	{
	 if (los==13||los==22||los==26) co=5;//sàaby trening poda‰
	 else if (los==14||los==23||los==27) co=3;//nie udana z kontry
	 else if (los==16) co=7;
	 //else if (los==7) co=8;//aut
	 else if (los==17) co=9;
	 else if (los==15||los==18||los==24) co=6;//pressing rywala3x
	 else co=2;//udana z kontryif (los==10||los==11||los==12||los==19||los==20||los==21||los==25)
	}}
     else if ((PnaP>20&&pilka==2)||(PnaP<-20&&pilka==1))//osàabienie duæe
      {
       if (x1==0&&x2>=2&&x3==0) co=4;
       else if (x1==0&&x2>=2&&x3==1)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=4;
	 else co=6;
	}
       else if (x1==0&&x2<2&&x3==0)
	{
	 los=random(18)+1;
	 if (los==1||los==2) co=4;
	 else co=5;
	}
       else if (x1==0&&x2<2&&x3==1)
	{
	 los=random(18)+1;
	 if (los==1||los==2) co=5;
	 else if (los==3||los==4) co=4;
	 else co=6;
	}
       else if (x1==1&&x2>=2&&x3==0)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=4;//nieudana normal
	 else co=3;//nieudana z kontry
	}
       else if (x1==1&&x2>=2&&x3==1)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=4;//nieudana normal
	 else if (los==4||los==5||los==6) co=6;//pressing rywala
	 else co=3;//nieudana z kontry
	}
       else if (x1==1&&x2<2&&x3==0)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=4;//nieudana normal
	 else if (los==4||los==5) co=5;//sàabe podania
	 else co=3;//nieudana z kontry
	}
       else if (x1==1&&x2<2&&x3==1)
	{
	 los=random(18)+1;
	 if (los==1||los==2||los==3) co=4;//nieudana normal
	 else if (los==4||los==5) co=5;//sàabe podania
	 else if (los==6||los==7) co=6;//pressing rywala
	 else co=3;//nieudana z kontry
	}
      }
     else if ((PnaP<-10&&PnaP>-21&&pilka==1)||(PnaP>10&&PnaP<21&&pilka==2))//osàabienie
      {
       if (x1==0&&x2>=2&&x3==0)
	{
	 if (los==14||los==22||los==23||los==27) co=1;
	 else if (los==15||los==24) co=7;
	 else if (los==16||los==25) co=8;
	 else if (los==17||los==18) co=9;
	 else co=4;//if (los==10||los==11||los==12||los==13||los==19||los==20||los==21||los==26)
	}
       else if (x1==0&&x2>=2&&x3==1)
	{
	 if (los==10||los==26) co=4;
	 else if (los==14||los==23||los==25||los==27) co=1;
	 else if (los==15||los==24) co=7;
	 else if (los==16) co=8;
	 else if (los==17||los==18) co=9;
	 else co=6;//pressing przeciwnikaif (los==11||los==12||los==13||los==19||los==20||los==21||los==22)
	}
       else if (x1==0&&x2<2&&x3==0)
	{
	 if (los==10||los==23) co=4;
	 else if (los==14||los==25||los==26||los==27) co=1;
	 else if (los==15||los==24) co=7;
	 else if (los==16) co=8;
	 else if (los==17||los==18) co=9;
	 else co=5;//sàabe podania if (los==11||los==12||los==13||los==19||los==20||los==21||los==22)
	}
       else if (x1==0&&x2<2&&x3==1)
	{
	 if (los==10) co=4;
	 else if (los==11||los==12||los==19||los==20) co=6;//pressing przeciwnika
	 else if (los==13||los==22||los==23||los==21) co=5;//sàabe podania
	 else if (los==14||los==25||los==26||los==27) co=1;
	 else if (los==15||los==24) co=7;
	 else if (los==16) co=8;
	 else co=9;
	}
       else if (x1==1&&x2>=2&&x3==0)
	{
	 //if (los==10||los==11||los==12||los==19||los==20||los==21||los==27) co=3;//nie udana z kontry
	 if (los==13||los==22||los==23||los==24) co=2;//udana z kontry
	 else if (los==14||los==25) co=4;
	 else if (los==15||los==26) co=7;
	 else if (los==16) co=8;
	 else if (los==17||los==18) co=9;
	 else co=3;//nie udana z kontry if (los==10||los==11||los==12||los==19||los==20||los==21||los==27)
	}
       else if (x1==1&&x2>=2&&x3==1)
	{
	 if (los==10||los==11||los==20||los==21) co=3;//nieudana z kontry
	 else if (los==12||los==22||los==23||los==27) co=6;//pressing przeciwnika
	 else if (los==13||los==24||los==25||los==19) co=2;//udana z kontry
	 else if (los==14) co=4;
	 else if (los==15||los==26) co=7;
	 else if (los==16) co=8;
	 else co=9;
	}
       else if (x1==1&&x2<2&&x3==0)
	{
	 if (los==10||los==11||los==19||los==20) co=3;//nieudana z kontry
	 else if (los==12||los==21||los==22||los==23) co=5;//sàaby trening poda‰
	 else if (los==13||los==24||los==25||los==26) co=2;//udana z kontry
	 else if (los==14) co=4;
	 else if (los==15||los==27) co=7;
	 else if (los==16) co=8;
	 else co=9;
	}
       else //if (x1==1&&x2<2&&x3==1)
	{
	 if (los==10||los==11||los==12||los==19) co=3;//nieudana z kontry
	 else if (los==13||los==22||los==23) co=5;//sàaby trening poda‰
	 else if (los==14||los==24||los==25||los==26) co=2;// udana z kontry
	 else if (los==15) co=4;
	 else if (los==16||los==27) co=7;
	 //else if (los==7) co=8;//aut
	 else if (los==17) co=9;
	 else co=6;//pressing rywala3x
	}}
     else //if (PnaP>=-10&&PnaP<=10)//siày wyr¢wnane
      {
       if (x1==0&&x2>=2&&x3==0)
	{
	 if (los==10||los==11||los==19||los==24||los==25) co=1;
	 else if (los==12||los==13||los==20||los==26) co=4;
	 else if (los==14||los==15||los==21) co=7;
	 else if (los==16||los==17||los==22||los==27) co=8;
	 else co=9;
	}
       else if (x1==0&&x2>=2&&x3==1)
	{
	 if (los==10||los==11||los==19||los==20||los==24) co=1;
	 else if (los==12||los==21) co=4;
	 else if (los==13||los==22) co=6;//pressing rywala
	 else if (los==14||los==15||los==23) co=7;
	 else if (los==16||los==17||los==25||los==26) co=8;
	 else co=9;
	}
       else if (x1==0&&x2<2&&x3==0)
	{
	 if (los==10||los==11||los==25||los==26||los==27) co=1;
	 else if (los==12||los==24) co=4;
	 else if (los==13||los==23) co=5;//sàabe podania
	 else if (los==14||los==15||los==22) co=7;
	 else if (los==16||los==17||los==20||los==21) co=8;
	 else co=9;
	}
       else if (x1==0&&x2<2&&x3==1)
	{
	 if (los==10||los==11||los==25||los==26||los==27) co=1;
	 else if (los==12||los==24) co=6;//pressing przeciwnika
	 else if (los==13||los==23) co=5;//sàabe podania
	 else if (los==14||los==15||los==22) co=7;
	 else if (los==16||los==17||los==20||los==21) co=8;
	 else co=9;
	}
       else if (x1==1&&x2>=2&&x3==0)
	{
	 if (los==10||los==11||los==25||los==26||los==27) co=2;//unanie z kontry
	 else if (los==12||los==13||los==23||los==24) co=3;//nieudana kontra
	 else if (los==14||los==15||los==22) co=7;
	 else if (los==16||los==17||los==20||los==21) co=8;
	 else co=9;
	}
       else if (x1==1&&x2>=2&&x3==1)
	{
	 if (los==10||los==11||los==25||los==26||los==27) co=2;//unanie z kontry
	 else if (los==12||los==24) co=3;//nieudana kontra
	 else if (los==13||los==23) co=6;//pressing przeciwnika
	 else if (los==14||los==15||los==22) co=7;
	 else if (los==16||los==17||los==20||los==21) co=8;
	 else co=9;
	}
       else if (x1==1&&x2<2&&x3==0)
	{
	 if (los==10||los==11||los==25||los==26||los==27) co=2;//unanie z kontry
	 else if (los==12||los==24) co=3;//nieudana kontra
	 else if (los==13||los==23) co=5;//sàaby trening
	 else if (los==14||los==15||los==22) co=7;
	 else if (los==16||los==17||los==20||los==21) co=8;
	 else co=9;
	}
       else //if (x1==1&&x2<2&&x3==1)
	{
	 if (los==10||los==11||los==25||los==26||los==27) co=2;//unanie z kontry
	 else if (los==12||los==24) co=3;//nieudana kontra
	 else if (los==13||los==23) co=5;//sàaby trening
	 else if (los==14||los==15||los==22) co=7;
	 else if (los==16||los==21) co=8;
	 else if (los==17||los==20) co=6;//pressing rywala
	 else co=9;
	}}}
    }//gdzie = 1 P/P
   else if (gdzie==2)//skrzydàowy przy piàce
    {
     //los=random(8)+1;
     los=random(12)+1;
     if (los<5) co=los+9;
     //else if (los>4&&los<9) co=los+1;
     else
     {
     if ((AnaO>10&&pilka==1)||(OnaA<-10&&pilka==2))//duæa przewaga ataku
      {
       los=random(10)+1;
       if (los==1||los==2) co=11;
       else co=10;
      }
     else if ((AnaO>0&&AnaO<11&&pilka==1)||(OnaA>-11&&OnaA<0&&pilka==2))//przewaga ataku
      {
       if (los==6||los==7||los==8||los==9) co=10;
       else if (los==10||los==11||los==12) co=11;
       else co=13;
      }//przewaga w ataku
     else if ((AnaO<-40&&pilka==1)||(OnaA>40&&pilka==2))//duæa przewaga obrony
      {
       los=random(10)+1;
       if (los==1||los==2) co=12;
       else co=13;
      }
     else if ((AnaO>-41&&AnaO<-20&&pilka==1)||(OnaA>20&&OnaA<41&&pilka==2))//przewaga obrony
      {
       if (los==5||los==6||los==7||los==8||los==9) co=13;
       else if (los==10||los==11) co=10;
       else co=11;
      }//przewaga w obronie
     else //if ((AnaO>-21&&AnaO<1&&pilka==1)||(OnaA<21&&OnaA>-1&&pilka==2))
      {if (los>4&&los<9) co=los+5;
       else co=los+1;}
     }//else
    }//gdzie =2 A/O
   else if (gdzie==3)//pole karne A/O
    {
     los=random(30)+1;
     if (pilka==1) x1=Klub.rinst[3];//puàapki ofsajdowe rywala
     else x1=Klub.inst[3];
     if (los<11) co=los+13;
     else
     {
      if ((AnaO>10&&pilka==1)||(OnaA<-10&&pilka==2))//duæa przewaga ataku
       {
	if (x1==0)
	 {
	  los=random(10)+1;
	  if (los==1||los==2||los==3) co=21;
	  else co=22;
	 }
	else if (x1==1)
	 {
	  los=random(15)+1;
	  if (los==1||los==2||los==3) co=21;
	  else if (los==4||los==5||los==6) co=14;
	  else if (los==7) co=15;
	  else co=22;
	 }}
      else if ((AnaO>0&&AnaO<11&&pilka==1)||(OnaA>-11&&OnaA<0&&pilka==2))//przewaga ataku
       {
	if (x1==0)
	 {
	  if (los==11||los==27) co=16;
	  else if (los==12||los==26) co=17;
	  else if (los==13||los==25) co=18;
	  else if (los==14||los==15||los==24||los==30) co=20;
	  else if (los==16||los==17||los==23) co=21;
	  else if (los==18||los==19||los==21||los==22||los==29) co=22;
	  else co=23;
	 }
	else if (x1==1)
	 {
	  if (los==11||los==23||los==27) co=14;
	  else if (los==12||los==28) co=15;
	  else if (los==13) co=16;
	  else if (los==14) co=17;
	  else if (los==15||los==29) co=18;
	  else if (los==16||los==24) co=20;
	  else if (los==17||los==22||los==26) co=21;
	  else if (los==18||los==19||los==21||los==25||los==30) co=22;
	  else co=23;
	 }}
      else if ((AnaO<-40&&pilka==1)||(OnaA>40&&pilka==2))//duæa przewaga obrony
       {
	if (x1==0)
	 {
	  los=random(10)+1;
	  if (los==1||los==2||los==3) co=17;
	  else co=16;
	 }
	else if (x1==1)
	 {
	  los=random(15)+1;
	  if (los==1||los==2||los==3) co=17;
	  else if (los==4||los==5||los==6) co=16;
	  else if (los==7) co=15;
	  else co=14;
	 }}
      else if ((AnaO>-41&&AnaO<-20&&pilka==1)||(OnaA>20&&OnaA<41&&pilka==2))//przewaga obrony
       {
	if (x1==0)
	 {
	  if (los==11||los==12||los==13||los==21||los==22||los==30) co=16;
	  else if (los==14||los==23||los==24) co=17;
	  else if (los==15) co=18;
	  else if (los==16||los==17||los==27) co=20;
	  else if (los==18||los==25) co=21;
	  else if (los==19||los==26) co=22;
	  else co=23;//3x
	 }
	else if (x1==1)
	 {
	  if (los==11||los==12||los==13||los==21||los==22||los==29) co=14;
	  else if (los==30) co=16;
	  else if (los==14||los==23) co=15;
	  else if (los==15||los==28) co=17;
	  else if (los==16) co=18;
	  else if (los==17||los==27) co=20;
	  else if (los==18||los==26) co=21;
	  else if (los==19||los==25) co=22;
	  else co=23;
	 }}
      else if ((AnaO>-21&&AnaO<1&&pilka==1)||(OnaA<21&&OnaA>-1&&pilka==2))
       {
	if (x1==0)
	 {
	  if (los==11||los==12||los==13||los==21) co=16;
	  else if (los==30) co=14;
	  else if (los==14||los==23) co=17;
	  else if (los==15) co=18;
	  else if (los==16||los==17||los==24||los==27) co=20;
	  else if (los==18||los==25||los==29) co=21;
	  else if (los==19||los==26||los==22) co=22;
	  else co=23;
	 }
	else if (x1==1)
	 {
	  los=random(10)+1; co=los+13;
	 }
       }}//else
    }//dla gdzie==3 A/O
   else if (gdzie==4||gdzie==6)//obrona B sumaB max 43, mini 8, òred 26
    {
     los=random(12)+1;
     if (gdzie==4)
      {
       if (pilka==1) x1=sumaB2;//umiej©tnoòci bramkarza
       else x1=sumaB;
      }
     else if (gdzie==6)
      {
       if (pilka==1) x1=sumaB2-(Klub.umie[2]*2);//umiej©tnoòci bramkarza
       else x1=sumaB-2;
      }
     if (x1<15)
      {
       if (los==1||los==2||los==3||los==4) co=28;//gol
       else if (los==5||los==6||los==7) co=27;//r¢g
       else if (los==8||los==9) co=26;//niepewnia obrona
       else if (los==10) co=29;//gol nieuznany
       else if (los==11) co=24;//obronià
       else co=30;//strzaà niecelny 1x
      }
     else if (x1>=15&&x1<25)
      {
       if (los==1||los==2||los==3) co=28;//gol
       else if (los==4||los==5||los==6) co=27;//r¢g
       else if (los==7||los==8) co=26;//niepewnia obrona
       else if (los==9) co=29;//gol nieuznany
       else if (los==10||los==11) co=24;//obronià
       else co=30;//strzaà niecelny
      }
     else if (x1>=25&&x1<35)
      {
       if (los==1||los==2||los==3) co=28;//gol
       else if (los==4||los==5) co=27;//r¢g
       else if (los==6||los==7) co=26;//niepewnia obrona
       else if (los==8) co=29;//gol nieuznany
       else if (los==9||los==10) co=24;//obronià
       else if (los==11) co=25;//bez problemu
       else co=30;//strzaà niecelny1x
      }
     else if (x1>34)
      {
       if (los==1||los==2) co=28;//gol
       else if (los==3||los==4) co=27;//r¢g
       else if (los==5) co=26;//niepewnia obrona
       else if (los==6) co=29;//gol nieuznany
       else if (los==7||los==8) co=24;//obronià
       else if (los==9||los==10) co=25;//bez problemu
       else co=30;//strzaà niecelny2x
      }
     //dystans=0;
    }//gdzie = 4//obrona B
   else if (gdzie==5)//obrona B - karny, sma na sam
    {
     los=random(9)+1;
     if (pilka==1) x1=sumaB2-Klub.umie[2];//um. bramkarza-staàe fragmenty
     else x1=sumaB-2;
     if (x1<15)
      {
       if (los==1||los==2||los==3||los==4||los==5||los==6||los==7) co=28;//gol
       else if (los==8) co=24;//obronià
       else co=30;//strzaà niecelny1x
      }
     else if (x1>=15&&x1<25)
      {
       if (los==1||los==2||los==3||los==4||los==5||los==6) co=28;//gol
       else if (los==8) co=24;//obronià
       else co=30;//strzaà niecelny 2x
      }
     else if (x1>=25&&x1<35)
      {
       if (los==1||los==2||los==3||los==4||los==5) co=28;//gol
       else if (los==8||los==6) co=24;//obronià
       else co=30;//strzaà niecelny 2x
      }
     else if (x1>34)
      {
       if (los==1||los==2||los==3||los==4) co=28;//gol
       else if (los==8||los==6||los==5) co=24;//obronià
       else co=30;//strzaà niecelny2x
      }
    }//gdzie = 5//obrona B
 return co;
}