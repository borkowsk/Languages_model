/// @file
/// @brief IMPLEMENTATION OF W O R L D FOR THE SIMULATION.  (LANGUAGES PROJECT WITH P.Culicover)
//  ============================================================================================
/// @date 2026-04-09 (modified)
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include <limits.h>
//#include <assert.h>
//#include <string.h>
//#include <math.h>
#include <cstring>
#include <cmath>
#include <strstream>

#include "wb_ptrio.h"

#include "clstsour.hpp" //Jest tez statsour
#include "spatcors.hpp"
#include "coincsou.hpp"
#include "gadgets.hpp"

//#include "SYMSHELL/ohistosou.hpp" //Stary histogram z ruchoma liczba klas
#include "dhistosou.hpp" //Dyskretny histogram ze stala liczba klas
#include "fhistosou.hpp" //Histogram z ustalona arbitralnie liczba klas

#include "jrand.h"
#include "jworld.h"

//Embarcadero głupieje w setce miejsc: code has no effect mimo że na pewno ma
//#pragma warn -8019

//Konstrukcja agentow
// /////////////////////////////////
void jagent::_clean()
{
    First=-1;
    Second=-1;
    Third=-1;
    Power=-1;
    Age=0;
    Politics=RANDOM(0xffffff);
}

jagent::jagent(const jagent& ini)
{
    if(&ini!=NULL)
    {
        First=ini.First;
        Second=ini.Second;
        Third=ini.Third;
        Power=RANDOM(max_sila+1);
        Age=0;
        Politics=RANDOM(0xffffff);
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
// //////////////////////////////////////////////////////////////
short jagent::ruchsily=1;  //Maksymalny skok sily
short jagent::max_sila=256;	//Maksymalna sila agenta
short jagent::min_sila=1;  //i minimalna 
short jagent::ile_kate=256;	//Ilosc kategori w mapach
short jagent::kate_shift=0;	//Przesuniecie dla wczytywania gifa
double	jagent::MutationLevel=0;	//Prawd. spontanicznej zmiany memu (0..1)
short jagent::Distribution=1;	//Stopien rozkladu. 0->n rozklady z *, -1->-n rozklady z +

//KONSTRUKCJA	SWIATA
// //////////////////////////////////
extern unsigned internal_log;
extern unsigned spatial_correlation_mode;
extern bool UseSpatialCorr;


jworld::jworld(size_t Width,		//Szerokosc torusa macierzy agentow
      char* log_name,	//Nazwa pliku do zapisywania histori
      char* mapl_name,	//Nazwa (bit)mapy inicjujacej "skladowe"
      char* mapp_name,	//Nazwa (bit)mapy inicjujacej "sily"
      char* live_mask,	//Czarne w tej mapie sa kasowane
      short Distribution,	//Rodzaj i stopien rozkladu sil
      double noise,		//Szum informacyjny przy decyzji
      short	max_sila,	//Maksymalna sila agenta
      short min_sila,   //Minimalna sila agenta
      short	ile_kate,	//Ilosc kategori w mapach
      short	odl_sasiad,	//Rozmiar sasiedztwa
      short	ile_sasiad, //8==Gestosc sasiedztwa
      short need_use_self,	//Czy ma uzywac siebie
      short walkpower,
      short trespower,
      double ispontanic,	//Ogolny szum informacyjny
      bool i_use_SW_links,	//Czy używamy dalekich linków
      double i_SW_startconnect_percent,
      double i_SW_reconect_percent//Procent zmian dalekich linków na krok
        ):
        world(log_name,50),
        MaplName(clone_str(mapl_name)),	//Nazwa (bit)mapy 1. inicjujacej agentow
        MappName(clone_str(mapp_name)),	//Nazwa (bit)mapy 2. inicjujacej agentow
        MaskName(clone_str(live_mask)),	//Nazwa bitmapy maskujacej (kasujacej agentow)
    //Sub-obiekty wlasciwe dla tej symulacji
        MyWidth(Width),
        Agenci(Width,Width,NULL),	//Initer == NULL wiec tworzone przez konstruktor a nie klonowanie
        FarLinks(Width,Width),
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
        Politics(NULL),
        BiasMode(NO_BIAS),
        spontanic(ispontanic),
        use_SW_links(i_use_SW_links),	//Czy używamy dalekich linków
        SW_startconnect_percent(i_SW_startconnect_percent), //Startowy procent dalekich linków
        SW_reconect_percent(i_SW_reconect_percent)	//Procent zmian dalekich linków na krok
{//!!!Niewiele mozna zrobic bo nie mozna tu jeszcze polegac na wirtualnych metodach klasy swiat
        jagent::ruchsily=walkpower;
        jagent::Distribution=Distribution;
        set_simulation_name("Languages_v2 SW");
        set_bias_from_str("");
        jagent::MutationLevel=spontanic;
        _far_link::MyWorld=this;	//Podłączenie do celu odczytywania counterów przez tablice
            assert(Agenci.get_rect_geometry()->is_torus());	//ZAWSZE TORUS. ZAŁOŻONE W ALGORYTMIE DALEKICH LINKÓW itd
        //Inicjowanie bezpośrednich statystyk
        SW_dynamic_perc=0;
}


//Generuje podstawowe zrodla dla wbudowanego menagera danych lub innego
//----------------------------------------------------------------------------
void jworld::make_basic_sources()
{
    world::make_basic_sources();	//Odziedziczone

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
    Politics=Agenci.make_source("Polit. affil.",&jagent::Politics);

    Classif=Agenci.make_source("Classification",&jagent::Classif);
    if(Classif)
        Classif->setminmax(0,IleKate*IleKate*IleKate-1);	//Max class ==IleKate^3 bo trzy niezalezne plaszczyzny

    //struct_matrix_source<_far_link,unsigned>		*
    FarA=FarLinks.make_source("F.links to A",&_far_link::a);
    if(FarA)
        FarA->setminmax(0,MyWidth-1);

    //struct_matrix_source<_far_link,unsigned>		*
    FarB=FarLinks.make_source("F.links to B",&_far_link::b);
    if(FarB)
        FarB->setminmax(0,MyWidth-1);

    FCount=FarLinks.make_source("counters of far connections",&_far_link::get_target_count);

    //Umieszczenie glownych serii w menagerze serii
    Sources.insert(Firsts);
    Sources.insert(Seconds);
    Sources.insert(Thirds);
    Sources.insert(Powers);
    Sources.insert(Age);
    Sources.insert(Politics);
    Sources.insert(Classif);
    Sources.insert(FarA);
    Sources.insert(FarB);
    Sources.insert(FCount);
}


//Przygotowuje wspolprace z menagerem wyswietlania oraz z logiem
//------------------------------------------------------------------
#define ON_ERROR_MAKE {cerr<<"Error occurred during 'make_default_visualisation'"<<endl; return; }

void jworld::make_default_visualisation()
//Tworzy złożone serie (źródła) danych i definiuje log a potem
//O ILE JEST DOSTĘPNY Manager obszarów/lufcików
//umieszcza w nim domyslne wizualizacje - grafy
{
    //Metoda klasy bazowej - może nic nie robić, ale na wszelki wypadek
    world::make_default_visualisation(); //Obszar domyślne - np obszar STATUSU
    bool WithGr=this->HasAreaMenager(); //Czy będzie tworzone wyjście graficzne?

    //Indeksy serii potrzebne dalej dla grafów
    //////////////////////////////////////////////

    //Indeksy głównych serii
    int iFirst=0,iSecond=0,iThird=0,iPower=0,iClassif=0;

    //Różne inne serie
    int iFarLinksMeans=-1,iFarLinksMaxs=-1,iEntropyFS=-1,iCorrFSR=-1,
        iEntropyST=-1,iCorrSTR=-1,iEntropyTF=-1,iCorrTFR=-1,
        iSFirst=-1,iSSecond=-1,iSThird=-1,
        iNumClassF=-1,iClassEntropy=-1,iMainClassF=-1;

    //Indeksy seri dla SpatialCorrelations
    int iSpatialCorr1=-1,iSpatialCorr2=-1,iSpatialCorr3=-1,iClusterSize1=-1,iClusterSize2=-1,iClusterSize3=-1;

    //Uzyskanie indeksow podstawowych serii z menagera
    {
    if(Firsts) iFirst=Sources.search(Firsts->name());
        else  ON_ERROR_MAKE
    if(Seconds) iSecond=Sources.search(Seconds->name());
        else  ON_ERROR_MAKE
    if(Thirds)   iThird=Sources.search(Thirds->name());
        else  ON_ERROR_MAKE
    if(Powers)   iPower=Sources.search(Powers->name());
        else  ON_ERROR_MAKE
    if(Classif)  iClassif=Sources.search(Classif->name());
        else  ON_ERROR_MAKE
    }

    //Oraz utworzenie pochodnych serii statystycznych

    generic_clustering_source*	FirstStat=new generic_clustering_source(Firsts);
    if(!FirstStat) ON_ERROR_MAKE
        else	Sources.insert(FirstStat);
    generic_clustering_source*	SecondStat=new generic_clustering_source(Seconds);
    if(!SecondStat) ON_ERROR_MAKE
        else	Sources.insert(SecondStat);
    generic_clustering_source*	ThirdStat=new generic_clustering_source(Thirds);
    if(!ThirdStat) ON_ERROR_MAKE
        else	Sources.insert(ThirdStat);

    //UWAGA:
    //Jesli IleKate > 16 to sie robią bardzo duze tablice spowalniajace program!!!
    generic_discrete_histogram_source*  ClassStat=new generic_discrete_histogram_source(0,IleKate*IleKate*IleKate,Classif,"DistrOf(%s[%d..%d])");	//histogram z klasyfikacji jezykow
    if(!ClassStat) ON_ERROR_MAKE
        else	Sources.insert(ClassStat);

    generic_fix_histogram_source* HistClass=new generic_fix_histogram_source(100,1,MyWidth*MyWidth,ClassStat,"Distr_%dcl(%s[%g..%g])",true);	//histogram jezyków
    if(!HistClass) ON_ERROR_MAKE
        else	Sources.insert(HistClass);

    generic_log_F_filter*  LogHistClass=new generic_log_F_filter(ClassStat);	//Zlogarytmowany histogram jezyków
    if(!LogHistClass)ON_ERROR_MAKE
        else	Sources.insert(LogHistClass);

    generic_fix_histogram_source*  LogLogHistClassStat=new generic_fix_histogram_source(16,0,8,LogHistClass,"LogDistr_%dcl(%s[%g..%g])");	//Histogram zlogarytmowanych jezyków
    if(!LogLogHistClassStat) ON_ERROR_MAKE
        else	Sources.insert(LogLogHistClassStat);

    //Statystyki do badania dalekich linków
    generic_basic_statistics_source* FarLinksStat=new generic_basic_statistics_source(this->FCount);
    if(!FarLinksStat) ON_ERROR_MAKE
        else	Sources.insert(FarLinksStat);

    fifo_source<double>* FarLinksMeans=new fifo_source<double>(FarLinksStat->Mean(),internal_log);
    if(!FarLinksMeans) ON_ERROR_MAKE
    iFarLinksMeans=Sources.insert(FarLinksMeans);

    fifo_source<double>* FarLinksMaxs=new fifo_source<double>(FarLinksStat->Max(),internal_log);
    if(!FarLinksMaxs) ON_ERROR_MAKE
    iFarLinksMaxs=Sources.insert(FarLinksMaxs);

    //A takze utworzenie seri liczacych ich wzajemne ko-statystyki
    coincidention_source* CorrFS=new coincidention_source(Firsts,Seconds);
    if(!CorrFS) ON_ERROR_MAKE
    Sources.insert(CorrFS);	//Zeby zostala kiedys zwolniona, a poza tym moze ktos kiedys...

    fifo_source<double>* EntropyFSLog=new fifo_source<double>(CorrFS->Entropy(),internal_log);
    if(!EntropyFSLog) ON_ERROR_MAKE
    iEntropyFS=Sources.insert(EntropyFSLog);

    fifo_source<double>* CorrFSLogR=new fifo_source<double>(CorrFS->Tau_a_Goodman_Kruskal(),internal_log);	//Fifo korelacji pierwszych z drugimi
    if(!CorrFSLogR) ON_ERROR_MAKE
    iCorrFSR=Sources.insert(CorrFSLogR);

    coincidention_source* CorrST=new coincidention_source(Seconds,Thirds);
    if(!CorrST) ON_ERROR_MAKE
    Sources.insert(CorrST);	//Zeby zostala kiedys zwolniona, a poza tym moze ktos kiedys...

    fifo_source<double>* EntropySTLog=new fifo_source<double>(CorrST->Entropy(),internal_log);
    if(!EntropySTLog) ON_ERROR_MAKE
    iEntropyST=Sources.insert(EntropySTLog);

    fifo_source<double>* CorrSTLogR=new fifo_source<double>(CorrST->Tau_a_Goodman_Kruskal(),internal_log);
    if(!CorrSTLogR) ON_ERROR_MAKE
    iCorrSTR=Sources.insert(CorrSTLogR);

    coincidention_source* CorrTF=new coincidention_source(Thirds,Firsts);
    if(!CorrTF) ON_ERROR_MAKE
    Sources.insert(CorrTF);	//Zeby zostala kiedys zwolniona, a poza tym moze ktos kiedys...

    fifo_source<double>* EntropyTFLog=new fifo_source<double>(CorrTF->Entropy(),internal_log);
    if(!EntropyTFLog) ON_ERROR_MAKE
    iEntropyTF=Sources.insert(EntropyTFLog);

    fifo_source<double>* CorrTFLogR=new fifo_source<double>(CorrTF->Tau_a_Goodman_Kruskal(),internal_log);	//Fifo korelacji pierwszych z drugimi
    if(!CorrTFLogR) ON_ERROR_MAKE
    iCorrTFR=Sources.insert(CorrTFLogR);

    //I utworzenie serii liczacych ich statystyki
    /*
    fifo_source<double>* MeanFirstLog=new fifo_source<double>(FirstStat->Mean(),internal_log);	//Fifo ze sredniej sily
    if(!MeanFirstLog) ON_ERROR_MAKE
    iMFirst=Sources.insert(MeanFirstLog);
    */
    fifo_source<double>* StressFirstLog=new fifo_source<double>(FirstStat->Stress(),internal_log);	//Fifo ze stresu
    if(!StressFirstLog) ON_ERROR_MAKE
    iSFirst=Sources.insert(StressFirstLog);
    /*
    fifo_source<double>* MeanSecondLog=new fifo_source<double>(SecondStat->Mean(),internal_log);	//Fifo ze sredniej sily
    if(!MeanSecondLog) ON_ERROR_MAKE
    iMSecond=Sources.insert(MeanSecondLog);
    */
    fifo_source<double>* StressSecondLog=new fifo_source<double>(SecondStat->Stress(),internal_log);	//Fifo ze stresu
    if(!StressSecondLog) ON_ERROR_MAKE
    iSSecond=Sources.insert(StressSecondLog);
    /*
    fifo_source<double>* MeanThirdLog=new fifo_source<double>(ThirdStat->Mean(),internal_log);	//Fifo ze sredniej sily
    if(!MeanThirdLog) ON_ERROR_MAKE
    iMThird=Sources.insert(MeanThirdLog);
    */
    fifo_source<double>* StressThirdLog=new fifo_source<double>(ThirdStat->Stress(),internal_log);//Fifo ze stresu
    if(!StressThirdLog) ON_ERROR_MAKE
    iSThird=Sources.insert(StressThirdLog);

    //iMainClassF,iWhichMainF,iNumClassF,
    fifo_source<double>* NumClassLog=new fifo_source<double>(ClassStat->NumOfClass(),internal_log);
    if(!NumClassLog) ON_ERROR_MAKE
    iNumClassF=Sources.insert(NumClassLog);
    /*
    fifo_source<double>* WhichMainLog=new fifo_source<double>(ClassStat->WhichMain(),internal_log);
    if(!WhichMainLog) ON_ERROR_MAKE
    int iWhichMainF=Sources.insert(WhichMainLog);
    */
    fifo_source<double>* ClassEntropyLog=new fifo_source<double>(ClassStat->Entropy(),internal_log);
    if(!ClassEntropyLog) ON_ERROR_MAKE
    iClassEntropy=Sources.insert(ClassEntropyLog);

    fifo_source<double>* MainClassLog=new fifo_source<double>(ClassStat->MainClass(),internal_log);
    if(!MainClassLog) ON_ERROR_MAKE
    iMainClassF=Sources.insert(MainClassLog);

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
    Log.insert(FarLinksStat->Mean());
    Log.insert(FarLinksStat->Max());

    //Wstawianie histogramu log-log do pliku logu - tak żeby był w ostatnich kolumnach)
    for(int i=0;i<LogLogHistClassStat->get_size();i++)
            Log.insert( LogLogHistClassStat->Class(i,"Log<%g,%g)(%s)")  );

    //Optional Spatial Correlation statistic and its logs
    if(UseSpatialCorr)
    {
        generic_spatial_correlation_source* SpatialCorr1=new generic_spatial_correlation_source(Firsts,-1,spatial_correlation_mode);
        if(!SpatialCorr1) ON_ERROR_MAKE
        iSpatialCorr1=Sources.insert(SpatialCorr1);

        generic_spatial_correlation_source* SpatialCorr2=new generic_spatial_correlation_source(Seconds,-1,spatial_correlation_mode);
        if(!SpatialCorr2) ON_ERROR_MAKE
        iSpatialCorr2=Sources.insert(SpatialCorr2);

        generic_spatial_correlation_source* SpatialCorr3=new generic_spatial_correlation_source(Thirds,-1,spatial_correlation_mode);
        if(!SpatialCorr3) ON_ERROR_MAKE
        iSpatialCorr3=Sources.insert(SpatialCorr3);

        fifo_source<double>* ClusterSize1Log=new fifo_source<double>(SpatialCorr1->ApproximatedClusterSize(),internal_log);//Fifo z rozmiaru klastra
        if(!ClusterSize1Log) ON_ERROR_MAKE
        iClusterSize1=Sources.insert(ClusterSize1Log);

        fifo_source<double>* ClusterSize2Log=new fifo_source<double>(SpatialCorr2->ApproximatedClusterSize(),internal_log);//Fifo z rozmiaru klastra
        if(!ClusterSize2Log) ON_ERROR_MAKE
        iClusterSize2=Sources.insert(ClusterSize2Log);

        fifo_source<double>* ClusterSize3Log=new fifo_source<double>(SpatialCorr3->ApproximatedClusterSize(),internal_log);//Fifo z rozmiaru klastra
        if(!ClusterSize3Log) ON_ERROR_MAKE
        iClusterSize3=Sources.insert(ClusterSize3Log);

        Log.insert(SpatialCorr1->ApproximatedClusterSize());
        Log.insert(SpatialCorr2->ApproximatedClusterSize());
        Log.insert(SpatialCorr3->ApproximatedClusterSize());
    }


    //PODSTAWOWA WIZUALIZACJA SERII DANYCH
    if(WithGr)
    {
        area_menager&	Menager=MyAreaMenager(); //Skrót do Menagera obszarów

        //WYMIARY DOMYSLNEGO OKNA
        unsigned szer=Menager.getwidth();
        unsigned wyso=Menager.getheight();
        assert(szer>100 && wyso>80);//Najmniejsze sensowne okno

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
        if(!pom1) ON_ERROR_MAKE
        pom1->setframe(128);
        pom1->settitle("History of classes");
        Menager.insert(pom1);

        pom1=new sequence_graph(szer/2-1,1,szer-50,wyso/4-1,
                                        3,Sources.make_series_info(
                                                iEntropyFS,
                                                iEntropyST,
                                                iEntropyTF,
                                                    -1
                                                ).get_ptr_val(),
                                       1/*Wspolne minimum/maximum*/);
        if(!pom1) ON_ERROR_MAKE
        pom1->setframe(128);
        pom1->settitle("History of ENTROPY of coincidention");
        Menager.insert(pom1);

        graph* pom=new true_color_carpet_graph(1,wyso/2,szer/2-1,wyso-1,	//domyslne wspolrzedne,
        //Kombinowana mapa memów - w tym samym miejscu co klasyfikacja wiec albo/albo
                     Firsts,0,
                     Seconds,0,
                     Thirds,0
                     );//Memy jako zrodla danych o kolorach
        pom->settitle("RGB map of languages");
        Menager.insert(pom);

        //pom=new fast_carpet_graph<ptr_to_struct_matrix_source<jagent,unsigned long> ,true>(szer/2-1,wyso/2,szer-50,wyso-1,	//domyslne wspolrzedne,
        pom=new carpet_graph(szer/2-1,wyso/2,szer-50,wyso-1,	//domyslne wspolrzedne,
                     Politics,0, //Przynależność polityczna
                     true);//Nadrzędni sąsiedzi jako zrodlo danych o kolorach
        pom->settitle("POLITICAL MAP");
        Menager.insert(pom);

        //Boczne menu dodatkowych wizualizacji
        /////////////////////////////////////////////////////////
        unsigned int MLeft=szer-49;      //Lewy bok bocznego menu
        unsigned int MStep=char_height('X')+2;//WYsokość jednego paska bocznego

        //Historia dalekich połączeń Small Worlds
        pom=new sequence_graph(MLeft,5*MStep,szer,6*MStep,	//domyslne wspolrzedne
                                        3,Sources.make_series_info(
                                                iFarLinksMeans,
                                                iFarLinksMaxs,
                                        //		iFarLinksDynam,
                                                    -1
                                                ).get_ptr_val(),
                                        //0// Z reskalowaniem
                                       1);//Wspolne minimum/maximum
        if(!pom) ON_ERROR_MAKE
        pom->setframe(128);
        pom->settitle("History of far links");
        Menager.insert(pom);

        //Historia stresu
        pom=new sequence_graph(MLeft,12*MStep,szer,13*MStep,	//domyslne wspolrzedne
                                        3,Sources.make_series_info(
                                                iSFirst,
                                                iSSecond,
                                                iSThird,
                                                    -1
                                                ).get_ptr_val(),
                                        //0// Z reskalowaniem
                                       1);//Wspolne minimum/maximum
        if(!pom) ON_ERROR_MAKE
        pom->setframe(128);
        pom->settitle("History of stress");
        Menager.insert(pom);

        pom=new sequence_graph(MLeft,13*MStep,szer,14*MStep,
                                        3,Sources.make_series_info(
                                                iCorrFSR,	//iCorrFS,
                                                iCorrSTR,	//iCorrST,
                                                iCorrTFR,	//iCorrTF,
                                                    -1
                                                ).get_ptr_val(),
                                        1
                                       );
        if(!pom) ON_ERROR_MAKE
        pom->setframe(128);
        pom->settitle("History of correlations");
        Menager.insert(pom);


        pom=new true_color_manhattan_graph(MLeft,4*MStep,szer,5*MStep,	//domyslne wspolrzedne
                                Powers,0,	//I zrodlo danych o wysokosciach, niezazadzane
                                Firsts,0,	//Zrodla danych o kolorach
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

        if(IleKate*IleKate*IleKate<=256)	//Dla wiekszej liczby jezyków taka wizualizacja nie ma sensu
        {
        pom=new manhattan_graph(MLeft,5*MStep,szer,6*MStep,	//domyslne wspolrzedne
                                Powers,0,	//I zrodlo danych o wysokosciach, niezazadzane
                                Classif,0,	//Zrodlo danych o kolorach - niezazadzane
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

        generic_log_1_plus_F_filter* LogAge=new generic_log_1_plus_F_filter(Age);
        if(!LogAge) ON_ERROR_MAKE
        Sources.insert(LogAge);               //STARE AGE: 15*char_height('X')+16,szer,16*char_height('X')+17

        pom=new carpet_graph(MLeft,6*MStep,szer,7*MStep,	//domyslne wspolrzedne
                                LogAge);//I zrodlo danych
        //pom->setdatacolors(255,511);
        pom->settitle("Age of agent's language");
        Menager.insert(pom);

        if(IleKate*IleKate*IleKate<=256)	//Dla wiekszej liczby jezyków taka wizualizacja nie ma sensu
        {
            pom=new carpet_graph(MLeft,6*MStep,szer,7*MStep,	//domyslne wspolrzedne
                Classif);//I zrodlo danych
            pom->setdatacolors(0,255);
            pom->settitle("Map of languages");
            Menager.insert(pom);
        }

        if(ClassStat->get_size()<max(1024,Menager.getwidth()-30))
        //Nie ma sensu tworzyc takiej wizualizacji jesli sie histogram nie zmiesci na ekranie
        {
            pom=new bars_graph(MLeft,7*MStep,szer,8*MStep,	//domyslne wspolrzedne
                ClassStat);
            pom->setdatacolors(0,255);
            pom->settitle("Histogram of languages");
            int ipom=Menager.insert(pom);
            Menager.minimize(ipom);
        }

        //LogLog histogram jezyków - ile jest jezyków w poszczegolnych klasach wielkosci (10,100,1000,10000 uzytkownikow)
        pom=new bars_graph(MLeft,7*MStep,szer,8*MStep,
                                LogLogHistClassStat);
        if(!pom) ON_ERROR_MAKE
        pom->setdatacolors(0,32);
        pom->setframe(200);
        pom->settitle("Log distribution of language size classes");
        Menager.insert(pom);

        if(!UseSpatialCorr)
        {
            pom=new bars_graph(MLeft,8*MStep,szer,9*MStep,	//domyslne wspolrzedne
                HistClass);
            pom->setdatacolors(0,255);
            pom->settitle("Histogram of language size classes");
            Menager.insert(pom);
        }
        else
        {
        /*
            function_source_base* Linear=new function_source<yeqx>(SpatialCorr->get_size(),0,SpatialCorr->get_size(),"lenght"); Sources.insert(Linear);
            pom1=new scatter_graph(szer-49,8*char_height('X')+8,szer,9*char_height('X')+10,
                Linear,0,
                SpatialCorr,0);
        */
        /*
        pom=new bars_graph(szer-49,7*char_height('X')+7,szer,8*char_height('X')+9,	//domyslne wspolrzedne
                                LogHistClass);
        pom->setdatacolors(0,255);
        pom->settitle("Log10 Histogram of languages");
        Menager.insert(pom);
        */
        }
        //Koincydencje cech
        pom=new manhattan_graph(MLeft,9*MStep,szer,10*MStep,	//domyslne wspolrzedne
                                    CorrFS,0,	//I zrodlo danych
                                    CorrFS,0,
                                    1,
                                    0.22,		//Ulamek szerokosci przeznaczony na perspektywe
                                    0.77);		//Ulamek wysokosci  przeznaczony na perspektywe
        pom->setdatacolors(0,255);
        pom->settextcolors(0);
        pom->settitle("First & Second coincidention");
        Menager.insert(pom);

        pom=new manhattan_graph(MLeft,10*MStep,szer,11*MStep,	//domyslne wspolrzedne
                                    CorrST,0,	//I zrodlo danych
                                    CorrST,0,
                                    1,
                                    0.22,		//Ulamek szerokosci przeznaczony na perspektywe
                                    0.77);		//Ulamek wysokosci  przeznaczony na perspektywe
        pom->setdatacolors(0,255);
        pom->settextcolors(0);
        pom->settitle("Second & Third coincidention");
        Menager.insert(pom);

        pom=new manhattan_graph(MLeft,11*MStep,szer,12*MStep,	//domyslne wspolrzedne
                                    CorrTF,0,	//I zrodlo danych
                                    CorrTF,0,
                                    1,
                                    0.22,		//Ulamek szerokosci przeznaczony na perspektywe
                                    0.77);		//Ulamek wysokosci  przeznaczony na perspektywe
        pom->setdatacolors(0,255);
        pom->settextcolors(0);
        pom->settitle("Third & First coincidention");
        Menager.insert(pom);

        //Mapy poszczegolnych memow
        pom=new carpet_graph(MLeft,15*MStep,szer,16*MStep,
                                Firsts);//I zrodlo danych
        pom->setdatacolors(0,255);
        pom->settitle("Map of FIRSTs");
        int inde=Menager.insert(pom);
        Menager.minimize(inde);

        pom=new true_color_carpet_graph(MLeft,15*MStep,szer,16*MStep,	//domyslne wspolrzedne
                                Firsts,0,NULL,0,NULL,0);//I zrodlo danych
        pom->setdatacolors(0,255);
        pom->settitle("Red map of FIRSTs");
        Menager.insert(pom);

        pom=new carpet_graph(MLeft,16*MStep,szer,17*MStep,	//domyslne wspolrzedne
                                Seconds);//I zrodlo danych
        pom->setdatacolors(0,255);
        pom->settitle("Map of SECONDs");
        inde=Menager.insert(pom);
        Menager.minimize(inde);

        pom=new true_color_carpet_graph(MLeft,16*MStep,szer,17*MStep,	//domyslne wspolrzedne
                                NULL,0,Seconds,0,NULL,0);//I zrodlo danych
        pom->setdatacolors(0,255);
        pom->settitle("Green map of SECONDs");
        Menager.insert(pom);

        pom=new carpet_graph(MLeft,17*MStep,szer,18*MStep,	//domyslne wspolrzedne
                                Thirds);//I zrodlo danych
        pom->setdatacolors(0,255);
        pom->settitle("Map of THIRDs");
        inde=Menager.insert(pom);
        Menager.minimize(inde);

        pom=new true_color_carpet_graph(MLeft,17*MStep,szer,18*MStep,	//domyslne wspolrzedne
                                NULL,0,NULL,0,Thirds,0);//I zrodlo danych
        pom->setdatacolors(0,255);
        pom->settitle("Blue map of THIRDs");
        Menager.insert(pom);

        pom=new scatter_graph(MLeft,18*MStep,szer,19*MStep,	//domyślne wspolrzedne
                                FarA,0,
                                FarB,0,
                                FCount,0,
                                FCount,0);
        pom->settitle("Sources of far influence");
        pom->setbackground(256+100);
        Menager.insert(pom);

        if(UseSpatialCorr) //Optional visualisation of Spatial Correlation,
        {
            pom1=new sequence_graph(MLeft,8*MStep,szer,9*MStep,
                3,Sources.make_series_info(
                iSpatialCorr1,
                iSpatialCorr2,
                iSpatialCorr3,
                -1
                ).get_ptr_val(),
                0);

            if(!pom1) ON_ERROR_MAKE
            pom1->setframe(128);
            pom1->settitle("SPATIAL CORRELATION");
            Menager.insert(pom1);

            pom=new sequence_graph(MLeft,14*MStep,szer,15*MStep,	//domyslne wspolrzedne
                3,Sources.make_series_info(
                iClusterSize1,	//iCorrFS,
                iClusterSize2,	//iCorrST,
                iClusterSize3,	//iCorrTF,
                -1
                ).get_ptr_val(),
                                        1
                                        );
            if(!pom) ON_ERROR_MAKE
            pom->setframe(128);
            pom->settitle("History of approximated cluster size");
            Menager.insert(pom);
    }

    //Tworzenie obszaru sterujacego
    {
    wb_dynarray<rectangle_source_base*> tmp(5,(rectangle_source_base*)Sources.get(iFirst),
                                              (rectangle_source_base*)Sources.get(iSecond),
                                              (rectangle_source_base*)Sources.get(iThird),
                                              (rectangle_source_base*)Sources.get(iPower),
                                              (rectangle_source_base*)Sources.get(iClassif),
                                              -1
                                              );
    drawable_base* pom=new steering_wheel(MLeft,0,szer,4*MStep,tmp);
    assert(pom!=NULL);
    pom->setbackground(10);
    pom->settitle(" (-o-) ");
    Menager.insert(pom);
    }
}

    //JUŻ GOTOWE - MOŻNA EWENTUALNIE ODRYSOWAĆ
    Sources.new_data_version(1,1);//Oznajmia seriom ze dane sie uaktualnily	(po inicjacji)
    if(WithGr)
        this->MyAreaMenager().maximize(0);

    //return;
    //ERROR:	//... tu akcja na niepogode
    //	;//error_message(...)
}



//AKCJE SYMULACYJNE
// ////////////////////
// ////////////////////
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
    //odl_sasiad=1,	//Rozmiar sasiedztwa
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
    }        //jkania@borland.pl
        if(first)

    print_experiment_info(Log.GetStream(),Log.separator());//...wydruk wartosci parametrow symulacji

    //			USTALANIE STANÓW AGENTÓW
    //Wczytuje uzywajac konstruktora lub klonowania gdy niema, wiec inicjuje reszte pól.
    int from1=0,from2=0;
    char *pos=NULL;
    char *old=NULL;

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

    if(use_SW_links)
        _connect_far_links(SW_startconnect_percent);//Probuje przyłączyc jakiś procent dalekich linków przed startem

    first=0;//Koniec pierwszego wywolania //TYMCZASOWO!!! ???? HAHA!!! Nie mam pojęcia dlaczego...
}


//Pojedynczy krok symulacji
void jworld::simulate_one_step()
//---------------------------------------
{
    _update_age();//Pomocnicza
    if(use_SW_links)
    {
        _connect_far_links(SW_reconect_percent);//Probuje przyłączyc N% procent dalekich linków
    }
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

//Probuje przełączyc pewien procent dalekich linków
void    jworld::_connect_far_links(double Percent)
{
    unsigned N=(double(MyWidth)*double(MyWidth)*Percent)/100;
    unsigned Counter=0;
    for(unsigned i=0;i<N;i++)	//N prób - choć czasem może być podwójnie albo i wiEcej
    {
        unsigned const a=RANDOM(MyWidth);
        unsigned const b=RANDOM(MyWidth);
        //Losowo - proporcjonalnie do zasięgu
        if(Agenci.filled(a,b))	//Uwaga na puste komórki!
        {
            const jagent& on=Agenci.get(a,b);
            double r=on.Power/double(jagent::max_sila)*MyWidth/2;
 //			r=OdlSasiad+r*DRAND();//Zwykle losowanie w obrębie promienia ale nie mniej niż OdlSasiad
            r=OdlSasiad+r*DRAND()*DRAND()*DRAND();//Zagęszczone blisko losowanie w obrębie promienia
            double Angle=DRAND()*2*M_PI;
            int ta=a+r*sin(Angle);//Target a
            int tb=b+r*cos(Angle);//Target b
            ta=(ta+MyWidth)%MyWidth;//Dopasowywanie do TORUSA - zawsze!!!
            tb=(tb+MyWidth)%MyWidth;
                                    assert(0<=ta && ta<MyWidth);
                                    assert(0<=tb && tb<MyWidth);
            if((a!=ta || b!=tb) && Agenci.filled(ta,tb)) //Nie ma sensu połaczenie ze sobą i z pustym polem
            {
                //ALGORYTM BEZ LOSOWOSCI
#if 0
                if(Agenci.get(ta,tb).Power>=on.Power) //dobrowolne oddanie się w protekcje lub czasem sojusz.
                {									 //Pomijając ingerencje zewnętrzne połączenie dąży do
                                                     //najsilniejszym agenta w zasięgu r

                    double power_of_protector=0;
                    if(FarLinks.get(a,b).a!=UINT_MAX)	//Jest już jakiś protektor
                        power_of_protector=Agenci.get(FarLinks.get(a,b).a,FarLinks.get(a,b).b).Power;
                    if(Agenci.get(ta,tb).Power+on.Power>power_of_protector)	//W sojuszu z nowym, trzeba pokonac starego protektora
                    {
                        _connect_flink_to(a,b,ta,tb); Counter++;
                    }
                }
                else //Próba przyłączenia - trzeba wysiudać aktualnego protektora
                {
                    double power_of_protector=0;
                    if(FarLinks.get(ta,tb).a!=UINT_MAX)	//Jest jakiś protektor
                        power_of_protector=Agenci.get(FarLinks.get(ta,tb).a,FarLinks.get(ta,tb).b).Power;
                    if(on.Power>power_of_protector+Agenci.get(ta,tb).Power)	//Trzeba pokonac sojusz najechanego i jego portektora
                    {
                        _connect_flink_to(ta,tb,a,b); Counter++;
                    }
                }
#else
                //ALGORYTM Z PROSTYM SZUMEM - SIŁA ZREALIZOWANA MOŻE WYNOSIĆ OD 50 do 150% NOMINALNEJ
                if(Agenci.get(ta,tb).Power>=on.Power) //dobrowolne oddanie się "on-ego" w protekcje
                                                     //lub czasem zawarcie sojuszu (gdy siły zblizone)
                {									 //Pomijając ingerencje zewnętrzne  dąży do połączenia z
                                                     //najsilniejszym agenta w zasięgu r

                    double power_of_protector=0;
                    if(FarLinks.get(a,b).a!=UINT_MAX)	//Gdy "on" ma już jakiegoś protektora
                        power_of_protector=Agenci.get(FarLinks.get(a,b).a,FarLinks.get(a,b).b).Power;
                    //W sojuszu z nowym, trzeba pokonac starego protektora
                    if((0.5+DRAND())*(Agenci.get(ta,tb).Power+on.Power)>power_of_protector)
                    {
                        _connect_flink_to(a,b,ta,tb); Counter++;
                    }
                }
                else //Próba przyłączenia sobie wasala - trzeba wysiudać aktualnego protektora
                {
                    double power_of_protector=0;
                    if(FarLinks.get(ta,tb).a!=UINT_MAX)	//Jest jakiś protektor
                        power_of_protector=Agenci.get(FarLinks.get(ta,tb).a,FarLinks.get(ta,tb).b).Power;
                    //Trzeba pokonac sojusz najechanego i jego protektora
                    if((0.5+DRAND())*on.Power>power_of_protector+Agenci.get(ta,tb).Power)
                    {
                        _connect_flink_to(ta,tb,a,b); Counter++;
                    }
                }
#endif
            }
        }
    }

    //Tylko ze względu na kolorowanie!
    for(int a=0;a<N;a++)
    for(int b=0;b<N;b++)
    if(Agenci.filled(a,b))	//Uwaga na puste komórki!
    {
       _far_link pom=FarLinks.get(a,b);
       if(pom.a!=UINT_MAX)	//Jest jakiś protektor
       {
           unsigned long politofprot=Agenci.get(pom.a,pom.b).Politics;
           Agenci.get(a,b).Politics=politofprot;
       }
    }

    //Statystyka bezpośrednia
    if(N>0)
        SW_dynamic_perc=Counter/double(N);
        else
        SW_dynamic_perc=0;

}

unsigned jworld::_far_link::get_target_count()
{
    if(/*0<=a &&*/ a<MyWorld->MyWidth
        && /*0<=b &&*/ b<MyWorld->MyWidth)
        return MyWorld->FarLinks.get(a,b).count;
    else
        return 0;
}

//Implementacja zapisu stanu symulacji w formacie NET lub NET2 (z atrybutami)
void jworld::dump_net_file(const char* core_name,unsigned long Step)
{
    unsigned N=(double(MyWidth)*double(MyWidth));
    wb_pchar Name(512);
    Name.prn("%s%06d.net",core_name,Step);
    ofstream Out(Name.get(),ios::out);
    if(Out)
    {   unsigned wer,col;
        Out<<N<<endl;
        for(wer=0;wer<MyWidth;wer++)	//Po wierszach
         for(col=0;col<MyWidth;col++)	//Po kolumnach we wierszu
         {
            Out<<col<<'\t'<<wer<<'\t'<<Agenci.get(col,wer).Power<<"  "<<col<<'x'<<wer<<endl;
         }

        for(wer=0;wer<MyWidth;wer++)	//Po wierszach
         for(col=0;col<MyWidth;col++)	//Po kolumnach we wierszu
         {
            _far_link& lnk=FarLinks.get(col,wer);
            if(/*0<=lnk.a &&*/ lnk.a<MyWidth
            && /*0<=lnk.b &&*/ lnk.b<MyWidth)
             Out<<(wer*MyWidth)+col<<'\t'<<(lnk.b*MyWidth)+lnk.a<<'\t'<<1<<endl;
         }
    }
    Out.close();
}

jworld* jworld::_far_link::MyWorld; //=NULL;


/* **************************************************************** */
/*           THIS CODE IS DESIGNED & COPYRIGHT  BY:                 */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zaklad Systematyki i Geografii Roslin Uniwersytetu Warszawskiego */
/*  & Instytut Studiow Spolecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */
