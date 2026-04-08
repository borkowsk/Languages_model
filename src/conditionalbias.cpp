#include <string.h>  
#include <math.h>
#include <strstrea.h>
#include "jrand.h"
#include "jworld.h"
#include "SYMSHELL/histosou.hpp"
#include "SYMSHELL/clstsour.hpp" //Jest tez statsour
#include "SYMSHELL/coincsou.hpp"
#include "SYMSHELL/gadgets.hpp" 
#include "INCLUDE/wb_ptrio.h"

void	jworld::_one_step_conditional_bias()
{  
	int testowanie=0;
	const geometry_base* MyGeom=Agenci.get_geometry();				assert(MyGeom);//Geometria "swiata" symulacji
	//TROJWYMIAROWA TABLICA NA ZLICZANIE WPLYWOW
	//Ilosc dopuszczalnych kategori w kazdym memeie + pozycje na pojedyncze biasy i kombinacje podwojne
	int Wplywy[BIAS_FOR_ANY+1][BIAS_FOR_ANY+1][BIAS_FOR_ANY+1];		assert(IleKate<=BIAS_FOR_ANY);//Czy nie ma za duzo kategori na taka tablice wplywow
	
	//Alokujemy iterator Monte-Carlo
	iteratorh Monte=MyGeom->make_random_global_iterator();	//Losowanie kolejnego agenta
	
	//Idziemy po agentach iteratorem Monte-Carlo. Niektórzy moga sie powtórzyc
	while(Monte)
	{	
		size_t index=MyGeom->get_next(Monte);		//Uzyskujemy index losowo wybranego agenta	
		//if(index==FULL) continue;					//Ignorujemy jesli trafil za tablice - Moze sie zdazyc tylko dla wycinkow?)
																	assert(index!=any_layer_base::FULL);	//... tutaj nie powinno sie zdarzyc
		
		jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());// Uzyskujemy referencje do agenta omijajac asercje na NULL
		
		if(Agenci.is_empty(CenterAgent))	// Sprawdzamy czy nie jest to pusta komórka (NULL)
				continue;					// bo wtedy robic dalej by³oby bez sensu.
		
		if(CenterAgent.Power>TrsSila)		// Czy nie ma juz immunitedu na zmiany
				goto STARZENIE;				// Ma - nie robimy nic
		
		{	//KOD SZUKANIA WPLYWOW
			/////////////////////////////////////			
			iteratorh Neigh=MyGeom->make_random_neighbour_iterator(index,OdlSasiad,IleSasiad);	// Alokujemy iterator sasiedztwa
			unsigned zliczanie=0;			//Zliczanie sasiadów
			
			//Czyszczenie tabeli licznika
			memset(Wplywy,0,sizeof(Wplywy));//Trzeba zerowac cala, nawet jesli nie cala uzywamy - bo sa kolumny dla BIAS_FOR_ANY
			
			while(Neigh)
			{
				size_t index2=MyGeom->get_next(Neigh);//Uzyskujemy index sasiada		
				if(index2==any_layer_base::FULL || index2==index)	//Jesli poza obszarem symulacji lub w 
					continue;				//centrum obszaru to dalej byloby bez sensu.
				
				jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val());//Uzyskujemy referencje do sasiada omijajac asercje na NULL
				if(Agenci.is_empty(PeryfAgent))		//Sprawdzamy czy nie jest to pusta komórka (NULL)
					continue;					   // bo wtedy robic dalej by³oby bez sensu.
				
				zliczanie++;						//Zlicza wylosowanych sasiadow

				//Dodawanie sil sasiadow do licznikow w tablicach
				Wplywy[PeryfAgent.First][PeryfAgent.Second][PeryfAgent.Third]+=3*PeryfAgent.Power;//"zlicznik" dla koincydencji ABC
				
				Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][PeryfAgent.Third]+=2*PeryfAgent.Power;//"zlicznik" histogramu BxC
				Wplywy[PeryfAgent.First][BIAS_FOR_ANY][PeryfAgent.Third]+=2*PeryfAgent.Power;//"zlicznik" histogramu AxC
				Wplywy[PeryfAgent.First][PeryfAgent.Second][BIAS_FOR_ANY]+=2*PeryfAgent.Power;//"zlicznik" histogramu AxB
				
				Wplywy[PeryfAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=PeryfAgent.Power;//"zlicznik" histogramu dla Axx
				Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][BIAS_FOR_ANY]+=PeryfAgent.Power;//"zlicznik" histogramu dla xBx
				Wplywy[BIAS_FOR_ANY][BIAS_FOR_ANY][PeryfAgent.Third]+=PeryfAgent.Power;//"zlicznik" histogramu dla xxC
				
			}
			
			MyGeom->destroy_iterator(Neigh);	// upewniamy sie ze iterator zostanie usuniety
			testowanie++;						//Zlicza wylosowanych agentow
			
			//Dodawanie wlasnych sil do licznikow w tablicach
			if(UseSelf)
			{
				Wplywy[CenterAgent.First][CenterAgent.Second][CenterAgent.Third]+=3*CenterAgent.Power;//"zlicznik" dla koincydencji ABC
				
				Wplywy[BIAS_FOR_ANY][CenterAgent.Second][CenterAgent.Third]+=2*CenterAgent.Power;//"zlicznik" histogramu BxC
				Wplywy[CenterAgent.First][BIAS_FOR_ANY][CenterAgent.Third]+=2*CenterAgent.Power;//"zlicznik" histogramu AxC
				Wplywy[CenterAgent.First][CenterAgent.Second][BIAS_FOR_ANY]+=2*CenterAgent.Power;//"zlicznik" histogramu AxB
				
				Wplywy[CenterAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=CenterAgent.Power;//"zlicznik" histogramu dla Axx
				Wplywy[BIAS_FOR_ANY][CenterAgent.Second][BIAS_FOR_ANY]+=CenterAgent.Power;//"zlicznik" histogramu dla xBx
				Wplywy[BIAS_FOR_ANY][BIAS_FOR_ANY][CenterAgent.Third]+=CenterAgent.Power;//"zlicznik" histogramu dla xxC
				
			}

			//W petli dodawanie szumu i biasu
			//--------------------------------------------------
			for(int i=0,width=(BIAS_FOR_ANY+1)*(BIAS_FOR_ANY+1)*(BIAS_FOR_ANY+1);i<width;i++)
			{
				((int*)Wplywy)[i]+=long(DRAND()*Noise*(4.5*MaxSila))+((float*)BiasData->Biases)[i];//cast!!! - sztuczka zeby uniknac potrojnie zagniezdzonej petli
			}

			//Szukanie maksimow - niebanalne (?)
			//////////////////////////////////////////////////////////////////////////
			wb_dynarray<int> FillStat(4);fill(FillStat,0);	//Tablica statystyki z petli szukania maksimow
			

			int indF=-1;
			int indS=-1;
			int indT=-1;
			
			do{	//Petla poszukiwania kolejnych maksimow - do wypelnienia ind{FST}'ow		
				////////////////////////////////////////////////////////////////////////////
			int width=BIAS_FOR_ANY+1;		//"Szerokosc" tablicy szesciennej na liczniki
			int offsetA=RANDOM(IleKate);			assert(0<=offsetA && offsetA<IleKate);//Jak IleKate==2 to 0 albo 1 itd..
			int offsetB=RANDOM(IleKate);			assert(0<=offsetB && offsetB<IleKate);//Jak IleKate==2 to 0 albo 1 itd..
			int offsetC=RANDOM(IleKate);			assert(0<=offsetC && offsetC<IleKate);//Jak IleKate==2 to 0 albo 1 itd..			
			int Max=-1,pA=-1,pB=-1,pC=-1;
			FillStat[0]++; //Ile nawrotów

			//Szukanie aktualnego maksimum  (nieco rozrzutne, mozna troche przyspieszyc przez jesli BIAS_FOR_ANY bedzie zmienna ==IleKate)
			for(int i=0;i<width;i++)
			{
				int a=(i+offsetA)%width;			assert(a>=0 && a<width);
				for(int j=0;j<width;j++)
				{
					int b=(j+offsetB)%width;		assert(b>=0 && b<width);
					for(int k=0;k<width;k++)
					{
						int c=(k+offsetC)%width;	assert(c>=0 && c<width);

						int pom=Wplywy[a][b][c];
						if(pom>Max)
						{
							Max=pom;
							pA=a;pB=b;pC=c;			//Zapamietanie gdzie znaleziono maksimum
						}
					}
				}
			}
			
													assert(pA!=-1 && pB!=-1 && pC!=-1);//Cos musial znalezc (?)
			//Co zrobic z maksimum ???
			Wplywy[pA][pB][pC]=0;//Wyzerowac, zeby nie bruzdzilo w nastepnym szukaniu

			if(pA!=BIAS_FOR_ANY && indF==-1)//Zapamientanie memow do zmiany - tylko wtedy gdy slot jest jeszcze wolny
				indF=pA;
			if(pB!=BIAS_FOR_ANY && indS==-1)
				indS=pB;
			if(pC!=BIAS_FOR_ANY && indT==-1)
				indT=pC;
			FillStat[(indF!=-1)+(indS!=-1)+(indT!=-1)]++;
			}while( indF==-1 || indS==-1 || indT==-1  );

													assert(indF!=-1 && indS!=-1 && indT!=-1);//Po wyjsciu z petli wszystkie musza juz byc ustawione
			
			CenterAgent.First=indF;			//zmieniamy w agencie centralnym			
			CenterAgent.Second=indS;		//zmieniamy w agencie centralnym
			CenterAgent.Third=indT;			//zmieniamy w agencie centralnym

			//cout<<FillStat[0]<<'='<<FillStat[1]<<'+'<<FillStat[2]<<'+'<<FillStat[3]<<flush<<endl;//Wypisanie stastyki nawrotow petli
		}//KONIEC ZMIAN STANU
		//////
		
		//Sila jako wiek
STARZENIE:
		if(jagent::ruchsily)
		{
			CenterAgent.Power+=jagent::ruchsily;
			CenterAgent.Power%=jagent::max_sila;//Nigdy nie przekracza sily maksymalnej
		}
	}
	// upewniamy sie ze iterator zostanie usuniety
	MyGeom->destroy_iterator(Monte);
}

