/// @file
/// @brief SIMULATION STEP WITH BIAS IMPLEMENTATION ("LANGUAGES" PROJECT WITH P.Culicover)
/// @date 2026-05-31 (modified)
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <strstream>

#include "compatyb.h"
//#include "histosou.hpp"
//#include "clstsour.hpp"
//#include "coincsou.hpp"
#include "compatyb.hpp"
#include "gadgets.hpp"
#include "wb_ptrio.h"

#include "jrand.h"
#include "jworld.h"

using namespace sym2;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
#pragma ide diagnostic ignored "modernize-use-auto"

/// The value that represents "all-the-same" in conditional biases.
/// Always greater than the largest value in the layer. Default `==8`. Announced in the header.
const int BIAS_FOR_ANY=8;

void jworld::set_bias_from_str(const char* lst)
//Setting additional simulation parameters from a text.
//Np.: NO_BIAS=0,SIMPLE_BIAS=1,CONDITIONAL_BIAS=2,SEQUENTIAL_BIAS=3,INVALID_BIAS_MODE=4.
{
    if(*lst=='\0')	//If the list is empty, clear it.
    {
        if(BiasDefinition.OK())
            BiasDefinition->clean();
        return;
    }
    else if(strchr(lst,'?')!=NULL)
    {
        cout<<"ASSUMED SEQUENTIAL BIAS\n"<<flush;
        BiasMode= SEQUENTIAL_BIAS;
        BiasDefinition=new _sequential_bias_information(&NumOfCate);
    }
    else if(strchr(lst,'&')!=NULL)
    {
        cout<<"ASSUMED CONDITIONAL BIAS\n"<<flush;
        BiasMode= CONDITIONAL_BIAS;
        BiasDefinition=new _conditional_bias_information(&NumOfCate);
    }
    else if(strchr(lst,':')!=NULL)
    {
        cout<<"ASSUMED SIMPLE BIAS\n"<<flush;
        BiasMode= SIMPLE_BIAS;
        BiasDefinition=new _simple_bias_information(&NumOfCate);
    }
    else
    {
        cout<<"ASSUMED NO BIAS\n"<<flush;
        BiasMode= NO_BIAS;
        return;
    }

    wb_pchar src(lst);	///< The local copy of `lst`.
    istrstream in(src.get_ptr_val());
    _read_bias_from_stream(in); 
}


void jworld::_read_bias_from_stream(istream& in)
{
    cout<<"PARSING OF BIAS DEFINITIONS: "<<flush;                                           assert(BiasDefinition.OK());
    
    do{
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
    cerr<<"Syntax example: A1:4 A2&B1:1 A1&B1&C2:3 B2:1"<<endl;
    return;
INVALID_PARAM:
    cerr<<"Invalid parameters in bias string!"<<endl;
    cerr<<"\aALL BIAS DEFINITIONS WILL BE IGNORED!"<<endl;
    //return;
}

// For pairs like a1 or b3 etc.
int jworld::_read_local(istream& in,int& Layer,int& Value)
{
    int  Znak;
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
    int reg(int Index,int Value)
    {
        if(Index>=0 && Index<=2)
        {
            if(Warstwa[Index]==BIAS_FOR_ANY)
            {
                Warstwa[Index]=Value;
                return 0;
            }
            else
            {
                cerr<<"Second definition for a layer "<<asserted<char>('A' + Index)<<". ";
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
        bool was=false;
        if(b.Warstwa[0]!=BIAS_FOR_ANY)
        {
            o<<'A'<<b.Warstwa[0]+1;
            was=true;
        }
        if(b.Warstwa[1]!=BIAS_FOR_ANY)
        {
            if(was) o << '&';
            o<<'B'<<b.Warstwa[1]+1;
            was=true;
        }
        if(b.Warstwa[2]!=BIAS_FOR_ANY)
        {
            if(was) o << '&';
            o<<'C'<<b.Warstwa[2]+1;
            was=true;
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
        {cerr<<"The value "<<info.Warstwa[0]+1<<" for the layer A is invalid"<<endl;return -11;}
    if( (info.Warstwa[1]<0 || info.Warstwa[1]>=IleKate()) && info.Warstwa[1]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[1]+1<<" for the layer B is invalid"<<endl;return -11;}
    if( (info.Warstwa[2]<0 || info.Warstwa[2]>=IleKate()) && info.Warstwa[2]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[2]+1<<" for the layer C is invalid"<<endl;return -11;}

    //If there are two BIAS_FOR_ANY it is unconditional bias
    assert((info.Warstwa[0]==BIAS_FOR_ANY)+(info.Warstwa[1]==BIAS_FOR_ANY)+(info.Warstwa[2]==BIAS_FOR_ANY)==2);
    
    {//Now write to the conditional bias table `UncdBias[3][8]`.
        if(info.Warstwa[0]!=BIAS_FOR_ANY)
        {
            UncdBias[0][info.Warstwa[0]]= asserted<short>(BiasValue);
        }
        else 
            if(info.Warstwa[1]!=BIAS_FOR_ANY)
            {
                UncdBias[1][info.Warstwa[1]]=asserted<short>(BiasValue);
            }
            else
                if(info.Warstwa[2]!=BIAS_FOR_ANY)
                {
                    UncdBias[2][info.Warstwa[2]]=asserted<short>(BiasValue);
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
        in.putback(char(test));
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
        {cerr<<"The value "<<info.Warstwa[0]+1<<" for the layer A is invalid"<<endl;return -11;}
    if( (info.Warstwa[1]<0 || info.Warstwa[1]>=IleKate()) && info.Warstwa[1]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[1]+1<<" for the layer B is invalid"<<endl;return -11;}
    if( (info.Warstwa[2]<0 || info.Warstwa[2]>=IleKate()) && info.Warstwa[2]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[2]+1<<" for the layer C is invalid"<<endl;return -11;}
    
    Biases[info.Warstwa[0]][info.Warstwa[1]][info.Warstwa[2]]=asserted<short>(BiasValue);	//All possible biases (???)
    
    //Can we continue next time?
    eat_blanks(in);

    int test=in.get();
    if(test!=0 && test!=-1)
    {
        in.putback(char(test));
        return 0;
    }
    else
        return EOF;
}


int jworld::_sequential_bias_information::IfBias::reg(int Index, int Value)
{                
    if(Index>=0 && Index<=2)
    {
        if(layer[Index] == BIAS_FOR_ANY)
        {
            layer[Index]=Value;
            return 0;
        }       
        else
        {
            cerr<<"Second definition for the layer "<<asserted<char>('A'+Index)<<". ";
            return -11;
        }
    }
    else
    {
        cerr<<"Invalid layer "<<asserted<char>('A'+Index)<<". ";
        return -11;
    }
}


int jworld::_sequential_bias_information::IfBias::set(int Index, int Value, float Premium)
{
    if(Index>=0 && Index<=2)
    {
        if(what_lay == BIAS_FOR_ANY)
        {
            what_lay=Index;
            wh_state=Value;
            value=Premium;
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


ostream& operator << (ostream& o,const jworld::_sequential_bias_information::IfBias& b)
{
    bool was=false;
    if(b.layer[0] != BIAS_FOR_ANY)
    {
        o<<'A'<< b.layer[0] + 1;
        was=true;
    }
    if(b.layer[1] != BIAS_FOR_ANY)
    {
        if(was) o << '&';
        o<<'B'<< b.layer[1] + 1;
        was=true;
    }
    if(b.layer[2] != BIAS_FOR_ANY)
    {
        if(was) o << '&';
        o<<'C'<< b.layer[2] + 1;
        was=true;
    }
    if(was)
        o<<'?';
    o << asserted<char>('A'+b.what_lay) << b.wh_state + 1 << ':' << b.value;
    return o;
}   


int  jworld::_sequential_bias_information::read_one_bias_item(istream& in)
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
    
    if(Separator!=':') //If it is not unconditional, it is "conditionally sequential" - i.e., OK
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

    ret=Item.set(Index,Number,asserted<float>(BiasValue));
    if(ret!=0)
        return ret;
    
    cout<<Item<<' '<<flush;	//Echo
    
    //Validation test
    short IleK=IleKate();
    if( !(
            Item.IsOK() &&
            (0<=Item.wh_state) && (Item.wh_state < IleK) &&
            (Item.layer[0] == BIAS_FOR_ANY || (0 <= Item.layer[0] && Item.layer[0] < IleK)) &&
            (Item.layer[1] == BIAS_FOR_ANY || (0 <= Item.layer[1] && Item.layer[1] < IleK)) &&
            (Item.layer[2] == BIAS_FOR_ANY || (0 <= Item.layer[2] && Item.layer[2] < IleK))
        ) )
    {
        cerr<<"Invalid value - probably the number of categories exceeds.\n";
        return -11;
    }
    
    //Can we continue next time?
    eat_blanks(in);
    int test=in.get();
    if(test!=0 && test!=-1)
    {
        in.putback(char(test));
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
    const geometry_base* MyGeom=Agents.get_geometry();
    //AUXILIARY ARRAYS OF COUNTERS:
    wb_dynarray<int> CFirsts(NumOfCate);
    wb_dynarray<int> CSeconds(NumOfCate);
    wb_dynarray<int> CThirds(NumOfCate);
    assert(MyGeom && CFirsts.IsOK() && CSeconds.IsOK() && CThirds.IsOK());
    
    //We allocate the Monte-Carlo iterator:
    iterator_h Monte=MyGeom->make_random_global_iterator();
    
    //We go through the agents with a Monte-Carlo iterator. Some may repeat themselves.
    while(Monte)
    {   
        size_t index=MyGeom->get_next(Monte);	//We get the index of a randomly selected agent
        //if(index==FULL)  //We ignore it if it goes behind the board (which happens with clippings, or happened in the past)
        //      continue;                        
        assert(index!=MyGeom->FULL);               //This definitely shouldn't happen here!
        jagent& CenterAgent=*(Agents.get_ptr(index).get_ptr_val());	//We get references to the agent by bypassing NULL assertions.
        if(Agents.is_empty(CenterAgent))    // We check whether it is not an empty cell (NULL?).
            continue;
        
        if(
                (CenterAgent.Power > TrsStrength)   //Is there no immunity to change anymore?
            ||
                (jagent::mutation_level > 0 && CenterAgent.try_mutate()) //Or didn't it just mutate spontaneously?!
            )
            goto STARZENIE;             // It has immunity - we do nothing more than age.
        
        {   // LOOKING FOR INFLUENCES:
            // ////////////////////////

            /// We allocate the neighborhood iterator...
            iterator_h Neigh=MyGeom->make_random_neighbour_iterator(index, NeighRadius, NeighDens);
            //iterator_h Neigh=MyGeom->make_neighbour_iterator(index,NeighRadius); //A simpler alternative (non-random).
            unsigned zliczanie=0;	///< To count real neighbors.
            
            // Cleaning the arrays of counters:
            fill(CFirsts, 0);	//Former: `memset(CFirsts.get_ptr_val(),0,sizeof(int)*NumOfCate);`
            fill(CSeconds, 0);	//memset(CSeconds.get_ptr_val(),0,sizeof(int)*NumOfCate);
            fill(CThirds, 0);	//memset(CThirds.get_ptr_val(),0,sizeof(int)*NumOfCate);

            if(Use_SW_links)
            { //RECORDING INFLUENCE FROM THE PROTECTOR ON A FAR LINK:
                size_t a,b;
                unsigned x,y;
                dynamic_cast<const rectangle_geometry*>(MyGeom)->WhatCoordinates(index,a,b); //You need to retrieve x and y from the agent index.
                    assert("NOT TESTED AFTER PORTING!"==nullptr);
                // if(_xy_of_far_link_of(a,b,x,y)) //Get the "protector" index of this agent, if it has one (?????)
                // 	{
                // 										assert((y!=UINT_MAX)&&(x!=UINT_MAX));
                // 	jagent& PeryAgent=Agents.get(x,y); //We obtain references to the "protector".
                // 										assert(!Agents.is_empty(PeryAgent));
                // 	CFirsts[PeryAgent.First]+=PeryAgent.Power;
                // 	CSeconds[PeryAgent.Second]+=PeryAgent.Power;
                // 	CThirds[PeryAgent.Third]+=PeryAgent.Power;
                // 	zliczanie++; //He's also kind of a neighbor.
                // 	}
            }

            // ORDINARY CLOSE NEIGHBORS (extended Moore):
            while(Neigh) //Let's check if the iterator still has any neighboring agent to provide.
            {
                size_t index2=MyGeom->get_next(Neigh); //We get the neighbor's index from geometry.
                if(index2==geometry::FULL || index2==index)   //If it was outside the simulation area or in the center of the area,
                    continue; //TODO But does it happen?      //it would still be pointless.

                jagent& PeryAgent=*(Agents.get_ptr(index2).get_ptr_val()); //We get a reference to the neighbor by bypassing NULL assertions
                if(Agents.is_empty(PeryAgent)) //We check whether it is not an empty cell.
                    continue;

                zliczanie++; //This is a real neighbor, not an empty field.

                //Adding neighbor forces to counters in tables:
                CFirsts[PeryAgent.First]+=PeryAgent.Power;
                CSeconds[PeryAgent.Second]+=PeryAgent.Power;
                CThirds[PeryAgent.Third]+=PeryAgent.Power;
            }

            //We make sure that the iterator that is no longer needed will be removed:
            MyGeom->destroy_iterator(Neigh);
            testowanie++;   //We count the number of randomly selected agents.
            
            //Adding your own strength to counters in tables, but only when this option is active.
            if(UseSelf)
            {
                CFirsts[CenterAgent.First]+=CenterAgent.Power;
                CSeconds[CenterAgent.Second]+=CenterAgent.Power;
                CThirds[CenterAgent.Third]+=CenterAgent.Power;
            }
            
            //Searching for influence maxima:
            //-------------------------------
            int maxF=0,indF=-1;
            int maxS=0,indS=-1;
            int maxT=0,indT=-1;

            //Multiple influences can be a maximum (same value).
            //Therefore, we must ensure that among them there will be a random selection.
            //Sometimes RANDOM returns NumOfCate instead of NumOfCate-1.
            const int offset= RANDOM(NumOfCate) % NumOfCate;			//If `NumOfCate==2` it comes out '0' or '1' etc.
                                                                                            assert(0<=offset);
                                                                                            assert(offset < NumOfCate);

            //In this loop, adding noise and searching for maxima:
            //----------------------------------------------------
            for(int g=0; g < NumOfCate; g++)
            {
                /// Index with a random shift:
                int h= (g+offset) % NumOfCate;											assert(h >= 0 && h < NumOfCate);

                //Now it's time to add noise:
                if(CFirsts[h] > 0)
                    CFirsts[h]+=asserted<int>(DRAND() * Noise * (4.5 * MaxStrength));
                if(CSeconds[h] > 0)
                    CSeconds[h]+=asserted<int>(DRAND() * Noise * (4.5 * MaxStrength));
                if(CThirds[h] > 0)
                    CThirds[h]+=asserted<int>(DRAND() * Noise * (4.5 * MaxStrength));
                
                //Testing if we have any new high values.
                if(CFirsts[h] > maxF)
                    {maxF=CFirsts[h];indF=h;}
                if(CSeconds[h] > maxS)
                    {maxS=CSeconds[h];indS=h;}
                if(CThirds[h] > maxT)
                    {maxT=CThirds[h];indT=h;}
            }
                                                                    // Are we sure that a maximum has always been found?
                                                                    // What if it's empty around?
                                                                    //         assert(indF!=-1 && indS!=-1 && indT!=-1);

            //We change the value in the central agent to the same value in the one that had the maximum.
            if(indF!=-1 && CenterAgent.First!=indF)
                { CenterAgent.First=static_cast<short>(indF); CenterAgent.Age=0;}
            if(indS!=-1 && CenterAgent.Second!=indS)
                { CenterAgent.Second=static_cast<short>(indS); CenterAgent.Age=0;}
            if(indT!=-1 && CenterAgent.Third!=indT)
                { CenterAgent.Third=static_cast<short>(indT); CenterAgent.Age=0;}
            
        } //END OF NORMAL STATE CHANGE

STARZENIE:
        if(jagent::pow_move) //Strength as age. Incrementation, if this option is activated.
        {
            int local=CenterAgent.Power;
            local+=jagent::pow_move;
            local%=jagent::max_pow; //Never exceeds maximum force. "0" is where a "new", weak agent (child) is created.
            CenterAgent.Power=static_cast<short>(local);
        }
    }

    // We make sure that the main iterator M-C is removed
    MyGeom->destroy_iterator(Monte);
}  //NO BIAS IMPLEMENTATION ENDED HERE.


void    jworld::_one_step_simple_bias()
{                                                                                            assert(BiasDefinition.OK());
    _simple_bias_information* BiasData=dynamic_cast<_simple_bias_information*>
                                                            (BiasDefinition.get_ptr_val());  assert(BiasData!=NULL);
    const geometry_base* MyGeom=Agents.get_geometry();                                       assert(MyGeom != NULL);

    //AUXILIARY BOARDS OF COUNTERS:
    wb_dynarray<int> CountFirsts(NumOfCate);
    wb_dynarray<int> CountSeconds(NumOfCate);
    wb_dynarray<int> CountThirds(NumOfCate);
                                      assert(MyGeom && CountFirsts.IsOK() && CountSeconds.IsOK() && CountThirds.IsOK());
    
    //OTHER AUXILIARY VARIABLES:
    int testowanie=0; ///< counter.
    iterator_h Monte=MyGeom->make_random_global_iterator(); ///< Monte-Carlo iterator (internal allocated)
    
    //We go through the agents with a Monte-Carlo iterator (some repetitions possible).
    while(Monte)
    {   
        size_t index=MyGeom->get_next(Monte);
        //if(index==FULL)
        //      continue;
        assert(index!=MyGeom->FULL);
        jagent& CenterAgent=*(Agents.get_ptr(index).get_ptr_val());
        if(Agents.is_empty(CenterAgent))
            continue;

         if(
                 (CenterAgent.Power > TrsStrength)   //Is there no immunity to change anymore?
            ||
                 (jagent::mutation_level > 0 && CenterAgent.try_mutate())
            )
            goto STARZENIE; //There is immunity or a fresh mutation - nothing to do.

        {   // INFLUENCE CALCULATION CODE:
            // ///////////////////////////

            /// Neighborhood iterator (with internal allocation perhaps!).
            iterator_h Neigh=MyGeom->make_random_neighbour_iterator(index, NeighRadius, NeighDens);
            //iterator_h Neigh=MyGeom->make_neighbour_iterator(index,NeighRadius); ///< Non-random neighborhood iterator.
            unsigned zliczanie=0; ///< To count real neighbors.

            // Cleaning the arrays of counters :
            fill(CountFirsts, 0); //memset(CountFirsts.get_ptr_val(),0,sizeof(int)*NumOfCate);
            fill(CountSeconds, 0);//memset(CountSeconds.get_ptr_val(),0,sizeof(int)*NumOfCate);
            fill(CountThirds, 0); //memset(CountThirds.get_ptr_val(),0,sizeof(int)*NumOfCate);

            if(Use_SW_links)
            { //RECORDING INFLUENCE FROM THE PROTECTOR ON A FAR LINK:
                size_t a,b;
                unsigned x,y;
                dynamic_cast<const rectangle_geometry*>(MyGeom)->WhatCoordinates(index,a,b);
                if(_xy_of_far_link_of(a,b,x,y))
                {
                                                        assert((y!=UINT_MAX)&&(x!=UINT_MAX));
                    jagent& PeryAgent=Agents.get(x, y); ///< The references to "protector".
                                                        assert(!Agents.is_empty(PeryAgent));
                    //Adding protector strength to counters in arrays
                    CountFirsts[PeryAgent.First]+=PeryAgent.Power;
                    CountSeconds[PeryAgent.Second]+=PeryAgent.Power;
                    CountThirds[PeryAgent.Third]+=PeryAgent.Power;

                    zliczanie++;
                }
            }

            // ORDINARY CLOSE NEIGHBORS (extended Moore):
            while(Neigh) //Let's check if the iterator still has any neighboring agent to provide.
            {
                size_t index2=MyGeom->get_next(Neigh);
                if(index2==geometry::FULL || index2==index)
                    continue;  //TODO But does it happen?

                jagent& PeryAgent=*(Agents.get_ptr(index2).get_ptr_val()); ///< reference to the neighbor
                if(Agents.is_empty(PeryAgent))
                    continue;

                zliczanie++; //This is a real neighbor, not an empty field.

                //Adding neighbor forces to counters in tables:
                CountFirsts[PeryAgent.First]+=PeryAgent.Power;
                CountSeconds[PeryAgent.Second]+=PeryAgent.Power;
                CountThirds[PeryAgent.Third]+=PeryAgent.Power;
            }

            //We make sure that the iterator that is no longer needed will be removed:
            MyGeom->destroy_iterator(Neigh);
            testowanie++;   //We count the number of randomly selected agents.

            //Adding your own strength to counters in tables, but only when this option is active.
            if(UseSelf)
            {
                CountFirsts[CenterAgent.First]+=CenterAgent.Power;
                CountSeconds[CenterAgent.Second]+=CenterAgent.Power;
                CountThirds[CenterAgent.Third]+=CenterAgent.Power;
            }

            //Searching for influence maxima:
            //-------------------------------
            int maxF=0,indF=-1;
            int maxS=0,indS=-1;
            int maxT=0,indT=-1;

            //Multiple influences can be a maximum (same value).
            //Therefore, we must ensure that among them there will be a random selection.
            int offset=RANDOM(NumOfCate);             assert(0 <= offset && offset < NumOfCate);

            //In this loop, adding noise and searching for maxima:
            //----------------------------------------------------
            for(int g=0; g < NumOfCate; g++)
            {
                int h= (g+offset) % NumOfCate; ///< Index with a random shift:
                                          assert(h>=0 && h < NumOfCate);

                //Now it's time to add noise AND BIAS:
                if(CountFirsts[h] > 0)
                    CountFirsts[h]+= asserted<int>(DRAND() * Noise * (4.5 * MaxStrength)) + BiasData->UncdBias[0][h];
                if(CountSeconds[h] > 0)
                    CountSeconds[h]+= asserted<int>(DRAND() * Noise * (4.5 * MaxStrength)) + BiasData->UncdBias[1][h];
                if(CountThirds[h] > 0)
                    CountThirds[h]+= asserted<int>(DRAND() * Noise * (4.5 * MaxStrength)) + BiasData->UncdBias[2][h];

                //Testing if we have any new high values.
                if(CountFirsts[h] > maxF)
                {maxF=CountFirsts[h];indF=h;}
                if(CountSeconds[h] > maxS)
                {maxS=CountSeconds[h];indS=h;}
                if(CountThirds[h] > maxT)
                {maxT=CountThirds[h];indT=h;}
            }                              //assert(indF!=-1 && indS!=-1 && indT!=-1); //But, what if it's empty around?
                                             assert(indF<NumOfCate && indS<NumOfCate && indT<NumOfCate);
            //We change the value in the central agent to the same value in the one that had the maximum.
            if(indF!=-1 && CenterAgent.First!=indF)
                { CenterAgent.First=static_cast<short>(indF); CenterAgent.Age=0;}
            if(indS!=-1 && CenterAgent.Second!=indS)
                { CenterAgent.Second=static_cast<short>(indS);CenterAgent.Age=0;}
            if(indT!=-1 && CenterAgent.Third!=indT)
                { CenterAgent.Third=static_cast<short>(indT); CenterAgent.Age=0;}

        } //END OF NORMAL STATE CHANGE

STARZENIE:
        if(jagent::pow_move) //Strength as age. Incrementation, if this option is activated.
        {
            int local=CenterAgent.Power;
            local+=jagent::pow_move;
            local%=jagent::max_pow; //Never exceeds maximum force. "0" is where a "new", weak agent (child) is created.
            CenterAgent.Power=static_cast<short>(local);
        }
    }

    // We make sure that the main iterator M-C is removed
    MyGeom->destroy_iterator(Monte);
}  //SIMPLE BIAS IMPLEMENTATION ENDED HERE.


//Auxiliary implementation of the sophisticated bias use:
//Quite complicated, it resembles the execution of a program
void jworld::_sequential_bias_information::UseBiasForAgent(
                                int FirstVal,int SecondVal,int ThirdVal,
                                wb_dynarray<int>& CountFirsts,
                                wb_dynarray<int>& CountSeconds,
                                wb_dynarray<int>& CountThirds
                             )
{
    for(int a=0;a<for_use;a++)	//For each record of the "conditional bias instruction" array.
    {
       if( SeqBiases[a].much(FirstVal,SecondVal,ThirdVal)) //If any condition is met...
       {
           // ...add bias to the correct state of the correct layer.
           switch(SeqBiases[a].what_lay){ //TODO Why `float` is used for bias values?
           case 0: CountFirsts[SeqBiases[a].wh_state]+=(int)SeqBiases[a].value;break;
           case 1: CountSeconds[SeqBiases[a].wh_state]+=(int)SeqBiases[a].value;break;
           case 2: CountThirds[SeqBiases[a].wh_state]+=(int)SeqBiases[a].value;break;
           default:
               assert("Invalid index of layer during bias processing."==NULL);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
               break;
#pragma clang diagnostic pop
           }
       }
    }
}


void    jworld::_one_step_sequential_bias0()
{                                                                                               assert(BiasDefinition.OK());
    _sequential_bias_information* BiasData=dynamic_cast<_sequential_bias_information*>
                                                            (BiasDefinition.get_ptr_val());     assert(BiasData!=NULL);
    const geometry_base* MyGeom=Agents.get_geometry();                                          assert(MyGeom != NULL);

    //AUXILIARY BOARDS OF COUNTERS:
    wb_dynarray<int> CFirst(NumOfCate); //!< Counters for firsts opinions.
    wb_dynarray<int> CSecond(NumOfCate);
    wb_dynarray<int> CThird(NumOfCate);               assert(MyGeom && CFirst.IsOK() && CSecond.IsOK() && CThird.IsOK());

    //OTHER AUXILIARY VARIABLES:
    int testowanie=0; ///< counter.
    iterator_h Monte=MyGeom->make_random_global_iterator(); ///< Iterator Monte-Carlo.

    //We go through the agents with a Monte-Carlo iterator (some repetitions possible).
    while(Monte)
    {
        size_t index=MyGeom->get_next(Monte); ///< index of a randomly selected agent.
        //if(index==MyGeom->FULL)
        //      continue;
        assert(index!=MyGeom->FULL);
        jagent& CenterAgent=*(Agents.get_ptr(index).get_ptr_val());
        if(Agents.is_empty(CenterAgent))
            continue;

         if(
                 (CenterAgent.Power > TrsStrength)    //Is there no immunity to change anymore?
            ||
                 (jagent::mutation_level > 0 && CenterAgent.try_mutate()) //Or didn't it just mutate spontaneously?
            )
            goto STARZENIE;   //There is immunity or a fresh mutation - nothing to do.

        {   // INFLUENCE CALCULATION CODE:
            // ///////////////////////////

            /// Neighborhood iterator (with internal allocation perhaps!).
            iterator_h Neigh=MyGeom->make_random_neighbour_iterator(index, NeighRadius, NeighDens);
            //iterator_h Neigh=MyGeom->make_neighbour_iterator(index,NeighRadius); ///< Non-random neighborhood iterator.
            unsigned zliczanie=0; ///< To count real neighbors.

            // Cleaning the arrays of counters:
            fill(CFirst,0); //memset(Firsts.get_ptr_val(),0,sizeof(int)*NumOfCate);
            fill(CSecond,0);//memset(Seconds.get_ptr_val(),0,sizeof(int)*NumOfCate);
            fill(CThird,0); //memset(Thirds.get_ptr_val(),0,sizeof(int)*NumOfCate);

            if(Use_SW_links)
            { //RECORDING INFLUENCE FROM THE PROTECTOR ON A FAR LINK:
            size_t a,b;
            unsigned x,y;
            dynamic_cast<const rectangle_geometry*>(MyGeom)->WhatCoordinates(index,a,b);
                assert("Not tested after porting!"==nullptr);
            // if(_xy_of_far_link_of(0,TODO,x,y)) //Download the "protector" index of this agent, if it has one
            // {
            // 										assert((y!=UINT_MAX)&&(x!=UINT_MAX));
            // 	jagent& PeryAgent=Agents.get(x,y); ///< The references to "protector".
            // 										assert(!Agents.is_empty(PeryAgent));
            // 										assert("NOT TESTED IMPLEMENTATION");
            //
            // 	//Adding protector strength to counters in arrays
            // 	Firsts[PeryAgent.First]+=PeryAgent.Power;
            // 	Seconds[PeryAgent.Second]+=PeryAgent.Power;
            // 	Thirds[PeryAgent.Third]+=PeryAgent.Power;
            //
            // 	zliczanie++;
            // }
            }

            // ORDINARY CLOSE NEIGHBORS (extended Moore):
            while(Neigh) //Let's check if the iterator still has any neighboring agent to provide.
            {
                size_t index2=MyGeom->get_next(Neigh);
                if(index2==geometry_base::FULL || index2==index)
                    continue;

                jagent& PeryAgent=*(Agents.get_ptr(index2).get_ptr_val()); ///< reference to the neighbor
                if(Agents.is_empty(PeryAgent))
                    continue;

                zliczanie++; //This is a real neighbor, not an empty field.

                //Adding neighbor forces to counters in tables:
                CFirst[PeryAgent.First]+=PeryAgent.Power;
                CSecond[PeryAgent.Second]+=PeryAgent.Power;
                CThird[PeryAgent.Third]+=PeryAgent.Power;
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

            //Multiple influences can be a maximum (same value).
            //Therefore, we must ensure that among them there will be a random selection.
            int offset=RANDOM(NumOfCate);             assert(0 <= offset && offset < NumOfCate);

            //Adding bias using the procedure from `BiasData`.
            //When the agent's state matches the given one, bias is added to the appropriate array:
            //-------------------------------------------------------------------------------------
            BiasData->UseBiasForAgent(CenterAgent.First,CenterAgent.Second,CenterAgent.Third,
                                        CFirst,CSecond,CThird);


            //In one loop, adding noise and searching for maxima:
            //---------------------------------------------------
            for(int g=0; g < NumOfCate; g++)
            {
                int h= (g+offset) % NumOfCate;                     assert(h >= 0 && h < NumOfCate);

                //Adding noise:
                if(Noise>0)
                {
                    if(CFirst[h]>0)
                        CFirst[h]+=asserted<int>(DRAND() * Noise * (4.5 * MaxStrength));
                    if(CSecond[h]>0)
                        CSecond[h]+=asserted<int>(DRAND() * Noise * (4.5 * MaxStrength));
                    if(CThird[h]>0)
                        CThird[h]+=asserted<int>(DRAND() * Noise * (4.5 * MaxStrength));
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
        if(jagent::pow_move) //Strength as age. Incrementation, if this option is activated.
        {
            int local=CenterAgent.Power;
            local+=jagent::pow_move;
            local%=jagent::max_pow; //Never exceeds maximum force. 0 is where a "new", weak agent (child) is created.
            CenterAgent.Power=static_cast<short>(local);
        }
    }

    // We make sure that the main iterator M-C is removed
    MyGeom->destroy_iterator(Monte);
} //SEQUENTIAL BIAS IMPLEMENTATION ENDED HERE.


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
