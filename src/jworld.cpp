//#include <limits.h>
//#include <assert.h>
//#include <string.h>
//#include <math.h>
#include <string.h>  
#include <math.h>
#include "INCLUDE/platform.hpp"

#ifdef NEW_FASHION_CPP
#include <strstream>
#elif defined(unix)
#include <strstream.h>
#	else
#include <strstrea.h>
#endif

#include "INCLUDE/wb_ptrio.h"

#include "jrand.h"
#include "jworld.h"
//#include "SYMSHELL/ohistosou.hpp" //Stary histogram z ruchoma liczba klas
#include "SYMSHELL/dhistosou.hpp" //Dyskretny histogram ze stala liczba klas
#include "SYMSHELL/fhistosou.hpp" //Histogram z ustalona arbitralnie liczba klas
#include "SYMSHELL/clstsour.hpp" //Jest tez statsour
#include "SYMSHELL/spatcors.hpp"
#include "SYMSHELL/coincsou.hpp"
#include "SYMSHELL/gadgets.hpp" 


//Konstrukcja agentow
///////////////////////////////////
jagent::jagent(const jagent& ini)
	{
		if(&ini!=NULL)
		{
			First=ini.First;
			Second=ini.Second;
			Third=ini.Third;
			Power=RANDOM(max_sila+1);
            Age=0;
		}
		else
			_clean();
	}

jagent::jagent()
	{
		_clean();
		First=RANDOM(ile_kate);
		Second=RANDOM(ile_kate);
		Third=RANDOM(ile_kate);	
        if(Distribution>=0)
        {
            double Power=1;
            for(int i=0;i<Distribution;i++)
                Power*=DRAND();
            Power*=max_sila+1;
            this->Power=Power;
        }
        else
        {
            double Power=0;
            for(int i=0;i<(-Distribution);i++)
		        Power+=DRAND();
            Power/=-Distribution;
            this->Power=Power*(max_sila+1);
        }
	}

//Statyczne pola jagentow dla inicjalizacji
////////////////////////////////////////////////////////////////
short jagent::ruchsily=1;  //Maksymalny skok sily
short jagent::max_sila=256;//Maksymalna sila agenta
short jagent::min_sila=1;  //i minimalna 
short jagent::ile_kate=256;//Ilosc kategori w mapach	
short jagent::kate_shift=0;//Przesuniecie dla wczytywania gifa
double	jagent::MutationLevel=0;//Prawd. spontanicznej zmiany memu (0..1)
short jagent::Distribution=1;//Stopien rozkladu. 0->n rozklady z *, -1->-n rozklady z +

//KONSTRUKCJA	SWIATA
////////////////////////////////////
extern unsigned internal_log;
extern unsigned spatial_correlation_mode;
extern bool UseSpatialCorr;

jworld::jworld(size_t Width,		//Szerokosc torusa macierzy agentow
	  char* log_name,	//Nazwa pliku do zapisywania histori
	  char* mapl_name,	//Nazwa (bit)mapy inicjujacej "skladowe"
	  char* mapp_name,	//Nazwa (bit)mapy inicjujacej "sily"
	  char* live_mask,	//Czarne w tej mapie sa kasowane
      short Distribution,//Rodzaj i stopien rozkladu sil
	  double noise,		//Szum informacyjny przy decyzji
	  short	max_sila,	//Maksymalna sila agenta
      short min_sila,   //Minimalna sila agenta
	  short	ile_kate,	//Ilosc kategori w mapach	
	  short	odl_sasiad,	//Rozmiar sasiedztwa
	  short	ile_sasiad, //8==Gestosc sasiedztwa
	  short need_use_self,//Czy ma uzywac siebie
	  short walkpower,
	  short trespower,
      double ispontanic	//Ogolny szum informacyjny
		):	
		world(log_name,50),		
		MaplName(clone_str(mapl_name)),//Nazwa (bit)mapy 1. inicjujacej agentow					
		MappName(clone_str(mapp_name)),//Nazwa (bit)mapy 2. inicjujacej agentow					
		MaskName(clone_str(live_mask)),//Nazwa bitmapy maskujacej (kasujacej agentow)
	//Sub-obiekty wlasciwe dla tej symulacji
		MyWidth(Width),
		Agenci(Width,Width,NULL),//Initer == NULL wiec tworzone przez konstruktor a nie klonowanie
		MaxSila(max_sila),	//Maksymalna sila agenta
        MinSila(min_sila),  //Maksymalna sila agenta
		TrsSila(trespower), //Sila dajaca odporosc na zmiany
		IleKate(ile_kate),	//Ilosc kategori w mapach
		IleSasiad(ile_sasiad),	//8==Gestosc sasiedztwa
		OdlSasiad(odl_sasiad),	//Rozmiar sasiedztwa
		Noise(noise),
		UseSelf(need_use_self),
		//Wskazniki do podstawowych seri danych
		Firsts(NULL),
		Seconds(NULL),
		Thirds(NULL),
		Powers(NULL),
		Classif(NULL),
		BiasMode(NO_BIAS),
		spontanic(ispontanic)
{//!!!Niewiele mozna zrobic bo nie mozna tu jeszcze polegac na wirtualnych metodach klasy swiat
			jagent::ruchsily=walkpower;
            jagent::Distribution=Distribution;
			set_simulation_name("Languages_v1.4");
			set_bias_from_str("");
            jagent::MutationLevel=spontanic;
		}


//Generuje podstawowe zrodla dla wbudowanego menagera danych lub innego
//----------------------------------------------------------------------------
void jworld::make_basic_sources(sources_menager& WhatSourMen)
{
world::make_basic_sources(WhatSourMen);//Odziedziczone

//Glowne serie 
Firsts=Agenci.make_source("First mem",&jagent::First);	
if(Firsts)
	Firsts->setminmax(0,IleKate-1);	

Seconds=Agenci.make_source("Second mem",&jagent::Second);
if(Seconds)
	Seconds->setminmax(0,IleKate-1);

Thirds=Agenci.make_source("Third mem",&jagent::Third);
if(Thirds)
	Thirds->setminmax(0,IleKate-1);

Powers=Agenci.make_source("Power",&jagent::Power);
Age=Agenci.make_source("Lang. age",&jagent::Age);

Classif=Agenci.make_source("Classification",&jagent::Classif);
if(Classif)
	Classif->setminmax(0,IleKate*IleKate*IleKate-1);//Max class ==IleKate^3 bo trzy niezalezne plaszczyzny

//Umieszczenie glownych serii w menagerze serii
WhatSourMen.insert(Firsts);
WhatSourMen.insert(Seconds);
WhatSourMen.insert(Thirds);
WhatSourMen.insert(Powers);
WhatSourMen.insert(Age);
WhatSourMen.insert(Classif);
}


//Przygotowuje wspolprace z menagerem wyswietlania oraz z logiem
//------------------------------------------------------------------
void jworld::make_default_visualisation(area_menager_base& Menager)
//Rejestruje pochodne serie, tworzy domyslne "lufciki" i wklada w "Menager"
{
int iFirst=0,iSecond=0,iThird=0,iPower=0,iClassif=0;
//Uzyskanie indeksow podstawowych serii z menagera
{
if(Firsts) iFirst=Sources.search(Firsts->name());
	else  goto ERROR;
if(Seconds) iSecond=Sources.search(Seconds->name());
	else  goto ERROR;
if(Thirds)   iThird=Sources.search(Thirds->name());
	else  goto ERROR;
if(Powers)   iPower=Sources.search(Powers->name());
	else  goto ERROR; 
if(Classif)  iClassif=Sources.search(Classif->name());
	else  goto ERROR;

//Oraz utworzenie pochodnych serii statystycznych
generic_clustering_source*	FirstStat=new generic_clustering_source(Firsts);
if(!FirstStat) goto ERROR;
	else	Sources.insert(FirstStat);
generic_clustering_source*	SecondStat=new generic_clustering_source(Seconds);
if(!SecondStat) goto ERROR;
	else	Sources.insert(SecondStat);
generic_clustering_source*	ThirdStat=new generic_clustering_source(Thirds);
if(!ThirdStat) goto ERROR;
	else	Sources.insert(ThirdStat);

//UWAGA:
//Jesli IleKate > 16 to sie robi¹ bardzo duze tablice spowalniajace program!!!
generic_discrete_histogram_source*  ClassStat=new generic_discrete_histogram_source(0,IleKate*IleKate*IleKate,Classif,"DistrOf(%s[%d..%d])");//histogram z klasyfikacji jezykow
if(!ClassStat) goto ERROR;
	else	Sources.insert(ClassStat);

generic_fix_histogram_source* HistClass=new generic_fix_histogram_source(100,1,MyWidth*MyWidth,ClassStat,"Distr_%dcl(%s[%g..%g])",true);//histogram jezyków
if(!HistClass) goto ERROR;
	else	Sources.insert(HistClass);
    
generic_log_F_filter*  LogHistClass=new generic_log_F_filter(ClassStat);//Zlogarytmowany histogram jezyków
if(!LogHistClass)goto ERROR;
	else	Sources.insert(LogHistClass);

generic_fix_histogram_source*  LogLogHistClassStat=new generic_fix_histogram_source(12,0,6,LogHistClass,"LogDistr_%dcl(%s[%g..%g])");//Histogram zlogarytmowanych jezyków
if(!LogLogHistClassStat) goto ERROR;
	else	Sources.insert(LogLogHistClassStat);

//A takze utworzenie seri liczacych ich wzajemne ko-statystyki
coincidention_source* CorrFS=new coincidention_source(Firsts,Seconds);
if(!CorrFS) goto ERROR;
Sources.insert(CorrFS);//Zeby zostala kiedys zwolniona, a poza tym moze ktos kiedys...

fifo_source<double>* EntropyFSLog=new fifo_source<double>(CorrFS->Entropy(),internal_log);
if(!EntropyFSLog) goto ERROR;
int iEntropyFS=Sources.insert(EntropyFSLog);

fifo_source<double>* CorrFSLogR=new fifo_source<double>(CorrFS->Tau_a_Goodman_Kruskal(),internal_log);//Fifo korelacji pierwszych z drugimi
if(!CorrFSLogR) goto ERROR;
int iCorrFSR=Sources.insert(CorrFSLogR);
 
coincidention_source* CorrST=new coincidention_source(Seconds,Thirds);
if(!CorrST) goto ERROR;
Sources.insert(CorrST);//Zeby zostala kiedys zwolniona, a poza tym moze ktos kiedys...

fifo_source<double>* EntropySTLog=new fifo_source<double>(CorrST->Entropy(),internal_log);
if(!EntropySTLog) goto ERROR;
int iEntropyST=Sources.insert(EntropySTLog);

fifo_source<double>* CorrSTLogR=new fifo_source<double>(CorrST->Tau_a_Goodman_Kruskal(),internal_log);
if(!CorrSTLogR) goto ERROR;
int iCorrSTR=Sources.insert(CorrSTLogR);

coincidention_source* CorrTF=new coincidention_source(Thirds,Firsts);
if(!CorrTF) goto ERROR;
Sources.insert(CorrTF);//Zeby zostala kiedys zwolniona, a poza tym moze ktos kiedys...

fifo_source<double>* EntropyTFLog=new fifo_source<double>(CorrTF->Entropy(),internal_log);
if(!EntropyTFLog) goto ERROR;
int iEntropyTF=Sources.insert(EntropyTFLog);

fifo_source<double>* CorrTFLogR=new fifo_source<double>(CorrTF->Tau_a_Goodman_Kruskal(),internal_log);//Fifo korelacji pierwszych z drugimi
if(!CorrTFLogR) goto ERROR;
int iCorrTFR=Sources.insert(CorrTFLogR);

 
//I utworzenie seri liczacych ich statystyki
/*
fifo_source<double>* MeanFirstLog=new fifo_source<double>(FirstStat->Mean(),internal_log);//Fifo ze sredniej sily
if(!MeanFirstLog) goto ERROR;
int iMFirst=Sources.insert(MeanFirstLog);
*/ 
fifo_source<double>* StressFirstLog=new fifo_source<double>(FirstStat->Stress(),internal_log);//Fifo ze stresu
if(!StressFirstLog) goto ERROR;
int iSFirst=Sources.insert(StressFirstLog);
/*
fifo_source<double>* MeanSecondLog=new fifo_source<double>(SecondStat->Mean(),internal_log);//Fifo ze sredniej sily
if(!MeanSecondLog) goto ERROR;
int iMSecond=Sources.insert(MeanSecondLog);
*/
fifo_source<double>* StressSecondLog=new fifo_source<double>(SecondStat->Stress(),internal_log);//Fifo ze stresu
if(!StressSecondLog) goto ERROR;
int iSSecond=Sources.insert(StressSecondLog);
/*
fifo_source<double>* MeanThirdLog=new fifo_source<double>(ThirdStat->Mean(),internal_log);//Fifo ze sredniej sily
if(!MeanThirdLog) goto ERROR;
int iMThird=Sources.insert(MeanThirdLog);
*/
fifo_source<double>* StressThirdLog=new fifo_source<double>(ThirdStat->Stress(),internal_log);//Fifo ze stresu
if(!StressThirdLog) goto ERROR;
int iSThird=Sources.insert(StressThirdLog);

//iMainClassF,iWhichMainF,iNumClassF,			
fifo_source<double>* NumClassLog=new fifo_source<double>(ClassStat->NumOfClass(),internal_log);
if(!NumClassLog) goto ERROR;
int iNumClassF=Sources.insert(NumClassLog);
/*
fifo_source<double>* WhichMainLog=new fifo_source<double>(ClassStat->WhichMain(),internal_log);
if(!WhichMainLog) goto ERROR;
int iWhichMainF=Sources.insert(WhichMainLog);
*/
fifo_source<double>* ClassEntropyLog=new fifo_source<double>(ClassStat->Entropy(),internal_log);
if(!ClassEntropyLog) goto ERROR;
int iClassEntropy=Sources.insert(ClassEntropyLog);

fifo_source<double>* MainClassLog=new fifo_source<double>(ClassStat->MainClass(),internal_log);
if(!MainClassLog) goto ERROR;
int iMainClassF=Sources.insert(MainClassLog);

//I umieszczanie w logu tego co trzeba
Log.insert(ClassStat->NumOfClass());
Log.insert(ClassStat->Entropy());
Log.insert(ClassStat->NormEntropy());
Log.insert(ClassStat->MainClass());
Log.insert(ClassStat->WhichMain());
Log.insert(FirstStat->Stress());
Log.insert(SecondStat->Stress());
Log.insert(ThirdStat->Stress());
Log.insert(CorrFS->Entropy());
Log.insert(CorrST->Entropy());
Log.insert(CorrTF->Entropy());
Log.insert(CorrFS->NormEntropy());
Log.insert(CorrST->NormEntropy());
Log.insert(CorrTF->NormEntropy());
Log.insert(CorrFS->Chi2());
Log.insert(CorrFS->LevelOfFreedom());
Log.insert(CorrST->Chi2());
Log.insert(CorrST->LevelOfFreedom());
Log.insert(CorrTF->Chi2());
Log.insert(CorrTF->LevelOfFreedom());
Log.insert(CorrFS->V2Cramer());
Log.insert(CorrST->V2Cramer());
Log.insert(CorrTF->V2Cramer());
Log.insert(CorrFS->T2Czupurow());
Log.insert(CorrST->T2Czupurow());
Log.insert(CorrTF->T2Czupurow());
Log.insert(CorrFS->Tau_b_Goodman_Kruskal());
Log.insert(CorrFS->Tau_a_Goodman_Kruskal());
Log.insert(CorrST->Tau_b_Goodman_Kruskal());
Log.insert(CorrST->Tau_a_Goodman_Kruskal());
Log.insert(CorrTF->Tau_b_Goodman_Kruskal());
Log.insert(CorrTF->Tau_a_Goodman_Kruskal());

//PODSTAWOWA WIZUALIZACJA SERII DANYCH
//WYMIARY DOMYSLNEGO OKNA
unsigned szer=Menager.getwidth();
unsigned wyso=Menager.getheight();
assert(szer>50 && wyso>40);//Najmniejsze sensowne okno

//Obszar domyœlne - np obszar STATUSU
world::make_default_visualisation(Menager);
if(OutArea) 
{
	OutArea->set(1,1,szer/2-1,wyso/2-1);
	Menager.as_orginal(Menager.search(OutArea->name()));
}

//WLASCIWE LUFCIKI
graph* pom1=new sequence_graph(szer/2-1,wyso/4,szer-50,wyso/2-1,
							    3,Sources.make_series_info(
										iClassEntropy,iNumClassF,iMainClassF,									
											-1
										).get_ptr_val(),
								0//* Z reskalowaniem 
							   );
if(!pom1) goto ERROR;
pom1->setframe(128);
pom1->settitle("HISTORY OF CLASS");
Menager.insert(pom1);

pom1=new sequence_graph(szer/2-1,1,szer-50,wyso/4-1,
							    3,Sources.make_series_info(
										iEntropyFS,
										iEntropyST,
										iEntropyTF,
											-1
										).get_ptr_val(),
							   1/*Wspolne minimum/maximum*/);
if(!pom1) goto ERROR;
pom1->setframe(128);
pom1->settitle("HISTORY OF ENTROPY OF COINCIDENTION");
Menager.insert(pom1);

if(IleKate*IleKate*IleKate<=256)//Dla wiekszej liczby jezyków taka wizualizacja nie ma sensu
{
graph* pom=new manhattan_graph(szer-49,5*char_height('X')+6,szer,6*char_height('X')+7,//domyslne wspolrzedne
						Powers,0,//I zrodlo danych o wysokosciach, niezazadzane
						Classif,0,//Zrodlo danych o kolorach - niezazadzane
						1,		//Slupki zaczynaja sie conajmniej od 0!
									//Jesli 0 to zaczynaja sie od min>0
						0.22,		//Ulamek szerokosci przeznaczony na perspektywe
						0.77		//Ulamek wysokosci  przeznaczony na perspektywe
						);//I zrodlo danych
pom->setdatacolors(0,255);
pom->settitle("Strength of agents versus languages");
int inde=Menager.insert(pom);
Menager.minimize(inde);
}

graph* pom=new true_color_manhattan_graph(szer-49,5*char_height('X')+6,szer,6*char_height('X')+7,//domyslne wspolrzedne
						Powers,0,//I zrodlo danych o wysokosciach, niezazadzane
						Firsts,0,//Zrodla danych o kolorach
						Seconds,0,
						Thirds,0,
						1,		//Slupki zaczynaja sie conajmniej od 0!
									//Jesli 0 to zaczynaja sie od min>0
						0.22,		//Ulamek szerokosci przeznaczony na perspektywe
						0.77		//Ulamek wysokosci  przeznaczony na perspektywe
						);//I zrodlo danych
pom->setdatacolors(0,255);
pom->settitle("Strength of agents versus RGB view of languages");
Menager.insert(pom);

if(IleKate*IleKate*IleKate<=256)//Dla wiekszej liczby jezyków taka wizualizacja nie ma sensu
{
	pom=new carpet_graph(szer-49,6*char_height('X')+6,szer,7*char_height('X')+8,//domyslne wspolrzedne
		Classif);//I zrodlo danych
	pom->setdatacolors(0,255);
	pom->settitle("Map of languages");
	Menager.insert(pom);
}

generic_log_1_plus_F_filter* LogAge=new generic_log_1_plus_F_filter(Age);
if(!LogAge) goto ERROR;
Sources.insert(LogAge);

pom=new carpet_graph(szer-49,15*char_height('X')+16,szer,16*char_height('X')+17,//domyslne wspolrzedne
						LogAge);//I zrodlo danych
//pom->setdatacolors(255,511);
pom->settitle("Age of agent's language");
Menager.insert(pom);

if(ClassStat->get_size()<max(1024,Menager.getwidth()-30))//Nie ma sensu tworzyc takiej wizualizacji jesli sie histogram nie zmiesci na ekranie
{
	pom=new bars_graph(szer-49,7*char_height('X')+7,szer,8*char_height('X')+9,//domyslne wspolrzedne
		ClassStat);
	pom->setdatacolors(0,255);
	pom->settitle("Histogram of languages");
	Menager.insert(pom);
}

if(!UseSpatialCorr)
{
    pom=new bars_graph(szer-49,8*char_height('X')+8,szer,9*char_height('X')+10,//domyslne wspolrzedne
        HistClass);
    pom->setdatacolors(0,255);
    pom->settitle("Histogram of language size classes");
    Menager.insert(pom);
}
else
{
}
/*
    function_source_base* Linear=new function_source<yeqx>(SpatialCorr->get_size(),0,SpatialCorr->get_size(),"lenght"); Sources.insert(Linear);
    pom1=new scatter_graph(szer-49,8*char_height('X')+8,szer,9*char_height('X')+10,
        Linear,0,
        SpatialCorr,0);
*/						  
/*	
pom=new bars_graph(szer-49,7*char_height('X')+7,szer,8*char_height('X')+9,//domyslne wspolrzedne
						LogHistClass);
pom->setdatacolors(0,255);
pom->settitle("Log10 Histogram of languages");
Menager.insert(pom);
*/
					
//inne mniej potrzebne


pom=new manhattan_graph(szer-49, 9*char_height('X')+9,
							szer,	10*char_height('X')+11,	//domyslne wspolrzedne
						    CorrFS,0,	//I zrodlo danych
							CorrFS,0,
							1,
							0.22,		//Ulamek szerokosci przeznaczony na perspektywe
							0.77);		//Ulamek wysokosci  przeznaczony na perspektywe
pom->setdatacolors(0,255);
pom->settextcolors(0);
pom->settitle("First & Second coincidention");
Menager.insert(pom);


pom=new manhattan_graph(szer-49, 10*char_height('X')+10,
							szer,	11*char_height('X')+12,	//domyslne wspolrzedne
						    CorrST,0,	//I zrodlo danych
							CorrST,0,
							1,
							0.22,		//Ulamek szerokosci przeznaczony na perspektywe
							0.77);		//Ulamek wysokosci  przeznaczony na perspektywe
pom->setdatacolors(0,255);
pom->settextcolors(0);
pom->settitle("Second & Third coincidention");
Menager.insert(pom);						 

pom=new manhattan_graph(szer-49, 11*char_height('X')+11,
							szer,	12*char_height('X')+13,	//domyslne wspolrzedne
						    CorrTF,0,	//I zrodlo danych
							CorrTF,0,
							1,
							0.22,		//Ulamek szerokosci przeznaczony na perspektywe
							0.77);		//Ulamek wysokosci  przeznaczony na perspektywe
pom->setdatacolors(0,255);
pom->settextcolors(0);
pom->settitle("Third & First coincidention");
Menager.insert(pom); 

pom=new sequence_graph(szer-49, 12*char_height('X')+12,
							  szer,	13*char_height('X')+13,	//domyslne wspolrzedne
							    3,Sources.make_series_info(
										iSFirst,
										iSSecond,
										iSThird,
											-1
										).get_ptr_val(),
								//0// Z reskalowaniem 
							   1);//Wspolne minimum/maximum
if(!pom) goto ERROR;
pom->setframe(128);
pom->settitle("HISTORY OF STRESS");
Menager.insert(pom);

pom=new sequence_graph(szer-49,13*char_height('X')+13,szer,14*char_height('X')+14,
							    3,Sources.make_series_info(
										iCorrFSR,//iCorrFS,
										iCorrSTR,//iCorrST,
										iCorrTFR,//iCorrTF,									
											-1
										).get_ptr_val(),
								1
							   );
if(!pom) goto ERROR;
pom->setframe(128);
pom->settitle("History of correlations");
Menager.insert(pom);

//LogLog histogram jezyków - ile jest jezyków w poszczegolnych klasach wielkosci (10,100,1000,10000 uzytkownikow)
pom=new bars_graph(
                   szer-49,14*char_height('X')+14,szer,15*char_height('X')+15,
						LogLogHistClassStat);
if(!pom) goto ERROR;
pom->setdatacolors(0,32);
pom->setframe(200);
pom->settitle("Log distribution of language size classes");
Menager.insert(pom);

//Wstawianie histogramu log-log do pliku logu :)
{
    for(int i=0;i<LogLogHistClassStat->get_size();i++)
        Log.insert(LogLogHistClassStat->Class(i,"Log<%g,%g)(%s)"));
}

/*Spatal Correlation, its logs and graphs*/
if(UseSpatialCorr)
{
    generic_spatial_correlation_source* SpatialCorr1=new generic_spatial_correlation_source(Firsts,-1,spatial_correlation_mode);
    if(!SpatialCorr1) goto ERROR;
    int iSpatialCorr1=Sources.insert(SpatialCorr1);
    
    generic_spatial_correlation_source* SpatialCorr2=new generic_spatial_correlation_source(Seconds,-1,spatial_correlation_mode);
    if(!SpatialCorr2) goto ERROR;
    int iSpatialCorr2=Sources.insert(SpatialCorr2);
    
    generic_spatial_correlation_source* SpatialCorr3=new generic_spatial_correlation_source(Thirds,-1,spatial_correlation_mode);
    if(!SpatialCorr3) goto ERROR;
    int iSpatialCorr3=Sources.insert(SpatialCorr3);
    
    fifo_source<double>* ClusterSize1Log=new fifo_source<double>(SpatialCorr1->ApproximatedClusterSize(),internal_log);//Fifo z rozmiaru klastra
    if(!ClusterSize1Log) goto ERROR;
    int iClusterSize1=Sources.insert(ClusterSize1Log);
    
    fifo_source<double>* ClusterSize2Log=new fifo_source<double>(SpatialCorr2->ApproximatedClusterSize(),internal_log);//Fifo z rozmiaru klastra
    if(!ClusterSize2Log) goto ERROR;
    int iClusterSize2=Sources.insert(ClusterSize2Log);
    
    fifo_source<double>* ClusterSize3Log=new fifo_source<double>(SpatialCorr3->ApproximatedClusterSize(),internal_log);//Fifo z rozmiaru klastra
    if(!ClusterSize3Log) goto ERROR;
    int iClusterSize3=Sources.insert(ClusterSize3Log);
    Log.insert(SpatialCorr1->ApproximatedClusterSize());
    Log.insert(SpatialCorr2->ApproximatedClusterSize());
    Log.insert(SpatialCorr3->ApproximatedClusterSize());
    
    pom1=new sequence_graph(szer-49,8*char_height('X')+8,szer,9*char_height('X')+10,
        3,Sources.make_series_info(
        iSpatialCorr1,
        iSpatialCorr2,
        iSpatialCorr3,
        -1
        ).get_ptr_val(),
        0);  
    
    
    if(!pom1) goto ERROR;
    pom1->setframe(128);
    pom1->settitle("SPATIAL CORRELATION");
    Menager.insert(pom1);
    
    pom=new sequence_graph(szer-49,15*char_height('X')+15,szer,16*char_height('X')+16,//domyslne wspolrzedne
        3,Sources.make_series_info(
        iClusterSize1,//iCorrFS,
        iClusterSize2,//iCorrST,
        iClusterSize3,//iCorrTF,									
        -1
        ).get_ptr_val(),
								1
                                );
    if(!pom) goto ERROR;
    pom->setframe(128);
    pom->settitle("History of approximated cluster size");
    Menager.insert(pom);
}

//Mapy poszczegolnych memow
pom=new carpet_graph(1,wyso/2,szer/3,wyso-1,//domyslne wspolrzedne
						Firsts);//I zrodlo danych
pom->setdatacolors(0,255);
pom->settitle("Map of FIRSTs");
int inde=Menager.insert(pom);
Menager.minimize(inde);

pom=new true_color_carpet_graph(1,wyso/2,szer/3,wyso-1,//domyslne wspolrzedne
						Firsts,0,NULL,0,NULL,0);//I zrodlo danych
pom->setdatacolors(0,255);
pom->settitle("Red map of FIRSTs");
Menager.insert(pom);


pom=new carpet_graph(szer/3+1,wyso/2,szer/3*2,wyso-1,//domyslne wspolrzedne
						Seconds);//I zrodlo danych
pom->setdatacolors(0,255);
pom->settitle("Map of SECONDs");
inde=Menager.insert(pom);
Menager.minimize(inde);

pom=new true_color_carpet_graph(szer/3+1,wyso/2,szer/3*2,wyso-1,//domyslne wspolrzedne
						NULL,0,Seconds,0,NULL,0);//I zrodlo danych
pom->setdatacolors(0,255);
pom->settitle("Green map of SECONDs");
Menager.insert(pom);


pom=new carpet_graph((szer/3*2)+1,wyso/2,szer,wyso-1,//domyslne wspolrzedne
						Thirds);//I zrodlo danych
pom->setdatacolors(0,255);
pom->settitle("Map of THIRDs");
inde=Menager.insert(pom);
Menager.minimize(inde);

pom=new true_color_carpet_graph((szer/3*2)+1,wyso/2,szer,wyso-1,//domyslne wspolrzedne
						NULL,0,NULL,0,Thirds,0);//I zrodlo danych
pom->setdatacolors(0,255);
pom->settitle("Blue map of THIRDs");
Menager.insert(pom);

pom=new true_color_carpet_graph(szer-49,6*char_height('X')+6,szer,7*char_height('X')+8,//Kombinowana mapa memów - w tym samym miejscu co klasyfikacja wiec albo/albo			
			 Firsts,0,
			 Seconds,0,
			 Thirds,0
			 );//Memy jako zrodla danych o kolorach
pom->settitle("RGB map of languages");
Menager.insert(pom);

//Tworzenie obszaru sterujacego
{
wb_dynarray<rectangle_source_base*> tmp(5,(rectangle_source_base*)Sources.get(iFirst),
										  (rectangle_source_base*)Sources.get(iSecond),
										  (rectangle_source_base*)Sources.get(iThird),
										  (rectangle_source_base*)Sources.get(iPower),
										  (rectangle_source_base*)Sources.get(iClassif),
										  -1
										  );
drawable_base* pom=new steering_wheel(szer-49,0,szer,5*char_height('X')+5,tmp);			
assert(pom!=NULL);
pom->setbackground(10);
pom->settitle(" (-.-) ");
Menager.insert(pom);
}

}
Sources.new_data_version(1,1);//Oznajmia seriom ze dane sie uaktualnily	(po inicjacji)
Menager.maximize(0);
ERROR://... tu akcja na niepogode
	;//error_message(...)
}
	


//AKCJE SYMULACYJNE
//////////////////////
//////////////////////
void jworld::after_read_from_image()
//actions after read state from file. Aktualizacja pol static jagent'a!!!
{
	jagent::max_sila=MaxSila;//Maksymalna sila agenta
    jagent::min_sila=MinSila;//Mnimalna sila agenta
	jagent::ile_kate=IleKate;//Ilosc kategori w mapach	
	
	switch(IleKate)
	{
	case   2:jagent::kate_shift=7;break;
	case   4:jagent::kate_shift=6;break;
	case   8:jagent::kate_shift=5;break;
	case  16:jagent::kate_shift=4;break;
	case  32:jagent::kate_shift=3;break;
	case  64:jagent::kate_shift=2;break;
	case 128:jagent::kate_shift=1;break;
	case 256:jagent::kate_shift=0;break;
	default:
		jagent::ile_kate=IleKate=256;
		jagent::kate_shift=0;
		cerr<<"Invalid number of class (not power of 2 less than 256). Using default.\n";
		Log.GetStream()<<"Invalid number of class (not power of 2). Using default.\n";
		break;
	}
}

// stan startowy symulacji
void jworld::initialize_layers()
//-------------------------------------
{
	static int first=1;//TYMCZASOWE WYLACZENIE NADMIARU WYDRUKOW!!!
	if(first)
		Log.GetStream()<<"LANGUAGE SIMULATION:";
	//odl_sasiad=1,//Rozmiar sasiedztwa
	//ile_sasiad=8 //8==Gestosc sasiedztwa
	jagent::max_sila=MaxSila;//Maksymalna sila agenta
    jagent::min_sila=MinSila;//Mnimalna sila agenta
	jagent::ile_kate=IleKate;//Ilosc kategori w mapach	
	
	switch(IleKate)
	{
	case   2:jagent::kate_shift=7;break;
	case   4:jagent::kate_shift=6;break;
	case   8:jagent::kate_shift=5;break;
	case  16:jagent::kate_shift=4;break;
	case  32:jagent::kate_shift=3;break;
	case  64:jagent::kate_shift=2;break;
	case 128:jagent::kate_shift=1;break;
	case 256:jagent::kate_shift=0;break;
	default:
		jagent::ile_kate=IleKate=256;
		jagent::kate_shift=0;
		cerr<<"Invalid number of class (not power of 2 less than 256). Using default.\n";
		Log.GetStream()<<"Invalid number of class (not power of 2). Using default.\n";
		break;
	}
		if(first)
	  
	print_experiment_info(Log.GetStream(),Log.separator());//...wydruk wartosci parametrow symulacji

	//			USTALANIE STANÓW AGENTÓW
	//Wczytuje uzywajac konstruktora lub klonowania gdy niema, wiec inicjuje reszte pól.
	int from1=0,from2=0;
    char *pos=NULL,*old=NULL;

    from1= Agenci.init_from_bitmap(MappName.get_ptr_val(),&jagent::assignPow); //Inicjowanie sily z pliku sil
    
    if((pos=strchr(MaplName.get_ptr_val(),';'))==NULL) //Jesli nie sa to trzy nazwy plikow
    {
        from2= Agenci.init_from_bitmap(MaplName.get_ptr_val(),&jagent::assign123); //Inicjowanie przekonan z jednego pliku
    }
    else
    {   
        *pos='\0';
        from2= Agenci.init_from_bitmap(MaplName.get_ptr_val(),&jagent::assign1); //Inicjowanie przekonan 1. z pierwszego pliku
        
        old=pos+1;
        if(from2==1 && (pos=strchr(old,';'))!=NULL)
        {
            clog<<"1. layer initialised by data file "<<MaplName.get_ptr_val()<<'\n'<<flush;

            *pos='\0';
            from2= Agenci.init_from_bitmap(old,&jagent::assign2); //Inicjowanie przekonan 2. z drugiego pliku
            
            if(from2==1)
            {
                clog<<"2. layer initialised by data file "<<old<<'\n'<<flush;
                old=pos+1;
                if((pos=strchr(old,';'))!=NULL) 
                                *pos='\0'; //Na wypadek gdyby byl konczacy ';'
                from2= Agenci.init_from_bitmap(old,&jagent::assign3); //Inicjowanie przekonan 3. z trzeciego pliku         
                
                if(from2==1)
                    clog<<"3. layer initialised by data file "<<old<<'\n'<<flush;
            }
        }
    }

	//Jesli nie zainicjowane to prowizoryczna inicjacja przez konstruktory lub klonowanie
	if(from1!=1 && from2!=1)
		Agenci.reallocate_all();
	
	//Zabija m jesli w masce jesc czarny kolor
	if(Agenci.init_from_bitmap(MaskName.get_ptr_val(),&jagent::killBlack)==1 )
		Agenci.deallocate_not_OK();

	first=0;//Koniec pierwszego wywolania //TYMCZASOWO!!!
}


//Pojedynczy krok symulacji
void jworld::simulate_one_step()
//---------------------------------------
{
    _update_age();//Pomocnicza
	switch(BiasMode){
	case NO_BIAS:	        _one_step_no_bias();       break;
	case SIMPLE_BIAS:	    _one_step_simple_bias();    break;
	case CONDITIONAL_BIAS:  _one_step_conditional_bias();break;
    case SEQUENTIONAL_BIAS: _one_step_sequentional_bias();break;
	case INVALID_BIAS_MODE:
	default:
		assert("This code should never be reached.\nPROBABLY INVALID BIAS MODE!"==0);
		break;}
}

