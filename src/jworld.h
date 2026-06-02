/// @file
/// @brief DECLARATION OF W O R L D FOR THE SIMULATION. (LANGUAGES PROJECT WITH P.Culicover)
/// @date 2026-06-02 (modified)
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <climits> //SHRT_MAX

#include "compatyb.h"
#include "world.hpp"
#include "layer.hpp"
#include "jagent.h" //Agent definition

using namespace sym2::data; //This has a global impact, as jworld.h is included almost everywhere!

extern bool				 Console;	///< Flag for working in console mode - without any graphics. By default, it is `false`.
extern const int	BIAS_FOR_ANY;	///< The value that represents "all-the-same" in conditional biases.
                                    ///< Always greater than the largest value in the layer. Default `==8`.

/// @brief The Whole World of Simulation.
class jworld:public sym2::shell::world
//------------------------------------
{
public:
    // Information necessary for effective implementation of various bias modes:
    // /////////////////////////////////////////////////////////////////////////

    /// Different simulation modes, depending on the bias type used.
    enum SimulMode {NO_BIAS=0,SIMPLE_BIAS=1,CONDITIONAL_BIAS=2,SEQUENTIAL_BIAS=3,INVALID_BIAS_MODE=4};

    /// @brief The base class for any-mode bias information.
    /// @details The descendant classes are used to store various bias information.
    class _bias_information_base
    {
    protected:
        static constexpr unsigned MAX_NUM_OF_CATEGORIES=8;
        short* PtrHowManyCategories;		//!< A pointer to the number of categories set in child classes.

    public:
        /// @brief Get how many bias categories are there. Interface to the pointer to the number of categories.
        short  NumberOfCategories() { return *PtrHowManyCategories;}

        /// @brief Constructor that sets a pointer.
        explicit _bias_information_base(short* ini): PtrHowManyCategories(ini){}
        /// @brief Virtual destructor to ensure correct deallocation.
        virtual ~_bias_information_base()= default;

        virtual void clean()= 0;					//!< @brief Bias definition content clearing is required.
        virtual int read_one_bias_item(istream& i)	//!< @brief Reading the elementary bias definition from a stream (required).
        {
            assert("Pure virtual _bias_information_base::read_one_bias_item() was called"==nullptr);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
            return EOF; //Unreachable code, but only in DEBUG mode.
#pragma clang diagnostic pop
        }
    };

    /// @brief Information about the agent's distant connection to some location [a,b].
    struct _far_link
    {
        static jworld* MyWorld;	//!< Static binding to a world. This may be because we only have one world in the program.

        unsigned int	a,b;	//!< World location [a,b]
        unsigned int	count;	//!< Statistic counter.

        unsigned get_target_count();	//!< @brief Main accessor which reads `count` from (a,b) location on far links layer.

        _far_link():a(UINT_MAX),b(UINT_MAX),count(0){}		//!< @brief DEFAULT CONSTRUCTOR (sole).

        friend ostream& operator<<(ostream& s,const _far_link& l)		//!< @brief Serialization.
        { s<<l.a<<' '<<l.b<<l.count; return s;}

        friend istream& operator>>(istream& s,_far_link& l)				//!< @brief Deserialization.
        { s>>l.a>>l.b>>l.count; return s;}
    };

    friend struct jworld::_far_link;

private:
    // Different model step implementations depending on bias type:
    // ////////////////////////////////////////////////////////////

    /// @name Auxiliary methods:
    //--------------------------
    /// @{
    void	_update_age();				//!< aging agents.

    /// \brief Reads the location pointed to by the far link starting at location [aa,bb].
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
    /// @}

    /// @name Implementations of single simulation steps depending on different types of bias implementations:
    //--------------------------------------------------------------------------------------------------------
    /// @{
    void	_one_step_no_bias();				//!< Single step implementation without bias.
    void	_one_step_simple_bias();			//!< Single step implementation with simple bias.
    void	_one_step_conditional_bias0();		//!< Single step implementation with conditional bias.
    void	_one_step_conditional_bias1();		//!< Alternative (more complete) implementation with conditional bias.
    void	_one_step_sequential_bias0();		//!< Single step implementation with sequential bias.
    /// @}

    // Simulation statistics directly calculated in the step:
    // //////////////////////////////////////////////////////
    double	SW_dynamic_perc;					//!< Percentage of Small Worlds dynamics in the last step.

public:
    /// Reads the percentage of the Small Worlds dynamics in the last step.
    MAYBE_UNUSED
    double get_last_SW_dynamic() const { return SW_dynamic_perc;}

private:
    /// @name Single-valued parameters/attributes of the world:
    // /////////////////////////////////////////////////
    /// @{
    int					MyWidth;		//!< Circumference of a torus.
    short				MaxStrength;	//!< Maximum agent power/strength.
    short				MinStrength;	//!< Minimum agent strength.
    short				TrsStrength;	//!< Threshold of strength above which there is no change.
    short				NumOfCate;		//!< Number of categories.
    short				NeighDens;		//!< The density of the neighborhood (1-8 is random, -1 means all not randomly).
    short				NeighRadius;	//!< Neighborhood radius.
    short				UseSelf;		//!< Determines whether to take himself into consideration.
    double				Noise;			//!< Information noise at the contacts.
    double				Spontaneous;	//!< Spontaneous mutations - random changes in language attributes.
    bool 				Use_SW_links;	//!< Determines whether we use far links.
    double				SW_start_connect_percent;	//!< Determines percentage of far link change attempts to perform before attitude dynamic launch.
    double				SW_reconnect_percent;		//!< Specifies the percentage of far links (SW structure) "per-step" changes.
    wb_pchar			MappName;		//!< Force initialization bitmap filename.
    wb_pchar			MapLName;		//!< The name of the bitmap file that initializes the language attributes.
    wb_pchar			MaskName;		//!< The name of the bitmap file that initializes the uninhabitable areas.
    /// @}

    // THE TOPIC OF BIAS:
    // //////////////////

    /// Specifies whether to use bias and what mode: 0-No, 1-Normal, 2-Conditional.
    SimulMode						BiasMode/*=0*/;
    wb_ptr<_bias_information_base>	BiasDefinition;		//!< Mode-specific "compiled" bias information.

    // Simulation layers (are tori):
    // /////////////////////////////

    // !< The layer defines the suitability for habitation.
    //rectangle_layer_of<unsigned char> livability; //livability layer.

    /// The layer of colonizing agents.
    sym2::shell::rectangle_layer_of_ptr_to_agents<jagent>	Agents;

    //!< A layer of long-distance connections. Not in agents, because the structure is supposed to be constant despite agent movement.
    sym2::shell::rectangle_layer_of_struct<_far_link>		FarLinks;

    /// @name Main data series. Because it's more convenient to have pointers than to search `Sources` by name:
    // ////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @{
    ptr_to_struct_matrix_source<jagent,short>		*Firsts;	//!< `=Agents.make_source("First mem",&jagent::First);`
    ptr_to_struct_matrix_source<jagent,short>		*Seconds;	//!< `=Agents.make_source("Second mem",&jagent::Second);`
    ptr_to_struct_matrix_source<jagent,short>		*Thirds;	//!< `=Agents.make_source("Third mem",&jagent::Third);`
    ptr_to_struct_matrix_source<jagent,short>		*Powers;	//!< `=Agents.make_source("Power",&jagent::Power);`
    ptr_to_struct_matrix_source<jagent,unsigned>	*Age;		//!< `=Agents.make_source("Lang age",&jagent::age);`
    ptr_to_struct_matrix_source<jagent,unsigned long>	*Politics;	//!< `=Agents.make_source("Polit. affiliation",&jagent::Politics);`
    method_by_ptr_matrix_source<jagent,unsigned long>	*Classify;	//!< `=Agents.make_source("Classification",&jagent::classify);`
    struct_matrix_source<_far_link,unsigned>		*FarA;		//!< `=FarLinks.make_source("f.links A",&_far_link::a)`
    struct_matrix_source<_far_link,unsigned>		*FarB;		//!< `=FarLinks.make_source("f.links B",&_far_link::b)`
    method_matrix_source<_far_link,unsigned>		*FCount;	//!< `=FarLinks.make_source("far counters",&_far_link::get_count)`
    /// @}
public:
    //CONSTRUCTION & DESTRUCTION

    /// @brief The sole constructor.
    jworld(size_t Width,			//!< Width of the torus for the agent matrix.
          char* log_name,			//!< File name for saving history.
          char* map_l_name,			//!< The name of the raster graphic that initializes the "language components".
          char* mapp_name,			//!< The name of the raster graphic that initiates agent powers.
          char* live_mask,			//!< The name of the raster graphic that initializes unusable areas. Black points on this map.
          short Distribution,		//!< Type and degree of strength distribution. Small negative or positive integers.
          double Noise=0,			//!< Information noise at the contacts.
          short	max_power=255,		//!< Maximum agent power/strength.
          short min_power=1,		//!< Minimum agent strength.
          short	ile_kate=256,		//!< Number of categories.
          short	neigh_radius=1,		//!< Neighborhood radius.
          short	neigh_dens=8,		//!< The density of the neighborhood (1-8 is random, -1 means all not randomly).
          short	need_use_self=0,	//!< Taking your own attributes in determining the majority.
          short	walk_of_power=0,	//!< Determines whether the strength increases with the age of the agent.
          short	thr_power=SHRT_MAX,	//!< The strength above which the agent is immune to influence.
          double	spontaneous=0,		//!< Probability (?) of spontaneous attribute mutations.
          bool		i_use_SW_links=true,			//!< Determines whether we use long distance links.
          double	i_SW_start_connect_percent=0,	//!< Determines what percentage of distant links we set at the beginning.
          double	i_SW_reconnect_percent=0		//!< Percentage of far link changes in each simulation step.
          );

    ~jworld() override = default;		//!< @brief The virtual destructor. It does a lot more than it looks.

    /// @brief Printout of simulation parameter values.
    /// @param out is a reference to the output stream.
    /// @param sep specifies the character used to separate individual fields. This can be a space or a tab.
    void	print_experiment_info(ostream& out,const char sep) const
    {
        char bufor1[100];
        char bufor2[100];
        out     << "\nNum of Kl=" << sep << NumOfCate
                << "\n" << this->MyWidth << sep << "x" << sep << MyWidth << sep << "=" << sep << MyWidth * MyWidth
                << "\nPower range:" << sep << MinStrength << '-' << MaxStrength
                << "\nDistribution:" << sep << (jagent::distribution < 0 ? "G" : "P") << jagent::distribution
                << "\nThresh of Power=" << sep << TrsStrength
                << "\nNoise %=" << sep << Noise * 100 << sep << " Spontaneous change %=" << sep << Spontaneous
                << "\nSelf=" << sep << UseSelf
                << "\nNeighborhood=" << sep << NeighDens << "/(" << (1 + 2 * NeighRadius) << "*" << (1 + 2 * NeighRadius) << ")"
                << "\nSmall World:" << sep << (!Use_SW_links?"NO":dtoa(SW_reconnect_percent, bufor1))
                << sep << (!Use_SW_links?"NO":dtoa(SW_start_connect_percent, bufor2))
                <<endl;
        cout<<"SW: "<<bufor1<<'/'<<bufor2<<endl;
    }

    /// Setting bias parameters from a text.
    /// The ":&?" characters define the `BiasInfo` object type and simulation mode.
    /// @param lst defines bias. If the list is empty, only clear a whole bias definition.
    void set_bias_from_str(const char* lst);

protected:
    /// @name Auxiliary methods for setting bias:
    //-------------------------------------------
    /// @{
    static int  _read_local(istream& in,int& Layer,int& Value);	//!< @brief Reading simple layer-value pairs. E.g. `a1` `b3` `s2` `t4`.
    void        _read_bias_from_stream(istream& i);		//!< @brief Setting additional simulation parameters from the stream.
    /// @}

    // ACTIONS THAT MUST BE IMPLEMENTED - the standard for each simulation:
    //---------------------------------------------------------------------

    void	initialize_layers() override;		//!< @brief Sets the starting state of the simulation.
    void	after_read_from_image() override;	//!< @briefActions after read state from a file. Also updating agent static fields.
    void	simulate_one_step() override;		//!< @brief Implementation of one simulation step.

    // Cooperation with the display manager:
    //--------------------------------------

    /// @brief Creates default display areas and places them in your display area manager.
    void	make_default_visualisation() override;
    //void actualize_out_area();	//!< Updating the `OutArea` content every `n` simulation steps

    // ... and with data manager:
    //---------------------------

    /// @brief Generates basic data sources for the built-in data manager.
    void	make_basic_sources() override;

    // I/O implementation:
    //--------------------
    int		implement_output(ostream& o) const override;	//!< @brief Virtual serialization.
    int		implement_input(istream& i) override;			//!< @brief Virtual deserialization.

    /// @brief Implementation of saving the simulation state in NET or NET2 format (without or with attributes).
    void dump_net_file(const char* core_name,unsigned long Step) override;

public:
    // BIAS HELPER CLASS DEFINITIONS:
    // //////////////////////////////

    /// @brief Dummy bias info. Empty implementation of bias information when we don't use it.
    class _no_bias_information:public _bias_information_base
    {
    public:
        MAYBE_UNUSED /// @brief Constructor.
        explicit _no_bias_information(short* ini):_bias_information_base(ini){}

        /// @brief Data loading dummy.
        int read_one_bias_item(istream& i) override {
            return EOF;
        }
    };

    /// @brief Information about simple unconditional additive bias.
    class _simple_bias_information:public _bias_information_base
    {
    public:
        /// @brief Unconditional additive bias table.
        short	UncdBias[3][MAX_NUM_OF_CATEGORIES]={};

        explicit _simple_bias_information(short* ini):_bias_information_base(ini) ///< @brief Constructor (sole).
        { _simple_bias_information::clean();}

        ~_simple_bias_information() override= default; ///< @brief Virtual destructor (empty).

        void clean() override ///< @brief Clearing the contents of the bias definition/information.
        { memset(UncdBias,0,sizeof(UncdBias));}

        int read_one_bias_item(istream& i) override; ///< @brief Loading the simple unconditional bias definition.
    };

    /// @brief Information about conditional bias.
    class _conditional_bias_information:public _bias_information_base
    {
    public:
        /// @brief Table of conditional conservative biases.
        /// @details Cell at index MAX_NUM_OF_CATEGORIES in the table means arbitrariness in a given coordinate.
        float	CndBiases[MAX_NUM_OF_CATEGORIES+1]
                         [MAX_NUM_OF_CATEGORIES+1]
                         [MAX_NUM_OF_CATEGORIES+1]={};

        explicit _conditional_bias_information(short* ini):_bias_information_base(ini) ///< @brief Constructor (sole).
        {_conditional_bias_information::clean();}

        ~_conditional_bias_information() override= default; ///< @brief Virtual destructor (empty).

        void clean() override ///< @brief Clearing the contents of the bias definition/information.
        {
            for(unsigned a=0;a<sizeof(CndBiases)/sizeof(CndBiases[0][0][0]);a++)
                ((float*)(&CndBiases))[a]=0.0; //If additive bias
        }

        int read_one_bias_item(istream& i) override; ///< Loading the conditional bias definition.
    };

    /// @brief Information about sequential conditional bias.
    class _sequential_bias_information: public _bias_information_base
    {
    public:
        /// @brief Single conditional bias data.
        struct IfBias
        {
            int		layer[3];	//!< Condition states for individual layers, e.g., a=1 b=3 c=*
            int		what_lay;	//!< Specifies which layer will be modified
            int		wh_state;	//!< For what state.
            float	   value;	//!< And what added value

            /// @brief Checks if the conditional bias is defined correctly.
            bool IsOK(int /*n_of_cate*/=256) const
            {
                return what_lay != BIAS_FOR_ANY &&
                       wh_state != -1 &&
                        value!=0
                        ;
            }

            IfBias() ///< @brief The constructor uses the clean() method instead of setting the fields directly.
            { clean();}

            void clean() ///< @brief A method that sets fields to default (neutral) values.
            { layer[0]= layer[1]= layer[2]=BIAS_FOR_ANY; what_lay=BIAS_FOR_ANY; wh_state=-1; value=0; }

            int reg(int Index,int Value); ///< @brief Records a value for a layer, provided it is the first time.

            int set(int Index, int Value, float Premium); ///< @brief Records target and bonus amount.

            int much(int FirstVal,int SecondVal,int ThirdVal) const ///< @brief Checking the fulfillment of the conditional bias (???)
            {
                return (layer[0] == BIAS_FOR_ANY || layer[0] == FirstVal) &&
                       (layer[1] == BIAS_FOR_ANY || layer[1] == SecondVal) &&
                       (layer[2] == BIAS_FOR_ANY || layer[2] == ThirdVal);
            }

            friend ostream& operator << (ostream& o,const IfBias& b); ///< @brief Serialization.
        };

    private:
        /// A dynamic array of conditional biases.
        /// The default size is set in the `_sequential_bias_information` class constructor.
        wb_dynarray<IfBias> SeqBiases;

        int for_use;                    //!< Counter of already used items in the `Seq Biases` array.

    public:
        /// @brief Incrementing the counter of already used items of the `Seq Biases` table.
        /// @returns counter value before incrementation, means last proper index.
        int use_next_item() { return for_use++;}

        explicit _sequential_bias_information(short* ini, unsigned maxN=20)
        : _bias_information_base(ini),SeqBiases(maxN),for_use(0)
        {
            //No cleaning is necessary because the `IfBias` constructor is implicitly used.
        }

        ~_sequential_bias_information() override= default; ///< @brief Virtual destructor (empty).

        void clean() override	///< @brief Clears the contents of the bias definition. Uses clearing for the `IfBias` structure.
        {
            for(int a=0;a<for_use;a++)
                SeqBiases[a].clean();
        }

        int read_one_bias_item(istream& i) override;	///< @brief Loading bias definition from a stream.

        /// @brief Implementing bias usage. It is quite complicated here and resembles executing a program.
        void UseBiasForAgent(   int FirstVal,int SecondVal,int ThirdVal,
                                wb_dynarray<int>& CountFirsts,
                                wb_dynarray<int>& CountSeconds,
                                wb_dynarray<int>& CountThirds
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
    { 																				assert(farLinkAABB.b != UINT_MAX);
        (FarLinks.get(farLinkAABB.a, farLinkAABB.b).count)--;	assert(FarLinks.get(farLinkAABB.a, farLinkAABB.b).count != UINT_MAX);
    }

    farLinkAABB.a=target_a;	//Sets `a` of the new connection
    farLinkAABB.b=target_b;	//Sets `b` of the new connection
    FarLinks.get(target_a,target_b).count++;	//We add to the counter in the new target

    unsigned long polit_of_prot=Agents.get(target_a, target_b).Politics;
    Agents.get(aa, bb).Politics=polit_of_prot;
    //			Agents.get(aa,bb).Politics=RANDOM(0xffffff); /// TODO Why changed?
}

/* **************************************************************** */
/*            THIS CODE IS DESIGNED & COPYRIGHT BY:                 */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zakład Systematyki i Geografii Roślin Uniwersytetu Warszawskiego */
/*  & Instytut Studiów Społecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */


