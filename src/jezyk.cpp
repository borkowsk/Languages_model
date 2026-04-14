/// @file
/// @brief MAIN SOURCE FILE OF LANGUAGES PROJECT WITH P.Culicover.
//  ==============================================================
/// @date 2026-04-14 (modified)
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  THIS PROGRAM IS DESIGNED FOR CFCS OF ISS UW!
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
#pragma ide diagnostic ignored "modernize-use-auto"
#include "compatyb.h"
const char* WINDOW_HEADER="LANGUAGES version SW 2.20c, compilation " __DATE__ ", " __TIME__ ;
const char* Authors="(programed by W. Borkowski for ISS UW & Ohio State Univ.)";
const char* SCREENDUMPNAME="LANGUAGES";
/// @details
/// Simulation of the spread of linguistic behavior using the multi-layered meme/belief transmission method.
///	Polish word "JEZYK" means LANGUAGE.
/// #HISTORY:
//  ////////////////////////
///			ver.  2.20c - English-language comments.
///			ver.  2.20b - ADAPTATION TO NEW COMPILATION CONDITIONS IN clion IN 2026.
///
///			...[a dozen or so years apart]...
///
///			ver.  2.20 - Introduction of 16 classes to Log-Log histogram added in version 1.401.
///					   - Changing the order of series/columns in the log file.
///					   - Introduction of console mode of operation (in the background, without graphics at all).
///			ver.  2.11 - Beginnings of using `OptionalParameters` classes to handle call parameters (later abandoned).
///			ver.  2.10 - Implementation of the process of imposing power and displaying the political map.
///			ver.  2.06 - Display rate control now working and `my_area_menager` declared.
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
///						- Introducing the ability to disable spatial correlation calculation (UseSpatialCorr) from CODE!!!
///						- Introduction of a log-log plot of the distribution of language sizes (type "dhistosou.h").
///						- Introduction of writing this histogram to the log.
///						- Supplementing some of the messages about parameter settings (but some were left without).
///						- Entering the `DSTB` parameter that determines the type and degree of force distribution (obtained by * or +).
///			TEST:
///			```
///			.........WIDTH=100 DSTB=-8 CLSS=8 MIPO=3 RSPC=1 VIEW=50 LOGF=10 LOGF=testW100.log
///			```
///
///			version 1.35a - ???
///			version 1.34a - Remove assertions against zero-strength agents and introduce a minimal strength (def. = 1).
///			version 1.33a - Recompiling to a new library and adding a menu for MSWindows.
///			version 1.32a - Recompiling with a new version of the visualization library and introducing
///						    the "about_languages.cpp" file with time control for DEBUG.
///			version 1.31a - Implemented initialization of language attributes from three grayscale image files.
///			version 1.30a - Double bias introduced in the follow-up version, but a parallel bias version still works poorly
///						    requires intelligent counter rebalancing.
///			version 1.20a - Preparation for the introduction of "double bias" (parallel).
///			version 1.01b - Improved default for a strength threshold.
///			version 1.05b - Built-in support for batch work and experiment repetition.
///			version 1.10b - Introducing "bias" for language parameters.

int My_Rand_seed=0; ///< Random generator initializer. If 0 it uses RANDOMIZE, if other it uses SRAND(My_Rand_seed).

#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

#define HIDE_WB_PTR_IO 0
#include "wb_ptr.hpp"
#include "optParam.hpp"

#include "jrand.h"
#include "jworld.h"
#include "lang_res.h"

unsigned	SWIDTH=1440*0.6666;		///< Screen/window inside width. (720 or 1440);
unsigned	SHEIGHT=1080*0.6666;	///< Screen/window inside height. (540 or 1080);
bool		Console=false;			///< Flag for working in console mode - no graphics.

//No Object-wise passed to the source initialization method:
unsigned	internal_log=10000;			///< Default length of internal logs (historical data sources).
bool		UseSpatialCorr=false;		///< Flag for using spatial correlation (expensive to compute).
unsigned	spatial_correlation_mode=50;	///< Number of sampling runs in calculating spatial correlation.

//Object-oriented passed to the world constructor.
char	 LogName[512]="languagesSW2.log\0-------------------+--";		///<
char	NetCName[512]="languagesSW_\0---------------------+--";			///<
char	HistName[512]="\0--+---------languagesSW2.otx----------";		///<
char	MapLName[512]="\0--+---------languagesSW2.gif----------";		///<
char	MapPName[512]="\0--+---------powersSW2.gif------------";		///<
char	MaskName[512]="\0--+---------maskSW2.gif--------------";		///<

unsigned	iWidth=100;		///<
unsigned	iMaxIterations=0xffffffff;	///<
unsigned	iLogRatio=10;		///<
unsigned	iViewRatio=1;		///<

bool	ZrzucajNET=false;				///< Flag specifying whether to dump network files. (SW?)
int		RuchomaSila=0;					///< Determines whether strength should increase "with age".
int		MaksymalnaSila=10000;			///< Determines what the agent's greatest strength can be.
int		MinimalnaSila=10;				///< Determines what the minimum force can be.
                                        ///< If the min and max are the same, then the value is the same everywhere.
int		TresProcent=10000;				///< Above what certain strength, changes in "attributes" become impossible.

int		IloscKlas=128;		///<
double	ProcentSzumu=0;		///<
double	MutacjeSpon=0;		///<
int		RozmiarSasiedztwa=1;		///<
int		IleSasiadow=8;		///<
int		BranieSiebie=1;		///<
int		iWychodzenie=0;		///<
int		Replay=0;		///<
int		AUTOSTART=0;		///<

int		DistributionLevel=6;		///< Type and degree of strength distribution.
const char*	BIAS_STR="";			///< Bias definition collected directly from the parameter line.

double	SW_start_perc=0;			///< Controlling the process of the world political hierarchization at the beginning of the simulation.
double	SW_step_perc=0;				///< Controlling the process of the world political hierarchization at each step of the simulation.
bool	SW_links=false;
/*
//int parse_options(const int argc,const char* argv[]);	//Zapowiedz!
OptionalParameterBase* Parameters[]={ //sizeof(Parameters)/sizeof(Parameters[])
new ParameterLabel("PARAMETERS FOR SINGLE SIMULATION"),
//new OptionalParameter<TYPE>(VARIABLE,LOWLIM,HIGHLIM,"PAR_NAME","help info"),
new OptionalParameter<double>(MutacjeSpon,0,1,"SPCH"," Ratio of spontaneous changes"),
new OptionalParameter<double>(ProcentSzumu,0,100,"NOIP","Noise in decision (in %)"),
new OptionalParameter<long>(IloscKlas,2,256,"CLSS","Number of classes in each mem"),
new OptionalParameter<long>( MinimalnaSila,0,100000,"MIPO","Minimal strength"),
new OptionalParameter<long>(MaksymalnaSila,1,1000000,"MPOW","Max strength"),
new OptionalParameter<long>(RuchomaSila,0,1000000,"WPOW","Moving strength"),
new OptionalParameter<long>(SW_start_perc,0,100,"SWBE","SW links will be used at start"),
new OptionalParameter<long>(SW_step_perc,0,50,"SWST","SW links will be add every step"),
new OptionalCheckFunction(SWLinksCheck(),"if(SWBE>0 || SWST>0) SW_links=true;"),
new OptionalParameter<bool>(ZrzucajNET,false,true,"NETD","dump network every step"),
new OptionalParameter<long>(TresProcent,0,100,"TRSP","Immunisation strength threshold"),
new OptionalCheckFunction(MovingSghCheck(),"if(TRSP>0 && RuchomaSila==0) RuchomaSila=1;");
new OptionalParameter<long>(SWIDTH,50,4096,"WIDTHWIN","Window width"),
new OptionalParameter<long>(SHEIGHT,50,4096,"HEIGHTWIN","Window height"),
new OptionalParameter<long>(iWidth,3,SWIDTH,"WIDTH","World width. Can't be larger than window/screen!"),
new OptionalParameter<long>(iMaxIterations,0,0x7FFFFFFF,"MAXI","Maximal number of steps"),
new OptionalParameter<long>(iMaxIterations,0,0x7FFFFFFF,"MAX",NULL),	//hidden alias
new OptionalParameter<long>(iLogRatio,1,iMaxIterations,"LOGC","Writing to log frequency (not higher than MAXI)"),
new OptionalParameter<long>(iViewRatio,1,iMaxIterations,"VIEW","visualization frequency"),
new OptionalParameter<long>(My_Rand_seed,0,0x7FFFFFFF,"SRND","SRND (rand seed). Must be >=0, but 0 means current time in seconds!"),
new OptionalParameter<bool>(jworld::continuous_dump,false,true,"DUMP","dumping the screen at every step"),
new OptionalParameter<bool>(BranieSiebie,false,true,"SELF","use own state for calculation the new one or not"),
new OptionalParameter<TYPE>(RozmiarSasiedztwa,1,iWidth,"INDI","Radius of neighborhood"),
new OptionalCheckFunction(NeighRadCheck(),"if(INDI>(WIDTH/2-1)) INDI=WIDTH/2-1;");
new OptionalParameter<long>(IleSasiadow,2,sqr(RozmiarSasiedztwa*2+1)-1,"PRTR","How many contacts chosen from neighbors"),
new OptionalParameter<bool>(AUTOSTART,false,true,"AUTO","Automatically start the simulation"),
new OptionalParameter<bool>(iWychodzenie,false,true,"STOP","Automatic exit when done"),
new OptionalParameter<long>(internal_log,50,iMaxIterations,"ILOG","yyy"),
new OptionalParameter<int>(DistributionLevel,-100,100,"DSTB","Distribution Level/Kind"),
new OptionalParameter<wb_pchar>(LogName,"*.log","*.txt","LOGF","name for log file"),
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
        cerr<<"\tWIDTH=NN - matrix size ("<<iWidth<<")\n";
        cerr<<"\tSRND=NNNN - random seed if you want particular one\n";
        cerr<<"\tBIAS=item item ... - setting bias by items string (NO BIAS)\n"<<
              "\t\the item string example: \"A1:1 A2:4 B2:2 C2:4 A4&C3:10 A5&B5&C5:12\"\n";
        cerr<<"\tCLSS=NN - number of class. Must be power of 2. ("<<IloscKlas<<")\n";
        cerr<<"\tMPOW=NN - max strength for initialization ("<<MaksymalnaSila<<")\n"	;
        cerr<<"\tMIPO=NN - min strength for initilization ("<<MinimalnaSila<<")\n"	;
        cerr<<"\tDSTB=N - level and kind of strength distribution ("<<DistributionLevel<<")\n";
        cerr<<"\nSWST=PP/PP - percent of SW links created at every step, and at the beginning (0)\n";
        cerr<<"\nNETD=N/Y - dumping net files for statistics parallely (N)\n";
//      cerr<<"\tWPOW=N	- walking step of strength	("<<RuchomaSila<<")\n";
        cerr<<"\tTRSP=N - % of threshold of strength ("<<TresProcent<<")\n";
        cerr<<"\tPRTR=2..WIDTH^2-1 - number of interaction partners ("<<IleSasiadow<<")\n";
        cerr<<"\tINDI=1..WIDTH/2-1 - interaction distance ("<<RozmiarSasiedztwa<<")\n";
        cerr<<"\tSELF=N/Y -use self for calculations ("<<(BranieSiebie?"Yes":"No")<<")\n";
        cerr<<"\tNOIP=NN - percent of noise ("<<ProcentSzumu<<")\n";
        cerr<<"\tSPCH=NN - percent of spontaneous change of attitudes ("<<MutacjeSpon*100<<")\n";
        cerr<<"\tMAX=NNNN - max simulation step ("<<iMaxIterations<<")\n";
        cerr<<"\tILOG=NNNN - length of internal statistic logs  ("<<internal_log<<")\n";
        cerr<<"\tSTOP=N/Y - exit after MAX steps ("<<(iWychodzenie?"Yes":"No")<<")\n";
        cerr<<"\tVIEV=NNN - visualisation frequency ("<<iViewRatio<<")\n";
        cerr<<"\tDUMP=Y/N - dump, or not, screen at every simulation step\n";
        cerr<<"\tRSPC=N/Y/+/- or 1..WIDTH - Random calculation of spatial correlation ("<<(spatial_correlation_mode?"N":"Y")<<")\n";
        cerr<<"\tLOGC=N - log file saving frequency ("<<iLogRatio<<")\n";
        cerr<<"\tLOGF=name.log - file for simulation log ("<<LogName<<")\n";
        cerr<<"\tHIST=hist.otx - file for full history of simulation.\n";
        cerr<<"\tWIDTHWIN,HEIGHTWIN=XXX - initial window size.("<<SWIDTH<<'x'<<SHEIGHT<<"\n";
        cerr<<"\nAUTO=XXX - number of auto-repetition of simulation.("<<AUTOSTART<<")\n";
*/

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
            goto ERROR; //NA PEWNO ZLE

    *pom='\0';strupr(rob);*pom='=';	//Part to the = sign

    if(strcmp(rob,"HELP")==0)
    {
        goto HELPPRINT;
    }
    else
    if((pom=strstr(rob,"SPCH="))!=NULL) //If not NULL then exists
    {
    MutacjeSpon=atof(pom+5);
    if(MutacjeSpon<=0 || MutacjeSpon>100)
        {
        cerr<<"!!! Bad SPCH ="<<MutacjeSpon<<" (must be in <0,100> )"<<endl;
        return 0;
        }
    cerr<<"* Spontaneous change ratio in %: SPCH= "<<MutacjeSpon<<endl;
    MutacjeSpon/=100;	//A fraction, not a percentage, really.
    }
    else
    if((pom=strstr(rob,"NOIP="))!=NULL) //If not NULL then exists
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
    if((pom=strstr(rob,"CLSS="))!=NULL) //If not NULL then exists
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
    if((pom=strstr(rob,"MIPO="))!=NULL)  //If not NULL then exists
    {
    MinimalnaSila=atol(pom+5);
    if(MinimalnaSila<0)	//0 czy 1???
        {
        cerr<<"!!! Bad MIPO ="<<MinimalnaSila<<" (must be >=1 )"<<endl;
        return 0;
        }
    cerr<<"* Minimal strength: MIPO= "<<MinimalnaSila<<endl;
    }
    else
    if((pom=strstr(rob,"MPOW="))!=NULL) //If not NULL then exists
    {
    MaksymalnaSila=atol(pom+5);
    if(MaksymalnaSila<0)	//0 or 1???
        {
        cerr<<"!!! Bad MPOW ="<<MaksymalnaSila<<" (must be >=1 )"<<endl;
        return 0;
        }
    cerr<<"* Max strenght: MPOW= "<<MaksymalnaSila<<endl;
    }
    else
    if((pom=strstr(rob,"WPOW="))!=NULL) //If not NULL then exists
    {
    RuchomaSila=atol(pom+5);
    if(RuchomaSila<0)
        {
        cerr<<"!!! Bad WPOW ="<<RuchomaSila<<" (must be >=0 )"<<endl;
        return 0;
        }
    cerr<<"* Moving strength: WPOW= "<<RuchomaSila<<endl;
    }
    else
    if((pom=strstr(rob,"SWST="))!=NULL) //If not NULL then exists
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
    else
    if((pom=strstr(rob,"NETD="))!=NULL) //If not NULL then exists
    {
        ZrzucajNET=(toupper(pom[5])=='Y');
        cerr<<"* NETD="<<(ZrzucajNET?"Yes":"No")<<endl;
    }
    else
    if((pom=strstr(rob,"TRSP="))!=NULL) //If not NULL then exists
    {
    TresProcent=atol(pom+5);
    if(TresProcent<0 || TresProcent>100)
        {
        cerr<<"!!! Bad TRSP = "<<int(TresProcent)<<"(must be in <0,100>"<<endl;
        return 0;
        }
        else
        {
        cerr<<"* Immunisation strength threshold : TRSP= "<<int(TresProcent)<<"%"<<endl;
        if(RuchomaSila==0)	//There is no point in TRSP if there is no mobile force
            {
            RuchomaSila=1;
            cerr<<"** Automatically set WPOW to "<<RuchomaSila<<endl;
            }
        }
    }
    else
    if((pom=strstr(rob,"WIDTH="))!=NULL) //If not NULL then exists
    {
    iWidth=atol(pom+6);
    if(iWidth<3)
        {
        cerr<<"!!! Bad WIDTH = "<<iWidth<<"(must be in <3,"<<SWIDTH<<">"<<endl;
        return 0;
        }
    if(iWidth>=SWIDTH)
        cerr<<"WIDTH ("<<iWidth<<") is really high!\n Simulation world may be larger than the window or even the screen."<<endl;
    cerr<<"* World width: WIDTH= "<<iWidth<<'x'<<iWidth<<endl;
    }
    else
    if((pom=strstr(rob,"WIDTHWIN="))!=NULL) //If not NULL then exists
    {
    SWIDTH=atol(pom+9);
    if(SWIDTH<50)
        {
        cerr<<"!!! Bad WIDTHWIN = "<<SWIDTH<<" (must be >50)"<<endl;
        return 0;
        }
    }
    else
    if((pom=strstr(rob,"HEIGHTWIN="))!=NULL) //If not NULL then exists
    {
    SHEIGHT=atol(pom+10);
    if(SHEIGHT<50)
        {
        cerr<<"!!! Bad HEIGHTWIN = "<<SHEIGHT<<" (must be >50)"<<endl;
        return 0;
        }
    }
    else
    if((pom=strstr(rob,"MAX="))!=NULL) //If not NULL then exists
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
    if((pom=strstr(rob,"LOGC="))!=NULL)  //If not NULL then exists
    {
    iLogRatio=atol(pom+5);
    if(iLogRatio<=0)
        {
        cerr<<"!!! Bad LOGC (write to log frequency). Must be >0"<<endl;
        return 0;
        }
    }
    else
    if((pom=strstr(rob,"VIEW="))!=NULL) //If not NULL then exists
    {
    iViewRatio=atol(pom+5);
    if(iViewRatio<=0)
        {
        cerr<<"!!! Bad VIEW (visualization frequency). Must be >0"<<endl;
        return 0;
        }
    }
    else
    if((pom=strstr(rob,"SRND="))!=NULL)  //If not NULL then exists
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
    if((pom=strstr(rob,"DUMP="))!=NULL)  //If not NULL then exists
    {
    jworld::continous_dump=(toupper(pom[5])=='Y');
    cerr<<"DUMP="<<(jworld::continous_dump?"Yes":"No")<<endl;
    }
    //continuous dump
    else
    if((pom=strstr(rob,"SELF="))!=NULL)  //If not NULL then exists
    {
    BranieSiebie=(toupper(pom[5])=='Y');
    cerr<<"SELF="<<(BranieSiebie?"Yes":"No")<<endl;
    }
    else
    if((pom=strstr(rob,"INDI="))!=NULL)  //If not NULL then exists
    {
    RozmiarSasiedztwa=atol(pom+5);
    if( RozmiarSasiedztwa>=1U &&
        RozmiarSasiedztwa<(iWidth/2-1))
        {
        cerr<<"* Radius of a neighborhood: INDI="<<RozmiarSasiedztwa<<endl;;
        }
        else
        {
        cerr<<"!!! Bad INDI="<<RozmiarSasiedztwa<<" Must from 1 to "<<iWidth/2-1<<endl;
        return 0;
        }
    }
    else
    if((pom=strstr(rob,"PRTR="))!=NULL)  //If not NULL then exists
    {
    IleSasiadow=atol(pom+5);
    if(IleSasiadow>1 && IleSasiadow<=sqr(RozmiarSasiedztwa*2+1)-1)
        {
        cerr<<"* How many real neighbors: PRTR="<<IleSasiadow<<endl;
        }
        else
        {
        cerr<<"!!! Bad PRTR="<<IleSasiadow
            <<"! Must from 2 to "<<sqr(RozmiarSasiedztwa*2+1)-1<<endl;
        return 0;
        }
    }
    else
    if((pom=strstr(rob,"AUTO="))!=NULL)  //If not NULL then exists
    {
    AUTOSTART=atol(pom+5);
    cerr<<"* AUTO="<<AUTOSTART<<endl;
    if(AUTOSTART)
        {
        iWychodzenie=1;
        cerr<<"** STOP="<<(iWychodzenie?"Yes":"No")<<endl;
        }
    }
    else 	//Console
    if((pom=strstr(rob,"CONS="))!=NULL)  //If not NULL then exists
    {
    Console=(atol(pom+5)!=0) || pom[5]=='Y' || pom[5]=='y' || pom[5]=='T' || pom[5]=='t';
    cerr<<"* CONSole="<<Console<<"!!!"<<endl;
    if(Console)
        {
        cerr<<"No graphics window will appear, so you can also use 'nohup' unix command for this program"<<endl;
        iWychodzenie=1;
        AUTOSTART=1;
        cerr<<"** AUTO="<<AUTOSTART<<endl;
        cerr<<"** STOP="<<(iWychodzenie?"Yes":"No")<<endl;
        }
    }
    else
    if((pom=strstr(rob,"BIAS="))!=NULL)  //If not NULL then exists
    {
    BIAS_STR=rob+5;
    cerr<<"* BIAS = "<<BIAS_STR<<endl;
    static wb_pchar taker;
    taker=hand.give();	//Takes away the management. Dismissal at the end of the program.
    }
    else
    if((pom=strstr(rob,"STOP="))!=NULL)  //If not NULL then exists
    {
    iWychodzenie=(toupper(pom[5])=='Y');
    cerr<<"* Automatic exit when done: STOP="<<(iWychodzenie?"Yes":"No")<<endl;
    }
    else
    if((pom=strstr(rob,"ILOG="))!=NULL)  //If not NULL then exists
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
    if((pom=strstr(rob,"DSTB="))!=NULL)  //If not NULL then exists
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
    if((pom=strstr(rob,"LOGF="))!=NULL)  //If not NULL then exists
    {
    strcpy(LogName,pom+5);
    }else
    if((pom=strstr(rob,"MAPL="))!=NULL)  //If not NULL then exists
    {
    strcpy(MapLName,pom+5);
    cerr<<"* Map of languages from file \""<<MapLName<<"\"\n";
    }
    else
    if((pom=strstr(rob,"MAPP="))!=NULL)  //If not NULL then exists
    {
    strcpy(MapPName,pom+5);
    cerr<<"* Map of individual power from file \""<<MapPName<<"\"\n";
    }
    else
    if((pom=strstr(rob,"MASK="))!=NULL)  //If not NULL then exists
    {
    strcpy(MaskName,pom+5);
    cerr<<"* Mask for alive agents from file \""<<MaskName<<"\"\n";
    }
    else
    if((pom=strstr(rob,"HIST="))!=NULL)  //If not NULL then exists
    {
    strcpy(HistName,pom+5);
    cerr<<"* History of the simulation will be saved to \""<<HistName<<"\"\n";
    }
    else
    if((pom=strstr(rob,"REPL="))!=NULL) //If not NULL then exists
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
                cerr<<"!!!! Spatial correlations are enabled. RSPC will be applied."<<endl;
                UseSpatialCorr=true;
            }
            else
            if(*(pom+5)=='-')
            {
                cerr<<"!!!! Spatial correlations were disabled."<<endl;
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
        cerr<<"\tREPL=hist.otx - not simulate, but replay simulation history file.\n";
        cerr<<"\tMAPL=initL.gif (or BMP)- file with an initialization map of languages or \n";
        cerr<<"\t    =\"init1.gif;init2.gif;init3.gif\" (or BMP)- 3. separate init file (RANDOM)\n";
        cerr<<"\tMAPP=initP.gif (or BMP)- file with an initialization map of strengths (RANDOM)\n";
        cerr<<"\tMASK=mask.gif	(or BMP)- mask file for alive (not black) agents (ALL ALIVE)\n";
        cerr<<"\tWIDTH=NN - matrix size ("<<iWidth<<")\n";
        cerr<<"\tSRND=NNNN - random seed if you want particular one\n";
        cerr<<"\tBIAS=item item ... - setting bias by items string (NO BIAS)\n"<<
              "\t\the item string example: \"A1:1 A2:4 B2:2 C2:4 A4&C3:10 A5&B5&C5:12\"\n";
        cerr<<"\tCLSS=NN - number of class. Must be power of 2. ("<<IloscKlas<<")\n";
        cerr<<"\tMPOW=NN - max strength for initialization ("<<MaksymalnaSila<<")\n"	;
        cerr<<"\tMIPO=NN - min strength for initialization ("<<MinimalnaSila<<")\n"	;
        cerr<<"\tDSTB=N - level and kind of strength distribution ("<<DistributionLevel<<")\n";
        cerr<<"\nSWST=PP/PP - percentage of SW links created at every step, and at the beginning (0)\n";
        cerr<<"\nNETD=N/Y - dumping net files parallel to statistics (N)\n";
//		cerr<<"\tWPOW=N	- walking step of strength	("<<RuchomaSila<<")\n";
        cerr<<"\tTRSP=N - % of threshold of strength ("<<TresProcent<<")\n";
        cerr<<"\tPRTR=2..WIDTH^2-1 - number of interaction partners ("<<IleSasiadow<<")\n";
        cerr<<"\tINDI=1..WIDTH/2-1 - interaction distance ("<<RozmiarSasiedztwa<<")\n";
        cerr<<"\tSELF=N/Y - use self for calculations ("<<(BranieSiebie?"Yes":"No")<<")\n";
        cerr<<"\tNOIP=NN - percent of noise ("<<ProcentSzumu<<")\n";
        cerr<<"\tSPCH=NN - percent of spontaneous change of attitudes ("<<MutacjeSpon*100<<")\n";
        cerr<<"\tMAX=NNNN - max simulation step ("<<iMaxIterations<<")\n";
        cerr<<"\tILOG=NNNN - length of internal statistic logs  ("<<internal_log<<")\n";
        cerr<<"\tSTOP=N/Y - exit after MAX steps ("<<(iWychodzenie?"Yes":"No")<<")\n";
        cerr<<"\tVIEV=NNN - visualisation frequency ("<<iViewRatio<<")\n";
        cerr<<"\tDUMP=Y/N - dump, or not, screen at every simulation step\n";
        cerr<<"\tRSPC=N/Y/+/- or 1..WIDTH - Random calculation of spatial correlation ("<<(spatial_correlation_mode?"N":"Y")<<")\n";
        cerr<<"\tLOGC=N - log file saving frequency ("<<iLogRatio<<")\n";
        cerr<<"\tLOGF=name.log - file for simulation log ("<<LogName<<")\n";
        cerr<<"\tHIST=hist.otx - file for full history of simulation.\n";
        cerr<<"\tWIDTHWIN,HEIGHTWIN=XXX - initial window size.("<<SWIDTH<<'x'<<SHEIGHT<<"\n";
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

class my_area_menager:public main_area_menager
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
            world::continous_dump=!world::continous_dump;
            clog<<"Screen dumping was set to "<<world::continous_dump<<endl;;
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
    my_area_menager(size_t size, ///< length of the list of possible areas.
                int width,int height,
                unsigned ibkg=default_half_gray
                ):main_area_menager(size,width,height,ibkg)
                {TheWorld=0;}

    // /// Constructor with a partially filled list (UNUSED!).
    // /// Attributes `bkg` and `frm` are default, but can be changed later.
    // my_area_menager(size_t size,
    //               int width,int height,
    //
    //               drawable_base* ptr/*first...NULL*/):main_area_menager(size,width,height,ptr)
    //               {TheWorld=nullptr;}
};

int main(const int argc,const char* argv[])
{
    cout<<WINDOW_HEADER<<endl;
    cout<<Authors<<endl;											assert((cerr<<"All assertions are active!"<<endl));
    cout<<endl<<flush;

    if(!parse_options(argc,argv))
            exit(1);

    //INITIALIZE global randomizer:
    if(My_Rand_seed==0)
        {RANDOMIZE();}
    else
        {SRAND(My_Rand_seed);}

    //INITIALIZATION of the sub-window system:
    my_area_menager Lufciki(24,SWIDTH,SHEIGHT,28);  //Or just a dummy if you don't want graphics

    if( Console || !Lufciki.start(WINDOW_HEADER,argc,argv,1) )
    {
        cerr<<"Graphic output isn't initialized"<<endl;
        if(!Console) exit(1);
    }

    //Creating a meaningful name for your screenshot file(s):
    if(!Console)
    {
        wb_pchar buf(strlen(SCREENDUMPNAME)+20);
        buf.prn("%s_%ld",SCREENDUMPNAME,time(NULL));
        Lufciki.set_dump_name(buf.get());
    }

    //INITIALIZATION OF THE SIMULATION MODEL
    jworld& tenSwiat=*new jworld(iWidth,
                               LogName,
                               MapLName,
                               MapPName,
                               MaskName,
                               DistributionLevel,
                               ProcentSzumu/100.0,	//Noise from 0 to 1
                               MaksymalnaSila,	//We want it to be within range
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

    if(&tenSwiat==NULL)	//As if something had gone wrong.
        {
        cerr<<"Can't allocate a simulation world!\n"<<endl;
        exit(1);
        }

    if(!Console)
        Lufciki.ConnectWorld(&tenSwiat); //The Area Manager must have access to the variables that control the simulation.

    tenSwiat.set_max_iteration(iMaxIterations);
    tenSwiat.set_input_ratio(iViewRatio);
    tenSwiat.set_log_ratio(iLogRatio);
    tenSwiat.set_bias_from_str(BIAS_STR);
    cout<<WINDOW_HEADER<<": LOADED."<<endl;
    tenSwiat.set_history_stream(HistName);

    if(ZrzucajNET && NetCName && *NetCName!='\0')
        tenSwiat.DumpNetName=NetCName;

    if(Replay)
    {
        if(Console)
        {
            cerr<<"You cannot replay in console mode!"<<endl;
            exit(2);
        }
        tenSwiat.initialize(&Lufciki,1); //visualization initialization
        cout<<WINDOW_HEADER<<": PREPARED FOR READING. WAIT!"<<endl;
        Lufciki.restore(0);
        Lufciki.replot(0);
        Lufciki.process_input(); //Handling the first events. They end after Ctrl-B.
        tenSwiat.read_loop(iWychodzenie);
    }
    else
    {
        tenSwiat.initialize(&Lufciki); //initialization of visualization and simulation layers.
        cout<<WINDOW_HEADER<<": INITIALISED."<<endl;

        if(!AUTOSTART)
        {
            //Lufciki.process_input(); //???
            /// @internal MAIN SIMULATION LOOP:
            cout<<WINDOW_HEADER<<": STARTED."<<endl;

            Lufciki.restore(0);
            Lufciki.replot(0);

            tenSwiat.simulation_loop(iWychodzenie);
        }
        else
        {                                                                              assert(AUTOSTART);
            if(!Console)
            {
                int statusWin=Lufciki.search("STATUS");
                Lufciki.maximize(statusWin); // The large status window obscures the rest and limits visualization and lazy calculations.
                set_char('\02');//ctrl-B ?
            }

            for(int symulacja=0;symulacja<AUTOSTART;symulacja++)
            {
                /// @internal MAIN SIMULATION LOOP WITH AUTOSTART:
                cout<<WINDOW_HEADER<<": SIMULATION "<<symulacja<<" STARTED."<<endl;
                tenSwiat.simulation_loop(1);
                cout<<WINDOW_HEADER<<": SIMULATION "<<symulacja<<" DONE."<<endl;
                if(symulacja<AUTOSTART-1)
                {
                    //Reinitialization when repetitions.
                    tenSwiat.restart();
                }
            }
        }

    }

    cout<<WINDOW_HEADER<<": CLOSING."<<endl;

    cout.flush();

    delete &tenSwiat; //Deallocation of the world with all its components
    cout<<"----------> See you later!!! <--------------\n"<<endl<<flush;
    return 0;
}


/* STATIC ALLOCATION */
//unsigned agent::max=0; //what is the largest class.

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

