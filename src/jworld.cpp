/// @file
/// @brief IMPLEMENTATION OF W O R L D FOR THE SIMULATION. (LANGUAGES PROJECT WITH P. Culicover)
/// @date 2026-05-29 (modified)
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
#pragma ide diagnostic ignored "modernize-use-auto"
#include <cstring>
#include <cmath>
#include <strstream>

#include "wb_ptrio.h"

#include "clstsour.hpp"
#include "spatcors.hpp"
#include "coincsou.hpp"
#include "gadgets.hpp"

//#include "SYMSHELL/ohistosou.hpp" //Old histogram with sliding number of classes
#include "dhistosou.hpp" //A discrete histogram with a fixed number of classes.
#include "fhistosou.hpp" //A histogram with an arbitrarily determined number of classes.

#include "jrand.h"
#include "jworld.h"
#include "jagent.h"

using namespace sym2;
using namespace sym2::data;
using namespace sym2::shell;
using namespace sym2::visual;

//When Embarcadero goes crazy in a hundred places: "code has no effect" even though it definitely has an effect!
//#pragma warn -8019

// Construction of agents:
// ///////////////////////

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
    First=ini.First;
    Second=ini.Second;
    Third=ini.Third;
    Power=RANDOM(max_pow + 1);
    Age=0;
    Politics=RANDOM(0xffffff);
}

jagent::jagent(const jagent* ini)
{
    if(ini!=NULL)
    {
        First=ini->First;
        Second=ini->Second;
        Third=ini->Third;
        Power=RANDOM(max_pow + 1);
        Age=0;
        Politics=RANDOM(0xffffff);
    }
    else
        _clean();
}

jagent::jagent()
{
    _clean();
    First=RANDOM(cate_num);
    Second=RANDOM(cate_num);
    Third=RANDOM(cate_num);
    Age=0;
    Politics=RANDOM(0xffffff);

    if(distribution >= 0)
    {
        double Power_loc=1;
        for(int i=0; i < distribution; i++)
            Power_loc*=DRAND();
        Power_loc*= max_pow + 1;
        this->Power=asserted<short>(Power_loc);
    }
    else
    {
        double Power_loc=0;
        for(int i=0;i<(-distribution); i++)
            Power_loc+=DRAND();
        Power_loc/=-distribution;
        this->Power=asserted<short>(Power_loc * (max_pow + 1));
    }
}

// Static "jagent-s" fields for initialization:
// ////////////////////////////////////////////

short jagent::pow_move=1;
short jagent::max_pow=256;
short jagent::min_pow=1;
short jagent::cate_num=256;
short jagent::cate_shift=0;
double	jagent::mutation_level=0;
short jagent::distribution=1;  //!< Degree of distribution:
                               //!< 0->n distributions with *, -1->-n distributions with + (Gaussian's like)

// CONSTRUCTION OF THE WORLD:
// //////////////////////////
extern unsigned internal_log;
extern int      spatial_correlation_mode;
extern bool use_spatial_corr;


jworld::jworld(size_t Width,
      char* log_name,
      char* mapl_name,
      char* mapp_name,
      char* live_mask,
      short distribution,
      double noise,
      short	max_power,
      short min_power,
      short	number_of_categories,
      short	neighborhood_radius,
      short	neighborhood_fill,
      short need_use_self,
      short walk_power,
      short tres_power,
      double i_spontan,
      bool i_use_SW_links,
      double i_SW_connect_percent_at_start,
      double i_SW_reconnect_percent
        ):
        world(log_name,50),
        MaplName(clone_str(mapl_name)),
        MappName(clone_str(mapp_name)),
        MaskName(clone_str(live_mask)),
        //Sub-objects specific to this simulation:
        MyWidth(Width),
        Agents(Width, Width, NULL),	//`iiniter == NULL`, so agents created by the constructor, not cloning.
        FarLinks(Width,Width),
        MaxStrength(max_power),
        MinStrength(min_power),
        TrsStrength(tres_power),
        NumOfCate(number_of_categories),
        NeighDens(neighborhood_fill),
        NeighRadius(neighborhood_radius),
        Noise(noise),
        UseSelf(need_use_self),
        //Pointers to basic data series:
        Firsts(NULL),
        Seconds(NULL),
        Thirds(NULL),
        Powers(NULL),
        Classif(NULL),
        Politics(NULL),
        Age(NULL),
        FarA(NULL),
        FarB(NULL),
        FCount(NULL),
        BiasMode(NO_BIAS),
        spontanic(i_spontan),
        use_SW_links(i_use_SW_links),
        SW_start_connect_percent(i_SW_connect_percent_at_start),
        SW_reconnect_percent(i_SW_reconnect_percent)
{ //There is not too much that can be done because we cannot rely on virtual methods of the `World` yet.
        jagent::pow_move=walk_power;
        jagent::distribution=distribution;
        world::set_simulation_name("Languages_v2 SW");
        set_bias_from_str("");
        jagent::mutation_level=spontanic;

        _far_link::MyWorld=this;	//Connection to read counters via arrays

            assert(Agents.get_rect_geometry()->is_torus());	//ALWAYS TORUS. ASSURED IN THE DISTANT LINK ALGORITHM, etc.

        //Initiating direct statistics, or rather the only one.
        SW_dynamic_perc=0;
}

void jworld::make_basic_sources()
{
    world::make_basic_sources();	//Inherited

    //Main series:
    Firsts=Agents.make_source("First mem", &jagent::First);
    if(Firsts)
        Firsts->set_min_max(0, NumOfCate - 1);

    Seconds=Agents.make_source("Second mem", &jagent::Second);
    if(Seconds)
        Seconds->set_min_max(0, NumOfCate - 1);

    Thirds=Agents.make_source("Third mem", &jagent::Third);
    if(Thirds)
        Thirds->set_min_max(0, NumOfCate - 1);

    Powers=Agents.make_source("Power", &jagent::Power);
    Age=Agents.make_source("Lang. age", &jagent::Age);
    Politics=Agents.make_source("Polit. affil.", &jagent::Politics);

    Classif=Agents.make_source("Classification", &jagent::classify);
    if(Classif)
        Classif->set_min_max(0, NumOfCate * NumOfCate * NumOfCate - 1);	//'Max class ==NumOfCate^3',because three independent layers.

    //struct_matrix_source<_far_link,unsigned>		*
    FarA=FarLinks.make_source("F.links to A",&_far_link::a);
    if(FarA)
        FarA->set_min_max(0, MyWidth - 1);

    //struct_matrix_source<_far_link,unsigned>		*
    FarB=FarLinks.make_source("F.links to B",&_far_link::b);
    if(FarB)
        FarB->set_min_max(0, MyWidth - 1);

    FCount=FarLinks.make_source("counters of far connections",&_far_link::get_target_count);

    //Placing the main data series in the series manager:
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


#define ON_ERROR_MAKE {cerr<<"Error occurred during 'make_default_visualisation'"<<endl; return; }

void jworld::make_default_visualisation()
//Prepares cooperation with the display manager and the log
//------------------------------------------------------------------
//Creates complex data series (sources) and defines the log, and then,
//if the Area Manager is available, place default
//visualization areas in it, i.e., mainly graphs.
{
    // Base class method - may do nothing, but we call it just in case.
    world::make_default_visualisation(); // Should create default areas - e.g. STATUS area
    bool WithGr=this->HasAreaMenager(); // Will there be graphical output?

    // Data series indexes needed for the statistical log and further for graphs:
    // //////////////////////////////////////////////////////////////////////////

    // Indexes of the main series:
    int	iFirst=0;		///< Raw data of the "Firsts" layer of language attributes (index of).
    int	iSecond=0;		///< Raw data of the "Seconds" layer of language attributes (index of).
    int	iThird=0;		///< Raw data of the "Thirds" layer of language attributes (index of).
    int	iPower=0;		///< Raw data of the agent strengths (index of).
    int	iClassif=0;		///< Raw data of the agent languages (index of).

    /// Indexes of various other derived series.
    int iFarLinksMeans=-1,iFarLinksMaxs=-1,iEntropyFS=-1,iCorrFSR=-1,
        iEntropyST=-1,iCorrSTR=-1,iEntropyTF=-1,iCorrTFR=-1,
        iSFirst=-1,iSSecond=-1,iSThird=-1,
        iNumClassF=-1,iClassEntropy=-1,iMainClassF=-1;

    /// Series Indexes for spatial correlations source.
    int iSpatialCorr1=-1,iSpatialCorr2=-1,iSpatialCorr3=-1,iClusterSize1=-1,iClusterSize2=-1,iClusterSize3=-1;

    // Obtaining the indexes of basic series from the data manager:
    //-------------------------------------------------------------
    {
    if(Firsts) iFirst=Sources.search(Firsts->name()); // Raw data of the "Firsts" layer of language attributes.
    else  ON_ERROR_MAKE

    if(Seconds) iSecond=Sources.search(Seconds->name()); // Raw data of the "Seconds" layer of language attributes.
    else  ON_ERROR_MAKE

    if(Thirds)   iThird=Sources.search(Thirds->name()); // Raw data of the "Thirds" layer of language attributes.
    else  ON_ERROR_MAKE

    if(Powers)   iPower=Sources.search(Powers->name()); // Raw data of the agent strengths.
    else  ON_ERROR_MAKE

    if(Classif)  iClassif=Sources.search(Classif->name()); // Raw data of the agent languages.
    else  ON_ERROR_MAKE
    }

    // And the creation of derived statistical series:
    //------------------------------------------------
    generic_clustering_source*	FirstStat=new generic_clustering_source(Firsts); ///< Statistics of the "Firsts".
    if(!FirstStat) ON_ERROR_MAKE
    Sources.insert(FirstStat);

    generic_clustering_source*	SecondStat=new generic_clustering_source(Seconds); ///< Statistics of the "Seconds".
    if(!SecondStat) ON_ERROR_MAKE
    Sources.insert(SecondStat);

    generic_clustering_source*	ThirdStat=new generic_clustering_source(Thirds); ///< Statistics of the "Thirds".
    if(!ThirdStat) ON_ERROR_MAKE
    Sources.insert(ThirdStat);

    //NOTE FOR HISTOGRAMS: If `NumOfCate > 16`, very large arrays are created, slowing down the program somewhat!

    /// Histogram of language classification (sizes of particular languages).
    generic_discrete_histogram_source*  ClassStat=new generic_discrete_histogram_source(0, NumOfCate * NumOfCate * NumOfCate, Classif, "DistrOf(%s[%d..%d])");
    if(!ClassStat) ON_ERROR_MAKE
    Sources.insert(ClassStat);

    /// Histogram of languages by size (haw many languages in particular size class).
    generic_fix_histogram_source* HistClass=new generic_fix_histogram_source(100,1,MyWidth*MyWidth,ClassStat,"Distr_%dcl(%s[%g..%g])",true);	//histogram jezyków
    if(!HistClass) ON_ERROR_MAKE
    Sources.insert(HistClass);

    generic_log_F_filter*  LogHistClass=new generic_log_F_filter(ClassStat);
    if(!LogHistClass)ON_ERROR_MAKE
    Sources.insert(LogHistClass);

    /// Histogram of languages in logarithmic size classes (10 users, 100 users, 1000 etc.).
    generic_fix_histogram_source*  LogLogHistClassStat=new generic_fix_histogram_source(16,0,8,LogHistClass,"LogDistr_%dcl(%s[%g..%g])");
    if(!LogLogHistClassStat) ON_ERROR_MAKE
    Sources.insert(LogLogHistClassStat);

    // Statistics for examining far links:
    generic_basic_statistics_source* FarLinksStat=new generic_basic_statistics_source(this->FCount);
    if(!FarLinksStat) ON_ERROR_MAKE
    Sources.insert(FarLinksStat);

    fifo_source<double>* FarLinksMeans=new fifo_source<double>(FarLinksStat->Mean(),internal_log);
    if(!FarLinksMeans) ON_ERROR_MAKE
    iFarLinksMeans=Sources.insert(FarLinksMeans);

    fifo_source<double>* FarLinksMaxs=new fifo_source<double>(FarLinksStat->Max(),internal_log);
    if(!FarLinksMaxs) ON_ERROR_MAKE
    iFarLinksMaxs=Sources.insert(FarLinksMaxs);

    /// The serie calculating the mutual co-statistics for fists and seconds. (NOTE: even larger tables)
    coincidence_source* CorrFS=new coincidence_source(Firsts, Seconds);
    if(!CorrFS) ON_ERROR_MAKE
    Sources.insert(CorrFS);

    /// A "fifo" queue of entropies of the firsts and the seconds.
    fifo_source<double>* EntropyFSLog=new fifo_source<double>(CorrFS->Entropy(),internal_log);
    if(!EntropyFSLog) ON_ERROR_MAKE
    iEntropyFS=Sources.insert(EntropyFSLog);

    /// A "fifo" queue of correlations of the firsts and the seconds.
    fifo_source<double>* CorrFSLogR=new fifo_source<double>(CorrFS->Tau_a_Goodman_Kruskal(),internal_log);
    if(!CorrFSLogR) ON_ERROR_MAKE
    iCorrFSR=Sources.insert(CorrFSLogR);

    coincidence_source* CorrST=new coincidence_source(Seconds, Thirds);
    if(!CorrST) ON_ERROR_MAKE
    Sources.insert(CorrST);

    fifo_source<double>* EntropySTLog=new fifo_source<double>(CorrST->Entropy(),internal_log);
    if(!EntropySTLog) ON_ERROR_MAKE
    iEntropyST=Sources.insert(EntropySTLog);

    fifo_source<double>* CorrSTLogR=new fifo_source<double>(CorrST->Tau_a_Goodman_Kruskal(),internal_log);
    if(!CorrSTLogR) ON_ERROR_MAKE
    iCorrSTR=Sources.insert(CorrSTLogR);

    coincidence_source* CorrTF=new coincidence_source(Thirds, Firsts);
    if(!CorrTF) ON_ERROR_MAKE
    Sources.insert(CorrTF);

    fifo_source<double>* EntropyTFLog=new fifo_source<double>(CorrTF->Entropy(),internal_log);
    if(!EntropyTFLog) ON_ERROR_MAKE
    iEntropyTF=Sources.insert(EntropyTFLog);

    fifo_source<double>* CorrTFLogR=new fifo_source<double>(CorrTF->Tau_a_Goodman_Kruskal(),internal_log);
    if(!CorrTFLogR) ON_ERROR_MAKE
    iCorrTFR=Sources.insert(CorrTFLogR);

    // Creating series that count even more derived statistics:
    /*
    fifo_source<double>* MeanFirstLog=new fifo_source<double>(FirstStat->Mean(),internal_log);
    if(!MeanFirstLog) ON_ERROR_MAKE
    iMFirst=Sources.insert(MeanFirstLog);
    */
    /// A fifo queue from first layer average stresses.
    fifo_source<double>* StressFirstLog=new fifo_source<double>(FirstStat->Stress(),internal_log);
    if(!StressFirstLog) ON_ERROR_MAKE
    iSFirst=Sources.insert(StressFirstLog);

    /*
    fifo_source<double>* MeanSecondLog=new fifo_source<double>(SecondStat->Mean(),internal_log);
    if(!MeanSecondLog) ON_ERROR_MAKE
    iMSecond=Sources.insert(MeanSecondLog);
    */
    /// A fifo queue from second layer average stresses.
    fifo_source<double>* StressSecondLog=new fifo_source<double>(SecondStat->Stress(),internal_log);
    if(!StressSecondLog) ON_ERROR_MAKE
    iSSecond=Sources.insert(StressSecondLog);
    /*
    fifo_source<double>* MeanThirdLog=new fifo_source<double>(ThirdStat->Mean(),internal_log);
    if(!MeanThirdLog) ON_ERROR_MAKE
    iMThird=Sources.insert(MeanThirdLog);
    */
    /// A fifo queue from third layer average stresses.
    fifo_source<double>* StressThirdLog=new fifo_source<double>(ThirdStat->Stress(),internal_log);
    if(!StressThirdLog) ON_ERROR_MAKE
    iSThird=Sources.insert(StressThirdLog);
    /// Language count fifo queue.
    fifo_source<double>* NumClassLog=new fifo_source<double>(ClassStat->NumOfClass(),internal_log);
    if(!NumClassLog) ON_ERROR_MAKE
    iNumClassF=Sources.insert(NumClassLog);
    /*
    fifo_source<double>* WhichMainLog=new fifo_source<double>(ClassStat->WhichMain(),internal_log);
    if(!WhichMainLog) ON_ERROR_MAKE
    int iWhichMainF=Sources.insert(WhichMainLog);
    */
    /// Fifo queue with language histogram entropy.
    fifo_source<double>* ClassEntropyLog=new fifo_source<double>(ClassStat->Entropy(),internal_log);
    if(!ClassEntropyLog) ON_ERROR_MAKE
    iClassEntropy=Sources.insert(ClassEntropyLog);
    /// A fifo queue with the size of the largest language.
    fifo_source<double>* MainClassLog=new fifo_source<double>(ClassStat->MainClass(),internal_log);
    if(!MainClassLog) ON_ERROR_MAKE
    iMainClassF=Sources.insert(MainClassLog);

    // Logging the statistical data series we are interested in:
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

    // Inserting the log-log histogram into the log file - so that it is in the last columns:
    for(int i=0;i<LogLogHistClassStat->get_size();i++)
            Log.insert(LogLogHistClassStat->Categories(i, "Log<%g,%g)(%s)")  );

    // Optional Spatial Correlation statistic and its columns in the log:
    if(use_spatial_corr)
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

        fifo_source<double>* ClusterSize2Log=new fifo_source<double>(SpatialCorr2->ApproximatedClusterSize(),internal_log);
        if(!ClusterSize2Log) ON_ERROR_MAKE
        iClusterSize2=Sources.insert(ClusterSize2Log);

        fifo_source<double>* ClusterSize3Log=new fifo_source<double>(SpatialCorr3->ApproximatedClusterSize(),internal_log);
        if(!ClusterSize3Log) ON_ERROR_MAKE
        iClusterSize3=Sources.insert(ClusterSize3Log);

        Log.insert(SpatialCorr1->ApproximatedClusterSize());
        Log.insert(SpatialCorr2->ApproximatedClusterSize());
        Log.insert(SpatialCorr3->ApproximatedClusterSize());
    }

    // Now we create data visualizations in a window if there is a window at all:
    // ///////////////////////////////////////////////////////////////////////////

    if(WithGr)
    {
        area_manager&	Menager=MyAreaManager(); ///< Shortcut to Area Manager

        // AVAILABLE WINDOW DIMENSIONS:
        unsigned szer= Menager.get_width();
        unsigned wyso= Menager.get_height();
                                        assert(szer>100 && wyso>80); //Is it larger than the smallest reasonable window?

        if(OutArea)
        {
            OutArea->set(1,1,szer/2.-1,wyso/2.-1);
            Menager.as_original(Menager.search(OutArea->name()));
        }

        // VISUALIZATION OF BASIC DATA SERIES:
        graph* pom1=new sequence_graph(szer / 2 - 1, wyso / 4, szer - 50, wyso / 2 - 1, //default coordinates of this display area
                                        3, Sources.make_series_info(
                                                iClassEntropy,iNumClassF,iMainClassF,
                                                    -1
                                                ).get_ptr_val(),
                                                   0 // 0 means with rescaling
                                       );
        if(!pom1) ON_ERROR_MAKE
        pom1->set_frame(128);
        pom1->set_title("History of classes");
        Menager.insert(pom1);

        pom1=new sequence_graph(szer/2-1,1,szer-50,wyso/4-1, //default coordinates of this display area
                                        3,Sources.make_series_info(
                                                iEntropyFS,
                                                iEntropyST,
                                                iEntropyTF,
                                                    -1
                                                ).get_ptr_val(),
                                       1 // 1 means common minimum and maximum.
                                       );
        if(!pom1) ON_ERROR_MAKE
        pom1->set_frame(128);
        pom1->set_title("History of ENTROPY of coincidence");
        Menager.insert(pom1);

        graph* pom=new true_color_carpet_graph(1,wyso/2,szer/2-1,wyso-1, //Combined meme map - in the same place as the classification, so either/or
                     Firsts,0,
                     Seconds,0,
                     Thirds,0 // Memes, i.e., language attributes, appear here as sources of color data
                     );
        pom->set_title("RGB map of languages");
        Menager.insert(pom);

        //pom=new fast_carpet_graph<ptr_to_struct_matrix_source<jagent,unsigned long> ,true>(szer/2-1,wyso/2,szer-50,wyso-1,
        pom=new carpet_graph(szer/2-1,wyso/2,szer-50,wyso-1,  //default coordinates of this display area
                     Politics,0, //Visualization of agents' political affiliation
                     true // Master agents as a source of color data
                     );
        pom->set_title("POLITICAL MAP");
        Menager.insert(pom);

        // Side menu for additional visualizations:

        unsigned int MLeft=szer-49;      ///< Left edge of the side menu.
        unsigned int MStep=char_height('X')+2; ///< Side menu one line height.

        // The history of changes in distant connections (the structure of Small Worlds):
        pom=new sequence_graph(MLeft,5*MStep,szer,6*MStep,
                                        3,Sources.make_series_info(
                                                iFarLinksMeans,
                                                iFarLinksMaxs,
                                        //		iFarLinksDynam,
                                                    -1
                                                ).get_ptr_val(),
                               1 //or 0?
                               );
        if(!pom) ON_ERROR_MAKE
        pom->set_frame(128);
        pom->set_title("History of far links");
        Menager.insert(pom);

        // Stress history:
        pom=new sequence_graph(MLeft,12*MStep,szer,13*MStep,
                                        3,Sources.make_series_info(
                                                iSFirst,
                                                iSSecond,
                                                iSThird,
                                                    -1
                                                ).get_ptr_val(),
                               1 //or 0?
                               );
        if(!pom) ON_ERROR_MAKE
        pom->set_frame(128);
        pom->set_title("History of stress");
        Menager.insert(pom);

        // History of correlation:
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
        pom->set_frame(128);
        pom->set_title("History of correlations");
        Menager.insert(pom);

        // Visualization of the world in the colors of the languages and with the height of the bars visualizing the strength of agents.
        pom=new true_color_manhattan_graph(MLeft,4*MStep,szer,5*MStep,
                                Powers,0,	//Bar height data source (unmanaged)
                                Firsts,0,	//Color component data sources (RGB)
                                Seconds,0,
                                Thirds,0,
                                1,		//If 1, then the bars start at least from 0! If 0, then they can start from `min>0`.
                                0.22,		//A fraction of the width is allocated to perspective
                                0.77		//A fraction of the height is dedicated to perspective
                                );
        pom->set_data_colors(0, 255);
        pom->set_title("Strength of agents versus RGB view of languages");
        Menager.insert(pom);

        if(NumOfCate * NumOfCate * NumOfCate <= 256) //For more languages, the following visualization does not make sense.
        {
            pom=new manhattan_graph(MLeft,5*MStep,szer,6*MStep,
                                    Powers,0,	//Bar height data source (unmanaged)
                                    Classif,0,	//Artificial color data source (unmanaged)
                                    1,		//If 1, then the bars start at least from 0! If 0, then they can start from `min>0`.
                                    0.22,		//A fraction of the width is allocated to perspective
                                    0.77		//A fraction of the height is dedicated to perspective
                                    );
            pom->set_data_colors(0, 255);
            pom->set_title("Strength of agents versus languages");
            int inde=Menager.insert(pom);
            Menager.minimize(inde);
        }

        generic_log_1_plus_F_filter* LogAge=new generic_log_1_plus_F_filter(Age); // AGE: 15*char_height('X')+16,szer,16*char_height('X')+17 (Something old???)
        if(!LogAge) ON_ERROR_MAKE
        Sources.insert(LogAge);

        // How long has the current language been used here?
        pom=new carpet_graph(MLeft,6*MStep,szer,7*MStep,
                                LogAge);
        //pom->set_data_colors(255,511);
        pom->set_title("Age of agent's language");
        Menager.insert(pom);

        if(NumOfCate * NumOfCate * NumOfCate <= 256) //For more languages, the following visualization does not make sense.
        {
            pom=new carpet_graph(MLeft,6*MStep,szer,7*MStep,
                Classif	//Artificial color data source, no more than 256
                );
            pom->set_data_colors(0, 255);
            pom->set_title("Map of languages");
            Menager.insert(pom);
        }

        if(ClassStat->get_size()<max(1024, Menager.get_width() - 30)) // There is no point in creating such a visualization if the histogram does not fit on the screen
        {
            pom=new bars_graph(MLeft,7*MStep,szer,8*MStep,
                ClassStat
                );
            pom->set_data_colors(0, 255);
            pom->set_title("Histogram of languages");
            int ipom=Menager.insert(pom);
            Menager.minimize(ipom);
        }

        // Log-Log histogram of languages - how many languages are there in each size class (10, 100, 1000, 10 000 users)
        pom=new bars_graph(MLeft,7*MStep,szer,8*MStep,
                                LogLogHistClassStat);
        if(!pom) ON_ERROR_MAKE
        pom->set_data_colors(0, 32);
        pom->set_frame(200);
        pom->set_title("Log distribution of language size classes");
        Menager.insert(pom);

        if(!use_spatial_corr)
        {
            pom=new bars_graph(MLeft,8*MStep,szer,9*MStep,
                HistClass);
            pom->set_data_colors(0, 255);
            pom->set_title("Histogram of language size classes");
            Menager.insert(pom);
        }
        else
        {
        /*
            function_source_base* Linear=new function_source<y_eq_x>(SpatialCorr->get_size(),0,SpatialCorr->get_size(),"length"); Sources.insert(Linear);
            pom1=new scatter_graph(szer-49,8*char_height('X')+8,szer,9*char_height('X')+10,
                Linear,0,
                SpatialCorr,0);
        */
        /*
            pom=new bars_graph(szer-49,7*char_height('X')+7,szer,8*char_height('X')+9,
                                    LogHistClass);
            pom->set_data_colors(0,255);
            pom->set_title("Log10 Histogram of languages");
            Menager.insert(pom);
        */
        }

        //Coincidences of features
        pom=new manhattan_graph(MLeft,9*MStep,szer,10*MStep,
                                    CorrFS,0,
                                    CorrFS,0,
                                    1,
                                    0.22,
                                    0.77);
        pom->set_data_colors(0, 255);
        pom->set_text_colors(0);
        pom->set_title("First & Second coincidence");
        Menager.insert(pom);

        pom=new manhattan_graph(MLeft,10*MStep,szer,11*MStep,
                                    CorrST,0,
                                    CorrST,0,
                                    1,
                                    0.22,
                                    0.77);
        pom->set_data_colors(0, 255);
        pom->set_text_colors(0);
        pom->set_title("Second & Third coincidence");
        Menager.insert(pom);

        pom=new manhattan_graph(MLeft,11*MStep,szer,12*MStep,
                                    CorrTF,0,
                                    CorrTF,0,
                                    1,
                                    0.22,
                                    0.77);
        pom->set_data_colors(0, 255);
        pom->set_text_colors(0);
        pom->set_title("Third & First coincidence");
        Menager.insert(pom);

        // Maps of individual memes - language attributes:
        //------------------------------------------------

        pom=new carpet_graph(MLeft,15*MStep,szer,16*MStep,
                                Firsts);
        pom->set_data_colors(0, 255);
        pom->set_title("Map of FIRSTs");
        int inde=Menager.insert(pom);
        Menager.minimize(inde);

        pom=new true_color_carpet_graph(MLeft,15*MStep,szer,16*MStep,
                                Firsts,0,
                                NULL,0,
                                NULL,0);
        pom->set_data_colors(0, 255);
        pom->set_title("Red map of FIRSTs");
        Menager.insert(pom);

        pom=new carpet_graph(MLeft,16*MStep,szer,17*MStep,
                                Seconds);
        pom->set_data_colors(0, 255);
        pom->set_title("Map of SECONDs");
        inde=Menager.insert(pom);
        Menager.minimize(inde);

        pom=new true_color_carpet_graph(MLeft,16*MStep,szer,17*MStep,
                                NULL,0,
                                Seconds,0,
                                NULL,0);
        pom->set_data_colors(0, 255);
        pom->set_title("Green map of SECONDs");
        Menager.insert(pom);

        pom=new carpet_graph(MLeft,17*MStep,szer,18*MStep,
                                Thirds);
        pom->set_data_colors(0, 255);
        pom->set_title("Map of THIRDs");
        inde=Menager.insert(pom);
        Menager.minimize(inde);

        pom=new true_color_carpet_graph(MLeft,17*MStep,szer,18*MStep,
                                NULL,0,
                                NULL,0,
                                Thirds,0);
        pom->set_data_colors(0, 255);
        pom->set_title("Blue map of THIRDs");
        Menager.insert(pom);

        // Information about far links:
        pom=new scatter_graph(MLeft,18*MStep,szer,19*MStep,
                                FarA,0,
                                FarB,0,
                                FCount,0,
                                FCount,0);
        pom->set_title("Sources of far influence");
        pom->set_background(256 + 100);
        Menager.insert(pom);

        if(use_spatial_corr)
        {
            //Optional visualization of Spatial Correlation
            pom1=new sequence_graph(MLeft,8*MStep,szer,9*MStep,
                3,Sources.make_series_info(
                iSpatialCorr1,
                iSpatialCorr2,
                iSpatialCorr3,
                -1
                ).get_ptr_val(),
                0);

            if(!pom1) ON_ERROR_MAKE
            pom1->set_frame(128);
            pom1->set_title("SPATIAL CORRELATION");
            Menager.insert(pom1);

            pom=new sequence_graph(MLeft,14*MStep,szer,15*MStep,
                                    3,Sources.make_series_info(
                                    iClusterSize1,	//iCorrFS,
                                    iClusterSize2,	//iCorrST,
                                    iClusterSize3,	//iCorrTF,
                                    -1
                                    ).get_ptr_val(),
                                1
                                );
            if(!pom) ON_ERROR_MAKE
            pom->set_frame(128);
            pom->set_title("History of approximated cluster size");
            Menager.insert(pom);
    }

    // Creating a zoom control area on maps.
    {
        wb_dynarray<rectangle_source_base*> tmp(5,(rectangle_source_base*)Sources.get(iFirst),
                                                  (rectangle_source_base*)Sources.get(iSecond),
                                                  (rectangle_source_base*)Sources.get(iThird),
                                                  (rectangle_source_base*)Sources.get(iPower),
                                                  (rectangle_source_base*)Sources.get(iClassif),
                                                  -1
                                                  );
        drawable_base* pWheel=new steering_wheel(MLeft, 0, szer, 4 * MStep, tmp);  assert(pWheel != NULL);
        pWheel->set_background(10);
        pWheel->set_title(" (-o-) ");
        Menager.insert(pWheel);
    }
}

    // FINALLY, DONE - YOU CAN ALSO DRAW IT ALL:
    Sources.new_data_version(1,1); // Notifies the series that data has been updated (after initialization)
    if(WithGr)
        this->MyAreaManager().maximize(0); // Minimizing the zero-index area (OutArea?)
}



// TYPICAL SIMULATION ACTIONS:
// ///////////////////////////

void jworld::after_read_from_image()
{
    jagent::max_pow=MaxStrength; // Sets the maximum agent strength
    jagent::min_pow=MinStrength; // Sets the minimum agent strength
    jagent::cate_num=NumOfCate; // Sets the number of categories

    switch(NumOfCate) //Sets the bit offset for reading categories
    {
    case   2:jagent::cate_shift=7;break;
    case   4:jagent::cate_shift=6;break;
    case   8:jagent::cate_shift=5;break;
    case  16:jagent::cate_shift=4;break;
    case  32:jagent::cate_shift=3;break;
    case  64:jagent::cate_shift=2;break;
    case 128:jagent::cate_shift=1;break;
    case 256:jagent::cate_shift=0;break;
    default:
        jagent::cate_num= NumOfCate=256;
        jagent::cate_shift=0;
        cerr<<"Invalid number of class (not power of 2 less than 256). Using default.\n";
        Log.GetStream()<<"Invalid number of class (not power of 2). Using default.\n";
        break;
    }
}


void jworld::initialize_layers()
//-------------------------------------
{
    static int first=1; //DISABLING EXCESS PRINTOUTS WHEN INITIATING SIMULATION REPETITIONS.
    if(first)
        Log.GetStream()<<"LANGUAGE SIMULATION:"; //There are a lot of prints in the first initialization.

    jagent::max_pow=MaxStrength; // Sets the maximum agent strength
    jagent::min_pow=MinStrength; // Sets the minimum agent strength
    jagent::cate_num=NumOfCate; // Sets the number of categories

    switch(NumOfCate) //Sets the bit offset for reading categories
    {
    case   2:jagent::cate_shift=7;break;
    case   4:jagent::cate_shift=6;break;
    case   8:jagent::cate_shift=5;break;
    case  16:jagent::cate_shift=4;break;
    case  32:jagent::cate_shift=3;break;
    case  64:jagent::cate_shift=2;break;
    case 128:jagent::cate_shift=1;break;
    case 256:jagent::cate_shift=0;break;
    default:
        jagent::cate_num= NumOfCate=256;
        jagent::cate_shift=0;
        cerr<<"Invalid number of class (not power of 2 less than 256). Using default.\n";
        Log.GetStream()<<"Invalid number of class (not power of 2). Using default.\n";
        break;
    }        //jkania@borland.pl ???

    if(first)
        print_experiment_info(Log.GetStream(),Log.separator()); //Printout of simulation parameter values.

    //DETERMINING AGENT STATES FROM GRAPHIC FILES WHERE AVAILABLE:
    int from1=0,from2=0;
    char *pos=NULL;
    char *old=NULL;

    from1= Agents.init_from_bitmap(MappName.get_ptr_val(), &jagent::assignPow); //Initializing powers from the map of powers.

    if((pos=strchr(MaplName.get_ptr_val(),';'))==NULL) // If there are not three file names in MapLName, we treat it as one common RGB map.
    {
        from2= Agents.init_from_bitmap(MaplName.get_ptr_val(), &jagent::assign123); //Initializing attributes from one file
    }
    else //When we have three separate files...
    {
        *pos='\0';
        from2= Agents.init_from_bitmap(MaplName.get_ptr_val(), &jagent::assign1); //Initializing the first attribute from the first file

        old=pos+1;
        if(from2==1 && (pos=strchr(old,';'))!=NULL)
        {
            clog<<"1. layer initialized by data file "<<MaplName.get_ptr_val()<<'\n'<<flush;

            *pos='\0';
            from2= Agents.init_from_bitmap(old, &jagent::assign2); //Initializing the second attribute from the next file

            if(from2==1)
            {
                clog<<"2. layer initialized by data file "<<old<<'\n'<<flush;
                old=pos+1;
                if((pos=strchr(old,';'))!=NULL)
                                *pos='\0'; //In case there is a trailing ';' present
                from2= Agents.init_from_bitmap(old, &jagent::assign3); //Initializing the third attribute from the last file

                if(from2==1)
                    clog<<"3. layer initialized by data file "<<old<<'\n'<<flush;
            }
        }
    }

    // If the agents and language attributes are not initialized from files,
    // a temporary random initialization is performed by constructors or by cloning.
    if(from1!=1 && from2!=1)
        Agents.reallocate_all();

    // Using the mask for uninhabitable areas. Resets an agent if the mask shows black in that area.
    if(Agents.init_from_bitmap(MaskName.get_ptr_val(), &jagent::killBlack) == 1 )
        Agents.deallocate_not_OK();

    if(use_SW_links)
        _connect_far_links(SW_start_connect_percent); //I try to connect some percentage of distant links before starting

    first=0; //End of the first call, the next ones will not be so eloquent.
}


void jworld::simulate_one_step()
{
    _update_age(); //First, we age the agents.

    if(use_SW_links)
    {
        _connect_far_links(SW_reconnect_percent); //Trying to connect some percentage of far distant links.
    }

    switch(BiasMode){ //Depending on the bias mode, we use one of the auxiliary implementations.
    case NO_BIAS:	        _one_step_no_bias();       break;
    case SIMPLE_BIAS:	    _one_step_simple_bias();    break;
    case CONDITIONAL_BIAS:  _one_step_conditional_bias1();break;
    case SEQUENTIONAL_BIAS: _one_step_sequentional_bias0();break;
    case INVALID_BIAS_MODE:
    default:
        assert("This code should never be reached.\nPROBABLY INVALID BIAS MODE!"==0);
        break;}
}


void    jworld::_connect_far_links(double Percent)
{
    unsigned N=asserted<unsigned>((double(MyWidth)*double(MyWidth)*Percent)/100); ///< Number of attempts.
    unsigned Counter=0; ///< Successful attempts counter.
    for(unsigned i=0;i<N;i++)	//Makes N attempts - although sometimes it can hit double or even triple
    {
        unsigned const a=RANDOM(MyWidth); ///< Random coordinate `a`.
        unsigned const b=RANDOM(MyWidth); ///< Random coordinate `b`.

        if(Agents.filled(a, b)) //Note! Only for a non-empty cell!
        {
            const jagent& on=Agents.get(a, b);
            double r=on.Power/asserted<double>(jagent::max_pow * MyWidth / 2); ///< Distance to random target.
//			r=NeighRadius+r*DRAND(); // Usually radius is draw within a radius, but not less than `NeighRadius`.
            r= NeighRadius + r * DRAND() * DRAND() * DRAND(); // Randomisation of radius, which is now draw condensed, closer to the agent.
            double Angle=DRAND()*2*M_PI; ///< Drawing the angle is rather simple (flat distribution).
            int ta=asserted<int>(a+r*sin(Angle)); ///< Calculated target coordinate `a`.
            int tb=asserted<int>(b+r*cos(Angle)); ///< Calculated target coordinate `b`.

            //Adjustment to TORUS - always necessary!!!
            ta=(ta+MyWidth)%MyWidth;
            tb=(tb+MyWidth)%MyWidth;
                                                                                   assert(0<=ta && ta<MyWidth);
                                                                                   assert(0<=tb && tb<MyWidth);
            if((a!=ta || b!=tb) && Agents.filled(ta, tb)) //There is no point in connecting with yourself or with an empty field
            {
#if 0
                //OLD ALGORITHM WITHOUT RANDOMNESS:
                if(Agents.get(ta,tb).Power>=on.Power) //Voluntary submission to protection or sometimes alliance.
                {									  //Ignoring external interference, the connection strives to connect
                                                      //with the strongest agent within the available radius (r).

                    double power_of_protector=0;
                    if(FarLinks.get(a,b).a!=UINT_MAX)	//When there is a protector, we read its strength/power.
                        power_of_protector=Agents.get(FarLinks.get(a,b).a,FarLinks.get(a,b).b).Power;

                    if(Agents.get(ta,tb).Power+on.Power>power_of_protector)	//In alliance with the new, you must defeat the old protector
                    {
                        _connect_flink_to(a,b,ta,tb);
                        Counter++; //Victory!
                    }
                }
                else //Attempt to annex a area of the weaker one - you have to defeat the current protector
                {
                    double power_of_protector=0;
                    if(FarLinks.get(ta,tb).a!=UINT_MAX)	//When there is a protector, we read its strength/power.
                        power_of_protector=Agents.get(FarLinks.get(ta,tb).a,FarLinks.get(ta,tb).b).Power;

                    if(on.Power>power_of_protector+Agents.get(ta,tb).Power)	//It is necessary to defeat the alliance of the invaded party and its protector
                    {
                        _connect_flink_to(ta,tb,a,b); //link built the other way!
                        Counter++; //Victory!
                    }
                }
#else
                //SIMPLE NOISE ALGORITHM - REALIZED POWER CAN BE FROM 50 TO 150% OF NOMINAL:
                if(Agents.get(ta, tb).Power >= on.Power) //Voluntary submission to protection or sometimes alliance.
                {									  //Ignoring external interference, the connection strives to connect
                                                      //with the strongest agent within the available radius (r).
                    double power_of_protector=0;
                    if(FarLinks.get(a,b).a!=UINT_MAX)	//When there is a protector, we read its strength/power.
                        power_of_protector=Agents.get(FarLinks.get(a, b).a, FarLinks.get(a, b).b).Power;

                    //Taking into account randomness, in alliance with the new one, you have to defeat the old protector.
                    if((0.5+DRAND())*(Agents.get(ta, tb).Power + on.Power) > power_of_protector)
                    {
                        _connect_flink_to(a,b,ta,tb);
                        Counter++;
                    }
                }
                else //Attempting to attach a new vassal - you have to get rid of the current protector.
                {
                    double power_of_protector=0;
                    if(FarLinks.get(ta,tb).a!=UINT_MAX)	//When there is a protector, we read its strength/power.
                        power_of_protector=Agents.get(FarLinks.get(ta, tb).a, FarLinks.get(ta, tb).b).Power;

                    //Taking into account randomness, you have to defeat the alliance of the invaded party and its protector.
                    if((0.5+DRAND())*on.Power> power_of_protector + Agents.get(ta, tb).Power)
                    {
                        _connect_flink_to(ta,tb,a,b);
                        Counter++;
                    }
                }
#endif
            }
        }
    }

    //Double looping through all agents just for coloring!
    for(int a=0;a<N;a++) {
        for (int b = 0; b < N; b++)
            if (Agents.filled(a, b))    //Watch out for empty cells!
            {
                _far_link pom = FarLinks.get(a, b);
                if (pom.a != UINT_MAX)    //If there is any protector at all.
                {
                    unsigned long politOfProtector= Agents.get(pom.a, pom.b).Politics; ///< A political marker of the Protector.
                    Agents.get(a, b).Politics = politOfProtector; // An agent adopts the political marker of his protector.
                }
            }
    }

    //Direct statistics are calculated here - real SW dynamics:
    if(N>0)
        SW_dynamic_perc=Counter/double(N);
    else
        SW_dynamic_perc=0;

}

unsigned jworld::_far_link::get_target_count()
{
    if(/*0<=a &&*/ a<MyWorld->MyWidth
        && /*0<=b &&*/ b<MyWorld->MyWidth)
        return MyWorld->FarLinks.get(a,b).count; // Reads `count` from [a,b] location on far links layer.
    else
        return 0;
}


void jworld::dump_net_file(const char* core_name,unsigned long Step)
{
    unsigned N=(double(MyWidth)*double(MyWidth)); ///< Number of cells.
    wb_pchar Name(512);
    Name.prn("%s%06d.net",core_name,Step);
    ofstream Out(Name.get(),ios::out);
    if(Out)
    {
        unsigned wer,col;

        Out<<N<<endl; //How many cells should I expect?

        // Recording agent forces at individual positions.
        for(wer=0;wer<MyWidth;wer++)	//Loop through the lines/rows.
         for(col=0;col<MyWidth;col++)	//Loop through columns in a given row.
         {
            Out << col << '\t' << wer << '\t' << Agents.get(col, wer).Power << "  " << col << 'x' << wer << endl;
         }

        // Saving information about distant links of individual items.
        for(wer=0;wer<MyWidth;wer++)	//Loop through the lines/rows.
         for(col=0;col<MyWidth;col++)	//Loop through columns in a given row.
         {
            _far_link& lnk=FarLinks.get(col,wer);
            if(/*0<=lnk.a &&*/ lnk.a<MyWidth
            && /*0<=lnk.b &&*/ lnk.b<MyWidth)
                Out<<(wer*MyWidth)+col<<'\t'<<(lnk.b*MyWidth)+lnk.a<<'\t'<<1<<endl;
         }
    }
    Out.close();
}

void    jworld::_update_age()
{
    const geometry_base* MyGeom=Agents.get_geometry();
    iterator_h Iter=MyGeom->make_global_iterator();
    while(Iter)
    {
        size_t index=MyGeom->get_next(Iter);	//We obtain the agent's index.
        assert(index!=MyGeom->FULL); //It shouldn't happen here, but...
        jagent& CenterAgent=*(Agents.get_ptr(index).get_ptr_val());	//We obtain references to the agent by bypassing NULL assertion.
        if(Agents.is_empty(CenterAgent))    //We check whether it is not an empty cell (NULL)
            continue;                   //Because then doing anything further would be pointless.

        CenterAgent.Age++;
    }
}

// Static fields require a separate definition in some cpp file. But why did I omit initialization?
jworld* jworld::_far_link::MyWorld; //=NULL;

#pragma clang diagnostic pop
/* **************************************************************** */
/*           THIS CODE IS DESIGNED & COPYRIGHT BY:                  */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zakład Systematyki i Geografii Roślin Uniwersytetu Warszawskiego */
/*  & Instytut Studiów Społecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */

