/// @file
/// @brief DECLARATION OF W O R L D FOR THE SIMULATION. (LANGUAGES PROJECT WITH P.Culicover)
//  =======================================================================================
/// @date 2026-04-14 (modified)
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <climits> //SHRT_MAX

#include "compatyb.h"
#include "world.hpp"
#include "layer.hpp"
#include "jagent.h" //Agent definition

extern bool			Console;	///< Flag for working in console mode - without any graphics. By default,: `==false`.
extern const int	BIAS_FOR_ANY;	///< The value that represents "all-the-same" in conditional biases.
                                    ///< Always greater than the largest value in the layer. Default `==8`.
/// The Whole World of Simulation.
class jworld:public world
//--------------------------------------------------
{
public:
    // Information necessary for effective implementation of various bias modes:
    // /////////////////////////////////////////////////////////////////////////

    /// Different simulation modes, depending on the bias type used.
    enum SimulMode {NO_BIAS=0,SIMPLE_BIAS=1,CONDITIONAL_BIAS=2,SEQUENTIONAL_BIAS=3,INVALID_BIAS_MODE=4};

    /// The base class for any-mode bias information.
    /// The descendant classes are used to store various bias information.
    class _bias_information_base
    {
    protected:
        short* PtrIleKate;		//!< A pointer to the number of categories set in child classes.

    public:
        /// Get how many bias categories are there. Interface to the pointer to the number of categories.
        short  IleKate() { return *PtrIleKate;}

        explicit _bias_information_base(short* ini):PtrIleKate(ini){}	//!< Constructor that sets a pointer.
        virtual ~_bias_information_base()= default;	//!< Virtual destructor to ensure correct deallocation.

        virtual void clean()= 0;					//!< Bias definition content clearing is required.
        virtual int read_one_bias_item(istream& i)	//!< Reading the elementary bias definition from a stream (required).
        {
            assert("Pure virtual _bias_information_base::read_one_bias_item() was called"==nullptr);
            return EOF;
        }
    };

    /// Information about the agent's distant connection to some location [a,b].
    struct _far_link
    {
        static jworld* MyWorld;	//!< Static binding to a world. This may be because we only have one world in the program.

        unsigned int	a,b;	//!< World location [a,b]
        unsigned int	count;	//!< Statistic counter.

        unsigned get_target_count();	//!< Main accessor which reads `count` from a,b location on far links layer.

        _far_link():a(UINT_MAX),b(UINT_MAX),count(0){}		//!< DEFAULT CONSTRUCTOR (sole).

        friend ostream& operator<<(ostream& s,const _far_link& l)		//!< Serialization.
        { s<<l.a<<' '<<l.b<<l.count; return s;}

        friend istream& operator>>(istream& s,_far_link& l)				//!< Deserialization.
        { s>>l.a>>l.b>>l.count; return s;}
    };

    friend struct jworld::_far_link;

private:
    // Different model step implementations depending on bias type:
    // ////////////////////////////////////////////////////////////

    // Auxiliary methods:
    //-------------------

    void	_update_age();				//!< aging agents.

    /// Reads the location pointed to by the far link starting at location [aa,bb].
    /// \param aa is `a`(means x) of source location.
    /// \param bb is `b`(means y) of source location.
    /// \param target_a is for returning `a`(means x) of target location.
    /// \param target_b is for returning `b`(means y) of target location.
    /// \returns If it returns false, you cannot check further, but what does it mean to check further?
    bool	_xy_of_far_link_of(size_t aa, size_t bb, unsigned& target_a, unsigned& target_b);

    /// Connects the agent in location [aa,bb] to the far location [target_a, target_b].
    /// The agent in this far location should usually be stronger, but it is not forced here.
    void	_connect_flink_to(unsigned aa, unsigned bb, unsigned target_a, unsigned target_b);

    /// Tries to switch a certain percentage of distant links.
    /// But only sometimes does it reach the exact required number.
    void	_connect_far_links(double Percent);

    // Implementations of single simulation steps depending on different types of bias implementations:
    //-------------------------------------------------------------------------------------------------
    void	_one_step_no_bias();				//!< Single step implementation without bias.
    void	_one_step_simple_bias();			//!< Single step implementation with simple bias.
    void	_one_step_conditional_bias();		//!< Single step implementation with conditional bias.
    void	_one_step_sequentional_bias();		//!< Single step implementation with sequential bias.
    void	_one_step_conditional_bias1();		//!< Alternative (more complete) implementation with conditional bias.

    // Simulation statistics directly calculated in the step:
    // //////////////////////////////////////////////////////
    double	SW_dynamic_perc;					//!< Percentage of Small Worlds dynamics in the last step.

public:
    /// Reads the percentage of the Small Worlds dynamics in the last step.
    double get_last_SW_dynamic() const { return SW_dynamic_perc;}

private:
    // Single-valued parameters/attributes of the world:
    // /////////////////////////////////////////////////

    size_t				MyWidth;	//!< Circumference of a torus.
    short				MaxSila;	//!< Maximum agent power/strength.
    short				MinSila;	//!< Minimum agent strength.
    short				TrsSila;	//!< Threshold of strength above which there is no change.
    short				IleKate;	//!< Number of categories.
    short				IleSasiad;	//!< The density of the neighborhood (1-8 is random, -1 means all not randomly).
    short				OdlSasiad;	//!< Neighborhood radius.
    short				UseSelf;	//!< Determines whether to take himself into consideration.
    double				Noise;		//!< Information noise at the contacts.
    double				spontanic;	//!< Spontaneous mutations - random changes in language attributes.
    bool 				use_SW_links;	//!< Determines whether we use far links.
    double				SW_startconnect_percent;	//!< Determines percentage of far link change attempts to perform before attitude dynamic launch.
    double				SW_reconect_percent;	//!< Specifies the percentage of far link (SW structure) changes per step.
    wb_pchar			MappName;	//!< Force initialization bitmap filename.
    wb_pchar			MaplName;	//!< The name of the bitmap file that initializes the language attributes.
    wb_pchar			MaskName;	//!< The name of the bitmap file that initializes the uninhabitable areas.

    // THE TOPIC OF BIAS:
    // //////////////////

    /// Specifies whether to use bias and what mode: 0-No, 1-Normal, 2-Conditional.
    SimulMode						BiasMode/*=0*/;
    wb_ptr<_bias_information_base>	BiasDefinition;		//!< Mode-specific "compiled" bias information.

    // Simulation layers (are tori):
    // /////////////////////////////

    // !< The layer defines the suitability for habitation.
    //rectangle_unilayer<unsigned char>			livability; //livability layer.

    /// The layer of colonizing agents.
    rectangle_layer_of_ptr_to_agents<jagent>	Agenci;

    //!< A layer of long-distance connections. Not in agents, because the structure is supposed to be constant despite agent movement.
    rectangle_layer_of_struct<_far_link>		FarLinks;

    // Main data series. Because it's more convenient to have pointers than to search `Sources` by name:
    // /////////////////////////////////////////////////////////////////////////////////////////////////

    ptr_to_struct_matrix_source<jagent,short>		*Firsts;	//!< `=Agenci.make_source("First mem",&jagent::First);`
    ptr_to_struct_matrix_source<jagent,short>		*Seconds;	//!< `=Agenci.make_source("Second mem",&jagent::Second);`
    ptr_to_struct_matrix_source<jagent,short>		*Thirds;	//!< `=Agenci.make_source("Third mem",&jagent::Third);`
    ptr_to_struct_matrix_source<jagent,short>		*Powers;	//!< `=Agenci.make_source("Power",&jagent::Power);`
    ptr_to_struct_matrix_source<jagent,unsigned long>	*Age;	//!< `=Agenci.make_source("Lang age",&jagent::age);`
    ptr_to_struct_matrix_source<jagent,unsigned long>	*Politics;	//!< `=Agenci.make_source("Polit. affil.",&jagent::Politics);`
    method_by_ptr_matrix_source<jagent,long>		*Classif;	//!< `=Agenci.make_source("Classification",&jagent::Classif);`
    struct_matrix_source<_far_link,unsigned>		*FarA;		//!< `=FarLinks.make_source("f.links A",&_far_link::a)`
    struct_matrix_source<_far_link,unsigned>		*FarB;		//!< `=FarLinks.make_source("f.links B",&_far_link::b)`
    method_matrix_source<_far_link,unsigned>		*FCount;	//!< `=FarLinks.make_source("far counters",&_far_link::getcount)`

public:
    //CONSTRUCTION & DESTRUCTION
    jworld(size_t Width,			//!< Width of the torus of the agent matrix.
          char* log_name,			//!< File name for saving history.
          char* mapl_name,			//!< The name of the raster graphic that initializes the "language components".
          char* mapp_name,			//!< The name of the raster graphic that initiates agent powers.
          char* live_mask,			//!< The name of the raster graphic that initializes unusable areas. Black points on this map.
          short Distribution,		//!< Type and degree of strength distribution. Small negative or positive integers.
          double Noise=0,			//!< Information noise at the contacts.
          short	max_sila=255,		//!< Maximum agent power/strength.
          short min_sila=1,			//!< Minimum agent strength.
          short	ile_kate=256,		//!< Number of categories.
          short	odl_sasiad=1,		//!< Neighborhood radius.
          short	ile_sasiad=8,		//!< The density of the neighborhood (1-8 is random, -1 means all not randomly).
          short need_use_self=0,	//!< Taking your own attributes in determining the majority.
          short walkpower=0,		//!< Determines whether the strength increases with the age of the agent.
          short trespower=SHRT_MAX,	//!< The strength above which the agent is immune to influence.
          double spontanic=0,		//!< Probability (?) of spontaneous attribute mutations.
          bool i_use_SW_links=true,	//!< Determines whether we use long distance links.
          double i_SW_startconnect_percent=0,	//!< Determines what percentage of distant links we set at the beginning.
          double i_SW_reconect_percent=0		//!< Percentage of far link changes in each simulation step.
          );

    ~jworld() override= default;		//!< Virtual destructor.

    /// Printout of simulation parameter values.
    /// @param out is a reference to the output stream.
    /// @param sep specifies the character used to separate individual fields. This can be a space or a tab.
    void	print_experiment_info(ostream& out,const char sep) const
    {
        char bufor1[100];
        char bufor2[100];
        out
                << "\nNum of Kl=" << sep << IleKate
                << "\n" << this->MyWidth << sep << "x" << sep << MyWidth << sep << "=" << sep << MyWidth * MyWidth
                << "\nPower range:" << sep << MinSila << '-' << MaxSila
                << "\nDistribution:" << sep << (jagent::Distribution < 0 ? "G" : "P") << jagent::Distribution
                << "\nTresh of Power=" << sep << TrsSila
                << "\nNoise %=" << sep << Noise * 100 << sep << " Spontanic %=" << sep << spontanic
                << "\nSelf=" << sep << UseSelf
                << "\nNeighborhood=" << sep << IleSasiad << "/(" << (1 + 2 * OdlSasiad) << "*" << (1 + 2 * OdlSasiad) << ")"
                << "\nSmall World:" << sep << (!use_SW_links ? "NO" : ltoa(SW_reconect_percent, bufor1, 10))
                << sep << (!use_SW_links ? "NO" : ltoa(SW_startconnect_percent, bufor2, 10))
                <<endl;
        cout<<"SW: "<<bufor1<<'/'<<bufor2<<endl;
    }

    /// Setting bias parameters from a text.
    /// The ":&?" characters define the `BiasInfo` object type and simulation mode.
    /// @param lst defines bias. If the list is empty, just clear whole bias definition.
    void set_bias_from_str(const char* lst);

protected:
    // Auxiliary methods for setting bias:
    //------------------------------------
    static int  _read_local(istream& in,int& Layer,int& Value);	//!< Reading simple layer-value pairs. E.g. `a1` `b3` `s2` `t4`.
    void        _read_bias_from_stream(istream& i);		//!< Setting additional simulation parameters from the stream.

    // ACTIONS THAT MUST BE IMPLEMENTED - the standard for each simulation:
    //---------------------------------------------------------------------

    void	initialize_layers() override;		//!< Sets the starting state of the simulation.
    void	after_read_from_image() override;	//!< Actions after read state from a file. Also updating agent static fields.
    void	simulate_one_step() override;		//!< Implementation of one simulation step.

    // Cooperation with the display manager:
    //--------------------------------------
    void	make_default_visualisation() override;	//!< Creates default display areas and places them in your display area manager.
    //void actualize_out_area();	//!< Updating the `OutArea` content every `n` simulation steps

    // ... and with data manager:
    //---------------------------

    /// Generates basic data sources for the built-in data manager.
    void	make_basic_sources() override;

    // I/O implementation:
    //--------------------
    int		implement_output(ostream& o) const override;	//!< Serialization.
    int		implement_input(istream& i) override;			//!< Deserialization.

    /// Implementation of saving the simulation state in NET or NET2 format (without or with attributes).
    void dump_net_file(const char* core_name,unsigned long Step) override;

public:
    // BIAS HELPER CLASS DEFINITIONS:
    // //////////////////////////////

    /// Dummy bias info. Empty implementation of bias information when we don't use it.
    class _no_bias_information:public _bias_information_base
    {
    public:
        explicit _no_bias_information(short* ini):_bias_information_base(ini){}
        int read_one_bias_item(istream& i) override { return EOF;} //!< Data loading dummy.
    };

    /// Information about simple unconditional additive bias.
    class _simple_bias_information:public _bias_information_base
    {
    public:
        /// Unconditional additive bias table.
        short	UncdBias[3][8];

        explicit _simple_bias_information(short* ini):_bias_information_base(ini) ///< Constructor (sole).
        { _simple_bias_information::clean();}

        ~_simple_bias_information() override= default; ///< Virtual destructor (empty).

        void clean() override ///< Clearing the contents of the bias definition/information.
        { memset(UncdBias,0,sizeof(UncdBias));}

        int read_one_bias_item(istream& i) override; ///< Loading the simple unconditional bias definition.
    };

    /// Information about conditional bias.
    class _conditional_bias_information:public _bias_information_base
    {
    public:
        /// Table of conditional conservative biases.
        /// Position 9 in the table means arbitrariness in a given coordinate.
        float	Biases[9][9][9]; /*TODO short	CnsrBias[9][9][9]; ????? */

        explicit _conditional_bias_information(short* ini):_bias_information_base(ini) ///< Constructor (sole).
        {_conditional_bias_information::clean();}

        ~_conditional_bias_information() override= default; ///< Virtual destructor (empty).

        void clean() override ///< Clearing the contents of the bias definition/information.
        {
            for(unsigned a=0;a<sizeof(Biases)/sizeof(Biases[0][0][0]);a++)
                ((float*)(&Biases))[a]=0.0;//If additive bias
        }

        int read_one_bias_item(istream& i) override; ///< Loading the conditional bias definition.
    };

    /// Information about sequential conditional bias.
    class _sequentional_bias_information:public _bias_information_base
    {
    public:
        /// Single conditional bias data.
        struct IfBias
        {
            int		leyer[3];	//!< Condition states for individual layers, e.g. a=1 b=3 c=*
            int		 whatley;	//!< Specifies which layer will be modified
            int		  lstate;	//!< For what state.
            float	   value;	//!< And what added value

            /// Checks if the conditional bias is defined correctly.
            bool IsOK(int IleKate=256) const
            {
                return  whatley!=BIAS_FOR_ANY &&
                        lstate!=-1 &&
                        value!=0
                        ;
            }

            IfBias() ///< The constructor uses the clean() method instead of setting the fields directly.
            { clean();}

            void clean() ///< A method that sets fields to default (neutral) values.
            { leyer[0]=leyer[1]=leyer[2]=BIAS_FOR_ANY;whatley=BIAS_FOR_ANY;lstate=-1;value=0;}

            int reg(int Index,int Wartosc); ///< Records a value for a layer, provided it is the first time.

            int set(int Index,int Wartosc,float Premia); ///< Records target and bonus amount.

            int much(int FirstVal,int SecondVal,int ThirdVal) const ///< Checking the fulfillment of the conditional bias (???)
            {
                return (leyer[0]==BIAS_FOR_ANY || leyer[0]==FirstVal) &&
                       (leyer[1]==BIAS_FOR_ANY || leyer[1]==SecondVal) &&
                       (leyer[2]==BIAS_FOR_ANY || leyer[2]==ThirdVal);
            }

            friend ostream& operator << (ostream& o,const IfBias& b); ///< Serialization.
        };

    private:
        /// A dynamic array of conditional biases.
        /// The default size is set in the `_sequential_bias_information` class constructor.
        wb_dynarray<IfBias> SeqBiases;

        int for_use;                    //!< Counter of already used items in the `Seq Biases` array.

    public:
        /// Incrementing the counter of already used items of the `Seq Biases` table.
        /// @returns counter value before incrementation, means last proper index.
        int use_next_item() { return for_use++;}

        explicit _sequentional_bias_information(short* ini,unsigned maxN=20):
            _bias_information_base(ini),SeqBiases(maxN),for_use(0)
        {
            //No cleaning is necessary because the `IfBias` constructor is implicitly used.
        }

        ~_sequentional_bias_information() override= default; ///< Virtual destructor (empty).

        void clean() override	///< Clears the contents of the bias definition. Uses clearing for the `IfBias` structure.
        {
            for(int a=0;a<for_use;a++)
                SeqBiases[a].clean();
        }

        int read_one_bias_item(istream& i) override;	///< Loading bias definition from a stream.

        /// Implementing bias usage. It is quite complicated here and resembles executing a program.
        void UseBiasForAgent(   int FirstVal,int SecondVal,int ThirdVal,
                                wb_dynarray<int>& Firsts,
                                wb_dynarray<int>& Seconds,
                                wb_dynarray<int>& Thirds
                             );
    };
};

// INLINE IMPLEMENTATIONS OF SOME METHODS:
// ///////////////////////////////////////

inline
bool	jworld::_xy_of_far_link_of(	size_t aa,
                                    size_t bb,
                                    unsigned& target_a,
                                    unsigned& target_b)
{
     auto& loc=FarLinks.get(aa,bb);
     if( ( loc.a ) !=  UINT_MAX  )
     {																			//assert(FarLinks.get(a,b).b!=UINT_MAX);
          target_a=loc.a; //FarLinks.get(aa,bb).a;
          target_b=loc.b; //FarLinks.get(aa,bb).b;
          return true;
     }
     else return false;
}


inline
void	jworld::_connect_flink_to(	unsigned aa,
                                    unsigned bb,
                                    unsigned target_a,
                                    unsigned target_b)
{
    auto& farLinkAABB=FarLinks.get(aa, bb);

    if((farLinkAABB.a) != UINT_MAX)	//You have to subtract the old target from the counter
    { 		 		 		 		 		 		 		 		 		assert(farLinkAABB.b != UINT_MAX);
        (FarLinks.get(farLinkAABB.a, farLinkAABB.b).count)--;
 		 		 		 assert(FarLinks.get(farLinkAABB.a, farLinkAABB.b).count != UINT_MAX);
    }

    farLinkAABB.a=target_a;	//Sets `a` of the new connection
    farLinkAABB.b=target_b;	//Sets `b` of the new connection
    FarLinks.get(target_a,target_b).count++;	//We add to the counter in the new target

    unsigned long politofprot=Agenci.get(target_a,target_b).Politics;
    Agenci.get(aa, bb).Politics=politofprot;
    //			Agenci.get(aa,bb).Politics=RANDOM(0xffffff); /// TODO Why changed?
}

/* **************************************************************** */
/*           THIS CODE IS DESIGNED & COPYRIGHT  BY:                 */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zakład Systematyki i Geografii Roślin Uniwersytetu Warszawskiego */
/*  & Instytut Studiów Społecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */


