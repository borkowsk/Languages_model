/// @file
/// @brief ALTERNATIVE CONDITIONAL BIAS SIMULATION STEP IMPLEMENTATION (LANGUAGES PROJECT WITH P.Culicover)
/// @date 2026-05-20 (created)
///     Split from "jbias.cpp" by borkowsk on 14.04.2026.
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include <limits.h>
//#include <assert.h>
//#include <string.h>
//#include <math.h>
#include <cstring>
#include <cmath>
#include <strstream>

//#include "compatyb.h"
//#include "compatyb.hpp"
//#include "histosou.hpp"
//#include "clstsour.hpp"
#include "coincsou.hpp"
#include "gadgets.hpp"
#include "wb_ptrio.h"

#include "jrand.h"
#include "jworld.h"
#include "asserted.h"

using namespace sym2::data;
using namespace sym2::shell;
using namespace sym2::visual;

// ...
void    jworld::_one_step_conditional_bias1()
{
    // Was there anything here before???
                                                                                            assert(BiasDefinition.OK());
    /// We want to use the specifics of this type directly, not just a common interface.
    _conditional_bias_information* BiasData=dynamic_cast<_conditional_bias_information*>
                                                            (BiasDefinition.get_ptr_val());      assert(BiasData!=NULL);
    /// Shortcut to the geometry of the simulation world.
    const geometry* MyGeom=Agenci.get_geometry();                                             assert(MyGeom != NULL);

    /// THREE-DIMENSIONAL TABLE FOR COUNTING INFLUENCES.
    /// Number of allowable categories in each meme + positions for single biases and double combinations.
    int Wplywy[BIAS_FOR_ANY+1][BIAS_FOR_ANY+1][BIAS_FOR_ANY+1];                           assert(NumOfCate <= BIAS_FOR_ANY); //Aren't there too many categories for such an influence board?

    /// Monte-Carlo iterator (may have data allocations inside).
    iterator_h Monte=MyGeom->make_random_global_iterator();
    int testowanie=0; ///< An auxiliary counter for testing the algorithm.

    //We go through the agents with a Monte-Carlo iterator. Some may be drawn again.
    while(Monte)
    {
        size_t index=MyGeom->get_next(Monte); //We obtain the index of a randomly selected agent
        //if(index==FULL) continue;
                                                                                            assert(index!=MyGeom->FULL);
        /// Reference to the agent. Obtained bypassing the NULL assertion.
        jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());
        if(Agenci.is_empty(CenterAgent)) // Check if it is not an empty cell (NULL)
                continue;

        if(
                (CenterAgent.Power > TrsStrength) // Is there no immunity to change anymore?
            ||
                (jagent::mutation_level > 0 && CenterAgent.try_mutate()) //Or didn't it just mutate spontaneously
          )
            goto STARZENIE;

        {   // INFLUENCE CALCULATION CODE:
            // ////////////////////////////
            iterator_h Neigh=MyGeom->make_random_neighbour_iterator(index, NeighRadius, NeighDens); // We allocate the neighborhood iterator
            unsigned zliczanie=0;           //For counting neighbors

            //The counter table need to be reset.
            //You have to zero the whole thing, even if you don't use all of it - because there are columns for BIAS_FOR_ANY.
            memset(Wplywy,0,sizeof(Wplywy));

            if(use_SW_links) //RECORDING IMPACT FROM THE PROTECTOR
            {
                size_t a,b;
                unsigned x,y;
                dynamic_cast<const rectangle_geometry*>(MyGeom)->WhatCoordinates(index, a, b); //Retrieve x and y from the agent index
                    assert("Not tested after porting!"==nullptr);
                // if(_xy_of_far_link_of(0,TODO,x,y)) //Pobrać indeks "protektora" tego agenta  o ile go ma
                // {
                // 										assert((y!=UINT_MAX)&&(x!=UINT_MAX));
                // 	    jagent& PeryfAgent=Agenci.get(x,y);
                // 										assert(!Agenci.is_empty(PeryfAgent));
                // 										assert("NOT TESTED IPLEMENTATION"==NULL);
                // 	    Wplywy[PeryfAgent.First][PeryfAgent.Second][PeryfAgent.Third]+=3*PeryfAgent.Power; //"counter" for ABC coincidence
                //
                // 	    Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][PeryfAgent.Third]+=2*PeryfAgent.Power; //"counter" of the BxC histogram
                // 	    Wplywy[PeryfAgent.First][BIAS_FOR_ANY][PeryfAgent.Third]+=2*PeryfAgent.Power; //"counter" of the AxC histogram
                // 	    Wplywy[PeryfAgent.First][PeryfAgent.Second][BIAS_FOR_ANY]+=2*PeryfAgent.Power; //"counter" of the AxB histogram
                //
                // 	    Wplywy[PeryfAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=PeryfAgent.Power; //histogram "counter" for Axx
                // 	    Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][BIAS_FOR_ANY]+=PeryfAgent.Power; //histogram "counter" for xBx
                // 	    Wplywy[BIAS_FOR_ANY][BIAS_FOR_ANY][PeryfAgent.Third]+=PeryfAgent.Power; //histogram "counter" for xxC
                //
                // 	    zliczanie++;
                // }
            }

            while(Neigh) //Loop through the neighborhood.
            {
                size_t index2=MyGeom->get_next(Neigh); ///< We get the neighbor's index.
                if(index2==MyGeom->FULL || index2==index) //If it was outside the simulation area or in the center of the area, it would still be pointless.
                    continue;

                jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val()); ///< A reference to a neighbor bypassing NULL assertions
                if(Agenci.is_empty(PeryfAgent))     //We check whether it is not an empty cell (NULL) because then it would be pointless to continue.
                    continue;

                zliczanie++;                        //Counts the number of randomly selected neighbors.

                //Adding the forces of each neighbor to the counters in the tables:
                Wplywy[PeryfAgent.First][PeryfAgent.Second][PeryfAgent.Third]+=3*PeryfAgent.Power; //"counter" for ABC coincidence

                Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][PeryfAgent.Third]+=2*PeryfAgent.Power; //BxC histogram "counter".
                Wplywy[PeryfAgent.First][BIAS_FOR_ANY][PeryfAgent.Third]+=2*PeryfAgent.Power; //AxC histogram "counter".
                Wplywy[PeryfAgent.First][PeryfAgent.Second][BIAS_FOR_ANY]+=2*PeryfAgent.Power; //AxB histogram "counter".

                Wplywy[PeryfAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=PeryfAgent.Power; //histogram "counter" for Axx
                Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][BIAS_FOR_ANY]+=PeryfAgent.Power; //histogram "counter" for xBx
                Wplywy[BIAS_FOR_ANY][BIAS_FOR_ANY][PeryfAgent.Third]+=PeryfAgent.Power; //histogram "counter" for xxC
            }

            MyGeom->destroy_iterator(Neigh);    // We make sure that the iterator will be removed.

            testowanie++;                       // Counts the number of randomly selected agents

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
                if(Noise>0)
                {
                    double Rnd=DRAND();
                    if(Rnd>0)
                    ((int*)Wplywy)[i]+=asserted<long>(Rnd*Noise*(4.5 * MaxStrength)); // NOLINT(*-narrowing-conversions)
                }
                ((int*)Wplywy)[i]+=((float*)BiasData->Biases)[i]; //cast!!! - trick to avoid triple nested loop
            }

            // Searching for maxima - less trivial here:
            ////////////////////////////////////////////

            /// Array of statistics from the maximum search loop.
            wb_dynarray<int> FillStat(4); //TODO FillStat.fill();
            fill(FillStat,0);

            int indF=-1;
            int indS=-1;
            int indT=-1;

            do{ // Loop of searching for subsequent maxima - to fill ind{FST}'s:
                ////////////////////////////////////////////////////////////////
                int width=BIAS_FOR_ANY+1;    ///< "Width" of the cube array for counters.

                int offsetA=RANDOM(NumOfCate);            assert(0 <= offsetA && offsetA < NumOfCate);
                int offsetB=RANDOM(NumOfCate);            assert(0 <= offsetB && offsetB < NumOfCate);
                int offsetC=RANDOM(NumOfCate);            assert(0 <= offsetC && offsetC < NumOfCate);

                int Max=-1,pA=-1,pB=-1,pC=-1;
                FillStat[0]++; //Relapse counting

                //Searching for the current maximum:
                //(a bit wasteful, you can speed it up a bit if BIAS_FOR_ANY is a variable == NumOfCate) (???)
                for(int i=0;i<width;i++)
                {
                    int a=(i+offsetA)%width;            assert(a>=0 && a<width);
                    for(int j=0;j<width;j++)
                    {
                        int b=(j+offsetB)%width;        assert(b>=0 && b<width);
                        for(int k=0;k<width;k++)
                        {
                            int c=(k+offsetC)%width;    assert(c>=0 && c<width);

                            int pom=Wplywy[a][b][c];
                            if(pom>Max)
                            {
                                Max=pom;
                                pA=a;pB=b;pC=c;         //Remembering where the maximum was found.
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

            assert(indF!=-1 && indS!=-1 && indT!=-1); //After leaving the loop, they all need to be set.
                                                      //And like a lonely agent with no neighbors.
            //We change in the central agent:
            if(CenterAgent.First!=indF)
                { CenterAgent.First=asserted<short>(indF); CenterAgent.Age=0;}
            if(CenterAgent.Second!=indS)
                { CenterAgent.Second=asserted<short>(indS);CenterAgent.Age=0;}
            if(CenterAgent.Third!=indT)
                { CenterAgent.Third=asserted<short>(indT); CenterAgent.Age=0;}

            //cout<<FillStat[0]<<'='<<FillStat[1]<<'+'<<FillStat[2]<<'+'<<FillStat[3]<<flush<<endl; //Print out the loop recurrence statistics
        }

STARZENIE:
        if(jagent::pow_move) //Strength as age
        {
            CenterAgent.Power=asserted<short>((CenterAgent.Power+jagent::pow_move) % jagent::max_pow); //Never exceeds maximum force
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
