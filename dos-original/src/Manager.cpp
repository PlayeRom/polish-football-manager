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
 int klub,kasa;//numer od 1 do 16 oznaczaj•cy klub,//kasa-blokada dla fund. na transfery
 int d,m,r,tydzien;//dzien,miesiac,rok
 int trening[28],treBOPN,inst[6],usta,asystent,lostmecz,gol1,gol2,zadyma;//asystent=1 widomoòÜ od asystenta
 //co trenuj•,+/-BOPN,instrukcje,ustawienie
 float umie[5],finanse[14],finanse2[14],ileTrans,dane2_m[2];
 int rywal[4],rinst[6],rywalTN,ilekontrol,nie;//dla rywala
 int gol[480],golK[10],dane_m[12],sezon,mecz;//wiosna,gole dla kontrolnych//mecz-dla opcji wyswietlania napisow w meczu
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
struct
{
 char newss[500];
 int num;
} news;
struct
{
 char imie[4],nazwisko[20];
 int dane[23];
 float waga[4];
} gracz;
//----------------FUNKCJE-----------
void info();
char MenuGlowne(int wiadomosc[5]);
void sklad(int usta,int tryb,int kto,int sort);
void zamiana();
void przepisz();
void instrukcje(int a,int b,int c,int d,int e,int f,int belka);
void takty(int usta,int kto);
void wykres(int usta,int kto);
//int usalenie(int pilka,int PnaP,int OnaA,int AnaO,int sumaB,int sumaB2,int gdzie);
//void mecz();
//void boisko(int usta);
//---------------------
#define F1 59
#define F2 60
#define F3 61
#define F4 62
#define F5 63
#define F6 64
#define F7 65
#define UP 72
#define LEFT 75
#define RIGHT 77
#define DOWN 80
void main()
{
 FILE *f,*f2,*f3;
 char pp1,pp2,pp_usta,pp_usta2,tn,*morale,tn2;
 char *taktyka[15]={"4-4-2","4-4-2 Obrona","4-4-2 Atak","4-4-2 Diamond",
 "3-4-3","3-5-2","3-5-2 Obrona","3-5-2 Atak","4-2-4","4-3-3","4-5-1","5-3-2",
 "5-3-2 Obrona","5-3-2 Atak","Bà•d"};
 char *tren[10]={"Kondycja","Podania","Staàe fra.","Taktyka","Bramkarze",
 "Obrona","Pomoc","Atak","Wolne"," "};
 char *dlawiadomosc[4],zastep[4][20];
 char krajMS[16][19]={"Amica Wronki","Garbarnia Szczak.","GKS Katowice",
 "G¢rnik Zabrze","Groclin Dyskobolia","KSZO Ostrowiec","Lech Pozna‰",
 "Legia Warszawa","Odra Wodzisàaw","Polonia Warszawa","Pogo‰ Szczecin",
 "Ruch Chorz¢w","Widzew ù¢d´","Wisàa Krak¢w","Wisàa Pàock","Zagà©bie Lubin"};
 int dalej=0,f_po=0,f_b=1,f_o=1,f_p=1,f_n=1,f_fo=1,kupno=0,OnaA,PnaP,AnaO;//zastrzeæony
 int ilex[10]={0,0,0,0,0,0,0,0,0,0},numerW[10]={0,0,0,0,0,0,0,0,0,0};
 int klub,nowa_gra=0,k=0,kk,wiadomosc[10]={0,0,0,0,0,0,0,0,0,0},i=0,tryb=0;
 int numer1=0,numer2=0,sumaO=0,sumaP=0,sumaN=0,lata,trening2[28]={5,5,1,4,3,2,9,6,6,2,1,4,3,10,7,7,3,2,1,4,10,8,8,10,10,10,10,10};;
 float transfer,cena,placa,premia,f_ce1=0,f_ce2=6000000;
 int belka=1,sort=0;//,dlameczu=1;
 int PrzechodzeDoMeczu=0;
 //int v=0,pkt[16],pkt2[16],golr[16],gol[16],blokada[16]
 struct  time t;
 do
  {
   clrscr();
   randomize();
   textcolor(15);cputs("PlayeRom");
   textcolor(7);cputs("\n\r present:");
   //textcolor(15);cputs("\n\r\n\rMANAGER LIGI POLSKIEJ 2002");
   textbackground(15);textcolor(4);cputs("\n\r\n\r MANAGER LIGI  ");
   textbackground(4);textcolor(7);cputs("\n\r POLSKIEJ 2002 ");textbackground(0);
   textcolor(8);cputs("  v.1.04");
   textcolor(2);cputs("\n\r\n\rN Nowa gra\n\rW Wczytaj gr©");
   //textcolor(6);cputs("\n\rO Opcje dla meczu");
   textcolor(4);cputs("\n\rQ Wyjòcie\n\r");textcolor(2);
   pp1=getch();
   switch(toupper(pp1))
    {
     case 'N':
     textcolor(2);
     cputs("\n\r\n\rCzy na pewno chcesz rozpocz•Ü now• gr© (t/n): ");
     tn=getch();
     if (tn!='t') break;
      f=fopen("Rywal.cfg","wb");
      fclose(f);//szyszczenie starej gry
      f=fopen("Rywal2.cfg","rb");
      while(fread(&gracz,sizeof(gracz),1,f)==1)
       {
	f2=fopen("Rywal.cfg","ab");
	fwrite(&gracz,sizeof(gracz),1,f2);
	fclose(f2);
       }
      fclose(f);
      f=fopen("Gra_mana.cfg","wb");
      fclose(f);//szyszczenie starej gry
      clrscr();textcolor(7);
      cputs("\n\rPodaj swoje imi©: ");scanf("%s",Klub.imie_m);
      Klub.imie_m[0]=toupper(Klub.imie_m[0]);
      cputs("Podaj swoje nazwisko: ");scanf("%s",Klub.nazw_m);
      Klub.nazw_m[0]=toupper(Klub.nazw_m[0]);
      cputs("\n\rCzy chcesz podaÜ swoj• ksyw© (t/n): ");
      tn=getche();
      if (tn=='t')
       {cputs("\n\rPodaj sw¢j nickname: ");scanf("%s",Klub.nick);
       Klub.czy_nick=1;}
      else Klub.czy_nick=0;
      k=0;
      while(!k)
      {
       clrscr();
       cputs("\n\rWybierz klub, kt¢ry ma Tobie podlegaÜ:\n\r 1. Amica Wronki\n\r 2. Garbarnia Szczakowianka Jaworzno\n\r 3. GKS Katowice\n\r 4. G¢rnik Zabrze\n\r 5. Groclin Dyskobolia\n\r 6. KSZO Ostrowiec\n\r 7. Lech Pozna‰\n\r 8. Legia Warszawa\n\r 9. Odra Wodzisàaw\n\r10. Polonia Warszawa\n\r11. Pogo‰ Szczecin\n\r12. Ruch Chorz¢w\n\r13. Widzew ù¢d´\n\r14. Wisàa Krak¢w\n\r15. Wisàa Pàock\n\r16. Zagà©bie Lubin\n\r");
       cputs("\n\rWpisz odpowiedni numer: ");scanf("%d",&Klub.klub);
       if (Klub.klub<1||Klub.klub>16) k=0;
       else k=1;
      }
      cputs("\n\r\n\rProsz© czekaÜ...");
	 f=fopen("Kluby_01.cfg","rb");
	 while(fread(&gracz,sizeof(gracz),1,f)==1)
	  {
	   if (gracz.dane[22]==Klub.klub)
	    {
	     f2=fopen("Gra_mana.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	  }}
	 fclose(f);
	//}
      wiadomosc[0]=1;wiadomosc[1]=1;dlawiadomosc[0]=Klub.nazw_m;wiadomosc[2]=7;
      f=fopen("News_m.cfg","w");// wyczyòÜ
      fclose(f);                // wyczyòÜ
      nowa_gra=1;
      Klub.d=1;Klub.m=7;Klub.r=2002;Klub.tydzien=1;Klub.treBOPN=0;Klub.jestM=0;
      Klub.kolejka=1;Klub.usta=1;Klub.rywalTN=0;Klub.ilekontrol=1;Klub.nie=0;
      Klub.asystent=0;Klub.gol1=0;Klub.gol2=0;Klub.dane2_m[0]=0;Klub.dane2_m[1]=0;
      Klub.zadyma=0;Klub.kasa=0;Klub.sezon=0;
      //Klub.mecz=dlameczu;
      Klub.mecz=1;
      for(i=0;i<28;i++) Klub.trening[i]=trening2[i];
      for(i=0;i<5;i++) Klub.umie[i]=0;
      for(i=0;i<14;i++) {Klub.finanse[i]=0;Klub.finanse2[i]=0;}
      Klub.finanse[12]=500000.0;
      for(i=0;i<480;i++) Klub.gol[i]=0;
      for(i=0;i<10;i++) Klub.golK[i]=0;
      for(i=0;i<12;i++) Klub.dane_m[i]=0;
      for(i=0;i<6;i++)
       {
	if (i==0||i==1||i==5) Klub.inst[i]=1;
	else Klub.inst[i]=0;
       }
      f=fopen("Tabela.cfg","wb");
      fclose(f);//wyczyòÜ tabel©
      for(i=0;i<16;i++)
       {
	tabela.num=i+1;
	for(k=0;k<9;k++) tabela.dane[k]=0;
	f=fopen("Tabela.cfg","ab");
	fwrite(&tabela,sizeof(tabela),1,f);
	fclose(f);
       }
      Klub.ileTrans=0;
      f=fopen("Trans.cfg","wb");
      fclose(f);
      f=fopen("Transfer.cfg","rb");
      while(fread(&gracz,sizeof(gracz),1,f)==1)
       {
	Klub.ileTrans++;
	f2=fopen("Trans.cfg","ab");
	fwrite(&gracz,sizeof(gracz),1,f2);
	fclose(f2);
       }
      fclose(f);
      f=fopen("Klub.cfg","wb");
      fwrite(&Klub,sizeof(Klub),1,f);
      fclose(f);
      f=fopen("Kolejka2.cfg","wb");
      fclose(f);//szyszczenie starej
      f=fopen("Kolejka.cfg","rb");
      while(fread(&kolej,sizeof(kolej),1,f)==1)
       {
	f2=fopen("Kolejka2.cfg","ab");
	fwrite(&kolej,sizeof(kolej),1,f2);
	fclose(f2);
       }
      fclose(f);
      f=fopen("Lostmecz.cfg","w");
      fclose(f);
     case 'W':
      if (nowa_gra==0)
       {
	//wczytania klubu,skadu
	f=fopen("Klub.cfg","rb");
	fread(&Klub,sizeof(Klub),1,f);
	fclose(f);
       }
      do
       {
	if (Klub.asystent==1)//wiadomoòÜ asystenta po meczu
	 {
	  Klub.lostmecz=Klub.rywal[0];
	  f=fopen("001.cfg","r");
	  fscanf(f,"%d %d %d",&OnaA,&PnaP,&AnaO);
	  fclose(f);
	  numer1=-1;
	  if (Klub.gol1<=Klub.gol2)
	  {
	  for(i=1;i<10;i++)
	   {
	    if (wiadomosc[i]==0&&numer1==-1)
	     {
	      numer1=0;
	      if (AnaO<-20&&Klub.inst[4]==0) {wiadomosc[i+numer1]=20;numer1++;}
	      if (PnaP<-10&&Klub.inst[0]!=4) {wiadomosc[i+numer1]=21;numer1++;}
	      if (OnaA<0&&Klub.inst[3]==0) {wiadomosc[i+numer1]=22;numer1++;}
	      if (OnaA>20&&Klub.inst[3]==1) {wiadomosc[i+numer1]=23;numer1++;}
	      if (PnaP>10&&Klub.inst[0]!=2) {wiadomosc[i+numer1]=24;numer1++;}
	      if (AnaO>0&&Klub.inst[4]==1) {wiadomosc[i+numer1]=25;numer1++;}
	      if (Klub.inst[2]==0) {wiadomosc[i+numer1]=26;numer1++;}
	      if (Klub.inst[2]==1) {wiadomosc[i+numer1]=27;numer1++;}
	      if (Klub.inst[1]!=3) {wiadomosc[i+numer1]=28;numer1++;}
	      if (numer1>0) wiadomosc[0]=1;
	     }
	   }}
	  else
	   {
	    kk=random(2);
	    for(i=1;i<10;i++)
	     {
	      if (wiadomosc[i]==0&&numer1==-1)
	       {
		numer1=1;
		if (kk==0) wiadomosc[i]=29;
		else wiadomosc[i]=30;
		wiadomosc[0]=1;
	       }}}
	  Klub.asystent=0;
	  f=fopen("Klub.cfg","wb");
	  fwrite(&Klub,sizeof(Klub),1,f);
	  fclose(f);
	 }//dla asystent
	if (Klub.rywalTN==0)
	{
	sumaO=0;sumaN=0;Klub.rywalTN=1;
	f=fopen("Kolejka2.cfg","rb");
	   while(fread(&kolej,sizeof(kolej),1,f)==1)
	    {
	     if (Klub.kolejka==kolej.numer)
	      {
	       for (i=0;i<16;i++)
		if (Klub.klub==kolej.nr[i]) sumaO=i;
	      }}
	   fclose(f);
	   if (sumaO%2==0) {Klub.rywal[3]=sumaO+1;Klub.rywal[1]=0;}//u siebie
	   else {Klub.rywal[3]=sumaO-1;Klub.rywal[1]=1;}//wyjazd
	   f=fopen("Kolejka2.cfg","rb");
	   while(fread(&kolej,sizeof(kolej),1,f)==1)
	    {
	     //cprintf("%s",kolej.kol[Klub.rywal[3]]);
	     if (Klub.kolejka==kolej.numer) Klub.rywal[0]=kolej.nr[Klub.rywal[3]];//sumaO to numer przeciwnika
	    }
	   fclose(f);
	   if (Klub.rywal[0]==1||Klub.rywal[0]==10||Klub.rywal[0]==12||Klub.rywal[0]==13||Klub.rywal[0]==16) Klub.rywal[2]=1;
	   if (Klub.rywal[0]==2||Klub.rywal[0]==4||Klub.rywal[0]==11) Klub.rywal[2]=12;
	   if (Klub.rywal[0]==3||Klub.rywal[0]==9) Klub.rywal[2]=6;
	   if (Klub.rywal[0]==5) Klub.rywal[2]=13;
	   if (Klub.rywal[0]==6) Klub.rywal[2]=2;
	   if (Klub.rywal[0]==7) Klub.rywal[2]=10;
	   if (Klub.rywal[0]==8||Klub.rywal[0]==14) Klub.rywal[2]=3;
	   if (Klub.rywal[0]==15) Klub.rywal[2]=14;
	   Klub.rinst[0]=random(4)+1;//podania zawsze los
	   sumaN=random(3);
	   if (sumaN==0) Klub.rinst[1]=3;
	   else Klub.rinst[1]=1; //nigdy delikatne
	   sumaN=random(4);
	   if (sumaN==0) Klub.rinst[2]=0;
	   else Klub.rinst[2]=1;//pressing 3/4 æe tak
	   //
	   if (Klub.rywal[2]==1||Klub.rywal[2]==6||Klub.rywal[2]==3||Klub.rywal[2]==10)
	     Klub.rinst[3]=1;//puàapki tak
	   else if (Klub.rywal[2]==12||Klub.rywal[2]==14)
	    Klub.rinst[3]=random(2);
	   else Klub.rinst[3]=0;//end puàapki ofsajdowe
	   //
	   if (Klub.rywal[2]==2||Klub.rywal[2]==13)
	    Klub.rinst[4]=1;
	   else if (Klub.rywal[2]==3||Klub.rywal[2]==10||Klub.rywal[2]==14)
	    Klub.rinst[4]=0;
	   else Klub.rinst[4]=random(2);//end gra z kontry
	   //
	   if (Klub.rywal[2]==2||Klub.rywal[2]==13)
	    {
	     sumaN=random(3);
	     if (sumaN==0) Klub.rinst[5]=1;
	     else Klub.rinst[5]=2;//2/3 æe obronne, atak wcale
	    }
	   else if (Klub.rywal[2]==3||Klub.rywal[2]==10||Klub.rywal[2]==14)
	    {
	     sumaN=random(3);
	     if (sumaN==0) Klub.rinst[5]=1;
	     else Klub.rinst[5]=3;//2/3 æe atak, obronne wcale
	    }
	   else if (Klub.rywal[2]==6||Klub.rywal[2]==12)
	    {
	     sumaN=random(4);
	     if (sumaN==0) Klub.rinst[5]=2;
	     else if (sumaN==1) Klub.rinst[5]=3;
	     else Klub.rinst[5]=1;//2/4 æe normalne
	    }
	   else //dla ustawienia=1
	    {
	     sumaN=random(3);
	     if (sumaN==0) Klub.rinst[5]=3;
	     else Klub.rinst[5]=1;//2/3 æe normalne, obronne wcale
	    }
	   f=fopen("Klub.cfg","wb");
	   fwrite(&Klub,sizeof(Klub),1,f);
	   fclose(f);
	   f=fopen("wymiana.cfg","wb");
	   fclose(f);
	   f=fopen("Rywal.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     k=random(4);
	     gracz.dane[7]=k; gracz.dane[11]=100;
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
	 }
	//ustal rywala end
	info();dalej=0;//cprintf("\n\rkolejka: %d",Klub.kolejka);
	pp2=MenuGlowne(wiadomosc);textcolor(2);
	switch(toupper(pp2))
	 {
	  case 'C'://***************** kontynuuj ********************
	   if (Klub.jestM==0&&Klub.klub>0)
	    {
	     textcolor(7);cputs("\n\rProsz© czekaÜ...");Klub.kasa=0;
	   //*********** lista transferowa ****************************
	   f=fopen("wymiana.cfg","wb");
	   fclose(f);
	   f=fopen("Trans.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     if (gracz.dane[2]==1) gracz.dane[3]=random(15)+1;
	     if (gracz.dane[2]==2) gracz.dane[4]=random(15)+1;
	     if (gracz.dane[2]==3) gracz.dane[5]=random(15)+1;
	     if (gracz.dane[2]==4) gracz.dane[6]=random(15)+1;
	     gracz.dane[7]=random(7)-3;
	     gracz.dane[9]=random(6)+5;
	     //cena zawodnika
	     gracz.waga[0]=0;
	     if (gracz.dane[2]==1) gracz.waga[0]=gracz.dane[3]*5;
	     if (gracz.dane[2]==2) gracz.waga[0]=gracz.dane[4]*5;
	     if (gracz.dane[2]==3) gracz.waga[0]=gracz.dane[5]*5;
	     if (gracz.dane[2]==4) gracz.waga[0]=gracz.dane[6]*5;
	     for (i=3;i<7;i++)
	      if (gracz.dane[2]+2==i) kk=gracz.dane[i];
	     if (kk==20) placa=36000.0;
	     else if (kk>=17) placa=27000.0;
	     else if (kk>=15) placa=18000.0;
	     else if (kk>=12) placa=14000.0;
	     else if (kk>=10) placa=9000.0;
	     else if (kk>=5) placa=7000.0;
	     else placa=6000.0;
	     gracz.waga[0]+=gracz.dane[3]+gracz.dane[4]+gracz.dane[5]+gracz.dane[6];
	     gracz.waga[0]*=placa;
	     gracz.waga[0]+=gracz.dane[9]*10000.0;
	     f2=fopen("wymiana.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	   f=fopen("Trans.cfg","wb");
	   fclose(f);
	   f=fopen("wymiana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     f2=fopen("Trans.cfg","ab");
	     fwrite(&gracz,sizeof(gracz),1,f2);
	     fclose(f2);
	    }
	   fclose(f);
	   //********************************************************
	   if (wiadomosc[0]==1)//czy wog¢le jest jakaò wiadomoòÜ
	    {
	     for (i=1;i<10;i++)
	      {
	       f=fopen("News.cfg","rb");
	       while(fread(&news,sizeof(news),1,f)==1)
		{
		 if (wiadomosc[i]==news.num)//=1
		  {
		   f2=fopen("News_m.cfg","aw");
		   fprintf(f2,"\n\n");
		   fprintf(f2,"%02d.%02d.%d - ",Klub.d,Klub.m,Klub.r);
		   fprintf(f2,news.newss,dlawiadomosc[i-1]);
		   fclose(f2);
		  }}
	       fclose(f);
	      }
	     for (i=0;i<10;i++)
	      wiadomosc[i]=0;
	    }
	   k=0;
	   while(!k)
	   {
	   Klub.d++;
	   Klub.tydzien++;sumaN=0;
	   if (Klub.tydzien==8)
	    {
	     Klub.tydzien=1; //k=1;
	     for(i=0;i<28;i++)
	      {if (Klub.trening[i]==9) sumaN++;}//wolne
	     f=fopen("wymiana.cfg","wb");
	     fclose(f);//szyszczenie starej
	     if (sumaN==0)
	      {
	       f=fopen("Gra_mana.cfg","rb");
	       while(fread(&gracz,sizeof(gracz),1,f)==1)
		{
		 gracz.dane[8]-=3;
		 if (gracz.dane[8]<=-5) {gracz.dane[7]--; gracz.dane[8]=0;}
		 if (gracz.dane[7]<-3) gracz.dane[7]=-3;
		 f2=fopen("wymiana.cfg","ab");
		 fwrite(&gracz,sizeof(gracz),1,f2);
		 fclose(f2);
		}
	       fclose(f);
	       przepisz();
	      }
	     if (sumaN>1)
	      {
	       f=fopen("Gra_mana.cfg","rb");
	       while(fread(&gracz,sizeof(gracz),1,f)==1)
		{
		 gracz.dane[8]+=sumaN-1;
		 if (gracz.dane[8]>=5) {gracz.dane[7]++; gracz.dane[8]=0;}
		 if (gracz.dane[7]>3) gracz.dane[7]=3;
		 f2=fopen("wymiana.cfg","ab");
		 fwrite(&gracz,sizeof(gracz),1,f2);
		 fclose(f2);
		}
	       fclose(f);
	       przepisz();
	      }}
	   if ((Klub.m==1||Klub.m==3||Klub.m==5||Klub.m==7||Klub.m==8||Klub.m==10)&&Klub.d==32) {Klub.d=1;Klub.m++;}
	   if (Klub.m==2&&Klub.d==29) {Klub.d=1;Klub.m++;}
	   if ((Klub.m==4||Klub.m==6||Klub.m==9||Klub.m==11)&&Klub.d==31) {Klub.d=1;Klub.m++;}
	   if (Klub.m==12&&Klub.d==32) {Klub.d=1;Klub.m=1;Klub.r++;}
	   //****************************** co miesi•c ****************
	   if (Klub.d==1)
	    {
	     k=1;numer1=0;wiadomosc[0]=1;
	     lata=random(2)+1;
		 for(i=1;i<10;i++)
		  {
		   if (wiadomosc[i]==0&&numer1==0)
		    {
		     f=fopen("Tabela.cfg","rb");
		     while(fread(&tabela,sizeof(tabela),1,f)==1)
		      {
		       if (tabela.num==Klub.klub) kk=tabela.dane[8];
		       if (tabela.dane[8]==lata) sort=tabela.num;
		      }
		     fclose(f);
		     if (kk==0) {wiadomosc[i]=43; numer1=1;}
		     else if (kk<4) {wiadomosc[i]=31; numer1=1;}
		     else if (kk>3&&kk<10) {wiadomosc[i]=32; numer1=1;}
		     else if (kk>9&&kk<14) {wiadomosc[i]=33; numer1=1;}
		     else if (kk>13) {wiadomosc[i]=34; numer1=1;}
		     if (Klub.finanse[12]<=500000&&Klub.finanse[11]<=0) wiadomosc[i+1]=35;
		     else if (Klub.finanse[12]<=500000&&Klub.finanse[11]>0) wiadomosc[i+1]=36;
		     else if (Klub.finanse[12]<=1000000&&Klub.finanse[11]<=0) wiadomosc[i+1]=37;
		     else if (Klub.finanse[12]<=1000000&&Klub.finanse[11]>0) wiadomosc[i+1]=38;
		     else if (Klub.finanse[12]<=2000000&&Klub.finanse[11]<=0) wiadomosc[i+1]=39;
		     else if (Klub.finanse[12]<=2000000&&Klub.finanse[11]>0) wiadomosc[i+1]=40;
		     else if (Klub.finanse[12]>2000000&&Klub.finanse[11]<=0) wiadomosc[i+1]=41;
		     else if (Klub.finanse[12]>2000000&&Klub.finanse[11]>0) wiadomosc[i+1]=42;
		     //***** manager miesi•ca *****
		     if (Klub.m==4||Klub.m==5||Klub.m==6||Klub.m==7||Klub.m==9||Klub.m==10||Klub.m==11||Klub.m==12)
		      {
		       if (sort==Klub.klub)
			{
			 wiadomosc[i+2]=sort+60;
			 dlawiadomosc[i+1]=Klub.nazw_m;
			 Klub.dane_m[2]++;
			 Klub.dane_m[3]+=(Klub.dane_m[2]*10);
			}
		       else wiadomosc[i+2]=sort+44;
		      }
		  }}
	    }
	   //**********************************************************
	   f=fopen("Kolejka2.cfg","rb");
	   while(fread(&kolej,sizeof(kolej),1,f)==1)
	    {
	     //if (Klub.d==kolej.d&&Klub.m==kolej.m) {Klub.jestM=1;k=1;}
	     if (Klub.d-1==kolej.d&&Klub.m==kolej.m)
	      {Klub.rywalTN=0;Klub.kolejka=kolej.numer+1;}
	     if (Klub.d==kolej.d&&Klub.m==kolej.m) {Klub.jestM=1;k=1;}
	    }
	   fclose(f);
	   //}
	   //ustalam kolejk© end
	   lata=0;transfer=0;cena=0;placa=0;premia=0;
	   for(i=Klub.tydzien-1;i<28;i+=7)//sprawdzamy dany dzien
	    {
	     if (Klub.trening[i]==5) lata++;
	     if (Klub.trening[i]==1) transfer++;//kond
	     if (Klub.trening[i]==2) cena++;//podania
	     if (Klub.trening[i]==3) placa++;//staàe
	     if (Klub.trening[i]==4) premia++;//taktyka
	     //if (Klub.trening[i]==9) sumaN++;//wolne
	    }
	   Klub.umie[0]+=transfer*0.2;
	   if (transfer==0) Klub.umie[0]-=0.1;
	   Klub.umie[1]+=cena*0.2;
	   if (cena==0) Klub.umie[1]-=0.1;
	   Klub.umie[2]+=placa*0.2;
	   if (placa==0) Klub.umie[2]-=0.1;
	   Klub.umie[3]+=premia*0.2;
	   if (premia==0) Klub.umie[3]-=0.1;
	   numer1=0;
	   f=fopen("Gra_mana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1) numer1++;
	   fclose(f);
	   f=fopen("wymiana.cfg","wb");
	   fclose(f);//szyszczenie starej
	   i=random(12);//losuje czy odj•Ü 1/12
	   if (lata==0&&i==0)
	    {
	     Klub.treBOPN--;
	     numer2=random(numer1)+1;
	     f=fopen("Gra_mana.cfg","rb");
	     while(fread(&gracz,sizeof(gracz),1,f)==1)
	      {
	       if (gracz.dane[0]==numer2)//--
		{
		 i=random(4)+3;
		 gracz.dane[i]--;
		 if (gracz.dane[i]<1) {gracz.dane[i]=1;Klub.treBOPN++;}
		}
	       f2=fopen("wymiana.cfg","ab");
	       fwrite(&gracz,sizeof(gracz),1,f2);
	       fclose(f2);
	      }
	     fclose(f);
	     przepisz();
	    }//dla lata==0
	   i=random(6);//losuje czy dodaÜ 1/6 szans
	   if (lata>0&&i==0)
	    {
	     /*kk=0;
	     while(kk!=3)
	     {
	     kk++; */
	     numer2=random(numer1)+1;
	     f=fopen("wymiana.cfg","wb");
	     fclose(f);//szyszczenie starej
	     f=fopen("Gra_mana.cfg","rb");
	     while(fread(&gracz,sizeof(gracz),1,f)==1)
	      {
	       if (gracz.dane[0]==numer2)//--
		{
		 for(i=1;i<5;i++)
		  {
		   if (gracz.dane[1]==i)
		    {
		     gracz.dane[i+2]++;Klub.treBOPN++;
		     if (gracz.dane[i+2]>20) {gracz.dane[i+2]=20;Klub.treBOPN--;}
		    }}}
	       f2=fopen("wymiana.cfg","ab");
	       fwrite(&gracz,sizeof(gracz),1,f2);
	       fclose(f2);
	      }
	     fclose(f);
	     przepisz();
	    }//}//dla if (lata>0)
	   //dni kontrakt¢w
	   f=fopen("wymiana.cfg","wb");
	   fclose(f);//szyszczenie starej
	   sumaP=0;sumaN=0;
	   Klub.finanse[6]=0;
	   f=fopen("Gra_mana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1)
	    {
	     //************ sprzedaæ zawodnika *****************
	     //lista transferowa
	     kupno=random(20);
	     if (gracz.dane[17]==1&&kupno==0)
	      {
	       //sumaO=random(20);
	       //if (sumaO==0) {kupno=gracz.dane[0];k=1;}
		 clrscr();tryb=0;
		 while(!tryb)
		  {
		   lata=random(16)+1;
		   if (lata!=Klub.klub) tryb=1;
		  }
		 textcolor(7);kk=random(10)-5;
		 transfer=kk*10000.0; cena=transfer+gracz.waga[0];
		 if (cena>gracz.waga[3]) cena=gracz.waga[3];
		 cprintf("\n\r%s jest zainteresowana kupnem %s%s\n\rWartoòÜ %s wynosi %.2f zà.\n\r%s æ•da %.2f zà.\n\r%s jest gotowa zapàaciÜ %.2f zà.",krajMS[lata],gracz.imie,gracz.nazwisko,gracz.nazwisko,gracz.waga[0],krajMS[Klub.klub-1],gracz.waga[3],krajMS[lata],cena);
		 cputs("\n\r\n\rCzy akceptujesz propozycj© (t/n): ");
		 tn=getch();
		 if (tn=='t')
		  {
		   textcolor(2);
		   cprintf("\n\r\n\rKontrakt %s wygasa. %s zostaje sprzedany do %s.",gracz.nazwisko,gracz.nazwisko,krajMS[lata]);
		   gracz.dane[18]=1;
		   gracz.dane[17]=0;
		   gracz.dane[22]=lata+1;
		   Klub.finanse[3]+=cena;
		   Klub.finanse[5]=Klub.finanse[0]+Klub.finanse[1]+Klub.finanse[2]+Klub.finanse[3]+Klub.finanse[4];
		   Klub.finanse[11]=Klub.finanse[5]-Klub.finanse[10];
		   Klub.dane_m[11]++;Klub.dane2_m[1]+=cena;
		   f3=fopen("Klub.cfg","wb");
		   fwrite(&Klub,sizeof(Klub),1,f3);
		   fclose(f3);
		   getch();
		  }
		 else
		  {
		   textcolor(4);
		   cprintf("\n\r\n\r%s odrzuca propozycj©.",krajMS[Klub.klub-1]);
		   getch();
		  }
	      }//dla if
	     //*********************************************
	     numer1=0;
	     gracz.dane[18]--;
	     //finanse - pàaca
	     Klub.finanse[6]+=gracz.waga[1];
	     //cena zawodnika
	     gracz.waga[0]=0;
	     if (gracz.dane[2]==1) gracz.waga[0]=gracz.dane[3]*5;
	     if (gracz.dane[2]==2) gracz.waga[0]=gracz.dane[4]*5;
	     if (gracz.dane[2]==3) gracz.waga[0]=gracz.dane[5]*5;
	     if (gracz.dane[2]==4) gracz.waga[0]=gracz.dane[6]*5;
	     for (i=3;i<7;i++)
	      if (gracz.dane[2]+2==i) cena=gracz.dane[i];
	     if (cena==20) placa=36000.0;
	     else if (cena>=17) placa=27000.0;
	     else if (cena>=15) placa=18000.0;
	     else if (cena>=12) placa=14000.0;
	     else if (cena>=10) placa=9000.0;
	     else if (cena>=5) placa=7000.0;
	     else placa=6000.0;
	     gracz.waga[0]+=gracz.dane[3]+gracz.dane[4]+gracz.dane[5]+gracz.dane[6];
	     gracz.waga[0]*=placa;
	     gracz.waga[0]+=gracz.dane[9]*10000.0;
	     //cena zawodnika end
	     //kondycja
	     if (gracz.dane[11]<100)
	      {
	       if (Klub.umie[0]>=0) gracz.dane[11]+=2+Klub.umie[0];
	       else gracz.dane[11]+=2;
	       if (gracz.dane[11]>100) gracz.dane[11]=100;
	      }
	     //kondycja end
	     //kontuzja
	     if (gracz.dane[19]==1)
	      {
	       gracz.dane[19]=0;k=1;wiadomosc[0]=1;
	       for(i=1;i<10;i++)
		  {
		   if (wiadomosc[i]==0&&numer1==0)
		    {
		     wiadomosc[i]=6; numer1=1;
		     for(sumaO=0;sumaO<20;sumaO++)
		      zastep[i-1][sumaO]=gracz.nazwisko[sumaO];
		     dlawiadomosc[i-1]=zastep[i-1];
		     numerW[i-1]=random(83)+8;
		     gracz.dane[15]=numerW[i-1];
		     gracz.dane[9]--;//spadek formy
		     if (gracz.dane[9]<1) gracz.dane[9]=1;
		  }}}
	     //numer1=0;
	     if (gracz.dane[15]>0)
	      {
	       gracz.dane[15]--;
	       gracz.dane[1]=0;//nie trenuje
	       if (gracz.dane[15]>7) gracz.dane[11]=0;
	       if (gracz.dane[15]<=7) gracz.dane[11]=90;
	       if (gracz.dane[15]==0||gracz.dane[15]==7)
		{
		 if (gracz.dane[15]==0) gracz.dane[1]=gracz.dane[2];
		 k=1;numer1=0;wiadomosc[0]=1;
		 for(i=1;i<10;i++)
		  {
		   if (wiadomosc[i]==0&&numer1==0)
		    {
		     if (gracz.dane[15]==0) {wiadomosc[i]=4; numer1=1;}
		     else if (gracz.dane[15]==7) {wiadomosc[i]=5; numer1=1;}
		     for(sumaO=0;sumaO<20;sumaO++)
		      zastep[i-1][sumaO]=gracz.nazwisko[sumaO];
		     dlawiadomosc[i-1]=zastep[i-1];
		  }}}}
	     //kontuzja end
	     if (gracz.dane[18]==0)//kontrakt
	      {
	       sumaP=gracz.dane[0]; sumaN++;
	       if (sumaN>=2) sumaP-=sumaN-1;
	      }
	     if (gracz.dane[18]>0)
	      {
	       if (sumaN>0) {gracz.dane[0]=sumaP;sumaP++;}
	       f2=fopen("wymiana.cfg","ab");
	       fwrite(&gracz,sizeof(gracz),1,f2);
	       fclose(f2);
	      }
	     numer1=0;
	     if (gracz.dane[18]==0)
	      {
	       k=1;
	       wiadomosc[0]=1;
	       for(i=1;i<10;i++)
		{
		 if (wiadomosc[i]==0&&numer1==0)
		  {
		   wiadomosc[i]=3; numer1=1;
		   for(sumaO=0;sumaO<20;sumaO++)
		    zastep[i-1][sumaO]=gracz.nazwisko[sumaO];
		   dlawiadomosc[i-1]=zastep[i-1];
		  }}
	       Klub.ileTrans++;
	       gracz.dane[0]=Klub.ileTrans;
	       if (gracz.dane[22]==Klub.klub) gracz.dane[22]=0;
	       f2=fopen("Trans.cfg","ab");
	       fwrite(&gracz,sizeof(gracz),1,f2);
	       fclose(f2);
	      }
	     numer1=0;
	     if (gracz.dane[18]==7)
	      {
	       k=1;
	       wiadomosc[0]=1;
	       for(i=1;i<10;i++)
		{
		 if (wiadomosc[i]==0&&numer1==0)
		  {
		   wiadomosc[i]=2; numer1=1;
		   for(sumaO=0;sumaO<20;sumaO++)
		    zastep[i-1][sumaO]=gracz.nazwisko[sumaO];
		   dlawiadomosc[i-1]=zastep[i-1];
		  }}}
	    }
	   fclose(f);
	   przepisz();
	   //******************** komunikaty kibic¢w ******************
	   kk=random(6);
	   numer1=0;//kk=0;
	   /*f=fopen("001.cfg","r");
	   fscanf(f,"%d %d",&gol1,&gol2);
	   fclose(f);*/
	   if (kk==0&&Klub.gol1!=Klub.gol2)
	    {
	       if (Klub.gol1>Klub.gol2) kk=random(6)+8;
	       else if (Klub.gol1<Klub.gol2) kk=random(6)+14;
	       k=1;wiadomosc[0]=1;
		 for(i=1;i<10;i++)
		  {
		   if (wiadomosc[i]==0&&numer1==0)
		    {
		     wiadomosc[i]=kk; numer1=1;
		     if (Klub.czy_nick==1) dlawiadomosc[i-1]=Klub.nick;
		     else dlawiadomosc[i-1]=Klub.nazw_m;
		     if (kk==9)
		      {if (Klub.czy_nick==1) dlawiadomosc[i-1]=Klub.nick;
		       else dlawiadomosc[i-1]=Klub.nazw_m;
		  }}}
	    }//dla k==0
	   //
	   //if (Klub.d==20) {k=1;wiadomosc[0]=1;wiadomosc[1]=1;wiadomosc[2]=1;}
	   //****************** zadyma **************
	   if (Klub.zadyma==1)
	      {
	       k=1;wiadomosc[0]=1;numer1=0;Klub.zadyma=0;
	       for(i=1;i<10;i++)
		  {
		   if (wiadomosc[i]==0&&numer1==0)
		    {
		     wiadomosc[i]=44; numer1=1;
		     dlawiadomosc[i-1]=krajMS[Klub.klub-1];
		     Klub.finanse[9]+=300000.0;
		  }}}
	   //******** wywalenie z klubu i koniec ligi**************
	   if (Klub.m==7&&Klub.d==1)//nowy sezon
	    {
	     Klub.treBOPN=0;Klub.jestM=0;Klub.kolejka=1;
	     Klub.rywalTN=0;Klub.ilekontrol=1;Klub.nie=0;
	     Klub.asystent=0;Klub.gol1=0;Klub.gol2=0;
	     Klub.zadyma=0;Klub.kasa=0;Klub.sezon++;
	     //for(i=0;i<28;i++) Klub.trening[i]=trening2[i];
	     for(i=0;i<5;i++) Klub.umie[i]=0;
	     for(i=0;i<480;i++) Klub.gol[i]=0;
	     for(i=0;i<10;i++) Klub.golK[i]=0;
	     f=fopen("Kolejka2.cfg","wb");
	     fclose(f);//szyszczenie starej
	     f=fopen("Kolejka.cfg","rb");
	     while(fread(&kolej,sizeof(kolej),1,f)==1)
	      {
	       f2=fopen("Kolejka2.cfg","ab");
	       fwrite(&kolej,sizeof(kolej),1,f2);
	       fclose(f2);
	      }
	     fclose(f);
	     f=fopen("Tabela.cfg","wb");
	     fclose(f);//wyczyòÜ tabel©
	     for(i=0;i<16;i++)
	      {
	       tabela.num=i+1;
	       for(kk=0;kk<9;kk++) tabela.dane[kk]=0;
	       f=fopen("Tabela.cfg","ab");
	       fwrite(&tabela,sizeof(tabela),1,f);
	       fclose(f);
	      }
	     k=1;
	    }
	   if (Klub.m==6&&Klub.d==23)
	    {
	     f2=fopen("Tabela.cfg","rb");
	     while(fread(&tabela,sizeof(tabela),1,f2)==1)
	      {
	       if (tabela.dane[8]==1) kk=tabela.num;
	      }
	     fclose(f2);
	     k=1;wiadomosc[0]=1;numer1=0;//Klub.zadyma=0;
	     for(i=1;i<10;i++)
	      {
	       if (wiadomosc[i]==0&&numer1==0)
		{
		 wiadomosc[i]=78; numer1=1;
		 dlawiadomosc[i-1]=krajMS[kk-1];
		}}
	    }
	   kk=0;
	   if ((Klub.m==2&&Klub.d==1)||(Klub.m==6&&Klub.d==23))
	    {
	     for(i=0;i<5;i++) Klub.umie[i]=0;
	     f=fopen("wymiana.cfg","wb");
	     fclose(f);//szyszczenie starej
	     f=fopen("Gra_mana.cfg","rb");
	     while(fread(&gracz,sizeof(gracz),1,f)==1)
	      {
	       gracz.dane[9]=6;gracz.dane[10]=0;
	       f2=fopen("wymiana.cfg","ab");
	       fwrite(&gracz,sizeof(gracz),1,f2);
	       fclose(f2);
	      }
	     fclose(f);
	     przepisz();
	    }
	   f2=fopen("Tabela.cfg","rb");
	   while(fread(&tabela,sizeof(tabela),1,f2)==1)
	    {
	     if (Klub.klub==tabela.num) kk=tabela.dane[8];
	    }
	   fclose(f2);
	   if (Klub.m==6&&Klub.d==23&&kk==1) //wygranie ligi
	    {
	     Klub.dane_m[0]++;
	     Klub.dane_m[3]+=(Klub.dane_m[0]*100);//+(Klub.dane_m[2]*10)+(Klub.dane_m[4]*10)+(Klub.dane_m[5]*3)+Klub.dane_m[6]-Klub.dane_m[7]+Klub.dane_m[8]-Klub.dane_m[9];
	     k=1;wiadomosc[0]=1;numer1=0;//Klub.zadyma=0;
	     for(i=1;i<10;i++)
	      {
	       if (wiadomosc[i]==0&&numer1==0)
		{
		 wiadomosc[i]=79; numer1=1;
		}}
	    }
	   if (((Klub.m==12&&Klub.d==1)||(Klub.m==6&&Klub.d==25))&&kk>13)
	    {
	     k=1;wiadomosc[0]=1;numer1=0;//Klub.zadyma=0;
	       for(i=1;i<10;i++)
		  {
		   if (wiadomosc[i]==0&&numer1==0)
		    {
		     wiadomosc[i]=77; numer1=1;
		     dlawiadomosc[i-1]=krajMS[Klub.klub-1];
		     Klub.klub=0;
		  }}
	    }
	   //finanse
	   Klub.finanse[5]=Klub.finanse[0]+Klub.finanse[1]+Klub.finanse[2]+Klub.finanse[3]+Klub.finanse[4];
	   Klub.finanse[10]=Klub.finanse[6]+Klub.finanse[7]+Klub.finanse[8]+Klub.finanse[9];
	   Klub.finanse[11]=Klub.finanse[5]-Klub.finanse[10];
	   if (Klub.d==1)
	    {
	     Klub.finanse[12]+=Klub.finanse[8];
	     for(i=0;i<12;i++) Klub.finanse2[i]=Klub.finanse[i];
	     for(i=0;i<11;i++) Klub.finanse[i]=0;
	     Klub.finanse[12]+=Klub.finanse[11];
	     f=fopen("Gra_mana.cfg","rb");
	     while(fread(&gracz,sizeof(gracz),1,f)==1)
	      {
	       Klub.finanse[6]+=gracz.waga[1];
	      }
	     fclose(f);
	     Klub.finanse[10]=Klub.finanse[6];
	     Klub.finanse[11]=Klub.finanse[5]-Klub.finanse[10];
	    }
	   Klub.finanse[13]=Klub.finanse[12]/2;
	   f=fopen("Klub.cfg","wb");
	   fwrite(&Klub,sizeof(Klub),1,f);
	   fclose(f);
	   }}
	   else if (Klub.jestM==1&&Klub.klub>0)
	    {
	     Klub.nie=0;
	     f=fopen("Gra_mana.cfg","rb");
	     while(fread(&gracz,sizeof(gracz),1,f)==1)
	      {
	       if (gracz.dane[0]<12&&(gracz.dane[13]==2||gracz.dane[14]==1||gracz.dane[15]>7))
		Klub.nie=1;
	      }
	     fclose(f);
	     f=fopen("Klub.cfg","wb");
	     fwrite(&Klub,sizeof(Klub),1,f);
	     fclose(f);
	     if (Klub.nie==0)
	      {
	       textcolor(2);cputs("\n\r\n\rCzy chcesz przejòÜ do meczu (t/n): ");
	       tn2=getch();
	       if (tn2=='t')
		{
		 pp2='Q';
		 pp1='Q';
		 PrzechodzeDoMeczu=1;
		}
	      }
	     else if (Klub.nie==1)
	      {
	       textcolor(12);
	       cputs("\n\r\n\rNie moæna rozegraÜ meczu! W skàadzie wyst©puj• niepowoàani zawodnicy!");
	       getch();
	      }}
	    else //Klub.klub==0
	     {
	      info();
	      k=0;kk=0;
	      f=fopen("Tabela.cfg","rb");
	      while(fread(&tabela,sizeof(tabela),1,f)==1)
	       {
		if (tabela.dane[8]==12) k=tabela.num;
		if (tabela.dane[8]==13) kk=tabela.num;
	       }
	      fclose(f);
	      textcolor(7);
	      cprintf("\n\r\n\rOtrzymujesz dwie propozycje pracy na stanowisku managera od klub¢w:\n\r1. %s\n\r2. %s",krajMS[k-1],krajMS[kk-1]);
	      cputs("\n\r\n\rW kt¢rym klubie chcesz pracowaÜ (wpisz odpowiedni numer): ");
	      scanf("%d",&numer1);
	      if (numer1==1||numer1==2)
	       {
		if (numer1==1) Klub.klub=k;
		else Klub.klub=kk;
		f=fopen("Gra_mana.cfg","wb");
		fclose(f);
		f=fopen("Kluby_01.cfg","rb");
		while(fread(&gracz,sizeof(gracz),1,f)==1)
		 {
		  if (gracz.dane[22]==Klub.klub)
		   {
		    f2=fopen("Gra_mana.cfg","ab");
		    fwrite(&gracz,sizeof(gracz),1,f2);
		    fclose(f2);
		   }}
		fclose(f);
		wiadomosc[0]=1;wiadomosc[1]=1;dlawiadomosc[0]=Klub.nazw_m;
		Klub.treBOPN=0;Klub.jestM=0;
		Klub.usta=1;Klub.rywalTN=0;Klub.ilekontrol=1;Klub.nie=0;
		Klub.asystent=0;Klub.gol1=0;Klub.gol2=0;
		Klub.zadyma=0;Klub.kasa=0;
		for(i=0;i<28;i++) Klub.trening[i]=trening2[i];
		for(i=0;i<5;i++) Klub.umie[i]=0;
		for(i=0;i<14;i++) {Klub.finanse[i]=0;Klub.finanse2[i]=0;}
		for(i=0;i<6;i++)
		 {
		  if (i==0||i==1||i==5) Klub.inst[i]=1;
		  else Klub.inst[i]=0;
		 }
		f=fopen("Kolejka2.cfg","wb");
		fclose(f);//szyszczenie starej
		f=fopen("Kolejka.cfg","rb");
		while(fread(&kolej,sizeof(kolej),1,f)==1)
		 {
		  f2=fopen("Kolejka2.cfg","ab");
		  fwrite(&kolej,sizeof(kolej),1,f2);
		  fclose(f2);
		 }
		fclose(f);
		f=fopen("Klub.cfg","wb");
		fwrite(&Klub,sizeof(Klub),1,f);
		fclose(f);
		textcolor(2);
		cprintf("\n\rPodj©to prac© w klubie %s",krajMS[Klub.klub-1]);
		getch();
	       }
	      else //if (numer1<1||numer1>2)
	       {
		textcolor(4);cputs("\n\rWpisano zày numer!");
		getch();
	       }
	     }
	   break;
	  case 'A'://taktyka
	   if (Klub.klub>0)
	   {
	   do
	   {
	   info();//sumaO=0;
	   takty(Klub.usta,0);
	   wykres(Klub.usta,0);
	   //tryb=11;//cena=0;
	   sklad(Klub.usta,11,0,0);
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case 'U':
	      i=0;
	      while(!i)
	      {
	      clrscr();textbackground(0);textcolor(12);
	      cprintf("Obecne ustawienie: %s",taktyka[Klub.usta-1]);textcolor(7);
	      cputs("\n\rWybierz ustawienie zespoàu:\n\r 1. 4-4-2\n\r 2. 4-4-2 Obrona\n\r 3. 4-4-2 Atak\n\r 4. 4-4-2 Diamond\n\r 5. 3-4-3\n\r 6. 3-5-2\n\r 7. 3-5-2 Obrona\n\r 8. 3-5-2 Atak\n\r 9. 4-2-4\n\r10. 4-3-3\n\r11. 4-5-1\n\r12. 5-3-2\n\r13. 5-3-2 Obrona\n\r14. 5-3-2 Atak\n\r\n\rWpisz odpowiedni• cyfr©: ");
	      scanf("%d",&Klub.usta);
	      if (Klub.usta<15&&Klub.usta>0) i=1;
	      else Klub.usta=15;
	      f=fopen("Klub.cfg","wb");
	      fwrite(&Klub,sizeof(Klub),1,f);
	      fclose(f);
	      }
	      break;
	     case 'P':
	      zamiana();
	      break;
	     case 'R':
	      info();textcolor(15);cputs("\n\rREZERWOWI");
	      //tryb=16;cena=0;
	      sklad(Klub.usta,16,0,0);
	      textcolor(7);cputs("\n\r\n\rNaciònij dowolny klawisz...");
	      getch();
	      break;
	     case 'I':
	      belka=1;
	      do
	      {
	      info();
	      textcolor(15);cputs("\n\r\n\rINSTRUKCJE DLA DRUΩYNY\n\r");
	      instrukcje(Klub.inst[0],Klub.inst[1],Klub.inst[2],Klub.inst[3],Klub.inst[4],Klub.inst[5],belka);
	      textcolor(4);cputs("\n\r Q Powr¢t\n\r");textcolor(2);
	      textbackground(0); //cputs("           - zmiana");
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
	   }
	   break;
	  case 'S'://skàad
	  if (Klub.klub>0)
	   {
	  sort=0;
	   do
	   {
	   info();textcolor(15);
	   cputs("\n\rSKùAD ");
	   if (sort==3) cputs("- sortowany wg umiej©tnoòci B");
	   if (sort==4) cputs("- sortowany wg umiej©tnoòci O");
	   if (sort==5) cputs("- sortowany wg umiej©tnoòci P");
	   if (sort==6) cputs("- sortowany wg umiej©tnoòci N");
	   if (sort==7) cputs("- sortowany wg morale");
	   if (sort==9) cputs("- sortowany wg formy");
	   if (sort==11) cputs("- sortowany wg kondycji");
	   if (sort==16) cputs("- sortowany wg goli");
	   if (sort==18) cputs("- sortowany wg kolejnoòci wygaòni©cia kontraktu");
	   textcolor(7);//i=0;
	   if (sort==0)
	    {
	     if (dalej==0) tryb=20;
	     if (dalej==1) tryb=40;
	    }
	   sklad(Klub.usta,tryb,0,sort);
	   i=0;//sort=0;
	   f=fopen("Gra_mana.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1) i++;
	   fclose(f);
	   textcolor(2);
	   if (sort==0)
	    {
	     if (i>20) cputs("\n\rD Dalej   ");
	     else cputs("\n\r");
	     cputs("P Zamiana zawodnik¢w   G Szczeg¢ày   S Sortuj");
	    }
	   else cputs("\n\rT Tradycyjny widok   G Szszeg¢ày   S Sortuj");
	   textcolor(4);cputs("   Q Powr¢t do MENU");textcolor(2);
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case 'T':
	      sort=0;
	      break;
	     case 'D':
	      if (sort==0)
	       {
		if (dalej==0&&i>20) dalej=1;
		else dalej=0;
	       }
	      break;
	     case 'S':
	     clrscr();
	      cputs("\n\rSortuj wedàug:\n\r B Umiej©tnoòci B\n\r O Umiej©tnoòci O\n\r P Umiej©tnoòci P\n\r N Umiej©tnoòci N\n\r M Morale\n\r F Formy\n\r K Kondycji\n\r G Goli\n\r W Kolejnoòci wygaòni©cia kontraktu");//\n\r C Ceny\n\r E Pensji");
	      textcolor(4);cputs("\n\r Q Powr¢t\n\r");textcolor(2);
	      pp_usta2=getch();
	      switch(toupper(pp_usta2))
	       {
		case 'B':
		 sort=3;
		 break;
		case 'O':
		 sort=4;//kk kt¢r• dan• grcza
		 break;
		case 'P':
		 sort=5;//kk kt¢r• dan• grcza
		 break;
		case 'N':
		 sort=6;//kk kt¢r• dan• grcza
		 break;
		case 'M'://morale
		 sort=7;//kk kt¢r• dan• grcza
		 break;
		case 'F'://forma
		 sort=9;
		 break;
		case 'K'://kondycja
		 sort=11;
		 break;
		case 'G'://gole
		 sort=16;
		 break;
		case 'W':
		 sort=18;
		 break;
		case 'C'://cena
		 break;
		case 'E'://pensja
		 break;
		default:
		 sort=0;
		 break;
		}//dla switch
	      break;
	     case 'G':
	      f=fopen("wymiana.cfg","wb");
	      fclose(f);//szyszczenie starej
	      textcolor(7);cprintf("\n\rPodaj numer Lp.: ");scanf("%d",&k);
	      f=fopen("Gra_mana.cfg","rb");
	      while(fread(&gracz,sizeof(gracz),1,f)==1)
	       {
		if (k==gracz.dane[0])
		 {
		  do
		  {
		  info();textcolor(7);
		  cprintf("\n\r\n\rNazwisko i imi©: %s %s",gracz.nazwisko,gracz.imie);
		  if (gracz.dane[2]==1) morale="Bramkarz";
		  if (gracz.dane[2]==2) morale="Obro‰ca";
		  if (gracz.dane[2]==3) morale="Pomocnik";
		  if (gracz.dane[2]==4) morale="Napastnik";
		  cprintf("\n\r\n\rPozycja: %s\n\rUmiej©tnoòci: B-%d, O-%d, P-%d, N-%d     Gole: %d",morale,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],gracz.dane[16]);
		  if (gracz.dane[7]==-3) morale="Fatalne";
		  if (gracz.dane[7]==-2) morale="Zàe";
		  if (gracz.dane[7]==-1) morale="Niskie";
		  if (gracz.dane[7]==0) morale="órednie";
		  if (gracz.dane[7]==1) morale="Dobre";
		  if (gracz.dane[7]==2) morale="B.dobre";
		  if (gracz.dane[7]==3) morale="Super";
		  cprintf("\n\r\n\rMorale: %s    Forma: %d    Kondycja: %d%%\n\r\n\rΩ¢àte kartki: %d    Czerwone kartki: %d",morale,gracz.dane[9],gracz.dane[11],gracz.dane[13],gracz.dane[14]);
		  //cprintf("\n\r\n\rΩ¢àte kartki: %d    Czerwone kartki: %d",gracz.dane[13],gracz.dane[14]);
		  if (gracz.dane[15]>0) {textcolor(9);cprintf("\n\r\n\rIloòÜ dni do wyleczenia kontuzji: %d",gracz.dane[15]);textcolor(7);}
		  textcolor(7);
		  cprintf("\n\r\n\rKontrakt wygaònie za: %d dni(dzie‰)\n\rCena zawodnika: %.2f zà.\n\rPàaca miesi©czna: %.2f zà.",gracz.dane[18],gracz.waga[0],gracz.waga[1]);
		  if (gracz.dane[2]==3||gracz.dane[2]==4) cprintf("\n\rPremia za gola: %.2f zà.",gracz.waga[2]);
		  if (gracz.dane[17]==1) {textcolor(9);cprintf("\n\r\n\rZawodnik na sprzedaæ za %.2f zà.",gracz.waga[3]);textcolor(7);}
		  textcolor(2);
		  cputs("\n\r\n\rK Nowy kontrakt\n\rT Na sprzedaæ/Anuluj\n\rW Wydalenie z klubu");textcolor(4);
		  cputs("\n\rQ Powr¢t\n\r");textcolor(2);
		  pp_usta2=getch();
		  switch(toupper(pp_usta2))
		   {
		    case 'W':
		     info();
		     textcolor(7);
		     cprintf("\n\r\n\rCzy na pewno chcesz wyrzuciÜ %s%s z klubu (t/n): ",gracz.imie,gracz.nazwisko);
		     tn=getch();
		     if (tn=='t')
		      {
		       cprintf("\n\rJutro wygaònie kontrakt %s%s.",gracz.imie,gracz.nazwisko);
		       gracz.dane[18]=1;
		       getch();
		      }
		     break;
		    case 'T':
		     if (gracz.dane[17]==0)
		      {
		       info();textcolor(15);
		       cprintf("\n\r\n\rSprzedaæ: %s %s",gracz.imie,gracz.nazwisko);
		       textcolor(7);
		       cprintf("\n\r\n\rPodaj cen© (%.2f): ",gracz.waga[0]);scanf("%f",&gracz.waga[3]);
		       if (gracz.dane[7]>0)
		       {gracz.dane[7]--;gracz.dane[8]=0;if (gracz.dane[7]<-3) gracz.dane[7]=-3;}
		       gracz.dane[17]=1;
		      }
		     else if (gracz.dane[17]==1) gracz.dane[17]=0;
		     break;
		    case 'K':
		     numer1=0;
		     info();textcolor(15);cprintf("\n\r\n\rKontrakt dla: %s %s",gracz.imie,gracz.nazwisko);
		     textcolor(7);//cprintf("\n\r\n\rPodaj cen© (%.2f): ",gracz.waga[0]);scanf("%f",&cena);
		     cprintf("\n\r\n\rPodaj pàac© miesi©czn• (%.2f): ",gracz.waga[1]);scanf("%f",&placa);
		     if (gracz.dane[2]==3||gracz.dane[2]==4){cprintf("\n\rPodaj premi© za gola (%.2f): ",gracz.waga[2]);scanf("%f",&premia);}
		     cprintf("\n\rPodaj przez ile lat ma obowi•zywaÜ kontrakt (1,2 lub 3): ");scanf("%d",&lata);
		     if (gracz.dane[2]==3||gracz.dane[2]==4)
		     {
		     transfer=premia-gracz.waga[2];
		     if (transfer==0) numer1++;
		     if (transfer>=50&&transfer<100) numer1+=2;
		     if (transfer>=100) numer1+=3;
		     if (transfer<0) numer1--;
		     }
		     else numer1++;
		     transfer=placa-gracz.waga[1];
		     if (transfer<0) numer1--;
		     if (transfer<-1000) numer1-=3;
		     if (transfer==0) numer1++;
		     if (transfer>=100&&transfer<500) numer1+=2;
		     if (transfer>=500&&transfer<1000) numer1+=3;
		     if (transfer>=1000) numer1+=4;
		     if (lata==1) numer1+=2;
		     if (lata==2) numer1++;
		     if (lata<1||lata>3) numer1=-3;
		     //cprintf("\n\rnumer1: %d",numer1);
		     sumaO=gracz.dane[3]+gracz.dane[4]+gracz.dane[5]+gracz.dane[6];
		     if (gracz.dane[2]==1&&sumaO>30) numer1-=2;
		     if (sumaO>=30) numer1--;
		     if (sumaO>=50) numer1-=2;
		     if (sumaO<20) numer1++;
		     if (sumaO<10) numer1+=2;
		     if (gracz.dane[17]==1) numer1++;
		     //numer1-=gracz.dane[7];
		     //cprintf("\n\rnumer1: %d",numer1);
		     if (numer1>=4)
		      {
		       textcolor(2);cprintf("\n\r%s: Zgadzam si© na proponowane warunki kontraktu.",gracz.nazwisko);
		       textcolor(7);cprintf("\n\r\n\rAkceptujesz (t/n): ");
		       tn=getche();
		       if (tn=='t')
		       {
			gracz.waga[1]=placa;
			if (gracz.dane[2]==3||gracz.dane[2]==4) gracz.waga[2]=premia;
			gracz.dane[18]=lata*365;
			if (gracz.dane[17]==1) gracz.dane[17]=0;
			if (tryb>=500&&tryb<1000)
			 {
			  gracz.dane[8]+=3;
			  if (gracz.dane[8]>=5) {gracz.dane[7]++; gracz.dane[8]=0;if (gracz.dane[7]>3) gracz.dane[7]=3;}
			 }
			if (tryb>=1000) {gracz.dane[7]++; gracz.dane[8]=0;if (gracz.dane[7]>3) gracz.dane[7]=3;}
		       }}
		     else
		      {
		       textcolor(4);cprintf("\n\r%s: Odrzucam proponowane warunki kontraktu.",gracz.nazwisko);
		       getch();
		      }
		     break;
		   }// dla switch
		  }//dla do
		  while(toupper(pp_usta2)!='Q');
		 }
		f2=fopen("wymiana.cfg","ab");
		fwrite(&gracz,sizeof(gracz),1,f2);
		fclose(f2);
	       }
	      fclose(f);
	      przepisz();
	      break;
	     case 'P':
	      if (sort==0) zamiana();
	      break;
	    }}
	   while(toupper(pp_usta)!='Q');
	   }
	   break;
	  case 'R'://trening
	   if (Klub.klub>0)
	   {
	   k=0;
	   do
	   {
	   info();textcolor(7);cputs("\n\r¬ƒƒƒƒƒƒƒƒƒƒ¬ƒƒƒƒƒƒƒƒƒƒ¬ƒƒƒƒƒƒƒƒƒƒ¬ƒ");
	   textcolor(15);cputs("TRENING");textcolor(7);cputs("ƒƒ¬ƒƒƒƒƒƒƒƒƒƒ¬ƒƒƒƒƒƒƒƒƒƒ¬ƒƒƒƒƒƒƒƒƒƒ¬\n\r");
	   textcolor(2);
	   cputs("≥ F1 - Pn  ≥ F2 - Wt  ≥ F3 - ór  ≥ F4 - Cz  ≥ F5 - Pt  ≥ F6 - So  ≥ F7 - N   ≥\n\r");
	   textcolor(7);cputs("≈ƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒ≈\n\r≥");
	   for(k=0;k<10;k++) ilex[k]=0;
	   k=0;i=0;
	   for(i=0;i<28;i++)
	    {
	     if (i==7||i==14||i==21||i==28) cputs("\n\r≥");
	     for(k=0;k<10;k++)
	      {
	       if (Klub.trening[i]==k+1) {ilex[k]++; cprintf("%-10s≥",tren[k]);}
	      }
	    }
	   cputs("\n\r¡ƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒ¡");
	   cprintf("\n\rIntensywnoòÜ:               Pkt:\n\r B, O, P, A: .........%3d%% %5d\n\r Kondycja: ...........%3d%% %5.1f\n\r Podania: ............%3d%% %5.1f\n\r Staàe fragmenty gry: %3d%% %5.1f\n\r Taktyka: ............%3d%% %5.1f\n\r Wolne: ..............%3d%%",ilex[4]*3*100/21,Klub.treBOPN,ilex[0]*100/21,Klub.umie[0],ilex[1]*100/21,Klub.umie[1],ilex[2]*100/21,Klub.umie[2],ilex[3]*100/21,Klub.umie[3],ilex[8]*3*100/21);
	   textcolor(2);
	   cputs("\n\r Zmiana planu: F1 - F7\n\r D Dob¢r zawodnik¢w do treningu indywidualnego");
	   textcolor(4);cputs("\n\r Q Powr¢t do MENU\n\r");
	   numer1=0;
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case F1:
	      morale="Poniedziaàku";numer1=1;
	     case F2:
	      if (numer1==0) {morale="Wtorku";numer1=2;}
	     case F3:
	      if (numer1==0) {morale="órody";numer1=3;}
	     case F4:
	      if (numer1==0) {morale="Czwartku";numer1=4;}
	     case F5:
	      if (numer1==0) {morale="Pi•tku";numer1=5;}
	     case F6:
	      if (numer1==0) {morale="Soboty";numer1=6;}
	     case F7:
	      if (numer1==0) {morale="Niedzieli";numer1=7;}
	      textcolor(7);k=0;
	      cprintf("Zmiana planu treningu dla %s\n\r1. Trening indywidualny (B,O,P,N)\n\r2. Trening druæynowy\n\r3. Dzie‰ wolny\n\r",morale);
	      tn=getch();
	      if (tn=='1')//B,O,P,N
	       {
		for(i=numer1-1;i<28;i+=7)
		 {
		  k++;
		  if (k==1) Klub.trening[i]=5;
		  if (k==2) Klub.trening[i]=6;
		  if (k==3) Klub.trening[i]=7;
		  if (k==4) Klub.trening[i]=8;
		 }f=fopen("Klub.cfg","wb");
		  fwrite(&Klub,sizeof(Klub),1,f);
		  fclose(f);}
	      else if (tn=='3')//wolne
	       {
		k=0;
		for(i=numer1-1;i<28;i+=7)
		 {
		  k++;
		  if (k==1) Klub.trening[i]=9;
		  if (k==2) Klub.trening[i]=10;
		  if (k==3) Klub.trening[i]=10;
		  if (k==4) Klub.trening[i]=10;
		 }f=fopen("Klub.cfg","wb");
		  fwrite(&Klub,sizeof(Klub),1,f);
		  fclose(f);}
	      else if (tn=='2')//druæynowy
	       {
		ilex[0]=0;ilex[1]=0;ilex[2]=0;ilex[3]=0;ilex[4]=0;
		//clrscr();
		k=0;
		while(k!=4)
		{
		k++;
		if (k<4)
		 {
		  clrscr();textcolor(15);cputs("\n\rTRENING DRUΩYNOWY");textcolor(7);
		  cprintf("\n\rMusisz obsadziÜ 3 rodzaje treningu.");textcolor(2);
		  for(i=0;i<4;i++)
		  {
		  if (ilex[i]==1) cputs("\n\rWybrano: 1. Kondycja");
		  if (ilex[i]==2) cputs("\n\rWybrano: 2. Podania");
		  if (ilex[i]==3) cputs("\n\rWybrano: 3. Staàe fragmenty gry");
		  if (ilex[i]==4) cputs("\n\rWybrano: 4. Taktyka");
		  }
		  textcolor(7);
		  cprintf("\n\rCo chcesz trenowaÜ (iloòÜ pozostaàych: %d):\n\r1. Kondycja\n\r2. Podania\n\r3. Staàe fragmenty gry\n\r4. Taktyka\n\r",4-k);
		  textcolor(2);scanf("%d",&ilex[k-1]);
		  if (ilex[k-1]<1||ilex[k-1]>4) k--;
		  //tn=getch();
		 }
		//else tn='x';
		if (ilex[k-1]==1)
		 {
		  if (k==1) Klub.trening[numer1-1]=1;
		  if (k==2) Klub.trening[numer1-1+7]=1;
		  if (k==3) Klub.trening[numer1-1+7+7]=1;
		 }
		else if (ilex[k-1]==2)
		 {
		  if (k==1) Klub.trening[numer1-1]=2;
		  if (k==2) Klub.trening[numer1-1+7]=2;
		  if (k==3) Klub.trening[numer1-1+7+7]=2;
		 }
		else if (ilex[k-1]==3)
		 {
		  if (k==1) Klub.trening[numer1-1]=3;
		  if (k==2) Klub.trening[numer1-1+7]=3;
		  if (k==3) Klub.trening[numer1-1+7+7]=3;
		 }
		else if (ilex[k-1]==4)
		 {
		  if (k==1) Klub.trening[numer1-1]=4;
		  if (k==2) Klub.trening[numer1-1+7]=4;
		  if (k==3) Klub.trening[numer1-1+7+7]=4;
		 }
		if (k==4) Klub.trening[numer1-1+7+7+7]=10;
		}//dla while
		f=fopen("Klub.cfg","wb");
		fwrite(&Klub,sizeof(Klub),1,f);
		fclose(f);
	       }
	      break;
	     case 'D'://kto ma trenowaÜ B,O,P,N
	      numer2=5;//jako bez filtra
	      do
	       {
	       info();textcolor(15);cputs("\n\rDOB‡R ZAWODNIK‡W DO TRENINGU INDYWIDUALNEGO");
	       if (dalej==0) tryb=20;
	       if (dalej==1) tryb=40;
	       sklad(Klub.usta,tryb,-1,0);
	       i=0;
	       f=fopen("Gra_mana.cfg","rb");
	       while(fread(&gracz,sizeof(gracz),1,f)==1) i++;
	       fclose(f);
	       textcolor(2);
	       if (i>20) cputs("\n\rD Dalej  ");
	       else cputs("\n\r");
	       //cputs("T Zmiana treningu  F Filtr");
	       cputs("T Zmiana treningu");
	       textcolor(4);cputs("  Q Powr¢t");textcolor(2);
	       pp_usta2=getch();
	       switch(toupper(pp_usta2))
		{
		 case 'D':
		  if (dalej==0&&i>20) dalej=1;
		  else dalej=0;
		  break;
		 case 'T':
		  textcolor(7);cputs(" Podaj nr. Lp.: ");scanf("%d",&numer1);
		  cputs(" Jaki trening (1-B, 2-O, 3-P, 4-N): ");scanf("%d",&lata);
		  if (lata<1||lata>4) break;
		  f=fopen("wymiana.cfg","wb");
		  fclose(f);//szyszczenie starej
		  f=fopen("Gra_mana.cfg","rb");
		  while(fread(&gracz,sizeof(gracz),1,f)==1)
		   {
		    if (gracz.dane[0]==numer1) gracz.dane[1]=lata;
		    if (gracz.dane[15]>0) gracz.dane[1]=0;
		    f2=fopen("wymiana.cfg","ab");
		    fwrite(&gracz,sizeof(gracz),1,f2);
		    fclose(f2);
		   }
		  fclose(f);
		  przepisz();
		  break;
		}}
	       while(toupper(pp_usta2)!='Q');
	       break; //dla case 'D':
	    }}
	   while(toupper(pp_usta)!='Q');
	   }
	   break;
	  case 'O'://ostatni mecz
	   clrscr();
	   sumaO=0;
	   textcolor(7);cprintf("RAPORT OSTATNIEGO MECZU: %s %d - %d %s\n\r",krajMS[Klub.klub-1],Klub.gol1,Klub.gol2,krajMS[Klub.lostmecz-1]);
	   textcolor(2);
	   sumaP=19;i=0;
	     f=fopen("Lostmecz.cfg","r");
	     while((tn=getc(f))!=EOF)
	      {
	       if (tn=='\n') i++;
	       printf("%c",tn);
	       if (sumaP==i)
		{
		 textcolor(2);cputs("\n\rDalej - dowolny klawisz");
		 textcolor(4);cputs("\n\rQ Wyjòcie ");textcolor(2);
		 tn=getch();
		 if (toupper(tn)=='Q') break;
		 //getch();
		 sumaP+=18;clrscr();
		 textcolor(7);cprintf("RAPORT OSTATNIEGO MECZU: %s %d - %d %s\n\r\n\r",krajMS[Klub.klub-1],Klub.gol1,Klub.gol2,krajMS[Klub.lostmecz-1]);//cputs("RAPORT OSTATNIEGO MECZU\n\r\n\r");
		 textcolor(2);
		}
	      }
	     fclose(f);
	   if (toupper(tn)!='Q')
	    {textcolor(7);cputs("\n\rKoniec meczu");getch();}
	   break;
	  case 'P'://najbliæszy przeciwnik
	  if (Klub.klub>0)
	   {
	   do
	   {
	   clrscr();textcolor(15);cputs("NAJBLIΩSZY PRZECIWNIK - ");
	   f=fopen("Kolejka2.cfg","rb");
	   while(fread(&kolej,sizeof(kolej),1,f)==1) //cprintf("%s %s",kolej.kol1[Klub.rywal[3]],kolej.kol2[Klub.rywal[3]]);
	    if (Klub.kolejka==kolej.numer) cprintf("%s",kolej.kol1[Klub.rywal[3]]);
	   fclose(f);
	   kk=0;
	   f2=fopen("Tabela.cfg","rb");
	   while(fread(&tabela,sizeof(tabela),1,f2)==1)
	    {
	     if (Klub.rywal[0]==tabela.num) kk=tabela.dane[8];
	    }
	   fclose(f2);
	   cprintf(" %d miejsce",kk);
	   if (Klub.rywal[1]==0) morale="(Dom)";
	   else morale="(Wyjazd)";
	   cprintf(" %s",morale);
	   //textcolor(2);cprintf(" Ustawienie: %s",taktyka[Klub.rywal[2]-1]);
	   takty(Klub.rywal[2],Klub.rywal[0]);//,Klub.rywal[0]);
	   wykres(Klub.rywal[2],Klub.rywal[0]);
	   wykres(Klub.usta,0);
	   //tryb=11*Klub.rywal[0];
	   sklad(Klub.rywal[2],11,Klub.rywal[0],0);
	   //textcolor(15);cputs("\n\rInstrukcje dla druæyny");
	   //instrukcje(Klub.rinst[0],Klub.rinst[1],Klub.rinst[2],Klub.rinst[3],Klub.rinst[4],Klub.rinst[5]);
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case 'R':
	      clrscr();textcolor(15);cputs("\n\rREZERWOWI");
	      //tryb=16;
	      sklad(Klub.rywal[2],16,Klub.rywal[0],0);
	      textcolor(7);cputs("\n\r\n\rNaciònij dowolny klawisz...");
	      getch();
	      break;
	     case 'I':
	      clrscr();textcolor(15);
	      cputs("\n\rInstrukcje dla druæyny:");
	      instrukcje(Klub.rinst[0],Klub.rinst[1],Klub.rinst[2],Klub.rinst[3],Klub.rinst[4],Klub.rinst[5],0);
	      textcolor(7);cputs("\n\r\n\rNaciònij dowolny klawisz...");
	      getch();
	      break;
	   }}
	   while(toupper(pp_usta)!='Q');
	   }
	   break;
	  case 'K'://kalendarz
	  if (Klub.klub>0)
	   {
	   if (Klub.m<7) {k=15;tryb=30;dalej=1;}//runda wosenna
	   else {k=0;tryb=15;dalej=0;}//runda jesienna
	   do
	   {
	   if (dalej==0) {k=0;tryb=15;}
	   else {k=15;tryb=30;}
	   info();textcolor(15);cputs("\n\rKALENDARZ - ");
	   if (dalej==0) cputs("runda jesienna");
	   else cputs("runda wosenna");
	   textcolor(7);
	   if (Klub.ilekontrol<=0&&dalej==0) k=Klub.ilekontrol-1;
	   while(k!=tryb)
	    {
	     k++;
	     sumaO=0;sumaN=0;sumaP=0;
	     f=fopen("Kolejka2.cfg","rb");
	     while(fread(&kolej,sizeof(kolej),1,f)==1)
	      {
	       if (k==kolej.numer)
		{
		 for (i=0;i<16;i++)
		  if (Klub.klub==kolej.nr[i]) sumaO=i;
		}}
	     fclose(f);
	     if (k<=0) textcolor(5);
	     else textcolor(7);
	     if (sumaO%2==0) {sumaN=sumaO+1;sumaP=0;}//u siebie
	     else {sumaN=sumaO-1;sumaP=1;}//wyjazd
	     f=fopen("Kolejka2.cfg","rb");
	     while(fread(&kolej,sizeof(kolej),1,f)==1)
	      {
	       if (kolej.numer==k) cprintf("\n\r%02d.%02d.%dr. %-19s",kolej.d,kolej.m,kolej.r+Klub.sezon,kolej.kol1[sumaN]);
		 //cprintf("\n\r%02d.%02d.%dr. %-19s",kolej.d,kolej.m,kolej.r,kolej.kol1[sumaN]);
	      }
	     fclose(f); // cprintf("\n\r%s",kolej.kol1[sumaN]);
	     if (sumaP==0) morale="Dom";
	     else morale="Wyjazd";
	     cprintf(" %-9s",morale);
	     if (k<=0) morale="Kontrolny";
	     else morale="I liga";
	     cprintf(" %-11s",morale);
	     if (Klub.jestM==0) kk=0;
	     if (Klub.jestM==1) kk=1;
	     f=fopen("Kolejka2.cfg","rb");
	     while(fread(&kolej,sizeof(kolej),1,f)==1)
	      {
	      if (Klub.kolejka==kolej.numer&&Klub.jestM==0&&Klub.d!=kolej.d)
	       kk++;
	      }
	     fclose(f);
	     //cprintf("jestM: %d,kk %d",Klub.jestM,kk);
	     f=fopen("Kolejka2.cfg","rb");
	     while(fread(&kolej,sizeof(kolej),1,f)==1)
	      {
	       if (k<=0&&k==kolej.numer&&k<=Klub.kolejka-kk)
		cprintf("%5d - %d",Klub.golK[(k+4)*2],Klub.golK[(k+4)*2+1]);
	       else if (k<=0&&k==kolej.numer&&k>=Klub.kolejka-kk)
		cputs("      - ");
	       else if (k>0&&k==kolej.numer&&k<=Klub.kolejka-kk)
		cprintf("%5d - %d",Klub.gol[(k*16)-16+sumaO],Klub.gol[(k*16)-16+sumaN]);
	       else if (k>0&&k==kolej.numer&&k>=Klub.kolejka-kk)
		cputs("      - ");
	       if (k==Klub.kolejka-kk+1&&k==kolej.numer) cputs(" ");
	      }
	     fclose(f);
	    }
	   textcolor(2);
	   if (dalej==0) cputs("\n\r\n\rD Pokaæ rund© wiosenn•");
	   else cputs("\n\r\n\rD Pokaæ rund© jesienn•");
	   cputs("   M Mecze kontrolne");
	   textcolor(4);cputs("   Q Powr¢t do MENU");textcolor(2);
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case 'D':
	      if (dalej==0) dalej=1;
	      else dalej=0;
	      break;
	     case 'M':
	      if (Klub.m==7)
	      {
	      textcolor(7);cputs("\n\rPodaj dzie‰, spacja, miesi•c - meczu kontrolnego: ");
	      scanf("%d %d",&sumaN,&sumaO);k=0;
	      f=fopen("Kolejka2.cfg","rb");
	      while(fread(&kolej,sizeof(kolej),1,f)==1)
	       if (kolej.numer==Klub.ilekontrol&&kolej.d<=sumaN&&kolej.m<=sumaO) k=1;
	      fclose(f);
	      if (sumaO<Klub.m||(sumaN==Klub.d&&sumaO==Klub.m)||sumaN>31) k=1;
	      info();textcolor(7);cputs("\n\r\n\rWybierz sparringpartnera:");
	      for (i=0;i<16;i++)
	       if (i+1!=Klub.klub) cprintf("\n\r%2d. %s",i+1,krajMS[i]);
	      cputs("\n\r\n\rWpisz odpowiedni numer: ");scanf("%d",&numer1);
	      if (numer1<0||numer1>16||numer1==Klub.klub||Klub.ilekontrol<-3) k=1;
	      if (k==1)
	       {textcolor(4);cputs("\n\rOdrzucono propozycj©.");getch();}
	      if (k==0)
	       {
		  Klub.ilekontrol--;
		  sumaP=Klub.ilekontrol;//numer kolejki
		  //kk=numer1+16;
		  Klub.kolejka--;
		  Klub.rywalTN=0;
		  kolej.d=sumaN; kolej.m=sumaO; kolej.r=2002;
		  kolej.numer=sumaP;
		  kolej.nr[0]=numer1; kolej.nr[1]=Klub.klub;
		  for (i=2;i<16;i++)
		   kolej.nr[i]=0;
		  for (i=0;i<16;i++)
		    for (k=0;k<19;k++)
		     kolej.kol1[i][k]=krajMS[kolej.nr[i]-1][k];
		  f=fopen("Kolejka3.cfg","wb");
		  fwrite(&kolej,sizeof(kolej),1,f);
		  fclose(f);
		  //przepisz
		  f=fopen("Kolejka2.cfg","rb");
		  while(fread(&kolej,sizeof(kolej),1,f)==1)
		   {
		    f2=fopen("Kolejka3.cfg","ab");
		    fwrite(&kolej,sizeof(kolej),1,f2);
		    fclose(f2);
		   }
		  fclose(f);
		  //przepisz
		  f=fopen("Kolejka2.cfg","wb");
		  fclose(f);//szyszczenie starej
		  f=fopen("Kolejka3.cfg","rb");
		  while(fread(&kolej,sizeof(kolej),1,f)==1)
		   {
		    f2=fopen("Kolejka2.cfg","ab");
		    fwrite(&kolej,sizeof(kolej),1,f2);
		    fclose(f2);
		   }
		  fclose(f);
		  f=fopen("Klub.cfg","wb");
		  fwrite(&Klub,sizeof(Klub),1,f);
		  fclose(f);
	       }
	      }//dla mies•ca 7
	      else
	       {
		textcolor(7);cputs("\n\rNie moæna rozegraÜ meczu kontrolnego.");
		gettime(&t);
		k=t.ti_sec;
		if (k==58) k=0;
		else if (k==59) k=1;
		else k+=2;
		while(t.ti_sec!=k) gettime(&t);
	       }
	      break;
	    }}//switch i do
	   while(toupper(pp_usta)!='Q');
	   }
	   break;
	  case 'T'://tabela
	   do
	   {
	   clrscr();//textcolor(15);cputs("TABELA - I LIGA");
	   //cputs("Linie:  _ ƒ  ø ⁄ ¿  Ÿ ≈  ¬ ¡ ≥ ¥√ ");
	   textcolor(7);
	   cputs("⁄ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ ");
	   textcolor(15); cputs("TABELA - I LIGA");
	   textcolor(7); cputs(" ƒƒƒƒƒƒ¬ƒƒƒƒƒƒƒƒ¬ƒƒƒƒø\n\r≥");
	   textcolor(2);
	   cputs("Lp. Klub                  M ≥  W  R  P ≥ Gz  Gs ≥ Pkt");textcolor(7);
	   cputs("≥\n\r√ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒ≈ƒƒƒƒ¥");
	   k=0;
	   f=fopen("Tabela.cfg","rb");
	   while(fread(&tabela,sizeof(tabela),1,f)==1)
	    {
	     k++;
	     cputs("\n\r≥");
	     if (tabela.num==Klub.klub) textcolor(14);
	     //if (tabela.dane[8]==k)
	     cprintf("%2d. %-20s %2d ≥ %2d %2d %2d ≥ %2d  %2d ≥ %2d",tabela.dane[8],krajMS[tabela.num-1],tabela.dane[0],tabela.dane[1],tabela.dane[2],tabela.dane[3],tabela.dane[4],tabela.dane[5],tabela.dane[7]);
	     textcolor(7);
	     cputs(" ≥");
	     if (k==1) cputs("\n\r√ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒ≈ƒƒƒƒ¥");
	     if (k==13) cputs("\n\r√ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒ≈ƒƒƒƒ¥");
	    }
	   fclose(f);
	   cputs("\n\r¿ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒƒƒ¡ƒƒƒƒƒƒƒƒ¡ƒƒƒƒŸ");
	   textcolor(2);cputs("\n\r W Wyniki    P Pozostaàe mecze");
	   textcolor(4);cputs("    Q Powr¢t do MENU");sumaP=0;sumaN=0;
	   if (Klub.jestM==0) kk=0;
	   if (Klub.jestM==1) kk=1;
	   f=fopen("Kolejka2.cfg","rb");
	   while(fread(&kolej,sizeof(kolej),1,f)==1)
	    {
	     if (Klub.kolejka==kolej.numer&&Klub.jestM==0&&Klub.d!=kolej.d)
	      kk++;
	    }
	   fclose(f);
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case 'W':
	      sumaO=1;sumaP=1;k=Klub.kolejka-kk;sumaN=1;k++;
	      if (Klub.kolejka<=0) break;
	     case 'P':
	      if (sumaP==0) {k=Klub.kolejka-kk;sumaO=30;}
	      while(k!=sumaO)
	       {
		if (sumaN==1) k--;
		else k++;
		info();
		if (sumaN==1) cprintf("\n\r\n\r  Wyniki mecz¢w I ligi - %d kolejka",k);
		else cprintf("\n\r\n\r  Pozostaàe mecze I ligi - %d kolejka",k);
		textcolor(7);
		for(i=0;i<16;i++)
		{
		f=fopen("Kolejka2.cfg","rb");
		while(fread(&kolej,sizeof(kolej),1,f)==1)
		 {
		  if (k==kolej.numer)
		   {
		    textcolor(7);
		    if (kolej.nr[i]==Klub.klub) textcolor(2);
		    if (sumaN==1)
		     {
		      if (i%2==0) cprintf("\n\r%19s %d -",krajMS[kolej.nr[i]-1],Klub.gol[k*16-16+i]);
		      else cprintf(" %d %s",Klub.gol[k*16-16+i],krajMS[kolej.nr[i]-1]);
		     }
		    else
		     {
		      if (i%2==0) cprintf("\n\r%19s -",krajMS[kolej.nr[i]-1]);
		      else cprintf(" %s",krajMS[kolej.nr[i]-1]);
		 }}}
		fclose(f);
		}
		textcolor(2);cputs("\n\r\n\r Dalej - dowolny klawisz");
		textcolor(4);cputs("\n\r Q Wyjòcie ");
		tn=getch();
		if (toupper(tn)=='Q') break;
	       }
	      break;
	    }//dla switch
	   }
	   while(toupper(pp_usta)!='Q');
	   break;
	  case 'L'://lista transferowa
	  if (Klub.klub>0)
	  {
	  f_ce2=Klub.finanse[13];
	  i=0;tryb=20;//k=dalej;
	  do
	   {
	   dalej=0;
	   f=fopen("Trans.cfg","rb");
	   while(fread(&gracz,sizeof(gracz),1,f)==1) dalej++;
	   fclose(f);
	   info();cprintf("\n\rLISTA TRANSFEROWA");
	   if (Klub.finanse[13]>0) textcolor(10);
	   else textcolor(12);
	   cprintf("   Fundusze na transfery: %.2f zà.",Klub.finanse[13]);
	   textcolor(2);
	   cputs("\n\r Lp.   Zawodnik         Po.  ");
	   textcolor(9);
	   cputs("B  ");textcolor(5);
	   cputs("O  ");textcolor(11);
	   cputs("P  ");textcolor(10);
	   cputs("N  ");textcolor(2);
	   cputs("Morale  For. Kon.    Cena");
	   if (i>=dalej) {i=0;tryb=20;}
	   while(i!=tryb)
	    {
	     i++;//k=-1;
	     textcolor(7);
	     f=fopen("Trans.cfg","rb");
	     while(fread(&gracz,sizeof(gracz),1,f)==1)
	      {
	       if (gracz.dane[0]==i&&gracz.dane[22]!=Klub.klub)//((gracz.dane[0]==i)&&(gracz.dane[3]>=f_b&&gracz.dane[4]>=f_o&&gracz.dane[5]>=f_p&&gracz.dane[6]>=f_n))
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
	       else cputs("  ");
	       if ((f_po==gracz.dane[2]||f_po==0)&&(gracz.dane[3]>=f_b&&gracz.dane[4]>=f_o&&gracz.dane[5]>=f_p&&gracz.dane[6]>=f_n&&gracz.dane[9]>=f_fo&&gracz.waga[0]>=f_ce1&&gracz.waga[0]<=f_ce2))
		textcolor(7);
	       else textcolor(8);
	       cprintf("\n\r%3d.  ",gracz.dane[0]);
	       cprintf("%3s%-15s %c  %2d %2d %2d %2d  %-7s  %2d  %3d%% %8.0f zà.",gracz.imie,gracz.nazwisko,tn,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],morale,gracz.dane[9],gracz.dane[11],gracz.waga[0]);
	      }}
	     fclose(f);
	     //if (k==20) getch();
	    }
	   textcolor(2);cputs("\n\rD Dalej   F Filrty   K Kupno zawodnika   ");
	   textcolor(4);cputs("Q Powr¢t do MENU");textcolor(2);
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case 'K':
	      if (Klub.finanse[13]>0)
	      {
	      sumaN=0;
	      f2=fopen("Gra_mana.cfg","rb");
	      while(fread(&gracz,sizeof(gracz),1,f2)==1) sumaN++;
	      fclose(f2);
	      textcolor(7);cputs("\n\rPodaj numer Lp.: ");scanf("%d",&sumaP);
	      f=fopen("wymiana.cfg","wb");
	      fclose(f);
	      f=fopen("Trans.cfg","rb");
	      while(fread(&gracz,sizeof(gracz),1,f)==1)
	       {
		if (gracz.dane[0]==sumaP)
		 {
		  clrscr();
		  numer1=0;
		  cprintf("\n\rFUNDUSZE NA TRANSFERY: %.2f zà.\n\r\n\rKupno zawodnika: %s%s",Klub.finanse[13],gracz.imie,gracz.nazwisko);
		  if (gracz.dane[22]>0) cprintf("\n\rKlub: %s",krajMS[gracz.dane[22]-1]);
		  else cputs("\n\rKlub: æaden");
		  if (gracz.dane[22]>0)
		   {
		    cprintf("\n\r\n\rPodaj cen© (%.2f): ",gracz.waga[0]);scanf("%f",&cena);
		    if (cena>Klub.finanse[13]||cena<0) numer1-=10;
		   }
		  else cputs("\n\r");
		  cprintf("\n\rPodaj pàac© miesi©czn• (%.2f): ",gracz.waga[1]);scanf("%f",&placa);
		  if (gracz.dane[2]==3||gracz.dane[2]==4)
		   {cprintf("\n\rPodaj premi© za gola (%.2f): ",gracz.waga[2]);scanf("%f",&premia);}
		  cprintf("\n\rPodaj przez ile lat ma obowi•zywaÜ kontrakt (1,2 lub 3): ");scanf("%d",&lata);
		  if (gracz.dane[22]>0)
		   {
		    transfer=cena-gracz.waga[0];
		     if (transfer<0) numer1--;
		     if (transfer<-5000) numer1-=5;
		     if (transfer==0) numer1++;
		     if (transfer>=1000&&transfer<8000) numer1+=2;
		     if (transfer>=8000&&transfer<10000) numer1+=3;
		     if (transfer>=10000) numer1+=4;
		   }
		   if (gracz.dane[2]==3||gracz.dane[2]==4)
		    {
		     transfer=premia-gracz.waga[2];
		     if (transfer<0) numer1--;
		     if (transfer==0) numer1++;
		     if (transfer>=50&&transfer<100) numer1+=2;
		     if (transfer>=100) numer1+=3;
		    }
		   else numer1++;
		     transfer=placa-gracz.waga[1];
		     if (transfer<0) numer1--;
		     if (transfer<-1000) numer1-=5;
		     if (transfer==0) numer1++;
		     if (transfer>=100&&transfer<500) numer1+=2;
		     if (transfer>=500&&transfer<1000) numer1+=3;
		     if (transfer>=1000) numer1+=4;
		     if (lata==1) numer1+=2;
		     if (lata==2) numer1++;
		     if (lata<1||lata>3) numer1=-5;
		     //cprintf("\n\rnumer1: %d",numer1);
		     sumaO=gracz.dane[3]+gracz.dane[4]+gracz.dane[5]+gracz.dane[6];
		     if (gracz.dane[2]==1&&sumaO>30) numer1-=2;
		     if (sumaO>=30) numer1--;
		     if (sumaO>=50) numer1-=2;
		     if (sumaO<20) numer1++;
		     if (sumaO<10) numer1+=2;
		     if (gracz.dane[17]==1) numer1++;
		     //numer1+=gracz.dane[7];
		     //cprintf("\n\rnumer1: %d",numer1);
		     if (numer1>=6)
		      {
		       textcolor(2);cprintf("\n\r%s: Zgadzam si© na proponowane warunki kontraktu.",gracz.nazwisko);
		       textcolor(7);cputs("\n\r\n\rAkceptujesz (t/n): ");
		       tn=getche();
		       if (tn=='t')
		       {
			gracz.dane[22]=Klub.klub;
			gracz.waga[1]=placa;gracz.waga[0]=cena;
			if (gracz.dane[2]==3||gracz.dane[2]==4) gracz.waga[2]=premia;
			gracz.dane[18]=lata*365;
			if (gracz.dane[17]==1) gracz.dane[17]=0;
			if (tryb>=500&&tryb<1000)
			 {
			  gracz.dane[8]+=3;
			  if (gracz.dane[8]>=5) {gracz.dane[7]++; gracz.dane[8]=0;if (gracz.dane[7]>3) gracz.dane[7]=3;}
			 }
			if (tryb>=1000) {gracz.dane[7]++; gracz.dane[8]=0;if (gracz.dane[7]>3) gracz.dane[7]=3;}
			//zapisanie do druæyny
			gracz.dane[0]=sumaN+1;
			Klub.finanse[8]+=cena;
			Klub.finanse[12]-=cena;
			Klub.finanse[13]=Klub.finanse[12]/2;
			Klub.finanse[10]=Klub.finanse[6]+Klub.finanse[7]+Klub.finanse[8]+Klub.finanse[9];
			Klub.finanse[11]=Klub.finanse[5]-Klub.finanse[10];
			Klub.dane_m[10]++;Klub.dane2_m[0]+=cena;
			f2=fopen("Gra_mana.cfg","ab");
			fwrite(&gracz,sizeof(gracz),1,f2);
			fclose(f2);
			f2=fopen("Klub.cfg","wb");
			fwrite(&Klub,sizeof(Klub),1,f2);
			fclose(f2);
		       }}
		     else
		      {
		       textcolor(4);cprintf("\n\r%s: Odrzucam proponowane warunki kontraktu.",gracz.nazwisko);
		       getch();
		      }}
		f2=fopen("wymiana.cfg","ab");
		fwrite(&gracz,sizeof(gracz),1,f2);
		fclose(f2);
	       }
	      fclose(f);
	      f=fopen("Trans.cfg","wb");
	      fclose(f);
	      f=fopen("wymiana.cfg","rb");
	      while(fread(&gracz,sizeof(gracz),1,f)==1)
	       {
		f2=fopen("Trans.cfg","ab");
		fwrite(&gracz,sizeof(gracz),1,f2);
		fclose(f2);
	       }
	      fclose(f);
	      }//masz fundudze
	      else
	       {
		textcolor(7);cputs("\n\rBrak funduszy na transfery!");
		gettime(&t);
		k=t.ti_sec;
		if (k==58) k=0;
		else if (k==59) k=1;
		else k+=2;
		while(t.ti_sec!=k) gettime(&t);
		//getch();
	       }
	      i=0;tryb=20;
	      break;
	     case 'D':
	      tryb+=20;i=tryb-20;
	      //if (i>dalej) {i=0;tryb=20;}
	      break;
	     case 'F':
	      i=0;tryb=20;belka=1;
	      do
	      {
	      clrscr();
	      textcolor(15);cputs("\n\rFILTRY DLA LISTY TRANSFEROWEJ");
	      textcolor(7);
	      cprintf("\n\rFundusze na transfery: %.2f zà.",Klub.finanse[13]);
	      textcolor(2);
	      if (f_po==0) morale="Wszyscy";
	      if (f_po==1) morale="Bramkarze";
	      if (f_po==2) morale="Obro‰cy";
	      if (f_po==3) morale="Pomocnicy";
	      if (f_po==4) morale="Napastnicy";
	      //cprintf("\n\r\n\r P Wzgl©dem pozycji: %s\n\r U Wzgl©dem umiej©tnoòci: B: %d, O: %d, P: %d, N: %d\n\r F Wzgl©dem formy: %d\n\r C Wzgl©dem ceny minimalnej:  od %.2f zà.\n\r M Wzgl©dem ceny maksymalnej: do %.2f zà.",morale,f_b,f_o,f_p,f_n,f_fo,f_ce1,f_ce2);
	      if (belka==1) {textbackground(1);textcolor(7);}
	      else {textbackground(0);textcolor(2);}
	      cprintf("\n\r\n\r   Wzgl©dem pozycji: %s ",morale);
	      if (belka==2) {textbackground(1);textcolor(7);}
	      else {textbackground(0);textcolor(2);}
	      cprintf("\n\r   Wzgl©dem umiej©tnoòci: B: %d ",f_b);
	      if (belka==3) {textbackground(1);textcolor(7);}
	      else {textbackground(0);textcolor(2);}
	      cprintf("\n\r   Wzgl©dem umiej©tnoòci: O: %d ",f_o);
	      if (belka==4) {textbackground(1);textcolor(7);}
	      else {textbackground(0);textcolor(2);}
	      cprintf("\n\r   Wzgl©dem umiej©tnoòci: P: %d ",f_p);
	      if (belka==5) {textbackground(1);textcolor(7);}
	      else {textbackground(0);textcolor(2);}
	      cprintf("\n\r   Wzgl©dem umiej©tnoòci: N: %d ",f_n);
	      if (belka==6) {textbackground(1);textcolor(7);}
	      else {textbackground(0);textcolor(2);}
	      cprintf("\n\r   Wzgl©dem formy: %d ",f_fo);
	      if (belka==7) {textbackground(1);textcolor(7);}
	      else {textbackground(0);textcolor(2);}
	      cprintf("\n\r C Wzgl©dem ceny minimalnej:  od %.2f zà. ",f_ce1);
	      if (belka==8) {textbackground(1);textcolor(7);}
	      else {textbackground(0);textcolor(2);}
	      cprintf("\n\r B Wzgl©dem ceny maksymalnej: do %.2f zà. ",f_ce2);
	      textcolor(4);textbackground(0);cputs("\n\r Q Powr¢t\n\r");
	      pp_usta2=getch();
	      switch(toupper(pp_usta2))
	       {
		case UP:
		 belka--;
		 if (belka==0) belka=8;
		 break;
		case DOWN:
		 belka++;
		 if (belka==9) belka=1;
		 break;
		case RIGHT:
		 if (belka==1)
		  {
		   f_po++;
		   if (f_po==5) f_po=0;
		  }
		 else if (belka==2)
		  {
		   f_b++;
		   if (f_b==21) f_b=1;
		  }
		 else if (belka==3)
		  {
		   f_o++;
		   if (f_o==21) f_o=1;
		  }
		 else if (belka==4)
		  {
		   f_p++;
		   if (f_p==21) f_p=1;
		  }
		 else if (belka==5)
		  {
		   f_n++;
		   if (f_n==21) f_n=1;
		  }
		 else if (belka==6)
		  {
		   f_fo++;
		   if (f_fo==11) f_fo=1;
		  }
		 else if (belka==7)
		  {
		   f_ce1+=100000.0;
		   if (f_ce1>Klub.finanse[13]) f_ce1=Klub.finanse[13];
		  }
		 else if (belka==8)
		  {
		   f_ce2+=100000.0;
		   if (f_ce2>Klub.finanse[13]) f_ce2=Klub.finanse[13];
		  }
		 break;
		case LEFT:
		 if (belka==1)
		  {
		   f_po--;
		   if (f_po==-1) f_po=4;
		  }
		 else if (belka==2)
		  {
		   f_b--;
		   if (f_b==0) f_b=20;
		  }
		 else if (belka==3)
		  {
		   f_o--;
		   if (f_o==0) f_o=20;
		  }
		 else if (belka==4)
		  {
		   f_p--;
		   if (f_p==0) f_p=20;
		  }
		 else if (belka==5)
		  {
		   f_n--;
		   if (f_n==0) f_n=20;
		  }
		 else if (belka==6)
		  {
		   f_fo--;
		   if (f_fo==0) f_fo=10;
		  }
		 else if (belka==7)
		  {
		   f_ce1-=100000.0;
		   if (f_ce1<0) f_ce1=0;
		  }
		 else if (belka==8)
		  {
		   f_ce2-=100000.0;
		   if (f_ce2<0) f_ce2=0;
		  }
		 break;
		case 'C':
		 textcolor(7);
		 cputs("\n\rPodaj minimaln• cen© zawodnika: ");scanf("%f",&f_ce1);
		 //cputs("\n\rPodaj maksymaln• cen© zawodnika: ");scanf("%f",&f_ce2);
		 break;
		case 'B':
		 textcolor(7);
		 cputs("\n\rPodaj maksymaln• cen© zawodnika: ");scanf("%f",&f_ce2);
		 break;
	       }
	       }//dla do
	      while(toupper(pp_usta2)!='Q');
	      if (f_ce1>f_ce2) f_ce2=Klub.finanse[13];
	      break;
	     default:
	      tryb=20;i=0;
	      break;
	    }
	   }//dla do
	   while(toupper(pp_usta)!='Q');
	   }//dla Klub.klub>0
	   break;
	  case 'F'://finanse
	  if (Klub.klub>0)
	   {
	   info();textcolor(14);cputs("\n\rFINANSE");
	   textcolor(10);
	   cprintf("\n\rPrzychody w tym miesi•cu:                  Przychody w zeszàym miesi•cu:\n\r%12.2f zà. - Bilety               %12.2f zà. - Bilety\n\r%12.2f zà. - Doch¢d z TV          %12.2f zà. - Doch¢d z TV\n\r%12.2f zà. - Handel               %12.2f zà. - Handel\n\r%12.2f zà. - Sprzedaæ zawodnik¢w  %12.2f zà. - Sprzedaæ zawodnik¢w\n\r%12.2f zà. - Reklamy              %12.2f zà. - Reklamy\n\r%12.2f zà. - RAZEM                %12.2f zà. - RAZEM",Klub.finanse[0],Klub.finanse2[0],Klub.finanse[1],Klub.finanse2[1],Klub.finanse[2],Klub.finanse2[2],Klub.finanse[3],Klub.finanse2[3],Klub.finanse[4],Klub.finanse2[4],Klub.finanse[5],Klub.finanse2[5]);
	   textcolor(12);
	   cprintf("\n\rStraty w tym miesi•cu:                     Straty w zeszàym miesi•cu:\n\r%12.2f zà. - Pàace                %12.2f zà. - Pàace\n\r%12.2f zà. - Premie               %12.2f zà. - Premie\n\r%12.2f zà. - Kupno zawodnik¢w     %12.2f zà. - Kupno zawodnik¢w\n\r%12.2f zà. - Kary ligowe          %12.2f zà. - Kary ligowe\n\r%12.2f zà. - RAZEM                %12.2f zà. - RAZEM",Klub.finanse[6],Klub.finanse2[6],Klub.finanse[7],Klub.finanse2[7],Klub.finanse[8],Klub.finanse2[8],Klub.finanse[9],Klub.finanse2[9],Klub.finanse[10],Klub.finanse2[10]);
	   if (Klub.finanse[11]>0) textcolor(2);
	   else textcolor(4);
	   cprintf("\n\r\n\rZysk w tym miesi•cu: %.2f zà.",Klub.finanse[11]);
	   textcolor(7);
	   cprintf("   Zysk w zeszàym miesi•cu: %.2f zà.",Klub.finanse2[11]);
	   cprintf("\n\r\n\rKasa klubu: %.2f zà.\n\rFundusze na transfery: %.2f zà.",Klub.finanse[12],Klub.finanse[13]);
	   cputs("\n\r\n\rPowr¢t do MENU - dowolny klawisz...");
	   getch();
	   }
	   break;
	  case 'D':
	  if (Klub.klub>0)
	   {
	  //sumaO=0;
	   do
	    {
	   info();cputs("\n\r\n\rZARZ§D KLUBU");
	   textcolor(7);
	   f=fopen("Tabela.cfg","rb");
	   while(fread(&tabela,sizeof(tabela),1,f)==1)
	    {
	     if (tabela.num==Klub.klub) k=tabela.dane[8];
	    }
	   fclose(f);
	   if (k==0) wiadomosc[8]=43;
	   else if (k<4) wiadomosc[8]=31;
	   else if (k>3&&k<10) wiadomosc[8]=32;
	   else if (k>9&&k<14) wiadomosc[8]=33;
	   else if (k>13) wiadomosc[8]=34;
	   if (Klub.finanse[12]<=500000&&Klub.finanse[11]<=0) wiadomosc[9]=35;
	   else if (Klub.finanse[12]<=500000&&Klub.finanse[11]>0) wiadomosc[9]=36;
	   else if (Klub.finanse[12]<=1000000&&Klub.finanse[11]<=0) wiadomosc[9]=37;
	   else if (Klub.finanse[12]<=1000000&&Klub.finanse[11]>0) wiadomosc[9]=38;
	   else if (Klub.finanse[12]<=2000000&&Klub.finanse[11]<=0) wiadomosc[9]=39;
	   else if (Klub.finanse[12]<=2000000&&Klub.finanse[11]>0) wiadomosc[9]=40;
	   else if (Klub.finanse[12]>2000000&&Klub.finanse[11]<=0) wiadomosc[9]=41;
	   else if (Klub.finanse[12]>2000000&&Klub.finanse[11]>0) wiadomosc[9]=42;
	     for (i=8;i<10;i++)
	      {
	       cputs("\n\r\n\r");
	       f=fopen("News.cfg","rb");
	       while(fread(&news,sizeof(news),1,f)==1)
		{
		 if (wiadomosc[i]==news.num)
		   cprintf(news.newss,dlawiadomosc[i-1],numerW[i-1]);
		 }
	       fclose(f);
	    }//getch();
	   wiadomosc[8]=0;
	   wiadomosc[9]=0;
	   textcolor(2);
	   cputs("\n\r\n\r T Proòba o dodatkowe fundusze na transfery");
	   textcolor(4);cputs("\n\r Q Powr¢t do MENU\n\r");
	   pp_usta=getch();
	   switch(toupper(pp_usta))
	    {
	     case 'T':
	      if (Klub.finanse[12]>0&&Klub.finanse[12]<=500000) kk=random(6);
	      else if (Klub.finanse[12]>500000&&Klub.finanse[12]<=1000000) kk=random(5);
	      else if (Klub.finanse[12]>1000000&&Klub.finanse[12]<=2000000) kk=random(4);
	      else if (Klub.finanse[12]>2000000) kk=random(3);
	      if (Klub.finanse[12]<=0) kk=1;
	      if (kk==0&&Klub.kasa==0)
	       {
		textcolor(2);
		cputs("\n\r\n\rZarz•d: Zgadzamy si© na pa‰sk• proòb©. Mamy nadziej©, iæ rozs•dnie wykorzysta pan fundusze i wzmocni zesp¢à.");
		Klub.finanse[13]=Klub.finanse[12];Klub.kasa=1;
		f=fopen("Klub.cfg","wb");
		fwrite(&Klub,sizeof(Klub),1,f);
		fclose(f);
	       }
	      else
	       {
		textcolor(4);Klub.kasa=1;
		cputs("\n\r\n\rZarz•d: Odrzucamy pa‰sk• proòb©.");
		f=fopen("Klub.cfg","wb");
		fwrite(&Klub,sizeof(Klub),1,f);
		fclose(f);
	       }
	      getch();
	      break;
	    }
	   }
	   while(toupper(pp_usta)!='Q');
	   }
	   break;
	  case 'M':
	   info();cputs("\n\r\n\rMANAGER - STAYSTKI");
	   textcolor(7);
	   cprintf("\n\r\n\rWygrane ligi:           %4d\n\rPrzyznane nagrody:      %4d\n\rPunkty managera:        %4d\n\rRozegrane mecze:        %4d\n\rMecze wygrane:          %4d\n\rMecze zremisowane:      %4d\n\rMecze przegrane:        %4d\n\rGole zdobyte:           %4d\n\rGole stracone:          %4d\n\rKupionych zawodnik¢w:   %4d - à•cznie %.2f zà.\n\rSprzedanych zawodnik¢w: %4d - à•cznie %.2f zà.",Klub.dane_m[0],Klub.dane_m[2],Klub.dane_m[3],Klub.dane_m[4],Klub.dane_m[5],Klub.dane_m[6],Klub.dane_m[7],Klub.dane_m[8],Klub.dane_m[9],Klub.dane_m[10],Klub.dane2_m[0],Klub.dane_m[11],Klub.dane2_m[1]);
	   cputs("\n\r\n\rPowr¢t do MENU - dowolny klawisz...");
	   getch();
	   break;
	  case 'E':
	   sumaO=0;
	   f=fopen("News_m.cfg","r");
	   while((tn=getc(f))!=EOF) if (tn=='\n') sumaO++;//34
	   fclose(f);
	   info();textcolor(2); cputs("\n\r\n\rSTARE WIADOMOóCI\n\r");
	   numer1=0;sumaP=0;sumaN=5;;
	   for (i=sumaO;i>0;i-=2)
	    {
	     sumaP=0;pp_usta='a';
	     f=fopen("News_m.cfg","r");
	     while((tn=getc(f))!=EOF)
	      {
	       if (tn=='\n') sumaP++;
	       textcolor(2);
	       if (i==sumaP) printf("%c",tn);
	      }
	     fclose(f);
	     numer1++;
	     if (numer1==sumaN)
	      {
	       textcolor(4);cputs("\n\r\n\rQ Przerwij wyòwietlanie");textcolor(7);cputs("\n\rDalej - Enter... ");textcolor(2);
	       pp_usta=getch();
	       info();textcolor(2);cprintf("\n\r\n\rSTARE WIADOMOóCI\n\r"); sumaN+=5;
	      }
	     if (toupper(pp_usta)=='Q') break;
	     cputs("\n\r");
	    }
	   textcolor(7);cputs("\n\r\n\rKoniec - naciònij dowolny klawisz...");getch();
	   break;
	  case 'W'://masz wiadomoòÜ
	   info();textcolor(2);cputs("\n\r\n\rWIADOMOóCI");textcolor(7);
	   if (wiadomosc[0]==1)//czy wog¢le jest sakaò wiadomoòÜ
	    {
	     for (i=1;i<10;i++)
	      {
	       cputs("\n\r\n\r");
	       f=fopen("News.cfg","rb");
	       while(fread(&news,sizeof(news),1,f)==1)
		{
		 if (wiadomosc[i]==news.num)//=1
		  {
		   cprintf(news.newss,dlawiadomosc[i-1],numerW[i-1]);
		   f2=fopen("News_m.cfg","aw");
		   fprintf(f2,"\n\n");
		   fprintf(f2,"%02d.%02d.%d - ",Klub.d,Klub.m,Klub.r);
		   fprintf(f2,news.newss,dlawiadomosc[i-1],numerW[i-1]);
		   fclose(f2);
		  }}
	       fclose(f);
	    }getch();}
	   for (i=0;i<10;i++)
	    wiadomosc[i]=0;
	   break;
	  case 'H':
	   clrscr();
	   textcolor(7);cputs("\n\rJeæeli czas wyòwietlania komunikat¢w w czasie meczu jest nieadekwatny do\n\rustawie‰ Szybkoòci zmie‰ poniæsz• opcj© na R©czne.");
	   cputs("\n\r\n\rWyòwietlanie kolejnych komunikat¢w w czasie meczu odbywaÜ ma si©:");
	   cputs("\n\r1. Automatycznie (zalecane)\n\r2. R©cznie (w razie problem¢w)\n\r\n\rWpisz odpowiedni• cyfr©: ");
	   scanf("%d",&Klub.mecz);
	   if (Klub.mecz<1||Klub.mecz>2) Klub.mecz=1;
	   f=fopen("Klub.cfg","wb");
	   fwrite(&Klub,sizeof(Klub),1,f);
	   fclose(f);
	   break;
	  case 'Q':
	   textcolor(12);cputs("\n\rCzy chcesz wyjòÜ z gry? (t/n): ");
	   tn=getch();
	   if (tn=='t') pp2='Q';
	   break;
	 }
       }//dla do pp2
      while(pp2!='Q');
      break;//dla pp1 W Wczytaj gr©
    }//dla switch pp1
  }//dla do
 while(toupper(pp1)!='Q');
 if (Klub.nie==0&&Klub.jestM==1)
  {
   clrscr();textcolor(10);
   cputs("\n\rAby rozegraÜ mecz uruchom plik Mecz.exe");
   if (PrzechodzeDoMeczu==1) execl("mecz.exe",NULL, NULL);
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
void info()
{
 FILE *f;
 int i;
 char *krajMS[16]={"Amica Wronki","Garbarnia Szczakowianka","GKS Katowice",
 "G¢rnik Zabrze","Groclin Dyskobolia","KSZO Ostrowiec","Lech Pozna‰",
 "Legia Warszawa","Odra Wodzisàaw","Polonia Warszawa","Pogo‰ Szczecin",
 "Ruch Chorz¢w","Widzew ù¢d´","Wisàa Krak¢w","Wisàa Pàock","Zagà©bie Lubin"};
 char *dniT[7]={"Pn","Wt","ór","Cz","Pt","So","N"};
 clrscr();
 if ((f=fopen("Klub.cfg","rb"))==NULL)
  {
   textcolor(4);
   cputs(" Bà•d! Nie moæna otworzyÜ pliku Klub.cfg!");
  }
 else
  {
   fread(&Klub,sizeof(Klub),1,f);
   textcolor(15);
   if (Klub.klub>0) cprintf(" %s",krajMS[Klub.klub-1]);
   else cputs(" Bezrobotny");
   textcolor(2);
   cprintf(" %s %s",Klub.imie_m,Klub.nazw_m);
   if (Klub.czy_nick==1) cprintf(" \"%s\"",Klub.nick);
   textcolor(15);
   cprintf(" Data: %s %02d.%02d.%d r.",dniT[Klub.tydzien-1],Klub.d,Klub.m,Klub.r);
   fclose(f);
  }}

char MenuGlowne(int wiadomosc[5])
{
 FILE *f;
 char *taktyka[14]={"4-4-2","4-4-2 Obrona","4-4-2 Atak","4-4-2 Diamond",
 "3-4-3","3-5-2","3-5-2 Obrona","3-5-2 Atak","4-2-4","4-3-3","4-5-1","5-3-2",
 "5-3-2 Obrona","5-3-2 Atak"};
 char *krajMS[16]={"Amica Wronki","Garbarnia Szczakowianka","GKS Katowice",
 "G¢rnik Zabrze","Groclin Dyskobolia","KSZO Ostrowiec","Lech Pozna‰",
 "Legia Warszawa","Odra Wodzisàaw","Polonia Warszawa","Pogo‰ Szczecin",
 "Ruch Chorz¢w","Widzew ù¢d´","Wisàa Krak¢w","Wisàa Pàock","Zagà©bie Lubin"};
 int k;
 f=fopen("Tabela.cfg","rb");
 while(fread(&tabela,sizeof(tabela),1,f)==1)
  if (Klub.klub==tabela.num) k=tabela.dane[8];
 fclose(f);
 textcolor(7);
 cputs("\n\r\n\r MENU\n\r");textcolor(10);
 if (Klub.jestM==0) cputs("\n\r C Kontynuuj");
 else cputs("\n\r C Mecz");
 textcolor(2);
 cprintf("\n\r A Taktyka: %s\n\r S Skàad\n\r R Trening",taktyka[Klub.usta-1]);
 textcolor(9);
 cprintf("\n\r O Ostatni mecz - raport\n\r P Przeciwnik: %s\n\r K Kalendarz\n\r T Tabela: %d miejsce\n\r L Lista transferowa",krajMS[Klub.rywal[0]-1],k);
 textcolor(14);cprintf("\n\r F Finanse: %.2f zà.\n\r D Zarz•d klubu\n\r M Manager",Klub.finanse[11]);
 if (wiadomosc[0]==1) {textcolor(12);cputs("\n\r W");textcolor(140);cputs(" Masz wiadomoòÜ");}
 else {textcolor(7);cputs("\n\r E Stare wiadomoòci");}//cputs("\n\r");
 //else {textcolor(8);cputs("\n\r   Ωadnych wiadomoòci");}
 textcolor(6);cputs("\n\r\n\r H Opcje dla meczu");
 textcolor(4);cputs("\n\r Q Wyjòcie\n\r");
 return getch();
}

void sklad(int usta,int tryb,int kto, int sort)
{
 FILE *f;
 int i=0,c=0,x,e=0;
 char tn,*morale;
 textcolor(2);
 cprintf("\n\rLp.   Zawodnik         Po.  ");
 textcolor(9);
 cputs("B  ");textcolor(5);
 cputs("O  ");textcolor(11);
 cputs("P  ");textcolor(10);
 cputs("N  ");textcolor(2);
 if (kto==-1) {cputs("Trening");textcolor(7);c=7;}//jeæeli e treningu
 else
  {cputs("Morale  For. Kon. Gole");
   if (sort==18) cputs("  Wygasa za:");
   textcolor(9);c=9;}//normalnie
 if (tryb==16) i=11;//dla pokazania rezerwowych w taktyce
 if (tryb==40) i=20;//dla dalej w Skàadzie
 if (sort==3||sort==4||sort==5||sort==6){i=21;tryb=1;}
 if (sort==7){i=4;tryb=-3;}
 if (sort==9){i=11;tryb=1;}
 if (sort==11){i=101;tryb=0;}
 if (sort==16){i=50;tryb=0;}
 if (sort==18){i=1;tryb=1095;}
 if (sort>0) {textcolor(7);c=7;}
 while(i!=tryb)
  {
   if (sort==0||sort==18) i++;
   else i--;
   if (tryb==16||i==12) {c=6;textcolor(6);}
   if (tryb==40) {c=7;textcolor(7);}
   if (sort>0) {textcolor(7);c=7;}
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
     if (kto==-1)//dla treningu
      {
       if (gracz.dane[1]==0) {morale="Nie trenuje";textcolor(7);c=7;}
       if (gracz.dane[1]==1) {morale="Bramkarze";textcolor(9);c=9;}
       if (gracz.dane[1]==2) {morale="Obrona";textcolor(5);c=5;}
       if (gracz.dane[1]==3) {morale="Pomoc";textcolor(11);c=11;}
       if (gracz.dane[1]==4) {morale="Atak";textcolor(10);c=10;}
      }
     if (i==gracz.dane[sort]&&x==gracz.dane[22])
      {
       e++;
       cprintf("\n\r%2d.",gracz.dane[0]);
	 if (gracz.dane[15]>0) {textbackground(4);textcolor(7);cputs("Ko");textbackground(0);/*cputs(" ");*/textcolor(c);}
	 else if (gracz.dane[14]==1) {textcolor(4);cputs("˛ ");textcolor(c);}
	 else if (gracz.dane[13]==1) {textcolor(14);cputs("˛ ");textcolor(c);}
	 else if (gracz.dane[13]==2) {textcolor(14);cputs("˛˛");textcolor(c);}
	 else if (gracz.dane[17]==1) {textcolor(13);cputs("T ");textcolor(c);}
	 else cputs("  ");
       //cprintf("%-15s %-10s %d %d %2d  %c  %2d %2d %2d %2d  %-7s %2d   %3d%%  %2d",gracz.nazwisko,gracz.imie,gracz.dane[13],gracz.dane[14],gracz.dane[15],tn,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],morale,gracz.dane[9],gracz.dane[11],gracz.dane[16]);
	 if (kto==-1) cprintf("%3s%-15s %c  %2d %2d %2d %2d  %-7s",gracz.imie,gracz.nazwisko,tn,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],morale);
	 else cprintf("%3s%-15s %c  %2d %2d %2d %2d  %-7s  %2d  %3d%%  %2d",gracz.imie,gracz.nazwisko,tn,gracz.dane[3],gracz.dane[4],gracz.dane[5],gracz.dane[6],morale,gracz.dane[9],gracz.dane[11],gracz.dane[16]);
	 if (sort==18) cprintf(" %6d dni",gracz.dane[18]);
	if (e>19) break;
      }}
   fclose(f);
   if (e>19) break;
   if (kto>-1&&sort==0)
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
   else {c=7;textcolor(7);}
   //if (tryb==11&&i==11) cputs("\n\r");
  }
}

void zamiana()
{
 FILE *f,*f2;
 int numer1,numer2,licz=0,kol[40],i=0,k=0;
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
}

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
void wykres(int usta,int kto)
{
 FILE *f;
 int sumaO=0,sumaP=0,sumaN=0,k=0,ile=0,x;
 char *mn=" - Rywal";
 if (kto>0) {f=fopen("Rywal.cfg","rb");x=kto;}
 else {f=fopen("Gra_mana.cfg","rb");x=Klub.klub;}
 while(fread(&gracz,sizeof(gracz),1,f)==1)
  {
   if (x==gracz.dane[22])
    {
     textcolor(9);
     if (gracz.dane[0]==1)
      {
       cputs("\n\rB-");ile=0;
       for(k=5;k<=gracz.dane[3];k+=5) {cputs("˛");ile++;}
       for(k=ile;k<4;k++) {textcolor(8);cputs("˛");}
      }
     if (gracz.dane[0]==2||gracz.dane[0]==3||gracz.dane[0]==4) sumaO+=gracz.dane[4];
     if ((gracz.dane[0]==5)&&(usta==1||usta==2||usta==3||usta==4||usta==9||usta==10||usta==11||usta==12||usta==13||usta==14)) sumaO+=gracz.dane[4];
     else if (gracz.dane[0]==5) sumaP+=gracz.dane[5];
     if ((gracz.dane[0]==6)&&(usta==12||usta==13||usta==14)) sumaO+=gracz.dane[4];
     else if (gracz.dane[0]==6) sumaP+=gracz.dane[5];
     if (gracz.dane[0]==7) sumaP+=gracz.dane[5];
     if ((gracz.dane[0]==8)&&(usta==9)) sumaN+=gracz.dane[6];
     else if (gracz.dane[0]==8) sumaP+=gracz.dane[5];
     if ((gracz.dane[0]==9)&&(usta==5||usta==10||usta==9)) sumaN+=gracz.dane[6];
     else if (gracz.dane[0]==9) sumaP+=gracz.dane[5];
     if ((gracz.dane[0]==10)&&(usta==11)) sumaP+=gracz.dane[5];
     else if (gracz.dane[0]==10) sumaN+=gracz.dane[6];
     if (gracz.dane[0]==11) sumaN+=gracz.dane[6];
    }}
 fclose(f);
 textcolor(5);ile=0;cputs(" O-");
 for(k=5;k<=sumaO;k+=5) {cputs("˛");ile++;}
 for(k=ile;k<20;k++) {textcolor(8);cputs("˛");}
 textcolor(11);ile=0;cputs(" P-");
 for(k=5;k<=sumaP;k+=5) {cputs("˛");ile++;}
 for(k=ile;k<20;k++) {textcolor(8);cputs("˛");}
 textcolor(10);ile=0;cputs(" N-");
 for(k=5;k<=sumaN;k+=5) {cputs("˛");ile++;}
 for(k=ile;k<16;k++) {textcolor(8);cputs("˛");}
 textcolor(7);
 if (kto>0) cprintf("%s",mn);
}