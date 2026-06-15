/// @file
/// @brief CONDITIONAL BIAS SIMULATION STEP IMPLEMENTATION (LANGUAGES PROJECT WITH P.Culicover)
/// @date 2026-06-15 (modified)
///       Created a long time ago.
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "jrand.h"
#include "jworld.h"

#include "gadgets.hpp"

#include <cstring>

using namespace sym2;
using namespace sym2::data;
using namespace sym2::shell;
using namespace sym2::visual;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
#pragma ide diagnostic ignored "modernize-use-auto"

void	jworld::_one_step_conditional_bias0()
{   /// Shortcut to the geometry of the simulation world.
    const geometry_base* MyGeom=Agents.get_geometry();													 assert(MyGeom);

    /// THREE-DIMENSIONAL TABLE FOR COUNTING INFLUENCES.
    /// Number of allowable categories in each meme plus positions for single biases and double combinations.
    int Influence[BIAS_FOR_ANY + 1][BIAS_FOR_ANY + 1][BIAS_FOR_ANY + 1];
                                                         //Aren't there too many categories for such an influence board?
                                                                                      assert(NumOfCate <= BIAS_FOR_ANY);

    /// Monte-Carlo iterator (may have data allocations inside).
    iterator_h Monte=MyGeom->make_random_global_iterator();
    int testing=0; ///< An auxiliary counter for testing the algorithm.

    //We go through the agents with a Monte-Carlo iterator. Some may be drawn again.
    while(Monte) // Loop through the neighborhood.
    {
        size_t index=MyGeom->get_next(Monte); //We get the index of a randomly selected agent.
        //--> if(index==any_layer_base::FULL) continue;
                                                                                    assert(index!=any_layer_base::FULL);
        /// Reference to the agent. Obtained bypassing the NULL assertion.
        jagent& CenterAgent=*(Agents.get_ptr(index).get_ptr_val());

        if(Agents.is_empty(CenterAgent)) // Check if it is not an empty cell (NULL)
                continue;

        if(CenterAgent.Power > TrsStrength)	// Is there no immunity to change any more?
            // TODO And why is there no possibility of mutation here?
                goto AGING;

        {	// INFLUENCE CALCULATION CODE:
            // ////////////////////////////
            iterator_h Neigh=MyGeom->make_random_neighbour_iterator(index, NeighRadius, NeighDens);	// We allocate a neighborhood iterator.
            unsigned counting=0;           //For counting neighbors

            //The table of counters needs to be reset.
            //You have to zero the whole thing, even if you don't use all of it - because there are columns for BIAS_FOR_ANY
            memset(Influence, 0, sizeof(Influence));
            // TODO And where is the influence from a distant link?
            while(Neigh)	//Loop through the neighborhood.
            {
                size_t index2=MyGeom->get_next(Neigh); ///< We get the neighbor's index.
                if(index2==any_layer_base::FULL || index2==index)	//If it was outside the simulation area or in the center of the area, it would still be pointless.
                    continue;

                jagent& OthAgent=*(Agents.get_ptr(index2).get_ptr_val()); ///< A reference to a neighbor bypassing NULL assertions.
                if(Agents.is_empty(OthAgent))		//We check whether it is not an empty cell (NULL),
                    continue;						//because then it would be pointless to continue.

                counting++;							//Counts the number of randomly selected neighbors.

                //Adding the forces of each neighbor to the counters in the tables:
                Influence[OthAgent.First][OthAgent.Second][OthAgent.Third]+= 3 * OthAgent.Power;	//"counter" for ABC coincidence

                Influence[BIAS_FOR_ANY][OthAgent.Second][OthAgent.Third]+= 2 * OthAgent.Power;	//BxC histogram "counter".
                Influence[OthAgent.First][BIAS_FOR_ANY][OthAgent.Third]+= 2 * OthAgent.Power;	//AxC histogram "counter".
                Influence[OthAgent.First][OthAgent.Second][BIAS_FOR_ANY]+= 2 * OthAgent.Power;	//AxB histogram "counter".

                Influence[OthAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=OthAgent.Power;	//histogram "counter" for Axx
                Influence[BIAS_FOR_ANY][OthAgent.Second][BIAS_FOR_ANY]+=OthAgent.Power;	//histogram "counter" for xBx
                Influence[BIAS_FOR_ANY][BIAS_FOR_ANY][OthAgent.Third]+=OthAgent.Power;	//histogram "counter" for xxC
            }

            MyGeom->destroy_iterator(Neigh);		// We make sure that the iterator will be removed.

            testing++;								// Counts the number of randomly selected agents

            if(UseSelf) //Adding your own forces to counters in tables
            {
                Influence[CenterAgent.First][CenterAgent.Second][CenterAgent.Third]+= 3 * CenterAgent.Power;

                Influence[BIAS_FOR_ANY][CenterAgent.Second][CenterAgent.Third]+= 2 * CenterAgent.Power;
                Influence[CenterAgent.First][BIAS_FOR_ANY][CenterAgent.Third]+= 2 * CenterAgent.Power;
                Influence[CenterAgent.First][CenterAgent.Second][BIAS_FOR_ANY]+= 2 * CenterAgent.Power;

                Influence[CenterAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=CenterAgent.Power;
                Influence[BIAS_FOR_ANY][CenterAgent.Second][BIAS_FOR_ANY]+=CenterAgent.Power;
                Influence[BIAS_FOR_ANY][BIAS_FOR_ANY][CenterAgent.Third]+=CenterAgent.Power;
            }

            //Adding noise and bias in the loop:
            //----------------------------------
            for(int i=0,width=(BIAS_FOR_ANY+1)*(BIAS_FOR_ANY+1)*(BIAS_FOR_ANY+1);i<width;i++)
            {
                // TODO There is no `BiasData`, I don't know why
                // TODO ((int*)Influence)[i]+=long(DRAND()*Noise*(4.5*MaxStrength))+((float*)BiasData->Biases)[i];
                // Why this cast??? Old info: "trick to avoid triple nested loop" (???)
//                if(Noise>0)
//                {
//                    double Rnd=DRAND();
//                    if(Rnd>0) //Using a cast, we change a three-dimensional array into a one-dimensional one.
//                        ((int*)Influence)[i]+=asserted<int>(Rnd * Noise * (4.5 * MaxStrength) ); // NOLINT(*-narrowing-conversions)
//                }
//                ((int*)Influence)[i]+=asserted<int>(((float*)BiasData->CndBiases)[i] ); //cast!!! - trick to avoid triple nested loop
            }

            // Searching for maxima - less trivial here:
            // //////////////////////////////////////////

            /// Array of statistics from the maximum search loop.
            wb_dynarray<int> FillStat(4);fill(FillStat,0);

            int indF=-1;
            int indS=-1;
            int indT=-1;

            do{	// Loop of searching for current maxima - to fill ind{FST}'s:
                // //////////////////////////////////////////////////////////
                int width=BIAS_FOR_ANY+1;									///< "Width" of the cube array for counters.
                int offsetA=RANDOM(NumOfCate);								assert(0 <= offsetA && offsetA < NumOfCate);
                int offsetB=RANDOM(NumOfCate);								assert(0 <= offsetB && offsetB < NumOfCate);
                int offsetC=RANDOM(NumOfCate);								assert(0 <= offsetC && offsetC < NumOfCate);

                int Max=-1,pA=-1,pB=-1,pC=-1;
                FillStat[0]++;  //Relapse counting (?)

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

                            int pom=Influence[a][b][c];
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
                Influence[pA][pB][pC]=0; //Reset it to zero so that it doesn't mess up in your next search!

                //Storing memes to change - only when the slot is still free.
                if(pA!=BIAS_FOR_ANY && indF==-1)
                    indF=pA;
                if(pB!=BIAS_FOR_ANY && indS==-1)
                    indS=pB;
                if(pC!=BIAS_FOR_ANY && indT==-1)
                    indT=pC;
                FillStat[(indF!=-1)+(indS!=-1)+(indT!=-1)]++;
            }while( indF==-1 || indS==-1 || indT==-1  );
                                                                      //After exiting the loop, all of them must be set!
                                                                               assert(indF!=-1 && indS!=-1 && indT!=-1);
            // Now we change in the central agent.
            CenterAgent.First=asserted<short>(indF);
            CenterAgent.Second=asserted<short>(indS);
            CenterAgent.Third=asserted<short>(indT);
            //Print out the loop recurrence statistics
            //c out<<FillStat[0]<<'='<<FillStat[1]<<'+'<<FillStat[2]<<'+'<<FillStat[3]<<flush<<endl;
        }//END OF STATE CHANGES

AGING:
        if(jagent::pow_move) //Strength as age
        {
            CenterAgent.Power=asserted<short>((static_cast<int>(CenterAgent.Power)
                             +jagent::pow_move) % jagent::max_pow); //Never exceeds maximum force
        }
    }

    // make sure the iterator is removed:
    MyGeom->destroy_iterator(Monte);
} //CONDITIONAL BIAS IMPLEMENTATION ENDS HERE.

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

