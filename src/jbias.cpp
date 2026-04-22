/// @file
/// @brief ... (LANGUAGES PROJECT WITH P.Culicover)
//  ===============================================
/// @date 2026-04-22 (modified)
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#include <limits.h>
//#include <assert.h>
//#include <string.h>
//#include <math.h>
#include <cstring>
#include <cmath>
#include <strstream>

#include "compatyb.h"
#include "histosou.hpp"
//#include "clstsour.hpp"
#include "coincsou.hpp"
#include "compatyb.hpp"
#include "gadgets.hpp"
#include "wb_ptrio.h"

#include "jrand.h"
#include "jworld.h"

/// The value that represents "all-the-same" in conditional biases.
/// Always greater than the largest value in the layer. Default `==8`. Announced in the header.
const int BIAS_FOR_ANY=8;

void jworld::set_bias_from_str(const char* lst)
//Setting additional simulation parameters from text.
//Np.: NO_BIAS=0,SIMPLE_BIAS=1,CONDITIONAL_BIAS=2,SEQUENTIONAL_BIAS=3,INVALID_BIAS_MODE=4.
{
    if(*lst=='\0')	//If the list is empty, just clear it.
    {
        if(BiasDefinition.OK())
            BiasDefinition->clean();
        return;
    }
    else
    if(strchr(lst,'?')!=NULL)
    {
        cout<<"ASSUMED SEQUENTIAL BIAS\n"<<flush;
        BiasMode= SEQUENTIONAL_BIAS;
        BiasDefinition=new _sequentional_bias_information(&IleKate);
    }
    else
    if(strchr(lst,'&')!=NULL)
    {
        cout<<"ASSUMED CONDITIONAL BIAS\n"<<flush;
        BiasMode= CONDITIONAL_BIAS;
        BiasDefinition=new _conditional_bias_information(&IleKate);
    }
    else
    if(strchr(lst,':')!=NULL)
    {
        cout<<"ASSUMED SIMPLE BIAS\n"<<flush;
        BiasMode= SIMPLE_BIAS;
        BiasDefinition=new _simple_bias_information(&IleKate);
    }
    else
    {
        cout<<"ASSUMED NO BIAS\n"<<flush;
        BiasMode= NO_BIAS;
        return;
    }

    wb_pchar src(lst);	///< Local copy of `lst`
    istrstream in(src.get_ptr_val());
    _read_bias_from_stream(in); 
}


void jworld::_read_bias_from_stream(istream& in)
{
    cout<<"PARSING OF BIAS DEFINITIONS: "<<flush;           assert(BiasDefinition.OK());
    
    do
    {
        switch(BiasDefinition->read_one_bias_item(in)) {
        case EOF:goto OK;
        case -10:goto ON_ERROR;
        case -11:goto INVALID_PARAM;
        case 0:
        default:
            break; //Do nothing!
        }
    }while(true);

OK:
    cout<<endl<<flush;
    return;
ON_ERROR:
    cerr<<"Syntax error on bias definition!"<<endl;
    cerr<<"\aALL BIAS DEFINITIONS WILL BE IGNORED!"<<endl;
    cerr<<"Syntax example:  A1:4 A2&B1:1 A1&B1&C2:3 B2:1"<<endl;
    return;
INVALID_PARAM:
    cerr<<"Invalid parameters in bias string!"<<endl;
    cerr<<"\aALL BIAS DEFINITIONS WILL BE IGNORED!"<<endl;
    //return;
}

// For pairs like a1, b3 etc.
int jworld::_read_local(istream& in,int& Layer,int& Value)
{
    char Znak;
    int  Numer;

    in>>Znak;
    if(in.fail())
        return -10;

    in>>Numer;
    if(in.fail())
        return -10;
    
    Znak=tolower(Znak);	//Unification of letter size
    
    switch(Znak){   //Conversion from f[irst], s[econd], t[hird]
    case 'f':
            Znak='a';break;
    case 's':
            Znak='b';break;
    case 't':
            Znak='c';break;
    default:break;
    }

    Layer=Znak-'a';
    Value=Numer-1;
    return 0;
}


/// Simple bias info.
struct BiasInfo
{  
    int Warstwa[3];	///< Value for each of the three layers.

    /// Constructor. The value for each of the three layers is initialized in the old style.
    BiasInfo(){Warstwa[0]=Warstwa[1]=Warstwa[2]=BIAS_FOR_ANY;} // NOLINT(*-pro-type-member-init)

    /// Sets the value for the layer, but only if it is the first time, otherwise it signals an error.
    int reg(int Index,int Wartosc)
    {
        if(Index>=0 && Index<=2)
        {
            if(Warstwa[Index]==BIAS_FOR_ANY)
            {
                Warstwa[Index]=Wartosc;
                return 0;
            }
            else
            {
                cerr<<"Second definition for layer "<<asserted<char>('A' + Index)<<". ";
                return -11;
            }
        }
        else
        {
            cerr<<"Invalid layer "<<asserted<char>('A' + Index)<<". ";
            return -11;
        }
    }

    friend 
        ostream& operator << (ostream& o,const BiasInfo& b)
    {
        bool bylo=false;
        if(b.Warstwa[0]!=BIAS_FOR_ANY)
        {
            o<<'A'<<b.Warstwa[0]+1;
            bylo=true;
        }
        if(b.Warstwa[1]!=BIAS_FOR_ANY)
        {
            if(bylo) o<<'&';
            o<<'B'<<b.Warstwa[1]+1;
            bylo=true;
        }
        if(b.Warstwa[2]!=BIAS_FOR_ANY)
        {
            if(bylo) o<<'&';
            o<<'C'<<b.Warstwa[2]+1;
            bylo=true;
        }
        return o;
    }   

};


int  jworld::_simple_bias_information::read_one_bias_item(istream& in)
{
    BiasInfo info;
    char    Separator;
    int     Index,Number;
    double  BiasValue;

    cout<<endl<<flush;

    int ret=_read_local(in,Index,Number);   
    if(ret!=0) 
        return ret;
    ret=info.reg(Index,Number);
    if(ret!=0)
        return ret;
    
    in>>Separator;
    if(in.fail())
        return -10;

    if(Separator!=':') //Requires `:` as separator.
    {
        cerr<<"Expected ':' not "<<Separator<<endl;
        return -10;
    }

    in>>BiasValue;
    if(in.fail())
        return -10;
    
    cout<<info<<':'<<BiasValue<<' '<<flush;	//Echo
    
    if( (info.Warstwa[0]<0 || info.Warstwa[0]>=IleKate()) && info.Warstwa[0]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[0]+1<<" for layer A are invalid"<<endl;return -11;}
    if( (info.Warstwa[1]<0 || info.Warstwa[1]>=IleKate()) && info.Warstwa[1]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[1]+1<<" for layer B are invalid"<<endl;return -11;}
    if( (info.Warstwa[2]<0 || info.Warstwa[2]>=IleKate()) && info.Warstwa[2]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[2]+1<<" for layer C are invalid"<<endl;return -11;}

    //If there are two BIAS_FOR_ANY it is unconditional bias
    assert((info.Warstwa[0]==BIAS_FOR_ANY)+(info.Warstwa[1]==BIAS_FOR_ANY)+(info.Warstwa[2]==BIAS_FOR_ANY)==2);
    
    {//Now write to the conditional bias table `UncdBias[3][8]`.
        if(info.Warstwa[0]!=BIAS_FOR_ANY)
        {
            UncdBias[0][info.Warstwa[0]]=BiasValue;
        }
        else 
            if(info.Warstwa[1]!=BIAS_FOR_ANY)
            {
                UncdBias[1][info.Warstwa[1]]=BiasValue;
            }
            else
                if(info.Warstwa[2]!=BIAS_FOR_ANY)
                {
                    UncdBias[2][info.Warstwa[2]]=BiasValue;
                }
                else
                {
                    assert("This line should never be reached."==NULL);	//Upss...
                }
    }
    
    //Can we continue next time?
    eat_blanks(in);
    int test=in.get();
    if(test!=0 && test!=-1)
    {
        in.putback(test);
        return 0;
    }
    else
        return EOF;
}


int  jworld::_conditional_bias_information::read_one_bias_item(istream& in)
{
    BiasInfo info;
    char   Separator;
    int    Index,Number;
    double BiasValue;

    cout<<endl<<flush;

    int ret=_read_local(in,Index,Number);   
    if(ret!=0) 
        return ret;
    ret=info.reg(Index,Number);
    if(ret!=0)
        return ret;
    
    in>>Separator;
    if(in.fail())
        return -10;

    if(Separator=='&') //There is one more
    {
        ret=_read_local(in,Index,Number);   
        if(ret!=0) 
            return ret;
        
        ret=info.reg(Index,Number);
        if(ret!=0)
            return ret;
        
        in>>Separator;
        if(in.fail())
            return -10;

        if(Separator=='&') //Oh! And one more.
        {
            ret=_read_local(in,Index,Number);   
            if(ret!=0) 
                return ret;

            ret=info.reg(Index,Number);
            if(ret!=0)
                return ret;
            
            in>>Separator;
            if(in.fail())
                return -10;
        }
    }

    if(Separator!=':')
    {
        cerr<<"Expected ':' not "<<Separator<<endl;
        return -10;
    }

    in>>BiasValue;
    if(in.fail())
        return -10;
    
    cout<<info<<':'<<BiasValue<<' '<<flush;	//Echo
    
    if( (info.Warstwa[0]<0 || info.Warstwa[0]>=IleKate()) && info.Warstwa[0]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[0]+1<<" for layer A are invalid"<<endl;return -11;}
    if( (info.Warstwa[1]<0 || info.Warstwa[1]>=IleKate()) && info.Warstwa[1]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[1]+1<<" for layer B are invalid"<<endl;return -11;}
    if( (info.Warstwa[2]<0 || info.Warstwa[2]>=IleKate()) && info.Warstwa[2]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[2]+1<<" for layer C are invalid"<<endl;return -11;}
    
    Biases[info.Warstwa[0]][info.Warstwa[1]][info.Warstwa[2]]=BiasValue;	//All possible biases (???)
    
    //Can we continue next time?
    eat_blanks(in);

    int test=in.get();
    if(test!=0 && test!=-1)
    {
        in.putback(test);
        return 0;
    }
    else
        return EOF;
}


int jworld::_sequentional_bias_information::IfBias::reg(int Index,int Wartosc)
{                
    if(Index>=0 && Index<=2)
    {
        if(leyer[Index]==BIAS_FOR_ANY)
        {
            leyer[Index]=Wartosc;
            return 0;
        }       
        else
        {
            cerr<<"Second definition for layer "<<asserted<char>('A'+Index)<<". ";
            return -11;
        }
    }
    else
    {
        cerr<<"Invalid layer "<<asserted<char>('A'+Index)<<". ";
        return -11;
    }
}


int jworld::_sequentional_bias_information::IfBias::set(int Index,int Wartosc,float Premia)
{
    if(Index>=0 && Index<=2)
    {
        if(whatley==BIAS_FOR_ANY)
        {
            whatley=Index;
            lstate=Wartosc;
            value=Premia;
            return 0;
        }
        else
        {
            cerr<<"Second definition for target layer "<<asserted<char>('A'+Index)<<". ";
            return -11;
        }
    }
    else
    {
        cerr<<"Invalid target layer "<<asserted<char>('A'+Index)<<". ";
        return -11;
    }
    
}


ostream& operator << (ostream& o,const jworld::_sequentional_bias_information::IfBias& b)
{
    bool bylo=false;
    if(b.leyer[0]!=BIAS_FOR_ANY)
    {
        o<<'A'<<b.leyer[0]+1;
        bylo=true;
    }
    if(b.leyer[1]!=BIAS_FOR_ANY)
    {
        if(bylo) o<<'&';
        o<<'B'<<b.leyer[1]+1;
        bylo=true;
    }
    if(b.leyer[2]!=BIAS_FOR_ANY)
    {
        if(bylo) o<<'&';
        o<<'C'<<b.leyer[2]+1;
        bylo=true;
    }
    if(bylo)
        o<<'?';
    o<<asserted<char>('A'+b.whatley)<<b.lstate+1<<':'<<b.value;
    return o;
}   


int  jworld::_sequentional_bias_information::read_one_bias_item(istream& in)
{
    char Separator;
    int    Index,Number;
    double BiasValue;
    IfBias& Item=SeqBiases[use_next_item()];
    
    cout<<endl<<flush;
    
    int ret=_read_local(in,Index,Number);   
    if(ret!=0) 
        return ret;
    
    in>>Separator;	//You need to load the separator to decide.
    if(in.fail())
        return -10;
    
    if(Separator!=':') //If it is not unconditional, it is conditional sequential - i.e. OK
    {
        assert(Separator=='&'||Separator=='?');                                                    
        ret=Item.reg(Index,Number);	//First registered.
        if(ret!=0)
            return ret;
        
        if(Separator=='&') //But one more...
        {              
            ret=_read_local(in,Index,Number);   
            if(ret!=0) 
                return ret;
            
            ret=Item.reg(Index,Number);
            if(ret!=0)
                return ret;
            
            in>>Separator;
            if(in.fail())
                return -10;
            
            if(Separator=='&') //And even one more!
            {
                ret=_read_local(in,Index,Number);   
                if(ret!=0) 
                    return ret;
                
                ret=Item.reg(Index,Number);
                if(ret!=0)
                    return ret;
                
                in>>Separator;
                if(in.fail())
                    return -10;
            }                       
        }
        
        if(Separator!='?')
        {
            cerr<<"Expected '?' not "<<Separator<<endl;
            return -10;
        }
        
        ret=_read_local(in,Index,Number);   
        if(ret!=0) 
            return ret;    
        
        in>>Separator;
        if(in.fail())
            return -10;
        
        if(Separator!=':')
        {
            cerr<<"Expected ':' not "<<Separator<<endl;
            return -10;
        }
    }
    
    in>>BiasValue;
    if(in.fail())
        return -10;
    
    ret=Item.set(Index,Number,BiasValue);
    if(ret!=0)
        return ret;
    
    cout<<Item<<' '<<flush;	//Echo
    
    //Validation test
    short IleK=IleKate();
    if( !(
        Item.IsOK() &&
        (0<=Item.lstate) && (Item.lstate<IleK) &&
        (Item.leyer[0]==BIAS_FOR_ANY || (0<=Item.leyer[0] && Item.leyer[0]<IleK)) &&
        (Item.leyer[1]==BIAS_FOR_ANY || (0<=Item.leyer[1] && Item.leyer[1]<IleK)) &&
        (Item.leyer[2]==BIAS_FOR_ANY || (0<=Item.leyer[2] && Item.leyer[2]<IleK)) 
        ) )
    {
        cerr<<"Invalid value - probably the number of categories exceed.\n";
        return -11;
    }
    
    //Can we continue next time?
    eat_blanks(in);
    int test=in.get();
    if(test!=0 && test!=-1)
    {
        in.putback(test);
        return 0;
    }
    else
        return EOF;
}

//          IMPLEMENTATION OF SIMULATION STEPS WITH DIFFERENT BIAS
// /////////////////////////////////////////////////////////////////////////

void    jworld::_one_step_no_bias()
{
    int testowanie=0;
    const geometry_base* MyGeom=Agenci.get_geometry();
    //AUXILIARY BOARDS:
    wb_dynarray<int> Firsts(IleKate);
    wb_dynarray<int> Seconds(IleKate);
    wb_dynarray<int> Thirds(IleKate);
    assert(MyGeom && Firsts.IsOK() && Seconds.IsOK() && Thirds.IsOK());
    
    //We allocate the Monte-Carlo iterator:
    iteratorh Monte=MyGeom->make_random_global_iterator();
    
    //We go through the agents with a Monte-Carlo iterator. Some may repeat themselves.
    while(Monte)
    {   
        size_t index=MyGeom->get_next(Monte);	//We obtain the index of a randomly selected agent
        //if(index==FULL)  //We ignore it if it goes behind the board (which happens with clippings, or happened in the past)
        //      continue;                        
        assert(index!=MyGeom->FULL);               //This definitely shouldn't happen here!
        jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());	//We obtain references to the agent by bypassing NULL assertions.
        if(Agenci.is_empty(CenterAgent))    // We check whether it is not an empty cell (NULL?).
            continue;
        
        if(
            (CenterAgent.Power>TrsSila)   //Is there no immunity to change anymore?
            ||
            (jagent::MutationLevel>0 && CenterAgent.try_mutate()) //Or didn't it just mutate spontaneously!
            )
            goto STARZENIE;             // It has immunity - we do nothing more than age.
        
        {   // LOOKING FOR INFLUENCES:
            // ////////////////////////

            /// We allocate the neighborhood iterator...
            iteratorh Neigh=MyGeom->make_random_neighbour_iterator(index,OdlSasiad,IleSasiad);
            //iteratorh Neigh=MyGeom->make_neighbour_iterator(index,OdlSasiad); //A simpler alternative (non random).
            unsigned zliczanie=0;	///< To count real neighbors.
            
            // Cleaning the counter arrays:
            fill(Firsts,0); //Former: `memset(Firsts.get_ptr_val(),0,sizeof(int)*IleKate);`
            fill(Seconds,0);	//memset(Seconds.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(Thirds,0); //memset(Thirds.get_ptr_val(),0,sizeof(int)*IleKate);

            if(use_SW_links)
            { //RECORDING INFLUENCE FROM THE PROTECTOR ON A FAR LINK:
                size_t a,b;
                unsigned x,y;
                dynamic_cast<const rectangle_geometry*>(MyGeom)->WhatCoordinates(index,a,b); //You need to retrieve x and y from the agent index.
                    assert("NOT TESTED AFTER PORTING!"==nullptr);
                // if(_xy_of_far_link_of(a,b,x,y)) //Get the "protector" index of this agent, if it has one (?????)
                // 	{
                // 										assert((y!=UINT_MAX)&&(x!=UINT_MAX));
                // 	jagent& PeryfAgent=Agenci.get(x,y); //We obtain references to the "protector".
                // 										assert(!Agenci.is_empty(PeryfAgent));
                // 	Firsts[PeryfAgent.First]+=PeryfAgent.Power;
                // 	Seconds[PeryfAgent.Second]+=PeryfAgent.Power;
                // 	Thirds[PeryfAgent.Third]+=PeryfAgent.Power;
                // 	zliczanie++; //He's also kind of a neighbor.
                // 	}
            }

            // ORDINARY CLOSE NEIGHBORS (extended Moore):
            while(Neigh) //Let's check if the iterator still has any neighboring agent to provide.
            {
                size_t index2=MyGeom->get_next(Neigh); //We obtain the neighbor's index from geometry.
                if(index2==MyGeom->FULL || index2==index)   //If it was outside the simulation area or in the center of the area, it would still be pointless.
                    continue; //TODO But does it happen?

                jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val()); //We obtain a reference to the neighbor by bypassing NULL assertions
                if(Agenci.is_empty(PeryfAgent)) //We check whether it is not an empty cell.
                    continue;

                zliczanie++; //This is a real neighbor, not an empty field.

                //Adding neighbor forces to counters in tables:
                Firsts[PeryfAgent.First]+=PeryfAgent.Power;
                Seconds[PeryfAgent.Second]+=PeryfAgent.Power;
                Thirds[PeryfAgent.Third]+=PeryfAgent.Power;
            }

            //We make sure that the iterator that is no longer needed will be removed:
            MyGeom->destroy_iterator(Neigh);
            testowanie++;   //We count the number of randomly selected agents.
            
            //Adding your own strength to counters in tables, but only when this option is active.
            if(UseSelf)
            {
                Firsts[CenterAgent.First]+=CenterAgent.Power;
                Seconds[CenterAgent.Second]+=CenterAgent.Power;
                Thirds[CenterAgent.Third]+=CenterAgent.Power;
            }
            
            //Searching for influence maxima:
            //-------------------------------
            int maxF=0,indF=-1;
            int maxS=0,indS=-1;
            int maxT=0,indT=-1;

            //Multiple influences can be maximum (same value).
            //Therefore, we must ensure that among them there will be a random selection.
            //Sometimes RANDOM returns IleKate instead of IleKate-1.
            const int offset=RANDOM(IleKate)%IleKate;			//If `IleKate==2` it comes out 0 or 1 etc...
                                                                assert(0<=offset);
                                                                assert(offset<IleKate);

            //In this loop, adding noise and searching for maxima:
            //----------------------------------------------------
            for(int g=0;g<IleKate;g++)
            {
                /// Index with random shift:
                int h=(g+offset)%IleKate;						assert(h>=0 && h<IleKate);

                //Now it's time to add noise:
                if(Firsts[h]>0)
                    Firsts[h]+=asserted<int>(DRAND()*Noise*(4.5*MaxSila));
                if(Seconds[h]>0)
                    Seconds[h]+=asserted<int>(DRAND()*Noise*(4.5*MaxSila));
                if(Thirds[h]>0)
                    Thirds[h]+=asserted<int>(DRAND()*Noise*(4.5*MaxSila));
                
                //Testing if we have any new high values.
                if(Firsts[h]>maxF)
                    {maxF=Firsts[h];indF=h;}
                if(Seconds[h]>maxS)
                    {maxS=Seconds[h];indS=h;}
                if(Thirds[h]>maxT)
                    {maxT=Thirds[h];indT=h;}
            }

            //assert(indF!=-1 && indS!=-1 && indT!=-1); //Are we sure that a maximum has always been found? What if it's empty around?

            //We change the value in the central agent to the same value in the one that had the maximum.
            if(indF!=-1 && CenterAgent.First!=indF)
                { CenterAgent.First=indF; CenterAgent.Age=0;}
            if(indS!=-1 && CenterAgent.Second!=indS)
                { CenterAgent.Second=indS;CenterAgent.Age=0;}
            if(indT!=-1 && CenterAgent.Third!=indT)
                { CenterAgent.Third=indT; CenterAgent.Age=0;}
            
        } //END OF NORMAL STATE CHANGE

STARZENIE:
        if(jagent::ruchsily) //Strength as age. Incrementation, if this option is activated.
        {
            CenterAgent.Power+=jagent::ruchsily;
            CenterAgent.Power%=jagent::max_sila; //Never exceeds maximum force. This is where a "new", weak agent (child) is created.
        }
    }

    // We make sure that the main iterator M-C is removed
    MyGeom->destroy_iterator(Monte);
}  //NO BIAS IMPLEMENTATION ENDED HERE.


void    jworld::_one_step_simple_bias()
{                                                                                            assert(BiasDefinition.OK());
    _simple_bias_information* BiasData=dynamic_cast<_simple_bias_information*>
                                                            (BiasDefinition.get_ptr_val());  assert(BiasData!=NULL);
    const geometry_base* MyGeom=Agenci.get_geometry();                                       assert(MyGeom!=NULL);

    //AUXILIARY BOARDS OF COUNTERS:
    wb_dynarray<int> Firsts(IleKate);
    wb_dynarray<int> Seconds(IleKate);
    wb_dynarray<int> Thirds(IleKate);               assert(MyGeom && Firsts.IsOK() && Seconds.IsOK() && Thirds.IsOK());
    
    //OTHER AUXILIARY VARIABLES:
    int testowanie=0; ///< counter.
    iteratorh Monte=MyGeom->make_random_global_iterator(); ///< Monte-Carlo iterator (internal allocated)
    
    //We go through the agents with a Monte-Carlo iterator (some repetitions possible).
    while(Monte)
    {   
        size_t index=MyGeom->get_next(Monte);
        //if(index==FULL)
        //      continue;
        assert(index!=MyGeom->FULL);
        jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());
        if(Agenci.is_empty(CenterAgent))
            continue;

         if(
            (CenterAgent.Power>TrsSila)   //Is there no immunity to change anymore?
            ||
            (jagent::MutationLevel>0 && CenterAgent.try_mutate())
            )
            goto STARZENIE; //There is immunity or a fresh mutation - nothing to do.

        {   // INFLUENCE CALCULATION CODE:
            // ///////////////////////////

            /// Neighborhood iterator (with internal allocation perhaps!).
            iteratorh Neigh=MyGeom->make_random_neighbour_iterator(index,OdlSasiad,IleSasiad);
            //iteratorh Neigh=MyGeom->make_neighbour_iterator(index,OdlSasiad); ///< Non-random neighborhood iterator.
            unsigned zliczanie=0; ///< To count real neighbors.

            // Cleaning the counter arrays:
            fill(Firsts,0); //memset(Firsts.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(Seconds,0);//memset(Seconds.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(Thirds,0); //memset(Thirds.get_ptr_val(),0,sizeof(int)*IleKate);

            if(use_SW_links)
            { //RECORDING INFLUENCE FROM THE PROTECTOR ON A FAR LINK:
                size_t a,b;
                unsigned x,y;
                dynamic_cast<const rectangle_geometry*>(MyGeom)->WhatCoordinates(index,a,b);
                if(_xy_of_far_link_of(a,b,x,y))
                {
                                                        assert((y!=UINT_MAX)&&(x!=UINT_MAX));
                    jagent& PeryfAgent=Agenci.get(x,y); ///< The references to "protector".
                                                        assert(!Agenci.is_empty(PeryfAgent));
                    //Adding protector strength to counters in arrays
                    Firsts[PeryfAgent.First]+=PeryfAgent.Power;
                    Seconds[PeryfAgent.Second]+=PeryfAgent.Power;
                    Thirds[PeryfAgent.Third]+=PeryfAgent.Power;

                    zliczanie++;
                }
            }

            // ORDINARY CLOSE NEIGHBORS (extended Moore):
            while(Neigh) //Let's check if the iterator still has any neighboring agent to provide.
            {
                size_t index2=MyGeom->get_next(Neigh);
                if(index2==MyGeom->FULL || index2==index)
                    continue;  //TODO But does it happen?

                jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val()); ///< reference to the neighbor
                if(Agenci.is_empty(PeryfAgent))
                    continue;

                zliczanie++; //This is a real neighbor, not an empty field.

                //Adding neighbor forces to counters in tables:
                Firsts[PeryfAgent.First]+=PeryfAgent.Power;
                Seconds[PeryfAgent.Second]+=PeryfAgent.Power;
                Thirds[PeryfAgent.Third]+=PeryfAgent.Power;
            }

            //We make sure that the iterator that is no longer needed will be removed:
            MyGeom->destroy_iterator(Neigh);
            testowanie++;   //We count the number of randomly selected agents.

            //Adding your own strength to counters in tables, but only when this option is active.
            if(UseSelf)
            {
                Firsts[CenterAgent.First]+=CenterAgent.Power;
                Seconds[CenterAgent.Second]+=CenterAgent.Power;
                Thirds[CenterAgent.Third]+=CenterAgent.Power;
            }

            //Searching for influence maxima:
            //-------------------------------
            int maxF=0,indF=-1;
            int maxS=0,indS=-1;
            int maxT=0,indT=-1;

            //Multiple influences can be maximum (same value).
            //Therefore, we must ensure that among them there will be a random selection.
            int offset=RANDOM(IleKate);             assert(0<=offset && offset<IleKate);

            //In this loop, adding noise and searching for maxima:
            //----------------------------------------------------
            for(int g=0;g<IleKate;g++)
            {
                int h=(g+offset)%IleKate; ///< Index with random shift:
                                          assert(h>=0 && h<IleKate);

                //Now it's time to add noise AND BIAS:
                if(Firsts[h]>0)
                    Firsts[h]+=asserted<int>(DRAND()*Noise*(4.5*MaxSila))+BiasData->UncdBias[0][h];
                if(Seconds[h]>0)
                    Seconds[h]+=asserted<int>(DRAND()*Noise*(4.5*MaxSila))+BiasData->UncdBias[1][h];
                if(Thirds[h]>0)
                    Thirds[h]+=asserted<int>(DRAND()*Noise*(4.5*MaxSila))+BiasData->UncdBias[2][h];

                //Testing if we have any new high values.
                if(Firsts[h]>maxF)
                {maxF=Firsts[h];indF=h;}
                if(Seconds[h]>maxS)
                {maxS=Seconds[h];indS=h;}
                if(Thirds[h]>maxT)
                {maxT=Thirds[h];indT=h;}
            }//assert(indF!=-1 && indS!=-1 && indT!=-1); //But, what if it's empty around?

            //We change the value in the central agent to the same value in the one that had the maximum.
            if(indF!=-1 && CenterAgent.First!=indF)
                { CenterAgent.First=indF; CenterAgent.Age=0;}
            if(indS!=-1 && CenterAgent.Second!=indS)
                { CenterAgent.Second=indS;CenterAgent.Age=0;}
            if(indT!=-1 && CenterAgent.Third!=indT)
                { CenterAgent.Third=indT; CenterAgent.Age=0;}

        } //END OF NORMAL STATE CHANGE

STARZENIE:
        if(jagent::ruchsily) //Strength as age. Incrementation, if this option is activated.
        {
            CenterAgent.Power+=jagent::ruchsily;
            CenterAgent.Power%=jagent::max_sila; //Never exceeds maximum force. This is where a "new", weak agent (child) is created.
        }
    }

    // We make sure that the main iterator M-C is removed
    MyGeom->destroy_iterator(Monte);
}  //SIMPLE BIAS IMPLEMENTATION ENDED HERE.


//Auxiliary implementation of the use of sophisticated bias.
//Quite complicated, it resembles the execution of a program
void jworld::_sequentional_bias_information::UseBiasForAgent(
                                int FirstVal,int SecondVal,int ThirdVal,
                                wb_dynarray<int>& Firsts,
                                wb_dynarray<int>& Seconds,
                                wb_dynarray<int>& Thirds
                             )
{
    for(int a=0;a<for_use;a++)	//For each record of the "conditional bias instruction" array.
    {
       if( SeqBiases[a].much(FirstVal,SecondVal,ThirdVal)) //If any condition is met...
       {
           // ...add bias to the correct state of the correct layer.
           switch(SeqBiases[a].whatley){ //TODO Why `float` is used for bias values?
           case 0: Firsts[SeqBiases[a].lstate]+=SeqBiases[a].value;break;
           case 1: Seconds[SeqBiases[a].lstate]+=SeqBiases[a].value;break;
           case 2: Thirds[SeqBiases[a].lstate]+=SeqBiases[a].value;break;
           default:
               assert("Invalid index of layer during bias processing."==NULL);
               break;
           }
       }
    }
}


void    jworld::_one_step_sequentional_bias()
{                                                                                               assert(BiasDefinition.OK());
    _sequentional_bias_information* BiasData=dynamic_cast<_sequentional_bias_information*>
                                                            (BiasDefinition.get_ptr_val());     assert(BiasData!=NULL);
    const geometry_base* MyGeom=Agenci.get_geometry();                                          assert(MyGeom!=NULL);

    //AUXILIARY BOARDS OF COUNTERS:
    wb_dynarray<int> CFirst(IleKate); //!< Counters for firsts opinions.
    wb_dynarray<int> CSecond(IleKate);
    wb_dynarray<int> CThird(IleKate);               assert(MyGeom && CFirst.IsOK() && CSecond.IsOK() && CThird.IsOK());

    //OTHER AUXILIARY VARIABLES:
    int testowanie=0; ///< counter.
    iteratorh Monte=MyGeom->make_random_global_iterator(); ///< Iterator Monte-Carlo.

    //We go through the agents with a Monte-Carlo iterator (some repetitions possible).
    while(Monte)
    {
        size_t index=MyGeom->get_next(Monte); ///< index of a randomly selected agent.
        //if(index==MyGeom->FULL)
        //      continue;
        assert(index!=MyGeom->FULL);
        jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());
        if(Agenci.is_empty(CenterAgent))
            continue;

         if(
            (CenterAgent.Power>TrsSila)    //Is there no immunity to change anymore?
            ||
            (jagent::MutationLevel>0 && CenterAgent.try_mutate()) //Or didn't it just mutate spontaneously?
            )
            goto STARZENIE;   //There is immunity or a fresh mutation - nothing to do.

        {   // INFLUENCE CALCULATION CODE:
            // ///////////////////////////

            /// Neighborhood iterator (with internal allocation perhaps!).
            iteratorh Neigh=MyGeom->make_random_neighbour_iterator(index,OdlSasiad,IleSasiad);
            //iteratorh Neigh=MyGeom->make_neighbour_iterator(index,OdlSasiad); ///< Non-random neighborhood iterator.
            unsigned zliczanie=0; ///< To count real neighbors.

            // Cleaning the counter arrays:
            fill(CFirst,0); //memset(Firsts.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(CSecond,0);//memset(Seconds.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(CThird,0); //memset(Thirds.get_ptr_val(),0,sizeof(int)*IleKate);

            if(use_SW_links)
            { //RECORDING INFLUENCE FROM THE PROTECTOR ON A FAR LINK:
            size_t a,b;
            unsigned x,y;
            dynamic_cast<const rectangle_geometry*>(MyGeom)->WhatCoordinates(index,a,b);
                assert("Not tested after porting!"==nullptr);
            // if(_xy_of_far_link_of(0,TODO,x,y)) //Download the "protector" index of this agent, if it has one
            // {
            // 										assert((y!=UINT_MAX)&&(x!=UINT_MAX));
            // 	jagent& PeryfAgent=Agenci.get(x,y); ///< The references to "protector".
            // 										assert(!Agenci.is_empty(PeryfAgent));
            // 										assert("NOT TESTED IPLEMENTATION");
            //
            // 	//Adding protector strength to counters in arrays
            // 	Firsts[PeryfAgent.First]+=PeryfAgent.Power;
            // 	Seconds[PeryfAgent.Second]+=PeryfAgent.Power;
            // 	Thirds[PeryfAgent.Third]+=PeryfAgent.Power;
            //
            // 	zliczanie++;
            // }
            }

            // ORDINARY CLOSE NEIGHBORS (extended Moore):
            while(Neigh) //Let's check if the iterator still has any neighboring agent to provide.
            {
                size_t index2=MyGeom->get_next(Neigh);
                if(index2==MyGeom->FULL || index2==index)
                    continue;

                jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val()); ///< reference to the neighbor
                if(Agenci.is_empty(PeryfAgent))
                    continue;

                zliczanie++; //This is a real neighbor, not an empty field.

                //Adding neighbor forces to counters in tables:
                CFirst[PeryfAgent.First]+=PeryfAgent.Power;
                CSecond[PeryfAgent.Second]+=PeryfAgent.Power;
                CThird[PeryfAgent.Third]+=PeryfAgent.Power;
            }

            //We make sure that the iterator that is no longer needed will be removed:
            MyGeom->destroy_iterator(Neigh);
            testowanie++;   //We count the number of randomly selected agents.

            //Adding your own strength to counters in tables, but only when this option is active.
            if(UseSelf)
            {
                CFirst[CenterAgent.First]+=CenterAgent.Power;
                CSecond[CenterAgent.Second]+=CenterAgent.Power;
                CThird[CenterAgent.Third]+=CenterAgent.Power;
            }

            //Searching for influence maxima:
            //-------------------------------
            int maxF=INT_MIN,indF=-1;
            int maxS=INT_MIN,indS=-1;
            int maxT=INT_MIN,indT=-1;

            //Multiple influences can be maximum (same value).
            //Therefore, we must ensure that among them there will be a random selection.
            int offset=RANDOM(IleKate);             assert(0<=offset && offset<IleKate);

            //Adding bias using the  procedure from `BiasData`.
            //When the agent's state matches the given one, bias is added to the appropriate array:
            //-------------------------------------------------------------------------------------
            BiasData->UseBiasForAgent(CenterAgent.First,CenterAgent.Second,CenterAgent.Third,
                                        CFirst,CSecond,CThird);


            //In one loop, adding noise and searching for maxima:
            //---------------------------------------------------
            for(int g=0;g<IleKate;g++)
            {
                int h=(g+offset)%IleKate;                     assert(h>=0 && h<IleKate);

                //Adding noise:
                if(Noise>0)
                {
                    if(CFirst[h]>0)
                        CFirst[h]+=asserted<int>(DRAND() * Noise * (4.5 * MaxSila));
                    if(CSecond[h]>0)
                        CSecond[h]+=asserted<int>(DRAND() * Noise * (4.5 * MaxSila));
                    if(CThird[h]>0)
                        CThird[h]+=asserted<int>(DRAND() * Noise * (4.5 * MaxSila));
                }

                //Testing to the max:
                if(CFirst[h]>maxF)
                {
                    maxF=CFirst[h]; indF=h;
                }
                if(CSecond[h]>maxS)
                {
                    maxS=CSecond[h]; indS=h;
                }
                if(CThird[h]>maxT)
                {
                    maxT=CThird[h]; indT=h;
                }
            }

            //assert(indF!=-1 && indS!=-1 && indT!=-1); //And how empty it is around?

            //We change the value in the central agent to the same value in the one that had the maximum.
            if(indF!=-1 && CenterAgent.First!=indF)
                { CenterAgent.First=asserted<short>(indF); CenterAgent.Age=0;}
            if(indS!=-1 && CenterAgent.Second!=indS)
                { CenterAgent.Second=asserted<short>(indS);CenterAgent.Age=0;}
            if(indT!=-1 && CenterAgent.Third!=indT)
                { CenterAgent.Third=asserted<short>(indT); CenterAgent.Age=0;}

        } //END OF NORMAL STATE CHANGE

STARZENIE:
        if(jagent::ruchsily) //Strength as age. Incrementation, if this option is activated.
        {
            CenterAgent.Power+=jagent::ruchsily;
            CenterAgent.Power%=jagent::max_sila; //Never exceeds maximum force. This is where a "new", weak agent (child) is created.
        }
    }

    // We make sure that the main iterator M-C is removed
    MyGeom->destroy_iterator(Monte);
} //SEQENTIONAL BIAS IMPLEMENTATION ENDED HERE.


// //friend
// int sort(BiasInfo tab[], size_t N)
// {
//     for (int i = 0; i < N; i++)
//         for (int j = 0; j < N - 1; j++)
//         {
//             if (tab[j].Warstwa != BIAS_FOR_ANY &&
//                 tab[j + 1].Warstwa != BIAS_FOR_ANY &&
//                 tab[j].Warstwa == tab[j + 1].Warstwa)
//             {
//                 cerr << tab[j] << " & " << tab[j + 1] << " concern the same layer" << endl;
//                 return -11; //Invalid definition
//             }
//             else if (tab[j].Warstwa > tab[j + 1].Warstwa)
//             {
//                 BiasInfo pom = tab[j];
//                 tab[j] = tab[j + 1];
//                 tab[j + 1] = pom;
//             }
//             else
//             {
//                 //Nothing to do ????
//             }
//         }
//     return 0;
// }
//
// //friend
// int MakeOrder(wb_dynarray<BiasInfo> tab)
// {
//     wb_dynarray<BiasInfo> old = tab; //Moves the entire vector.
//     tab.alloc(old.get_size()); //Allocates and fills using the constructor.
//
//     //Search for coordinates and check if there is at most one for each layer!!!
//     //------------------------------------------------------------------------------------
//     for (int i = 0; i < tab.get_size(); i++)
//     {
//         // ?
//         // ?
//         // ? ///...
//     }
//     return 0;
// }
//
// //friend
// ostream& PrintConditionalBias(ostream& o, wb_dynarray<BiasInfo> tab)
// {
//     o << tab[0];
//     for (int i = 1; i < tab.get_size(); i++)
//         if (tab[i].Warstwa != BIAS_FOR_ANY)
//         {
//             o << " " << tab[i];
//         }
//
//     return o;
// }

/* **************************************************************** */
/*           THIS CODE IS DESIGNED & COPYRIGHT BY:                  */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zakład Systematyki i Geografii Roślin Uniwersytetu Warszawskiego */
/*  & Instytut Studiów Społecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */
