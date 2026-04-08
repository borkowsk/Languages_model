////////////////////////////////////////////////////////////////////////////////
//  THIS PROGRAMM IS DESIGNED FOR CFCS OF ISS UW
////////////////////////////////////////////////////////////////////////////////
// Symulacja rozprzestrzeniania sie zachowan jezykowych
// metoda wielowarstwowego przekazywania przekonan
//
//	JEZYK czyli LANGUAGES
//			versja  2.10 - implementacja procesu zaci¹gania w³adzy i wyœwietlanie mapy politycznej
//			versja  2.06 - dzia³aj¹ce sterowanie czêstoœci¹ wyœwietlania i my_area_menager zadeklarowany
//			versja  2.05 - rozbudowane menu, zw³aszcza nowe opcje wizualizacji, parametr SRND i DUMP
//			versja  2.04 - Ciag³e zrzucanie sieci jako parametr
//			versja  2.03a - Implementacja zrzutu sieci SW w postaci plików NET
//			vrsja   2.01-2 - Uruchomienie modelu Small Worlds
//			versja  1.99b - przejœcie do modelu Small Worlds:
//							Przygotowanie nowego uk³adu wizualizacji
//							Przygotowanie ¿róde³ danych i grafu dla dalekich po³¹czeñ
//							Implemetcja algorytmu dynamicznych dalekich po³¹czeñ "politycznych"
//							W³¹czenie wplywów z dalekich polaczeñ do implementacji modeli wp³ywu
//							(nie przetestowane dla skomplikowanych biasów)
//
//			versja  1.53a - drobne zmiany konieczne do uruchomienia komplacji pod BDS 2006
//
//			versja  1.51-2 - drobne zmiany kosmetyczne.
//
//			version 1.5 - dodanie mapy jezyków TrueColor, i map skladowych w kolorach skladowych RGB. 			
//							Zmiana domyslnych parametrów startowych
//          version 1.41a                                                                      
//                        - dodanie wieku jezyka danego agenta i seri danych to przedstawiajacej 
//                          na wykresie logarytmicznym
//          
//          version 1.402a
//                        - Wprowadzenie 12 klas do LogLog histogramu dodanego w wersji 1.401  
//          version 1.401b              
//                        - zmiana histogramu LogLog rozmiaru jezyków na typ fix o 6 klasach
//                          czyli efekt wizualny bardzo podobny, ale inaczej oznakowane klasy
//                        - dodanie fix-histogramu klas rozmiaru jezyków 
//          version 1.4 - dodanie mutacji spontanicznych we wszystkich wersjach basiowania, 
//                      - wprowadzenie mozliwosci wylaczenia korelacji przestrzennej (UseSpatialCorr) z KODU!!!
//                      - wprowadzenie wykresu log-log rozkladu rozmiarów jêzyków (dhistosou.h),
//                      - wprowadzenie wpisywania tego histogramu do logu 
//                      - uzupelnienie czesci komunikatów o ustawieniach parametrów (ale czesc zostalo bez)
//                      - wprowadzenie parametru DSTB ostalajace rodzaj i stopien rozkladu sil (uzyskiwany przez * lub +)
//          TEST:
//          .........WIDTH=100 DSTB=-8 CLSS=8 MIPO=3 RSPC=1 VIEW=50 LOGF=10 LOGF=testW100.log
//
//          version 1.35a - ???
//          version 1.34a - usuniecie asercji zabezpieczajacych przed agentami o zerowej sile i wprowadzenie minimalnej sily (def. = 1)
//          version 1.33a - rekompilacja do nowej biblioteki i dodanie menu
//          version 1.32a - rekompilacja z nowa wersja biblioteki wizualizacji i wprowadzenie pliku "about_labguages.cpp" z kontrola czasu dla DEBUG
//          version 1.31a - wprowadzono inicjalizacje jezykow (pogladow) z trzech plikow graficznych w odcieniach szarosci
//          version 1.3a - wprowadzony podwojny baias w wersji nastepczej, rownolegly dziala zle - 
//                              wymaga inteligentnego przewazenia licznikow        
//          version 1.2a - przygotowanie do wprowadzenia "podwojnego biasu"
//			version 1.01b - poprawiony default dla tresholdu sily
//			version 1.05b - wbudowana obsluga pracy w batch'u i powtarzania eksperymentu
//			version 1.10b - wprowadzenie "biasu" dla parametrow jezyka

const char* WINDOW_HEADER="LANGUAGES version SW 2.10a, compilation "__DATE__ ", " __TIME__ ;
const char* Authors="(programed by W.Borkowski for ISS UW & Ohio State Univ.)";
const char* SCREENDUMPNAME="LANGUAGES";

int My_Rand_seed=0; //Jak 0 to RANDOMIZE jak inny to SRAND(My_Rand_seed)

#include <stdlib.h>
#include <time.h>

#include "INCLUDE/platform.hpp"
#ifdef NEW_FASHION_CPP
#include <iostream>
using namespace std;
#else
#include <iostream.h>
#endif

#include "INCLUDE/wbminmax.hpp"
#include "jrand.h"
#include "jworld.h"
#include "lang_res.h"

unsigned SWIDTH=1440*0.6666;//720;//1440;
unsigned SHEIGHT=1080*0.6666;/*na typowy rozmiar menu*/;//540;//1080;

//Nieobiektowo przekazywane do metody inicializacji zrodel 
unsigned internal_log=10000;	//Domyslna dlugosc wewnetrznych logów
bool UseSpatialCorr=false;		//Uzywanie korelacji przestrzennej (kosztownej w liczeniu)
unsigned spatial_correlation_mode=50;	//Liczba przebiegow losowan w ekonomiczniejszym trybie liczenia korelacji przestrzennej

char  LogName[512]="languagesSW2.log\0-------------------+--";
char NetCName[512]="languagesSW_\0---------------------+--";
char HistName[512]="\0--+---------languagesSW2.otx----------";
char MapLName[512]="\0--+---------languagesSW2.gif----------";
char MapPName[512]="\0--+---------powersSW2.gif------------";
char MaskName[512]="\0--+---------maskSW2.gif--------------";

unsigned iWidth=100;
unsigned iMaxIterations=0xffffffff;
unsigned iLogRatio=10;
unsigned iViewRatio=1;

bool ZrzucajNET=false;      //Czy zrzucaæ pliki sieci?
int  RuchomaSila=0;			//Czy sila ma sie powiekrzac "z wiekiem"
int  MaksymalnaSila=10000;		//Jaka najwieksza sila
int  MinimalnaSila=10;      //Jaka najmniejsza sila - jak takie same to ta sama wartosc wszedzie
int  TresProcent=10000;		//Powyzej jakiej sily zmiany "pogladu" sa juz niemozliwe

int  IloscKlas=128;
double ProcentSzumu=0;
double MutacjeSpon=0;
int  RozmiarSasiedztwa=1;
int  IleSasiadow=8;
int  BranieSiebie=1;
int  iWychodzenie=0;
int  Replay=0;
int	 AUTOSTART=0;
int  DistributionLevel=6;      //Rodzaj i stopien rozkladu sil
const char* BIAS_STR="";		//Zapis biasu zebrany z linii parametrow
double SW_start_perc=0;	//Sterowanie procesem hierarchizacjia œwiata
double SW_step_perc=0;	//c.d.
bool SW_links=false;

int parse_options(const int argc,const char* argv[]);//Zapowiedz


/*  OGOLNA FUNKCJA MAIN */
/************************/
class my_area_menager:public main_area_menager
{
	jworld* TheWorld;
public:
	void ConnectWorld(jworld* W) {TheWorld=W;}
	int _post_process_input(int input_char)
	//Po obsludze domyslnej. Zwraca 1 jesli obsluzyl.
	{                      			//	assert(TheWorld!=NULL);
										if(TheWorld==NULL) return 0;
		switch(input_char)
		{
		case ID_VIEWOPT_DUMPCO://               60100
			TheWorld->continous_dump=!TheWorld->continous_dump;
			clog<<"Screen dumping set to "<<TheWorld->continous_dump<<endl;;
		break;
		case ID_VIEWOPT_LESSOFT://              60101
		TheWorld->InputRatio*=2;
		TheWorld->LogRatio=TheWorld->InputRatio;
		clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
		break;
		case ID_VIEWOPT_MOREOFT://              60102
		TheWorld->InputRatio/=2;if(TheWorld->InputRatio<1) TheWorld->InputRatio=1;
		TheWorld->LogRatio=TheWorld->InputRatio;
		clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
		break;
		case ID_VIEWOPT_EVERY1://               60110
		TheWorld->InputRatio=1;
		TheWorld->LogRatio=TheWorld->InputRatio;
		clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
		break;
		case ID_VIEWOPT_EVERY10://              60111
		TheWorld->InputRatio=10;
		TheWorld->LogRatio=TheWorld->InputRatio;
		clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
		break;
		case ID_VIEWOPT_EVERY100://             60112
		TheWorld->InputRatio=100;
		TheWorld->LogRatio=TheWorld->InputRatio;
		clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
		break;
		case ID_VIEWOPT_EVERY1000://            60113
		TheWorld->InputRatio=1000;
		TheWorld->LogRatio=TheWorld->InputRatio;
		clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
		break;
		default:
			return 2;
		}
		return 1; //Uznaj za obsluzone
	}
	//KONSTRUKTORY
	//---------------
	//Wywolanie wiecej niz jednego konstruktora powoduje aborcje procesu!!!
  my_area_menager(size_t size, //Konstruktor dajacy zarzadce o okreslonym rozmiarze listy
				int width,int height,
				unsigned ibkg=default_half_gray
				):main_area_menager(size,width,height,ibkg)
				{TheWorld=0;}
  my_area_menager(size_t size,//Konstruktor z lista czesciowo wypelniona
				  int width,int height,
				  //bkg i frm domyslne - mozna zmienic potem
				  drawable_base* ptr/*first...NULL*/):main_area_menager(size,width,height,ptr)
				  {TheWorld=0;}
};

int main(const int argc,const char* argv[])
{
cout<<WINDOW_HEADER<<endl;
cout<<Authors<<endl;			assert((cerr<<"All assertions are active!"<<endl));
cout<<endl<<flush;

if(!parse_options(argc,argv))
		exit(1);

//INICJALIZACJA globalnego randomizera
if(My_Rand_seed==0)
	{RANDOMIZE();}
	else
	{SRAND(My_Rand_seed);}

//INICJALIZACJA systemu sub-okienek
my_area_menager Lufciki(24,SWIDTH,SHEIGHT,28);

if(!Lufciki.start(WINDOW_HEADER,argc,argv,1))
	{
	cerr<<"Can't initialize graphics"<<endl;
	exit(1);
	}

//Utworzenie sensownej nazwy pliku(-ów) do zrzutow ekranu
{
wb_pchar buf(strlen(SCREENDUMPNAME)+20);
buf.prn("%s_%ld",SCREENDUMPNAME,time(NULL));
Lufciki.set_dump_name(buf.get());
}

//INICJALIZACJA MODELU SYMULACYJNEGO
jworld& tenSwiat=*new jworld(iWidth,
						   LogName,
						   MapLName,
						   MapPName,
						   MaskName,
						   DistributionLevel,
						   ProcentSzumu/100.0,//Szum od 0-1
						   MaksymalnaSila,//Zeby byla w przedziale
						   MinimalnaSila,
						   IloscKlas,
						   RozmiarSasiedztwa,
						   IleSasiadow,
						   BranieSiebie,
						   RuchomaSila,
						   (MaksymalnaSila*TresProcent)/100.0,
							MutacjeSpon,
							SW_links,
							SW_start_perc,
							SW_step_perc
						   );

if(&tenSwiat==NULL)//Jakby siê coœ nie uda³o
	{
	cerr<<"Can't allocate simulation world!\n"<<endl;
	exit(1);
	}

Lufciki.ConnectWorld(&tenSwiat); //Menager musi mieæ dostêp do zmiennych steruj¹cych symulacj¹

tenSwiat.set_max_iteration(iMaxIterations);//Ile najwiecej krokow
tenSwiat.set_input_ratio(iViewRatio);
tenSwiat.set_log_ratio(iLogRatio);
tenSwiat.set_bias_from_str(BIAS_STR);
cout<<WINDOW_HEADER<<": LOADED."<<endl;
tenSwiat.set_history_stream(HistName);
if(ZrzucajNET && NetCName && *NetCName!='\0')
	tenSwiat.DumpNetName=NetCName;

if(Replay)
{
	tenSwiat.initialize(&Lufciki,1);//inicjalizacja wizualizacji
	cout<<WINDOW_HEADER<<": PREPARED FOR READING. WAIT!"<<endl;
	Lufciki.restore(0);
	Lufciki.replot(0);
	Lufciki.process_input();//Pierwsze zdazenia. Koncza sie po ctrl-B
	tenSwiat.read_loop(iWychodzenie);
}
else
{
	tenSwiat.initialize(&Lufciki);//inicjalizacja wizualizacji i warst symulacji
	cout<<WINDOW_HEADER<<": INITIALISED."<<endl;
	if(!AUTOSTART)
	{
		//Lufciki.process_input();//Pierwsze zdazenia. Koncza sie po ctrl-B
		//GLOWNA PETLA SYMULACJI
		cout<<WINDOW_HEADER<<": STARTED."<<endl;
		Lufciki.restore(0);
		Lufciki.replot(0);
		tenSwiat.simulation_loop(iWychodzenie);
	}
	else
	{
		int statusWin=Lufciki.search("STATUS");
		Lufciki.maximize(statusWin);
		set_char('\02');//ctrl-B ?
		for(int symulacja=0;symulacja<AUTOSTART;symulacja++)
			{
			//GLOWNA PETLA SYMULACJI
			cout<<WINDOW_HEADER<<": SIMULATION "<<symulacja<<" STARTED ."<<endl;
			tenSwiat.simulation_loop(1);
			cout<<WINDOW_HEADER<<": SIMULATION "<<symulacja<<" DONE ."<<endl;
			if(symulacja<AUTOSTART-1)
				{
				//Reinicjalizacja
				tenSwiat.restart();
				}
			}
	}

}

cout<<WINDOW_HEADER<<": CLOSING."<<endl;

cout.flush();

delete &tenSwiat;//Dealokacja swiata wraz ze wszystkimi skladowymi
cout<<"----------> See you later!!! <--------------\n"<<endl<<flush;
return 0;
}


int parse_options(const int argc,const char* argv[])
{
	for(int i=1;i<argc;i++)
    {
    if( *argv[i]=='-' ) /* Opcja X lub symshella */
		continue;

	//Make modifable 
	wb_pchar hand(clone_str(argv[i]));
	char*    rob=hand.get_ptr_val();

	//Uppercasing
	char* pom=strchr(rob,'=');
	if(pom==NULL) 
			goto ERROR; //NA PEWNO ZLE

	*pom='\0';strupr(rob);*pom='=';//Czêœæ do znaku =

	if(strcmp(rob,"HELP")==0)
	{
		goto HELPPRINT;
	}
	else
	if((pom=strstr(rob,"SPCH="))!=NULL) //Nie NULL czyli jest
	{
	MutacjeSpon=atof(pom+5);
    if(MutacjeSpon<=0 || MutacjeSpon>100)
		{
		cerr<<"!!! Bad SPCH ="<<MutacjeSpon<<" (must be in <0,100> )"<<endl;
		return 0;
		}
	cerr<<"* Spontanic change ratio in %: SPCH= "<<MutacjeSpon<<endl;
	MutacjeSpon/=100;//Ulamek a nie procent tak naprawde
	}
	else
	if((pom=strstr(rob,"NOIP="))!=NULL) //Nie NULL czyli jest
	{
	ProcentSzumu=atof(pom+5);
    if(ProcentSzumu<=0 || ProcentSzumu>100)
		{
			cerr<<"!!! Bad NOIP ="<<ProcentSzumu<<" (must be in <0,100> )"<<endl;
			return 0;
		}
		else
		{
			cerr<<"* Noise in decision in %: NOIP= "<<ProcentSzumu<<"%"<<endl;
		}
	}
    else
	if((pom=strstr(rob,"CLSS="))!=NULL) //Nie NULL czyli jest
	{
	IloscKlas=atol(pom+5);
    if(IloscKlas<2)
		{
		cerr<<"!!! Bad CLSS ="<<IloscKlas<<" (must be greater than 2 )"<<endl;
		return 0;
		}
	if(IloscKlas>256)
		{
		cerr<<"!!! Bad CLSS ="<<IloscKlas<<" (must be less or equal to 8 )"<<endl;
		return 0;
		}
    cerr<<"* Number of classes in each mem: CLSS= "<<IloscKlas<<endl;
	}
    else
	if((pom=strstr(rob,"MIPO="))!=NULL) //Nie NULL czyli jest
	{
	MinimalnaSila=atol(pom+5);
    if(MinimalnaSila<0)//0 czy 1???
		{
		cerr<<"!!! Bad MIPO ="<<MinimalnaSila<<" (must be >=1 )"<<endl;
		return 0;
		}
    cerr<<"* Minimal strenght: MIPO= "<<MinimalnaSila<<endl;
	}
    else
	if((pom=strstr(rob,"MPOW="))!=NULL) //Nie NULL czyli jest
	{
	MaksymalnaSila=atol(pom+5);
    if(MaksymalnaSila<0)//0 czy 1???
		{
		cerr<<"!!! Bad MPOW ="<<MaksymalnaSila<<" (must be >=1 )"<<endl;
		return 0;
		}
    cerr<<"* Max strenght: MPOW= "<<MaksymalnaSila<<endl;
	}
    else
	if((pom=strstr(rob,"WPOW="))!=NULL) //Nie NULL czyli jest
	{
	RuchomaSila=atol(pom+5);
    if(RuchomaSila<0)
		{
		cerr<<"!!! Bad WPOW ="<<RuchomaSila<<" (must be >=0 )"<<endl;
		return 0;
		}
	cerr<<"* Moving strenght: WPOW= "<<RuchomaSila<<endl;
	}
	else
	if((pom=strstr(rob,"SWST="))!=NULL) //Nie NULL czyli jest
	{
	   wb_pchar pom2(pom+5);
	   char* pom3=strchr(pom2.get_ptr_val(),'/');
	   if(pom3)
	   {
	   *(pom3)='\0';
	   SW_start_perc=atol(pom3+1);
	   }
	   SW_step_perc=atol(pom2.get());
	   cerr<<"* SW links will be used. "<<SW_start_perc<<"% at start, and "
			<<SW_step_perc<<"% at every step"<<endl;
	   SW_links=true;
	}
	else //ZrzucajNET
	if((pom=strstr(rob,"NETD="))!=NULL) //Nie NULL czyli jest
	{
		ZrzucajNET=(toupper(pom[5])=='Y');
		cerr<<"NETD="<<(ZrzucajNET?"Yes":"No")<<endl;
	}
	else
	if((pom=strstr(rob,"TRSP="))!=NULL) //Nie NULL czyli jest
	{
	TresProcent=atol(pom+5);
    if(TresProcent<0 || TresProcent>100)
		{
		cerr<<"!!! Bad TRSP = "<<int(TresProcent)<<"(must be in <0,100>"<<endl;
		return 0;
		}
		else
		{
		cerr<<"* Immunisation strenght treshold : TRSP= "<<int(TresProcent)<<"%"<<endl;
		if(RuchomaSila==0)//Nie ma sensu TRSP jesli nie jest ruchoma sila
			{
			RuchomaSila=1;
			cerr<<"** Automatically set WPOW to "<<RuchomaSila<<endl;
			}
		}
	}
	else
	if((pom=strstr(rob,"WIDTH="))!=NULL) //Nie NULL czyli jest
	{
	iWidth=atol(pom+6);
	if(iWidth<3)
		{
		cerr<<"!!! Bad WIDTH = "<<iWidth<<"(must be in <3,"<<SWIDTH<<">"<<endl;
		return 0;
		}
	if(iWidth>=SWIDTH)
		cerr<<"!!! WIDTH ("<<iWidth<<") is realy high!\n Simulation world may be larger than the window or even the screen."<<endl;
	cerr<<"* World width: WIDTH= "<<iWidth<<'x'<<iWidth<<endl;
	}
	else
	if((pom=strstr(rob,"WIDTHWIN="))!=NULL) //Nie NULL czyli jest
	{
	SWIDTH=atol(pom+9);
	if(SWIDTH<50)
		{
		cerr<<"!!! Bad WIDTHWIN = "<<SWIDTH<<" (must be >50)"<<endl;
		return 0;
		}
	}
	else
	if((pom=strstr(rob,"HEIGHTWIN="))!=NULL) //Nie NULL czyli jest
	{
	SHEIGHT=atol(pom+10);
	if(SHEIGHT<50)
		{
		cerr<<"!!! Bad HEIGHTWIN = "<<SHEIGHT<<" (must be >50)"<<endl;
		return 0;
		}
	}
	else
	if((pom=strstr(rob,"MAX="))!=NULL) //Nie NULL czyli jest
	{
	iMaxIterations=atol(pom+4);
	if(iMaxIterations<=0)
		{
		cerr<<"!!! Bad MAX iterations. Must be >0"<<endl;
		return 0;
		}    
	cerr<<"Maximal number of steps: MAX= "<<iMaxIterations<<endl;
	}
	else
	if((pom=strstr(rob,"LOGC="))!=NULL) //Nie NULL czyli jest
	{
	iLogRatio=atol(pom+5);
	if(iLogRatio<=0)
		{
		cerr<<"!!! Bad LOGC (write to log frequency). Must be >0"<<endl;
		return 0;
		}
	}
	else
	if((pom=strstr(rob,"VIEW="))!=NULL) //Nie NULL czyli jest
	{
	iViewRatio=atol(pom+5);
	if(iViewRatio<=0)
		{
		cerr<<"!!! Bad VIEW (visualisation frequency). Must be >0"<<endl;
		return 0;
		}
	}
	else
	if((pom=strstr(rob,"SRND="))!=NULL) //Nie NULL czyli jest
	{
	My_Rand_seed=atol(pom+5);
	if(My_Rand_seed<=0)
		{
		cerr<<"!!! Bad SRND (rand seed). Must be >=0, but 0 means time()"<<endl;
		return 0;
		}
		else
		{
			clog<<" SRAND was set to "<<My_Rand_seed<<endl;
        }
	}
	else
	if((pom=strstr(rob,"DUMP="))!=NULL) //Nie NULL czyli jest
	{
	jworld::continous_dump=(toupper(pom[5])=='Y');
	cerr<<"DUMP="<<(jworld::continous_dump?"Yes":"No")<<endl;
	}
	//continous_dump
	else
	if((pom=strstr(rob,"SELF="))!=NULL) //Nie NULL czyli jest
	{
	BranieSiebie=(toupper(pom[5])=='Y');
	cerr<<"SELF="<<(BranieSiebie?"Yes":"No")<<endl;
	}
	else
	if((pom=strstr(rob,"INDI="))!=NULL) //Nie NULL czyli jest
	{
	RozmiarSasiedztwa=atol(pom+5);
	if( RozmiarSasiedztwa>=1U && 
		RozmiarSasiedztwa<(iWidth/2-1))
		{
		cerr<<"* Radius of neighborhood: INDI="<<RozmiarSasiedztwa<<endl;;
		}
		else
		{
		cerr<<"!!! Bad INDI="<<RozmiarSasiedztwa<<" Must from 1 to "<<iWidth/2-1<<endl;
		return 0;
		}
	}
	else		
	if((pom=strstr(rob,"PRTR="))!=NULL) //Nie NULL czyli jest
	{
	IleSasiadow=atol(pom+5);
	if(IleSasiadow>1 && IleSasiadow<=sqr(RozmiarSasiedztwa*2+1)-1)
		{
		cerr<<"* How many real neighbours: PRTR="<<IleSasiadow<<endl;
		}
		else
		{
		cerr<<"!!! Bad PRTR="<<IleSasiadow
			<<"! Must from 2 to "<<sqr(RozmiarSasiedztwa*2+1)-1<<endl;
		return 0;
		}
	}
	else
	if((pom=strstr(rob,"AUTO="))!=NULL) //Nie NULL czyli jest
	{
	AUTOSTART=atol(pom+5);
	cerr<<"* AUTO="<<AUTOSTART<<endl;
	if(AUTOSTART)
		{
		iWychodzenie=1;
		cerr<<"** STOP="<<(iWychodzenie?"Yes":"No")<<endl;
		}
	}
	else
	if((pom=strstr(rob,"BIAS="))!=NULL) //Nie NULL czyli jest
	{
	BIAS_STR=rob+5;
	cerr<<"* BIAS = "<<BIAS_STR<<endl;
	static wb_pchar taker;
	taker=hand.give();//Zabiera zarzad. Zwolnienie na koncu programu.
	}
	else
	if((pom=strstr(rob,"STOP="))!=NULL) //Nie NULL czyli jest
	{
	iWychodzenie=(toupper(pom[5])=='Y');
	cerr<<"* Automatic exit when done: STOP="<<(iWychodzenie?"Yes":"No")<<endl;
	}
	else
	if((pom=strstr(rob,"ILOG="))!=NULL) //Nie NULL czyli jest
	{
	internal_log=atoi(pom+5);
	if(internal_log<50)
			{
			internal_log=50;
			cerr<<"!!! Internal log to short. Reset to default minimum ="<<internal_log<<endl;
			}
	}
	//cerr<<"\tDSTB=N - level and kind of strength distribution ("<<DistributionLevel<<")\n";
	 else
	if((pom=strstr(rob,"DSTB="))!=NULL) //Nie NULL czyli jest
	{
	DistributionLevel=atoi(pom+5);
	if(DistributionLevel==0 || abs(DistributionLevel)>100)
			{			
			cerr<<"!!! Invalid value of Distribution Level/Kind ="<<DistributionLevel<<endl;
			}
			else
			cerr<<"* Distribution Level/Kind DSTB="<<DistributionLevel<<endl;
	}
	else
	if((pom=strstr(rob,"LOGF="))!=NULL) //Nie NULL czyli jest
	{
	strcpy(LogName,pom+5);
	}else
	if((pom=strstr(rob,"MAPL="))!=NULL) //Nie NULL czyli jest
	{
	strcpy(MapLName,pom+5);
	cerr<<"* Map of languages from file \""<<MapLName<<"\"\n";
	}
	else
	if((pom=strstr(rob,"MAPP="))!=NULL) //Nie NULL czyli jest
	{
	strcpy(MapPName,pom+5);
	cerr<<"* Map of individual power from file \""<<MapPName<<"\"\n";
	}
	else
	if((pom=strstr(rob,"MASK="))!=NULL) //Nie NULL czyli jest
	{
	strcpy(MaskName,pom+5);
	cerr<<"* Mask for alive agents from file \""<<MaskName<<"\"\n";
	}	
	else
	if((pom=strstr(rob,"HIST="))!=NULL) //Nie NULL czyli jest
	{
	strcpy(HistName,pom+5);
	cerr<<"* History of the simulation will be saved to \""<<HistName<<"\"\n";
	}
	else
	if((pom=strstr(rob,"REPL="))!=NULL) //Nie NULL czyli jest
	{
	strcpy(HistName,pom+5);
	Replay=1;
	cerr<<"* The simulation will be replayed from \""<<HistName<<"\"\n";
	}
	else
	if((pom=strstr(rob,"RSPC="))!=NULL) 
	{
		if(UseSpatialCorr)
		{
		const char* lpom=pom+5;
		if(toupper(*lpom)=='N')
			spatial_correlation_mode=0;
		else
		if(toupper(*lpom)=='Y')
			spatial_correlation_mode=16;
		else
			spatial_correlation_mode=atoi(lpom);
		 cerr<<"* Random calculation of spatial correlation is "<<(spatial_correlation_mode==0?"d i s a b l e d":"e n a b l e d")<<". Multiplication="<<spatial_correlation_mode<<"\n";
		}
		else
		{
			if(*(pom+5)=='+')
			{
				cerr<<"!!!! Default mode of spatial correlation is enabled. Use RSCP again"<<endl;
				UseSpatialCorr=true;
			}
			else
			if(*(pom+5)=='-')
			{
				cerr<<"!!!! Spatial correlation was disabled."<<endl;
				UseSpatialCorr=true;
			}
			else
			cerr<<"!!! Sorry but spatial correlation is disabled, RSPC was ignored."<<endl;
		}
	}
	else
	if((pom=strstr(rob,"HELP"))!=NULL) //Nie NULL czyli jest
	{
HELPPRINT:
		cerr<<"Unknown parameter \""<<argv[i]<<"\"\n";
		cerr<<"YOU CAN USE:\n";
		cerr<<"\tREPL=hist.otx - not symulate but replay symulation history file.\n";
		cerr<<"\tMAPL=initL.gif (or BMP)- file with initialization map of languages or \n";
        cerr<<"\t    =\"init1.gif;init2.gif;init3.gif\" (or BMP)- 3. separate init file (RANDOM)\n";
		cerr<<"\tMAPP=initP.gif (or BMP)- file with initialization map of powers (RANDOM)\n";
		cerr<<"\tMASK=mask.gif	(or BMP)- mask file for alive (not black) agents (ALL ALIVE)\n";
		cerr<<"\tWIDTH=NN - matrix size ("<<iWidth<<")\n";
		cerr<<"\tSRND=NNNN - random seed if you want particular one\n";
		cerr<<"\tBIAS=item item ... - setting bias by items string (NO BIAS)\n"<<
			  "\t\the item string example: \"A1:1 A2:4 B2:2 C2:4 A4&C3:10 A5&B5&C5:12\"\n";
		cerr<<"\tCLSS=NN - number of class. Must be power of 2. ("<<IloscKlas<<")\n";
		cerr<<"\tMPOW=NN - max strength for initilization ("<<MaksymalnaSila<<")\n"	;
		cerr<<"\tMIPO=NN - min strength for initilization ("<<MinimalnaSila<<")\n"	;
		cerr<<"\tDSTB=N - level and kind of strength distribution ("<<DistributionLevel<<")\n";
		cerr<<"\nSWST=PP/PP - percent of SW links created at every step, and at the beginingg (0)\n";
		cerr<<"\nNETD=N/Y - dumping net files paralelly to statistics (N)\n";
//		cerr<<"\tWPOW=N	- walking step of strenght	("<<RuchomaSila<<")\n";
		cerr<<"\tTRSP=N - % of treshold of strenght ("<<TresProcent<<")\n";
		cerr<<"\tPRTR=2..WIDTH^2-1 - number of interaction partners ("<<IleSasiadow<<")\n";
		cerr<<"\tINDI=1..WIDTH/2-1 - interaction distance ("<<RozmiarSasiedztwa<<")\n";
		cerr<<"\tSELF=N/Y -use self for calculations ("<<(BranieSiebie?"Yes":"No")<<")\n";
		cerr<<"\tNOIP=NN - percent of noise ("<<ProcentSzumu<<")\n";
		cerr<<"\tSPCH=NN - percent of spontanic change of attitudes ("<<MutacjeSpon*100<<")\n";
		cerr<<"\tMAX=NNNN - max simulation step ("<<iMaxIterations<<")\n";
		cerr<<"\tILOG=NNNN - lenght of internal statistic logs  ("<<internal_log<<")\n";
		cerr<<"\tSTOP=N/Y - exit after MAX steps ("<<(iWychodzenie?"Yes":"No")<<")\n";
		cerr<<"\tVIEV=NNN - visualisation frequency ("<<iViewRatio<<")\n";
		cerr<<"\tDUMP=Y/N - dump, or not, screen at every simulation step\n";
		cerr<<"\tRSPC=N/Y/+/- or 1..WIDTH - Random calculation of spatial correlation ("<<(spatial_correlation_mode?"N":"Y")<<")\n";
		cerr<<"\tLOGC=N - log file saving frequency ("<<iLogRatio<<")\n";
		cerr<<"\tLOGF=name.log - file for simulation log ("<<LogName<<")\n";
		cerr<<"\tHIST=hist.otx - file for full history of simulation.\n";
		cerr<<"\tWIDTHWIN,HEIGHTWIN=XXX - initial window size.("<<SWIDTH<<'x'<<SHEIGHT<<"\n";
		cerr<<"\nAUTO=XXX - number of auto-repetition of simulation.("<<AUTOSTART<<")\n";
	return 0;
	}
	else
	{
		/* Ostatecznie wychodzi ze nie ma takiej opcji */
		ERROR:
		cerr<<"Unknown parameter \""<<argv[i]<<"\"\n";
		return 0;
	}

    }
return 1;
}



/* STATIC ALLOCATION */
//unsigned agent::max=0;//jaki jest najwiekszy taxon

