/// @file
/// @brief CONDITIONAL BIAS SIMULATION STEP IMPLEMENTATION (LANGUAGES PROJECT WITH P.Culicover)
/// @date 2026-05-14 (modified)
///     Created long time ago.
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include <limits.h>
//#include <assert.h>
//#include <string.h>
//#include <math.h>
#include <cstring>
#include <cmath>
//#include <strstream>

//#include "compatyb.h"
#include "histosou.hpp"
//#include "clstsour.hpp" //Jest tez statsour
#include "coincsou.hpp"
//#include "compatyb.hpp"
#include "gadgets.hpp"
#include "wb_ptrio.h"

#include "jrand.h"
#include "jworld.h"

using namespace symshell2;

void	jworld::_one_step_conditional_bias0()
{   /// Shortcut to the geometry of the simulation world.
    const symshell2::geometry_base* MyGeom=Agenci.get_geometry();													 assert(MyGeom);

    /// THREE-DIMENSIONAL TABLE FOR COUNTING INFLUENCES.
    /// Number of allowable categories in each meme + positions for single biases and double combinations.
    int Wplywy[BIAS_FOR_ANY+1][BIAS_FOR_ANY+1][BIAS_FOR_ANY+1];							  assert(NumOfCate <= BIAS_FOR_ANY);	//Aren't there too many categories for such an influence board?

    /// Monte-Carlo iterator (may have data allocations inside).
    symshell2::iterator_h Monte=MyGeom->make_random_global_iterator();
    int testowanie=0;; ///< An auxiliary counter for testing the algorithm.

    //We go through the agents with a Monte-Carlo iterator. Some may be drawn again.
    while(Monte) // Loop through the neighborhood.
    {
        size_t index=MyGeom->get_next(Monte); //We obtain the index of a randomly selected agent
        //if(index==FULL) continue;
                                                                                    assert(index!=any_layer_base::FULL);
        /// Reference to the agent. Obtained bypassing the NULL assertion.
        jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());

        if(Agenci.is_empty(CenterAgent)) // Check if it is not an empty cell (NULL)
                continue;

        if(CenterAgent.Power > TrsStrength)	// Is there no immunity to change anymore?
            // TODO And why is there no possibility of mutation here?
                goto STARZENIE;

        {	// INFLUENCE CALCULATION CODE:
            //////////////////////////////
            iterator_h Neigh=MyGeom->make_random_neighbour_iterator(index, NeighRadius, NeighDens);	// Alokujemy iterator sasiedztwa
            unsigned zliczanie=0;           //For counting neighbors

            //The counter table need to be reset.
            //You have to zero the whole thing, even if you don't use all of it - because there are columns for BIAS_FOR_ANY
            memset(Wplywy,0,sizeof(Wplywy));
            // TODO And where is the influence from a distant link?
            while(Neigh)  //Loop through the neighborhood.
            {
                size_t index2=MyGeom->get_next(Neigh); ///< We get the neighbor's index.
                if(index2==any_layer_base::FULL || index2==index)	//If it was outside the simulation area or in the center of the area, it would still be pointless.
                    continue;

                jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val());///< A reference to a neighbor bypassing NULL assertions.
                if(Agenci.is_empty(PeryfAgent))		//We check whether it is not an empty cell (NULL) because then it would be pointless to continue.
                    continue;

                zliczanie++;						//Counts the number of randomly selected neighbors.

                //Adding the forces of each neighbor to the counters in the tables:
                Wplywy[PeryfAgent.First][PeryfAgent.Second][PeryfAgent.Third]+=3*PeryfAgent.Power;	//"counter" for ABC coincidence

                Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][PeryfAgent.Third]+=2*PeryfAgent.Power;	//BxC histogram "counter".
                Wplywy[PeryfAgent.First][BIAS_FOR_ANY][PeryfAgent.Third]+=2*PeryfAgent.Power;	//AxC histogram "counter".
                Wplywy[PeryfAgent.First][PeryfAgent.Second][BIAS_FOR_ANY]+=2*PeryfAgent.Power;	//AxB histogram "counter".

                Wplywy[PeryfAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=PeryfAgent.Power;	//histogram "counter" for Axx
                Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][BIAS_FOR_ANY]+=PeryfAgent.Power;	//histogram "counter" for xBx
                Wplywy[BIAS_FOR_ANY][BIAS_FOR_ANY][PeryfAgent.Third]+=PeryfAgent.Power;	//histogram "counter" for xxC

            }

            MyGeom->destroy_iterator(Neigh);    // We make sure that the iterator will be removed.

            testowanie++;						// Counts the number of randomly selected agents

            if(UseSelf) //Adding your own forces to counters in tables
            {
                Wplywy[CenterAgent.First][CenterAgent.Second][CenterAgent.Third]+=3*CenterAgent.Power;

                Wplywy[BIAS_FOR_ANY][CenterAgent.Second][CenterAgent.Third]+=2*CenterAgent.Power;
                Wplywy[CenterAgent.First][BIAS_FOR_ANY][CenterAgent.Third]+=2*CenterAgent.Power;
                Wplywy[CenterAgent.First][CenterAgent.Second][BIAS_FOR_ANY]+=2*CenterAgent.Power;

                Wplywy[CenterAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=CenterAgent.Power;
                Wplywy[BIAS_FOR_ANY][CenterAgent.Second][BIAS_FOR_ANY]+=CenterAgent.Power;
                Wplywy[BIAS_FOR_ANY][BIAS_FOR_ANY][CenterAgent.Third]+=CenterAgent.Power;
            }

            //Adding noise and bias in the loop:
            //----------------------------------
            for(int i=0,width=(BIAS_FOR_ANY+1)*(BIAS_FOR_ANY+1)*(BIAS_FOR_ANY+1);i<width;i++)
            {
                // TODO Nie ma BiasData, nie wiem dlaczego
                // TODO ((int*)Wplywy)[i]+=long(DRAND()*Noise*(4.5*MaxStrength))+((float*)BiasData->Biases)[i];//cast!!! - sztuczka zeby uniknac potrojnie zagniezdzonej petli
            }

            // Searching for maxima - less trivial here:
            ////////////////////////////////////////////

            /// Array of statistics from the maximum search loop.
            wb_dynarray<int> FillStat(4);fill(FillStat,0);

            int indF=-1;
            int indS=-1;
            int indT=-1;

            do{	// Loop of searching for subsequent maxima - to fill ind{FST}'s:
                ////////////////////////////////////////////////////////////////
                int width=BIAS_FOR_ANY+1;		///< "Width" of the cube array for counters.
                int offsetA=RANDOM(NumOfCate);			assert(0 <= offsetA && offsetA < NumOfCate);
                int offsetB=RANDOM(NumOfCate);			assert(0 <= offsetB && offsetB < NumOfCate);
                int offsetC=RANDOM(NumOfCate);			assert(0 <= offsetC && offsetC < NumOfCate);

                int Max=-1,pA=-1,pB=-1,pC=-1;
                FillStat[0]++;  //Relapse counting

                //Searching for the current maximum:
                //(a bit wasteful, you can speed it up a bit if BIAS_FOR_ANY is a variable == NumOfCate) (???)
                for(int i=0;i<width;i++)
                {
                    int a=(i+offsetA)%width;													assert(a>=0 && a<width);
                    for(int j=0;j<width;j++)
                    {
                        int b=(j+offsetB)%width;												assert(b>=0 && b<width);
                        for(int k=0;k<width;k++)
                        {
                            int c=(k+offsetC)%width;											assert(c>=0 && c<width);

                            int pom=Wplywy[a][b][c];
                            if(pom>Max)
                            {
                                Max=pom;
                                pA=a;pB=b;pC=c;			//Remembering where the maximum was found.
                            }
                        }
                    }
                }
                                                                                     //He had to find something (?)
                                                                                     assert(pA!=-1 && pB!=-1 && pC!=-1);

                //What to do with the maximum???
                Wplywy[pA][pB][pC]=0; //Reset it to zero so that it doesn't mess up in your next search!

                //Storing memes to change - only when the slot is still free.
                if(pA!=BIAS_FOR_ANY && indF==-1)
                    indF=pA;
                if(pB!=BIAS_FOR_ANY && indS==-1)
                    indS=pB;
                if(pC!=BIAS_FOR_ANY && indT==-1)
                    indT=pC;
                FillStat[(indF!=-1)+(indS!=-1)+(indT!=-1)]++;
            }while( indF==-1 || indS==-1 || indT==-1  );

                                                    assert(indF!=-1 && indS!=-1 && indT!=-1);//Po wyjsciu z petli wszystkie musza juz byc ustawione

            CenterAgent.First=asserted<short>(indF);		//zmieniamy w agencie centralnym
            CenterAgent.Second=asserted<short>(indS);		//zmieniamy w agencie centralnym
            CenterAgent.Third=asserted<short>(indT);		//zmieniamy w agencie centralnym

            //cout<<FillStat[0]<<'='<<FillStat[1]<<'+'<<FillStat[2]<<'+'<<FillStat[3]<<flush<<endl; //Print out the loop recurrence statistics
        }//END OF STATE CHANGES

STARZENIE:
        if(jagent::pow_move) //Strength as age
        {
            CenterAgent.Power=asserted<short>((static_cast<int>(CenterAgent.Power)
                             +jagent::pow_move) % jagent::max_pow); //Never exceeds maximum force
        }
    }

    // make sure the iterator is removed:
    MyGeom->destroy_iterator(Monte);
} //CONDITIONAL BIAS IMPLEMENTATION ENDS HERE.

/* **************************************************************** */
/*           THIS CODE IS DESIGNED & COPYRIGHT BY:                  */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zakład Systematyki i Geografii Roślin Uniwersytetu Warszawskiego */
/*  & Instytut Studiów Społecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */

