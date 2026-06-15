#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-assert-side-effect"
/// @file
/// @brief MAIN SOURCE FILE OF LANGUAGES PROJECT WITH P.Culicover.
/// @date 2026-06-15 (modified)
///
///     THIS PROGRAM IS DESIGNED FOR CFCS OF ISS University of Warsaw!
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "cert-err34-c"

#include "compatyb.h"
const char* SCREEN_DUMP_NAME="LANGUAGES";
const char* SIMULATION_NAME= "LANGUAGES version SW 2.20d, compilation " __DATE__ ", " __TIME__ ;
/// @details
/// Simulation of the linguistic behaviors spread using the multi-layered meme/belief transmission method.
///	Polish word "J Ę Z Y K" means "LANGUAGE". Hence, the "j" prefix appearing here and there.
/// #HISTORY:
//  ////////////////////////
///			ver.  2.20c - English-language comments.
///			ver.  2.20b - ADAPTATION TO NEW COMPILATION CONDITIONS IN clion IN 2026.
///
///			...[a dozen or so years apart]...
///
///			ver.  2.20 - Introduction of 16 classes to Log-Log histogram added in version 1.401.
///					   - Changing the order of series/columns in the log file.
///					   - Console mode of operation introduced (in the background, without graphics completely).
///			ver.  2.11 - Beginnings of using `OptionalParameters` classes to handle call parameters (later abandoned).
///			ver.  2.10 - Implementation of the process of imposing power and displaying the political map.
///			ver.  2.06 - Display rate control now working and `my_area_manager` declared.
///			ver.  2.05 - Expanded menus, especially new visualization options. Also, `SRND` and `DUMP` parameters.
///			ver.  2.04 - Continuous SW network dump as command line parameter.
///			ver.  2.03a - Implementation of SW network dump in the form of NET files.
///			ver.  2.01-2 - Fully working model in the Small Worlds version.
///			ver.  1.99b - Transitioning to using the Small Worlds model:
///							- Preparing a new visualization layout.
///							- Preparing data sources and graph for long-distance connections.
///							- Implementable algorithm for dynamic long-distance "political" connections.
///							- Incorporating influence from distant connections into the implementation of influence models
///								(not tested for complex biases).
///
///			ver.  1.53a - Minor changes necessary to run the build on BDS 2006.
///
///			ver.  1.51-2 - Minor cosmetic changes.
///
///			version 1.5 - Added language map in "TrueColor", and component maps in RGB component colors.
///						- Changing default startup parameters.
///
///			version 1.41a
///						- Adding the language age of a given agent and the data series representing it on a logarithmic graph.
///
///			version 1.402a
///						- Introduction of 12 classes to Log-Log histogram added in version 1.401.
///
///			version 1.401b
///						- Changing the Log-Log histogram of the language sizes to a 6-class fix histogram,
///						  i.e., a very similar visual effect, but with differently labeled classes.
///						- Added fix histogram for language size classes.
///
///			version 1.4
/// 					- Adding spontaneous mutations in all bias modes.
///						- Introducing the ability to disable spatial correlation calculation (use_spatial_corr) from CODE!!!
///						- The log-log plot of the language sizes distribution introduced (type "dhistosou.h").
///						- Introduction of writing this histogram to the log.
///						- Supplementing some messages about parameter settings (but some were left without).
///						- Entering the `DSTB` parameter that determines the type and degree of force distribution (obtained by * or +).
///			TEST:
///			```
///			.........WIDTH=100 DSTB=-8 CLSS=8 MIPO=3 RSPC=1 VIEW=50 LOGF=10 LOGF=testW100.log
///			```
///
///			version 1.35a - ???
///			version 1.34a - Remove assertions against zero-strength agents and introduce a minimal strength limit (def. = 1).
///			version 1.33a - Recompiling to a new library and adding a menu for MSWindows.
///			version 1.32a - Recompiling with a new version of the visualization library and introducing
///						    the "about_languages.cpp" file with time control for DEBUG mode.
///			version 1.31a - Implemented initialization of language attributes from three grayscale image files.
///			version 1.30a - Double bias introduced in the follow-up version, but a parallel bias version still works poorly
///						    requires intelligent counter rebalancing.
///			version 1.20a - Preparation for the introduction of "double bias" (parallel).
///			version 1.01b - Improved default for a strength threshold.
///			version 1.05b - Built-in support for batch work and experiment repetition.
///			version 1.10b - Introducing "bias" for language parameters.
const char* Authors="(programed by W. Borkowski for ISS UW & Ohio State Univ.)";

int My_Rand_seed=0; ///< Random generator initializer. If 0 it uses RANDOMIZE, if other it uses SRAND(My_Rand_seed).

#include <iostream>
#include <cstdlib>
#include <ctime>

#define HIDE_WB_PTR_IO 0
#include "wb_ptr.hpp"
//#include "optParam.hpp"

#include "jrand.h"
#include "jworld.h"
#include "lang_res.h"

using namespace std;
using namespace sym2;
using namespace sym2::data;
using namespace sym2::shell;
using namespace sym2::visual;

/// @name General application control flags.
/// @{
bool		Console=false;		///< Flag for working in console mode - no graphics.
bool		Replay=false;		///< Ability to play back a saved simulation (probably not implemented)
bool		DUMP_NET=false;		///< Flag specifying whether to dump network files. (SW?)
bool		AUTO_END=false;		///< Specifies whether to automatically exit the program after all simulations have finished.
bool		AUTOSTART=false;	///< Specifies whether to automatically start the simulation.
/// @}

/// @name Dimensions of the usable window/screen space.
/// @{
unsigned	SCR_WIDTH=asserted<unsigned>(1440 * 0.6666);	///< Screen/window inside width. (720 or 1440);
unsigned	SCR_HEIGHT=asserted<unsigned>(1080 * 0.6666);	///< Screen/window inside height. (540 or 1080);
/// @}

/// @name No Object-wise passed to the source initialization method or used to configure general loops:
/// @{
unsigned	internal_log=10000;			///< Default length of internal logs (stats history data sources).
bool		use_spatial_corr=false;			///< Flag for using spatial correlation (expensive to compute).
int			spatial_correlation_mode=50;	///< Number of sampling runs in calculating spatial correlation.
unsigned	sim_to_log_ratio=10;			///< Defines the number of simulation steps after which statistics are counted and written to the log.
unsigned	sim_to_view_ratio=1;			///< Determines after how many simulation steps the visualization is performed.
/// @}

/// @name Simulation parameters.
/// @details Object-oriented passed to the constructor of world object:
/// @{
char	LogFName[512]="languagesSW2.log\0-------------------+--";		///< Statistics log file name.
char	NetCName[512]="languagesSW_\0---------------------+--";			///< The core of the network dumps filenames.
char	HistName[512]="\0--+---------languagesSW2.otx----------";		///< Simulation history file name (probably not working).
char	MapLName[512]="\0--+---------languagesSW2.gif----------";		///< A graphic file initializing states layers.
char	MapPName[512]="\0--+---------powersSW2.gif------------";		///< A graphic file initializing the force layer.
char	MaskName[512]="\0--+---------maskSW2.gif--------------";		///< A graphic file initializing areas suitable for settlement.

unsigned	WorldWidth=100;				///< Side length of the simulation world.
unsigned	MaxIterations=0xffffffff;	///< Maximum number of simulation steps.
short		NofCategories=128;			///< Number of linguistic/cultural categories in each of the three layers.

int		DistributionLevel=6;	///< Type and degree of strength distribution.
int		GrowingStrength=0;		///< Determines whether strength should increase "with age".
int		MaximumStrength=10000;	///< Determines what the agent's greatest strength can be.
int		MinimumStrength=10;		///< Determines what the minimum force can be.
                                ///< If the min and max are the same, then the value is the same everywhere.
int		ThreshPercent=101;		///< Above what certain strength, changes in "attributes" become impossible.

short	NeighborhoodR=1;		///< Neighborhood radius.
short	NeighborhoodD=8;		///< How many agents within the radius are randomly selected (can they be duplicated?).
short	ConsiderSelf=1;			///< Take yourself into account.
const char*	BIAS_STR=""; //"A100:10";		///< Bias definition collected directly from the parameter line.

double	NoisePercent=0;			///< Percentage of noise when collecting information about influence.
double	MutationProb=0;			///< Probability of spontaneous state change.
double	SW_start_perc=0;		///< Controlling the process of the world political hierarchization at the beginning of the simulation.
double	SW_step_perc=10;			///< Controlling the process of the world political hierarchization at each step of the simulation.
bool	SW_links=true;			///< Determines whether long links are used.
/// @}

/*
// Not everything can be done this way.
// Parameter consistency testing in particular cannot.

--> int parse_options(const int argc,const char* argv[]);	//Announcement!

OptionalParameterBase* Parameters[]={ //sizeof(Parameters)/sizeof(Parameters[])
new ParameterLabel("PARAMETERS FOR SINGLE SIMULATION"),
//new OptionalParameter<TYPE>(VARIABLE,LOWLIM,HIGHLIM,"PAR_NAME","help info"),
new OptionalParameter<double>(MutationProb,0,1,"SPCH"," Ratio of spontaneous changes"),
new OptionalParameter<double>(NoisePercent,0,100,"NOIP","Noise in decision (in %)"),
new OptionalParameter<long>(NofCategories,2,256,"CLSS","Number of classes in each mem"),
new OptionalParameter<long>( MinimumStrength,0,100000,"MIPO","Minimal strength"),
new OptionalParameter<long>(MaximumStrength,1,1000000,"MPOW","Max strength"),
new OptionalParameter<long>(GrowingStrength,0,1000000,"WPOW","Moving strength"),
new OptionalParameter<long>(SW_start_perc,0,100,"SWBE","SW links will be used at start"),
new OptionalParameter<long>(SW_step_perc,0,50,"SWST","SW links will be add every step"),
new OptionalCheckFunction(SWLinksCheck(),"if(SWBE>0 || SWST>0) SW_links=true;"),
new OptionalParameter<bool>(DUMP_NET,false,true,"NETD","dump network every step"),
new OptionalParameter<long>(ThreshPercent,0,100,"TRSP","Immunisation strength threshold"),
new OptionalCheckFunction(MovingSghCheck(),"if(TRSP>0 && GrowingStrength==0) GrowingStrength=1;");
new OptionalParameter<long>(SCR_WIDTH,50,4096,"WIDTHWIN","Window width"),
new OptionalParameter<long>(SCR_HEIGHT,50,4096,"HEIGHTWIN","Window height"),
new OptionalParameter<long>(WorldWidth,3,SCR_WIDTH,"WIDTH","World width. Can't be larger than window/screen!"),
new OptionalParameter<long>(MaxIterations,0,0x7FFFFFFF,"MAXI","Maximal number of steps"),
new OptionalParameter<long>(MaxIterations,0,0x7FFFFFFF,"MAX",NULL),	//hidden alias
new OptionalParameter<long>(sim_to_log_ratio,1,MaxIterations,"LOGC","Writing to log frequency (not higher than MAXI)".),
new OptionalParameter<long>(sim_to_view_ratio,1,MaxIterations,"VIEW","visualization frequency"),
new OptionalParameter<long>(My_Rand_seed,0,0x7FFFFFFF,"SRND","SRND (rand seed). Must be >=0, but 0 means current time in seconds!"),
new OptionalParameter<bool>(jworld::continuous_dump,false,true,"DUMP","dumping the screen at every step"),
new OptionalParameter<bool>(ConsiderSelf,false,true,"SELF","use own state for calculation the new one or not"),
new OptionalParameter<TYPE>(NeighborhoodR,1,WorldWidth,"INDI","Radius of neighborhood"),
new OptionalCheckFunction(NeighRadCheck(),"if(INDI>(WIDTH/2-1)) INDI=WIDTH/2-1;");
new OptionalParameter<long>(NeighborhoodD,2,sqr(NeighborhoodR*2+1)-1,"PRTR","How many contacts chosen from neighbors"),
new OptionalParameter<bool>(AUTOSTART,false,true,"AUTO","Automatically start the simulation"),
new OptionalParameter<bool>(AUTO_END,false,true,"STOP","Automatic exit when done"),
new OptionalParameter<long>(internal_log,50,MaxIterations,"ILOG","yyy"),
new OptionalParameter<int>(DistributionLevel,-100,100,"DSTB","distribution Level/Kind"),
new OptionalParameter<wb_pchar>(LogFName,"*.log","*.txt","LOGF","name for log file"),
new OptionalParameter<wb_pchar>(MapLName,"*.jpg","*.gif","MAPL","Map of languages from file"),
new OptionalParameter<wb_pchar>(MapPName,"*.jpg","*.gif","MAPP","Map of individual power from file"),
new OptionalParameter<wb_pchar>(MaskName,"*.jpg","*.gif","MASK","Mask for alive agents from file"),
new OptionalParameter<wb_pchar>(HistName,"*.hist","*.txt","HIST","name of simulation history file"),
//new OptionalParameter<TYPE>(HistName,"*.hist","*.txt","REPL","yyy"),
new OptionalParameter<wb_pchar>(SpatialCorrMode,"N/Y/+/-","1..WIDTH","RSPC","mode of spatial correlation"),
new OptionalCheckFunction(SpatialCorrCheck(),"Parsing spatial correlation mode string"),
//new OptionalParameter<TYPE>(,,,"xxx","yyy"),
new OptionalParameter<wb_pchar>(BIAS_STR,"100","a:100;b:200","BIAS","Syntax of BIAS definition need help reading!"),
new ParameterLabel("END OF LIST")
};   */
/*
cerr<<"YOU CAN USE:\n";
        cerr<<"\tREPL=hist.otx - not simulate but replay simulation history file.\n";
        cerr<<"\tMAPL=initL.gif (or BMP)- file with initialization map of languages or \n";
        cerr<<"\t    =\"init1.gif;init2.gif;init3.gif\" (or BMP)- 3. separate init file (RANDOM)\n";
        cerr<<"\tMAPP=initP.gif (or BMP)- file with initialization map of powers (RANDOM)\n";
        cerr<<"\tMASK=mask.gif	(or BMP)- mask file for alive (not black) agents (ALL ALIVE)\n";
        cerr<<"\tWIDTH=NN - matrix size ("<<WorldWidth<<")\n";
        cerr<<"\tSRND=NNNN - random seed if you want particular one\n";
        cerr<<"\tBIAS=item item ... - setting bias by items string (NO BIAS)\n"<<
              "\t\the item string example: \"A1:1 A2:4 B2:2 C2:4 A4&C3:10 A5&B5&C5:12\"\n";
        cerr<<"\tCLSS=NN - number of class. Must be power of 2. ("<<NofCategories<<")\n";
        cerr<<"\tMPOW=NN - max strength for initialization ("<<MaximumStrength<<")\n"	;
        cerr<<"\tMIPO=NN - min strength for initilization ("<<MinimumStrength<<")\n"	;
        cerr<<"\tDSTB=N - level and kind of strength distribution ("<<DistributionLevel<<")\n";
        cerr<<"\nSWST=PP/PP - percent of SW links created at every step, and at the beginning (0)\n";
        cerr<<"\nNETD=N/Y - parallelly dumping net files for statistics (N)\n";
//      cerr<<"\tWPOW=N	- walking step of strength	("<<GrowingStrength<<")\n";
        cerr<<"\tTRSP=N - % of threshold of strength ("<<ThreshPercent<<")\n";
        cerr<<"\tPRTR=2..WIDTH^2-1 - number of interaction partners ("<<NeighborhoodD<<")\n";
        cerr<<"\tINDI=1..WIDTH/2-1 - interaction distance ("<<NeighborhoodR<<")\n";
        cerr<<"\tSELF=N/Y -use self for calculations ("<<(ConsiderSelf?"Yes":"No")<<")\n";
        cerr<<"\tNOIP=NN - percent of noise ("<<NoisePercent<<")\n";
        cerr<<"\tSPCH=NN - percent of spontaneous change of attitudes ("<<MutationProb*100<<")\n";
        cerr<<"\tMAX=NNNN - max simulation step ("<<MaxIterations<<")\n";
        cerr<<"\tILOG=NNNN - length of internal statistic logs  ("<<internal_log<<")\n";
        cerr<<"\tSTOP=N/Y - exit after MAX steps ("<<(AUTO_END?"Yes":"No")<<")\n";
        cerr<<"\tVIEV=NNN - visualisation frequency ("<<sim_to_view_ratio<<")\n";
        cerr<<"\tDUMP=Y/N - dump, or not, screen at every simulation step\n";
        cerr<<"\tRSPC=N/Y/+/- or 1..WIDTH - Random calculation of spatial correlation ("<<(spatial_correlation_mode?"N":"Y")<<")\n";
        cerr<<"\tLOGC=N - log file saving frequency ("<<sim_to_log_ratio<<")\n";
        cerr<<"\tLOGF=name.log - file for simulation log ("<<LogFName<<")\n";
        cerr<<"\tHIST=hist.otx - file for full history of simulation.\n";
        cerr<<"\tWIDTHWIN,HEIGHTWIN=XXX - initial window size.("<<SCR_WIDTH<<'x'<<SCR_HEIGHT<<"\n";
        cerr<<"\nAUTO=XXX - number of auto-repetition of simulation.("<<AUTOSTART<<")\n";
*/

/// @brief Reading application parameters from the command line.
/// @details Example of parameters list: `WIDTH=100 DSTB=-8 CLSS=8 MIPO=3 RSPC=1 VIEW=50 LOGF=10 LOGF=testW100.log`
int parse_options(const int argc,const char* argv[])
{
    for(int i=1;i<argc;i++)
    {
    if( *argv[i]=='-' ) /* Option for X11 or symshell */
        continue;

    //Make modifiable:
    wb_pchar hand(clone_str(argv[i]));
    char*    rob=hand.get_ptr_val();

    //Uppercasing
    char* pom=strchr(rob,'=');
    if(pom==NULL) // NOLINT(*-use-nullptr)
            goto ERROR; //DEFINITELY BAD

    *pom='\0';strupr(rob);*pom='=';	//Part to the = sign

    if(strcmp(rob,"HELP")==0)
    {
        goto HELP_PRINT;
    }
    else
    if((pom=strstr(rob,"SPCH="))!=NULL) //If not NULL, then exists
    {
        MutationProb=atof(pom + 5);
    if(MutationProb <= 0 || MutationProb > 100)
        {
        cerr << "!!! Bad SPCH =" << MutationProb << " (must be in <0,100>)" << endl;
        return 0;
        }
    cerr << "* Spontaneous change ratio in %: SPCH= " << MutationProb << endl;
        MutationProb/=100;	//A fraction, not a percentage, really.
    }
    else
    if((pom=strstr(rob,"NOIP="))!=NULL) //If not NULL, then exists
    {
        NoisePercent=atof(pom + 5);
    if(NoisePercent <= 0 || NoisePercent > 100)
        {
            cerr << "!!! Bad NOIP =" << NoisePercent << " (must be in <0,100>)" << endl;
            return 0;
        }
    else
        {
            cerr << "* Noise in decision in %: NOIP= " << NoisePercent << "%" << endl;
        }
    }
    else
    if((pom=strstr(rob,"CLSS="))!=NULL) //If not NULL, then exists
    {
        NofCategories=asserted<short>(atol(pom + 5));
        if(NofCategories < 2)
            {
            cerr << "!!! Bad CLSS =" << NofCategories << " (must be greater than 2)" << endl;
            return 0;
            }
        if(NofCategories > 256)
            {
            cerr << "!!! Bad CLSS =" << NofCategories << " (must be less or equal to 8)" << endl;
            return 0;
            }
        cerr << "* Number of classes in each mem: CLSS= " << NofCategories << endl;
    }
    else
    if((pom=strstr(rob,"MIPO="))!=NULL)  //If not NULL, then exists
    {
        MinimumStrength=asserted<int>(atol(pom + 5));
        if(MinimumStrength < 0)	//0 czy 1???
            {
            cerr << "!!! Bad MIPO =" << MinimumStrength << " (must be >=1 )" << endl;
            return 0;
            }
        cerr << "* Minimal strength: MIPO= " << MinimumStrength << endl;
    }
    else
    if((pom=strstr(rob,"MPOW="))!=NULL) //If not NULL, then exists
    {
        MaximumStrength=asserted<int>(atol(pom + 5));
        if(MaximumStrength < 0)	//0 or 1???
            {
            cerr << "!!! Bad MPOW =" << MaximumStrength << " (must be >=1 )" << endl;
            return 0;
            }
        cerr << "* Max strenght: MPOW= " << MaximumStrength << endl;
    }
    else
    if((pom=strstr(rob,"WPOW="))!=NULL) //If not NULL, then exists
    {
        GrowingStrength=asserted<int>(atol(pom + 5));
        if(GrowingStrength < 0)
            {
            cerr << "!!! Bad WPOW =" << GrowingStrength << " (must be >=0 )" << endl;
            return 0;
            }
        cerr << "* Moving strength: WPOW= " << GrowingStrength << endl;
    }
    else
    if((pom=strstr(rob,"SWST="))!=NULL) //If not NULL, then exists
    {
       wb_pchar pom2(pom+5);
       char* pom3=strchr(pom2.get_ptr_val(),'/');
       if(pom3)
       {
            *(pom3)='\0';
            SW_start_perc=asserted<double>(atol(pom3+1));
       }
       SW_step_perc=asserted<double>(atol(pom2.get()));
       cerr << "* SW links will be used. "<<SW_start_perc<<"% at start, and "
            << SW_step_perc<<"% at every step"<<endl;
       SW_links=true;
    }
    else
    if((pom=strstr(rob,"NETD="))!=NULL) //If not NULL, then exists
    {
        DUMP_NET=(toupper(pom[5]) == 'Y');
        cerr << "* NETD=" << (DUMP_NET?"Yes":"No") << endl;
    }
    else
    if((pom=strstr(rob,"TRSP="))!=NULL) //If not NULL, then exists
    {
        ThreshPercent=asserted<int>(atol(pom + 5));
        if(ThreshPercent < 0 || ThreshPercent > 100)
            {
            cerr << "!!! Bad TRSP = " << ThreshPercent << "(must be in <0,100>" << endl;
            return 0;
            }
        else
            {
            cerr << "* Immunisation strength threshold : TRSP= " << ThreshPercent << "%" << endl;
            if(GrowingStrength == 0)	//There is no point in TRSP if there is no mobile force
                {
                    GrowingStrength=1;
                cerr << "** Automatically set WPOW to " << GrowingStrength << endl;
                }
            }
    }
    else
    if((pom=strstr(rob,"WIDTH="))!=NULL) //If not NULL, then exists
    {
        WorldWidth=atol(pom + 6);
        if(WorldWidth < 3)
            {
            cerr << "!!! Bad WIDTH = " << WorldWidth << "(must be in <3," << SCR_WIDTH << ">" << endl;
            return 0;
            }
        if(WorldWidth >= SCR_WIDTH)
            cerr << "WIDTH (" << WorldWidth << ") is really high!\n"
                 << "Simulation world may be larger than the window or even the screen." << endl;
        cerr << "* World width: WIDTH= " << WorldWidth << 'x' << WorldWidth << endl;
    }
    else
    if((pom=strstr(rob,"WIDTHWIN="))!=NULL) //If not NULL, then exists
    {
        SCR_WIDTH=atol(pom + 9);
        if(SCR_WIDTH < 50)
            {
            cerr << "!!! Bad WIDTHWIN = " << SCR_WIDTH << " (must be >50)" << endl;
            return 0;
            }
    }
    else
    if((pom=strstr(rob,"HEIGHTWIN="))!=NULL) //If not NULL, then exists
    {
        SCR_HEIGHT=atol(pom + 10);
        if(SCR_HEIGHT < 50)
            {
            cerr << "!!! Bad HEIGHTWIN = " << SCR_HEIGHT << " (must be >50)" << endl;
            return 0;
            }
    }
    else
    if((pom=strstr(rob,"MAX="))!=NULL) //If not NULL, then exists
    {
        MaxIterations=atol(pom + 4);
        if(MaxIterations <= 0)
            {
            cerr<<"!!! Bad MAX iterations. Must be >0"<<endl;
            return 0;
            }
        cerr << "Maximal number of steps: MAX= " << MaxIterations << endl;
    }
    else
    if((pom=strstr(rob,"LOGC="))!=NULL)  //If not NULL, then exists
    {
        sim_to_log_ratio=atol(pom + 5);
        if(sim_to_log_ratio <= 0)
            {
            cerr<<"!!! Bad LOGC (write to log frequency). Must be >0"<<endl;
            return 0;
            }
    }
    else
    if((pom=strstr(rob,"VIEW="))!=NULL) //If not NULL, then exists
    {
        sim_to_view_ratio=atol(pom + 5);
        if(sim_to_view_ratio <= 0)
            {
            cerr<<"!!! Bad VIEW (visualization frequency). Must be >0"<<endl;
            return 0;
            }
    }
    else
    if((pom=strstr(rob,"SRND="))!=NULL)  //If not NULL, then exists
    {
        My_Rand_seed=asserted<int>(atol(pom+5));
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
    if((pom=strstr(rob,"DUMP="))!=NULL)  //If not NULL, then exists
    {
        jworld::continuous_dump=(toupper(pom[5]) == 'Y');
        cerr << "DUMP=" << (jworld::continuous_dump?"Yes":"No") << endl;
    }
    //continuous dump
    else
    if((pom=strstr(rob,"SELF="))!=NULL)  //If not NULL, then exists
    {
        ConsiderSelf=(toupper(pom[5]) == 'Y');
        cerr << "SELF=" << (ConsiderSelf?"Yes":"No") << endl;
    }
    else
    if((pom=strstr(rob,"INDI="))!=NULL)  //If not NULL, then exists
    {
        NeighborhoodR=asserted<short>(atol(pom + 5));
        if(NeighborhoodR >= 1U &&
           NeighborhoodR < (WorldWidth / 2 - 1))
            {
            cerr << "* Radius of a neighborhood: INDI=" << NeighborhoodR << endl;
            }
        else
            {
            cerr << "!!! Bad INDI=" << NeighborhoodR << " Must from 1 to " << WorldWidth / 2 - 1 << endl;
            return 0;
            }
    }
    else
    if((pom=strstr(rob,"PRTR="))!=NULL)  //If not NULL, then exists
    {
        NeighborhoodD=asserted<short>(atol(pom + 5));
        if(NeighborhoodD > 1 && NeighborhoodD <= sqr(NeighborhoodR * 2 + 1) - 1)
            {
            cerr << "* How many real neighbors: PRTR=" << NeighborhoodD << endl;
            }
        else
            {
            cerr << "!!! Bad PRTR=" << NeighborhoodD
                 << "! Must from 2 to " << sqr(NeighborhoodR * 2 + 1) - 1 << endl;
            return 0;
            }
    }
    else
    if((pom=strstr(rob,"AUTO="))!=NULL)  //If not NULL, then exists
    {
        AUTOSTART=asserted<int>(atol(pom+5));
        cerr<<"* AUTO="<<AUTOSTART<<endl;
        if(AUTOSTART)
            {
                AUTO_END=true;
                cerr<<"** STOP=Yes"<<endl;
            }
    }
    else 	//Console
    if((pom=strstr(rob,"CONS="))!=NULL)  //If not NULL, then exists
    {
        Console=(atol(pom+5)!=0) || pom[5]=='Y' || pom[5]=='y' || pom[5]=='T' || pom[5]=='t';
        cerr<<"* CONSole="<<Console<<"!!!"<<endl;
        if(Console)
            {
            cerr<<"No graphics window will appear, so you can also use 'nohup' unix command for this program"<<endl;
            AUTO_END=true;
            AUTOSTART=true;
            cerr<<"** AUTO="<<AUTOSTART<<endl;
            cerr<<"** STOP=Yes"<<endl;
            }
    }
    else
    if((/*pom=*/strstr(rob,"BIAS="))!=NULL)  //If not NULL, then exists
    {
        BIAS_STR=rob+5;
        cerr<<"* BIAS = "<<BIAS_STR<<endl;
        static wb_pchar taker;
        taker=hand.give();	//Takes away the management. Dismissal at the end of the program.
    }
    else
    if((pom=strstr(rob,"STOP="))!=NULL)  //If not NULL, then exists
    {
        AUTO_END=(toupper(pom[5]) == 'Y');
        cerr << "* Automatic exit when done: STOP=" << (AUTO_END?"Yes":"No") << endl;
    }
    else
    if((pom=strstr(rob,"ILOG="))!=NULL)  //If not NULL, then exists
    {
        internal_log=atol(pom+5);
        if(internal_log<50)
                {
                internal_log=50;
                cerr<<"!!! An internal log length to small. Reset to a default minimum ="<<internal_log<<endl;
                }
    }
    //cerr<<"\tDSTB=N - level and kind of strength distribution ("<<DistributionLevel<<")\n";
    else
    if((pom=strstr(rob,"DSTB="))!=NULL)  //If not NULL, then exists
    {
        DistributionLevel=atoi(pom+5);
        if(DistributionLevel==0 || abs(DistributionLevel)>100)
                {
                cerr<<"!!! Invalid value of distribution Level/Kind ="<<DistributionLevel<<endl;
                }
        else cerr<<"* distribution Level/Kind DSTB="<<DistributionLevel<<endl;
    }
    else
    if((pom=strstr(rob,"LOGF="))!=NULL)  //If not NULL, then exists
    {
        strcpy(LogFName, pom + 5);
        cerr<<"* Statistics log will be in file \""<<LogFName<<"\"\n";
    }else
    if((pom=strstr(rob,"MAPL="))!=NULL)  //If not NULL, then exists
    {
        strcpy(MapLName,pom+5);
        cerr<<"* Map of languages from file \""<<MapLName<<"\"\n";
    }
    else
    if((pom=strstr(rob,"MAPP="))!=NULL)  //If not NULL, then exists
    {
        strcpy(MapPName,pom+5);
        cerr<<"* Map of individual power from file \""<<MapPName<<"\"\n";
    }
    else
    if((pom=strstr(rob,"MASK="))!=NULL)  //If not NULL, then exists
    {
        strcpy(MaskName,pom+5);
        cerr<<"* Mask for living agents from file \""<<MaskName<<"\"\n";
    }
    else
    if((pom=strstr(rob,"HIST="))!=NULL)  //If not NULL, then exists
    {
        strcpy(HistName,pom+5);
        cerr<<"* History of the simulation will be saved to \""<<HistName<<"\"\n";
    }
    else
    if((pom=strstr(rob,"REPL="))!=NULL) //If not NULL, then exists
    {
        strcpy(HistName,pom+5);
        Replay=true;
        cerr<<"* The simulation will be replayed from \""<<HistName<<"\"\n";
    }
    else
    if((pom=strstr(rob,"RSPC="))!=NULL)
    {
        if(use_spatial_corr)
        {
        const char* loc_hlp= pom + 5;
        if(toupper(*loc_hlp) == 'N')
            spatial_correlation_mode=0;
        else
        if(toupper(*loc_hlp) == 'Y')
            spatial_correlation_mode=16;
        else
            spatial_correlation_mode=atoi(loc_hlp);
         cerr<<"* Random calculation of spatial correlation is "
             <<(spatial_correlation_mode==0?"d i s a b l e d":"e n a b l e d")
             <<". Multiplication="<<spatial_correlation_mode<<"\n";
        }
        else
        {
            if(*(pom+5)=='+')
            {
                cerr<<"!!!! Spatial correlations are enabled. RSPC will be applied."<<endl;
                use_spatial_corr=true;
            }
            else
            if(*(pom+5)=='-')
            {
                cerr<<"!!!! Spatial correlations were disabled."<<endl;
                use_spatial_corr=true;
            }
            else
            cerr<<"!!! Sorry, but spatial correlation is disabled, RSPC was ignored."<<endl;
        }
    }
    else
    if((/*pom=*/strstr(rob,"HELP"))!=NULL) //Nie NULL, czyli jest
    {
HELP_PRINT:
        cerr<<"Unknown parameter \""<<argv[i]<<"\"\n";
        cerr<<"YOU CAN USE:\n";
        cerr<<"\tREPL=hist.otx - not simulate, but replay simulation history file.\n";
        cerr<<"\tMAPL=initL.gif (or BMP)- file with an initialization map of languages or \n";
        cerr<<"\t    =\"init1.gif;init2.gif;init3.gif\" (or BMP)- 3. separate init file (RANDOM)\n";
        cerr<<"\tMAPP=initP.gif (or BMP)- file with an initialization map of strengths (RANDOM)\n";
        cerr<<"\tMASK=mask.gif	(or BMP)- mask file for alive (not black) agents (ALL ALIVE)\n";
        cerr << "\tWIDTH=NN - matrix size (" << WorldWidth << ")\n";
        cerr<<"\tSRND=NNNN - random seed if you want particular one\n";
        cerr<<"\tBIAS=\"item item\" - setting bias using item string (NO BIAS)\n"<<
              "\t\the item string example: \"A1:1 A2:4 B2:2 C2:4 A4&C3:10 A5&B5&C5:12\"\n";
        cerr << "\tCLSS=NN - number of class. Must be power of 2. (" << NofCategories << ")\n";
        cerr << "\tMPOW=NN - max strength for initialization (" << MaximumStrength << ")\n"	;
        cerr << "\tMIPO=NN - min strength for initialization (" << MinimumStrength << ")\n"	;
        cerr<<"\tDSTB=N - level and kind of strength distribution ("<<DistributionLevel<<")\n";
        cerr<<"\nSWST=PP/PP - percentage of SW links created at every step, and at the beginning (0)\n";
        cerr<<"\nNETD=N/Y - dumping net files parallel to statistics (N)\n";
        cerr<<"\tWPOW=N	- walking step of strength	("<<GrowingStrength<<")\n";
        cerr << "\tTRSP=N - % of threshold of strength (" << ThreshPercent << ")\n";
        cerr << "\tPRTR=2...WIDTH^2-1 - number of interaction partners (" << NeighborhoodD << ")\n";
        cerr << "\tINDI=1...WIDTH/2-1 - interaction distance (" << NeighborhoodR << ")\n";
        cerr << "\tSELF=N/Y - use self for calculations (" << (ConsiderSelf?"Yes":"No") << ")\n";
        cerr << "\tNOIP=NN - percent of noise (" << NoisePercent << ")\n";
        cerr << "\tSPCH=NN - percent of spontaneous change of attitudes (" << MutationProb * 100 << ")\n";
        cerr << "\tMAX=NNNN - max simulation step (" << MaxIterations << ")\n";
        cerr<<"\tILOG=NNNN - length of internal statistic logs ("<<internal_log<<")\n";
        cerr << "\tSTOP=N/Y - exit after MAX steps (" << (AUTO_END?"Yes":"No") << ")\n";
        cerr << "\tVIEV=NNN - visualisation frequency (" << sim_to_view_ratio << ")\n";
        cerr<<"\tDUMP=Y/N - dump, or not, screen at every simulation step\n";
        cerr<<"\tRSPC=N/Y/+/- or 1..WIDTH - Random calculation of spatial correlation ("<<(spatial_correlation_mode?"N":"Y")<<")\n";
        cerr << "\tLOGC=N - log file saving frequency (" << sim_to_log_ratio << ")\n";
        cerr << "\tLOGF=name.log - file for simulation log (" << LogFName << ")\n";
        cerr<<"\tHIST=hist.otx - file for full history of simulation.\n";
        cerr << "\tWIDTHWIN,HEIGHTWIN=XXX - initial window size.(" << SCR_WIDTH << 'x' << SCR_HEIGHT << "\n";
        cerr<<"\nAUTO=XXX - number of auto-repetition of simulation.("<<AUTOSTART<<")\n";
        cerr<<"\nCONS=N/Y - switch on/off console mode.("<<Console<<")\n";
    return 0;
    }
    else
    {
        /* Ultimately, it turns out that there is no such option */
        ERROR:
        cerr<<"Unknown parameter \""<<argv[i]<<"\"\n";
        return 0;
    }

    }
return 1;
}


/* AREA MANAGER'S OWN RE-IMPLEMENTATION AND GENERAL MAIN FUNCTION */
/* ************************************************************** */

class my_area_manager: public main_area_manager
{
    jworld* TheWorld;
public:
    void ConnectWorld(jworld* W) {TheWorld=W;}

    /// Handler executed after the default handler. @return 1 if it was successful.
    int _post_process_input(int input_char) override
    {                      			//	assert(TheWorld!=NULL);
        if(TheWorld==NULL) return 0; // The world has not been built yet.

        switch(input_char)
        {
        case ID_VIEWOPT_DUMPCO:	//               60100
            world::continuous_dump=!world::continuous_dump;
            clog << "Continuous screen dumping was set to " << (world::continuous_dump?"Y":"N") << endl;
        break;
        case ID_VIEWOPT_LESSOFT:	//              60101
            TheWorld->InputRatio*=2;
            TheWorld->LogRatio=TheWorld->InputRatio;
            clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
        break;
        case ID_VIEWOPT_MOREOFT:	//              60102
            TheWorld->InputRatio/=2;
            if(TheWorld->InputRatio<1) TheWorld->InputRatio=1;
            TheWorld->LogRatio=TheWorld->InputRatio;
            clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
        break;
        case ID_VIEWOPT_EVERY1:	//               60110
            TheWorld->InputRatio=1;
            TheWorld->LogRatio=TheWorld->InputRatio;
            clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
        break;
        case ID_VIEWOPT_EVERY10:	//              60111
            TheWorld->InputRatio=10;
            TheWorld->LogRatio=TheWorld->InputRatio;
            clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
        break;
        case ID_VIEWOPT_EVERY100:	//             60112
            TheWorld->InputRatio=100;
            TheWorld->LogRatio=TheWorld->InputRatio;
            clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
        break;
        case ID_VIEWOPT_EVERY1000:	//            60113
            TheWorld->InputRatio=1000;
            TheWorld->LogRatio=TheWorld->InputRatio;
            clog<<"Visualisation and statistics every "<<TheWorld->InputRatio<<" steps"<<endl;
        break;
        default:
            return 2;
        }
        return 1; //Consider the event as handled
    }

    // CONSTRUCTORS:
    //--------------

    /// Constructor providing a manager with a specified list size.
    /// @warning: Calling more than one constructor will abort the process!!!
    my_area_manager( size_t size, ///< length of the possible areas list.
                     long int width, long int height,
                     unsigned ibkg=default_half_gray
                   )
    : main_area_manager(size, asserted<int>(width), asserted<int>(height), ibkg)
    {TheWorld=0;}

    // /// Constructor with a partially filled list (UNUSED!).
    // /// Attributes `bkg` and `frm` are default, but can be changed later.
    // --> my_area_manager(size_t size,
    //               int width,int height,
    //
    //               drawable_base* ptr/*first...NULL*/):main_area_manager(size,width,height,ptr)
    //               {TheWorld=nullptr;}
};

int main(const int argc,const char* argv[])
{
    cout << SIMULATION_NAME << endl;
    cout<<Authors<<endl;
    assert((cerr<<"All assertions are active!"<<endl)); //THE SIDE EFFECT ON ASSERTION IS intended!
    cout<<endl<<flush;

    if(!parse_options(argc,argv))
            exit(1);

    //INITIALIZE global randomizer:
    if(My_Rand_seed==0)
        {RANDOMIZE()}
    else
        {SRAND(My_Rand_seed)}

    //INITIALIZATION of the sub-window system:
    my_area_manager Lufciki(24, SCR_WIDTH, SCR_HEIGHT, 28);  //Or just a placeholder if you don't want graphics.

    if( Console || !Lufciki.start(SIMULATION_NAME, argc, argv, 1) )
    {
        cerr<<"Graphic output isn't initialized"<<endl;
        if(!Console) exit(1);
    }

    //Creating a meaningful name for your screenshot file(s):
    if(!Console)
    {
        wb_pchar buf(strlen(SCREEN_DUMP_NAME) + 20);
        buf.prn("%s_%ld", SCREEN_DUMP_NAME, time(NULL));
        Lufciki.set_dump_name(buf.get());
    }

    //INITIALIZATION OF THE SIMULATION MODEL
    jworld& theWorld=*new jworld(WorldWidth,
                                 LogFName,
                                 MapLName,
                                 MapPName,
                                 MaskName,
                                 asserted<short>(DistributionLevel),
                                 NoisePercent / 100.0,	//Noise from 0 to 1
                                 asserted<short>(MaximumStrength),	//We want it to be within range
                                 asserted<short>(MinimumStrength),
                                 asserted<short>(NofCategories),
                                 asserted<short>(NeighborhoodR),
                                 asserted<short>(NeighborhoodD),
                                 asserted<short>(ConsiderSelf),
                                 asserted<short>(GrowingStrength),
                                 asserted<short>((MaximumStrength * ThreshPercent) / 100.0),
                                 MutationProb,
                                 SW_links,
                                 SW_start_perc,
                                 SW_step_perc
                               );

//    if(&theWorld == NULL)	//From C++11 it is impossible.
//        {
//        cerr<<"Can't allocate a simulation world!\n"<<endl;
//        exit(1);
//        }

    if(!Console)
        Lufciki.ConnectWorld(&theWorld); //The Area Manager must have access to the variables that control the simulation.

    theWorld.set_max_iteration(MaxIterations);
    theWorld.set_input_ratio(sim_to_view_ratio);
    theWorld.set_log_ratio(sim_to_log_ratio);
    theWorld.set_bias_from_str(BIAS_STR);
    cout << SIMULATION_NAME << ": LOADED." << endl;
    theWorld.set_history_stream(HistName);

    if(DUMP_NET && *NetCName != '\0')
        theWorld.DumpNetName=NetCName;

    if(Replay)
    {
        if(Console)
        {
            cerr<<"You cannot replay in console mode!"<<endl;
            exit(2);
        }
        theWorld.initialize(&Lufciki, 1); //visualization initialization
        cout << SIMULATION_NAME << ": PREPARED FOR READING. WAIT!" << endl;
        Lufciki.restore(0);
        Lufciki.replot(0);
        Lufciki.process_input(); //Handling the first events. They end after Ctrl-B.
        theWorld.read_loop(AUTO_END);
    }
    else
    {
        theWorld.initialize(&Lufciki); //initialization of visualization and simulation layers.
        cout << SIMULATION_NAME << ": INITIALISED." << endl;

        if(!AUTOSTART)
        {
            //Lufciki.process_input(); //???
            /// @internal MAIN SIMULATION LOOP:
            cout << SIMULATION_NAME << ": STARTED." << endl;

            Lufciki.restore(0);
            Lufciki.replot(0);

            theWorld.simulation_loop(AUTO_END);
        }
        else
        {                                                                              assert(AUTOSTART);
            if(!Console)
            {
                int statusWin=Lufciki.search("STATUS");
                Lufciki.maximize(statusWin); // The large status window obscures the rest and limits visualization and lazy calculations.
                set_char('\02'); //ctrl-B ?
            }

            for(int simulation=0; simulation < AUTOSTART; simulation++)
            {
                /// @internal MAIN SIMULATION LOOP WITH AUTOSTART:
                cout << SIMULATION_NAME << ": SIMULATION " << simulation << " STARTED." << endl;
                theWorld.simulation_loop(1);
                cout << SIMULATION_NAME << ": SIMULATION " << simulation << " DONE." << endl;
                if(simulation < AUTOSTART - 1)
                {
                    //Reinitialization when repetitions.
                    theWorld.restart();
                }
            }
        }

    }

    cout << SIMULATION_NAME << ": CLOSING." << endl;

    cout.flush();

    delete &theWorld; //Deallocation of the world with all its components
    cout<<"----------> See you later!!! <--------------\n"<<endl<<flush;
    return 0;
}


/* STATIC ALLOCATION */
/// `unsigned agent::max=0;` - what is the largest class.

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

#pragma clang diagnostic pop
