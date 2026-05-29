/// @file
/// @brief DECLARATION OF A G E N T FOR "LANGUAGES" SIMULATION. (LANGUAGES PROJECT WITH P.Culicover)
/// @date 2026-05-29 (modified)
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "layer.hpp"
#include "asserted.h"
#include "maybe_unused.h"

/// Language Evolution Simulation Agent.
class jagent:public sym2::shell::agent_base
{
    friend class jworld;	//To simplify access to the attributes of the world.

    // STATIC ATTRIBUTES - AGENT INITIATION PARAMETERS:
    static short	pow_move;		//!< Determines whether strength change (increase) with age.
    static short	max_pow;		//!< Maximum agent strength.
    static short	min_pow;		//!< Maximum agent strength.
    static short	cate_num;		//!< Number of categories in each language (or culture) attribute.
    static short	cate_shift;		//!< Bit shift for loading from a graphics file.
    static short	distribution;	//!< Degree of power/strength distribution. 0->n distributions with multiplication, -n->-1 distributions using summation.
    static double	mutation_level;	//!< Probability of meme spontaneous change, i.e., an attribute of language/culture.


    //AGENT ATTRIBUTES IMPORTANT IN SIMULATION:
    short			Power;		//!< The power/strength of this agent.
    unsigned long	Age;		//!< Age of the agent's current language/culture (i.e., how many steps since the last change).
    unsigned long	Politics;	//!< Political affiliation

    /// Union for language/culture attributes viewed by name and simultaneously as an array.
    union{
        struct{
            short	First;	//!< The first belief/meme/language attribute.
            short	Second;	//!< Second belief/meme/language attribute.
            short	Third;	//!< Third belief/meme/language attribute.
        };
        //MAYBE_UNUSED
        short	FST[3]={0,0,0};	///< The entire union seen as an array of shorts.
    };

    void _clean();

    // WHAT MUST always be defined:
    // ////////////////////////////
public:
    int IsOK() const	//!< Checking the correctness of language attributes and agent strength.
    {
        return First!=-1 && Second!=-1 && Third!=-1 && Power!=-1;
    }

    jagent();					//!< Default constructor. Real implementation in "jworld.cpp"!
    jagent(const jagent& ini);	//!< Copy constructor. Real implementation in "jworld.cpp"!
    explicit jagent(const jagent* ini);	//!< Constructor from a pointer. Implemented in "jworld.cpp", of course!

    ~jagent() override			//!< Virtual destructor.
    {_clean();}

    jagent* clone() const		//!< Make a copy of the agent on the heap.
    { return new jagent(*this);}

    void clean() override		//!< Virtual cleaner.
    {_clean();}
    
    bool try_mutate()			//!< A rare, spontaneous change in language/culture attribute.
    {
        if(DRAND() <= mutation_level)
        {            
           int what=RANDOM(3);  assert(0<=what && what<3);
           FST[what]=RANDOM(cate_num);
            return true;
        }        
        else return false;
    }

    void assign123(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Loading three attributes from one RGB pixel.
    {
        First=asserted<short>(Red >> cate_shift);
        Second=asserted<short>(Green >> cate_shift);
        Third=asserted<short>(Blue >> cate_shift);
    }

    void assign1(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Loading the first attribute from one RGB/gray pixel.
    {
        First=( (int(Red)+int(Green)+int(Blue))/3 ) >> cate_shift;	//Average color intensity classified. Best when `R = G = B`
    }
    
    void assign2(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Loading a second attribute from one RGB pixel.
    {
        Second=( (int(Red)+int(Green)+int(Blue))/3 ) >> cate_shift;	//Average color intensity classified. Best when `R = G = B`
    }

    void assign3(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Loading the third attribute from one RGB pixel.
    {
        Third=( (int(Red)+int(Green)+int(Blue))/3 ) >> cate_shift;	//Average color intensity classified. Best when `R = G = B`
    }

    void assignPow(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Loading agent strength from one RGB pixel.
    {
        Power= min_pow + short((int(Red) + int(Green) + int(Blue)) / (3. * 255) * (max_pow - min_pow));
    }

    void killBlack(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Agent cleaning in non-residential areas.
    {
        if(Red==0 && Green==0 && Blue==0)
            _clean();
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    unsigned long classify() override	//!< Converting agent attributes to language classification number. NOLINT(*-make-member-function-const)
    {
        return First + cate_num * (Second + cate_num * Third);
    }

    long RGB() const	//!< Agent color in true-color visualizations.
    {
        return ((unsigned long) (((unsigned char) (First) |
            ((unsigned short) (Second) << 8)) |
            (((unsigned long) (unsigned char) (Third)) << 16))) ;
    }

    friend ostream& operator << (ostream& o,const jagent& a)	//!< Serialization.
    {
        o<<'{';
        o<<' '<<a.Power<<' '<<a.First<<' '<<a.Second<<' '<<a.Third<<' '<<a.Age<<' '<<a.Politics<<' ';
        o<<'}';
        return o;
    }

    friend istream& operator >> (istream& i,jagent& a)	//!< Deserialization.
    {
        char pom;
        i>>pom;		// ignores {
        i>>a.Power>>a.First>>a.Second>>a.Third>>a.Age>>a.Politics;
        i>>pom;		// ignores }
        return i;
    }

};

/* **************************************************************** */
/*           THIS CODE IS DESIGNED & COPYRIGHT BY:                  */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zakład Systematyki i Geografii Roślin Uniwersytetu Warszawskiego */
/*  & Instytut Studiów Społecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */


