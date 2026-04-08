//DECLARATION OF   W O R L D  FOR "LANGUAGES" SIMULATION
/////////////////////////////////////////////////////////
#include <limits.h> //SHRT_MAX
#include "world.hpp"
#include "layer.hpp"
#include "jagent.h" //Definicja agenta

  
const BIAS_FOR_ANY=8;//wartoœc oznaczajaca "wszystko jedno" w bias'ach warunkowych - wieksza niz najwieksza wartosc w warstwie

class jworld:public world	//Caly swiat symulacji
//--------------------------------------------------
{
public:
//Informacje niezbedne do efektywnej implementacji róznych trybów biasu
////////////////////////////////////////////////////////////////////////////////
enum SymulMode {NO_BIAS=0,SIMPLE_BIAS=1,CONDITIONAL_BIAS=2,SEQUENTIONAL_BIAS=3,INVALID_BIAS_MODE=4};
    
    class _bias_information_base //Klasy potomne sluza do przechowywania roznych informacji biasowych
    {
    protected:
        short* PtrIleKate; 
    public:
        short  IleKate() { return *PtrIleKate;} //Zabezpiecza przed przypadkowym zapisaniem
        _bias_information_base(short* ini):PtrIleKate(ini){}//Nie ma nic do roboty
        virtual ~_bias_information_base(){} //Wirtualny destruktor dla zapewnienia wlasciwej dealokacji
        virtual void clean() {}             //Czyszczenie zawartosci definicji biasu - tu puste
        virtual int read_one_bias_item(istream& i)=0//Wczytanie elementarnej definicji bias'u - tu atrapa
        {
            assert("Pure virtual _bias_information_base::read_one_bias_item() was called"==0);
            return EOF;
        }	
    };

private:
//Rozne implementacje kroku modelu
////////////////////////////////////////
void	_one_step_no_bias();
void	_one_step_simple_bias();
void	_one_step_conditional_bias();
void	_one_step_sequentional_bias();

//Parametry jednowartosciowe
/////////////////////////////////
size_t				MyWidth;	//Obwod torusa
short				MaxSila;	//Maksymalna sila agenta
short				MinSila;	//Minimalna sila agenta
short				TrsSila;	//Treshold sily powyzej ktorego nie ma zmian
short				IleKate;	//Ilosc kategori w mapach
short				IleSasiad;	//8==Gestosc sasiedztwa
short				OdlSasiad;	//Rozmiar sasiedztwa
short				UseSelf;	//Czy ma brac siebie pod uwage
double				Noise;		//Szum informacyjny
wb_pchar			MappName;	//nazwa pliku inicjujacej bitmapy
wb_pchar			MaplName;	//nazwa pliku inicjujacej bitmapy
wb_pchar			MaskName;	//nazwa pliku inicjujacej bitmapy

SymulMode			            BiasMode/*=0*/;		//Czy uzywac bias i jaki (0-Nie 1-zwykly 2-warunkowy
wb_ptr<_bias_information_base>  BiasDefinition;     //"Skompilowane" informacje o bias'ie wlasciwe dla trybu

//Warstwy symulacji (sa torusami)
/////////////////////////////////
//rectangle_unilayer<unsigned char> zdatnosc;//Warstwa definiujaca zdatnosc do zasiedlenia
rectangle_layer_of_ptr_to_agents<jagent> Agenci;  //Wlaœciwa warstwa agentow zasiedlajacych

//Glowne serie - wygodniej miec wskazniki niz odszukiwac z Sources po nazwach
////////////////////////////////////////////////////////////////////////////////
ptr_to_struct_matrix_source<jagent,short>		*Firsts;//=Agenci.make_source("First mem",&jagent::First);		
ptr_to_struct_matrix_source<jagent,short>		*Seconds;//=Agenci.make_source("Second mem",&jagent::Second);
ptr_to_struct_matrix_source<jagent,short>		*Thirds;//=Agenci.make_source("Third mem",&jagent::Third);
ptr_to_struct_matrix_source<jagent,short>		*Powers;//=Agenci.make_source("Power",&jagent::Power);
method_by_ptr_matrix_source<jagent,long>		*Classif;//=Agenci.make_source("Classification",&jagent::Classif);

public:
//KONSTRUKCJA DESTRUKCJA
jworld(size_t Width,	//Szerokosc torusa macierzy agentow
	  char* log_name,	//Nazwa pliku do zapisywania histori
	  char* mapl_name,	//Nazwa (bit)mapy inicjujacej "skladowe"
	  char* mapp_name,	//Nazwa (bit)mapy inicjujacej "sily"
	  char* live_mask,	//Czarne w tej mapie sa kasowane
	  double Noise=0,
	  short	max_sila=255,//Maksymalna sila agenta
      short min_sila=1,  //Minimalna sila agenta
	  short	ile_kate=256,//Ilosc kategori w mapach	
	  short	odl_sasiad=1,//Rozmiar sasiedztwa
	  short	ile_sasiad=8, //8==Gestosc sasiedztwa		
	  short need_use_self=0,
	  short walkpower=0,	//Czy sila rosnie z wiekiem agenta 
	  short trespower=SHRT_MAX  //Sila powyzej ktorej agent jest odporny na wplyw
	  );	

~jworld(){}

void set_bias_from_str(const char* pom);	//Ustawianie dodatkowych parametrow symulacji z tekstu. Znaki :&? definiuja typ obiektu obiektu BiasInfo i tryb

static int  _read_local(istream& in,int& Layer,int& Value);//Czytanie prostej pary leyer-wartosc np a1 b3 s2 t4 
protected:
void        _read_bias_from_stream(istream& i);	              //Ustawianie dodatkowych parametrow symulacji ze strumienia

//AKCJE KTORE MUSZA BYC ZAIMPLEMENTOWANE - sztampa symulacji itp
//------------------------------------------------------------------
void	initialize_layers();	//Stan startowy symulacji
void	after_read_from_image();//actions after read state from file. Aktualizacja pol static jagent'a!!!
void	simulate_one_step();	//Wlasciwa implementacja kroku symulacji

//Wspolpraca z menagerem wyswietlania
//------------------------------------------------------------------
void	make_default_visualisation(area_menager_base& Menager);//Tworzy domyslne "lufciki" i umieszcza w 
//void actualize_out_area(); // aktualizacja zawartosci OutArea po n krokach symulacji

//Generuje podstawowe zrodla dla wbudowanego menagera danych lub innego
void	make_basic_sources(sources_menager& WhatSourMen);

//Implementacja wejscia/wyjscia. Zwracaj 1 jesli sukces!
virtual
int		implement_output(ostream& o) const;
virtual
int		implement_input(istream& i);


//DEFINICJE KLAS POMOCNICZYCH DO BIASU
//////////////////////////////////////////////
public:
    
    class _no_bias_information:public _bias_information_base
    {
    public:
        _no_bias_information(short* ini):_bias_information_base(ini){}
        int read_one_bias_item(istream& i){ return EOF;} //Atrapa wczytywania danych
    };
    
    class _simple_bias_information:public _bias_information_base
    {
    public:
        short	UncdBias[3][8];//Tablica bezwarunkowych biasow addytywnych
        
        _simple_bias_information(short* ini):_bias_information_base(ini)
        {
            _simple_bias_information::clean();
        }
        
        ~_simple_bias_information()//Czyszczenie lub usuwanie
        {}
        
        void clean() //Czyszczenie zawartosci definicji biasu - tu puste 
        {
            memset(UncdBias,0,sizeof(UncdBias));//Kompletne zerowanie
        }
        
        int read_one_bias_item(istream& i);//Wczytanie elementarnej definicji bias'u
    };
    
    class _conditional_bias_information:public _bias_information_base
    {
    public:
        float	Biases[9][9][9];//Tablica bias'ow warunkowych - pozycja 9 oznacza dowolnosc w tej wspolrzednej /*short	CnsrBias[9][9][9];//Tablica warunkowych bias'ow konserwatywnych - analogiczna do poprzedniej */
        
        _conditional_bias_information(short* ini):_bias_information_base(ini)
        {
            _conditional_bias_information::clean();
        }
        
        ~_conditional_bias_information()
        {}
        
        void clean() //Czyszczenie zawartosci definicji biasu - tu puste 
        {
            for(int a=0;a<sizeof(Biases)/sizeof(Biases[0][0][0]);a++)
                ((float*)(&Biases))[a]=0.0;//If additive bias
        }
        
        int read_one_bias_item(istream& i);//Wczytanie elementarnej definicji bias'u	
    };
    
    class _sequentional_bias_information:public _bias_information_base
    {
    public:
        struct IfBias
        {
            int     leyer[3];   //Stany warunku dla poszczegolnych layerow np a=1 b=3 c=*
            int     whatley;    //Ktory layer bedzie zmodyfikowany
            int     lstate;     //Dla jakiego stanu
            float   value;      //i o jaka wartosc dodana
            IfBias() 
            { clean();}
            
            bool IsOK(int IleKate=256)
            { 
                return  whatley!=BIAS_FOR_ANY && 
                        lstate!=-1 && 
                        value!=0 
                        ;
            }
            
            void clean() 
            { leyer[0]=leyer[1]=leyer[2]=BIAS_FOR_ANY;whatley=BIAS_FOR_ANY;lstate=-1;value=0;}   
            
            int reg(int Index,int Wartosc);//Rejestruje wartosc dla warstwy, pod warunkiem ze to po raz pierwszy
            
            int set(int Index,int Wartosc,float Premia);//Rejestruje target i wysokosc premii           

            int much(int FirstVal,int SecondVal,int ThirdVal)
            {
                return (leyer[0]==BIAS_FOR_ANY || leyer[0]==FirstVal) &&
                       (leyer[1]==BIAS_FOR_ANY || leyer[1]==SecondVal) &&
                       (leyer[2]==BIAS_FOR_ANY || leyer[2]==ThirdVal);
            }

            friend 
                ostream& operator << (ostream& o,const IfBias& b);
        };        
        
        
        wb_dynarray<IfBias> SeqBiases;  //Rozmiar domyslny ustawiany w konstruktorze
        int for_use;                    //Licznik juz uzytych itemow
        int use_next_item() { return for_use++;}

        _sequentional_bias_information(short* ini):_bias_information_base(ini),SeqBiases(20),for_use(0)
        { 
            //Czyszczenie niepotrzebne bo jest konstruktor w IfBias
        }
        
        ~_sequentional_bias_information()
        {
            //Niejawna dealokacja tablicy SeqBiases
        }
        
        void clean() //Czyszczenie zawartosci definicji biasu - tu puste 
        {
            for(int a=0;a<for_use;a++)
                SeqBiases[a].clean();
        }
        
        int read_one_bias_item(istream& i);//Wczytanie elementarnej definicji bias'u

        void UseBiasForAgent(       //Implementacja uzycia biasu - tu dosyc skomplikowana, przypomina wykonanie programu
                                int FirstVal,int SecondVal,int ThirdVal,
                                wb_dynarray<int>& Firsts,
                                wb_dynarray<int>& Seconds,
                                wb_dynarray<int>& Thirds
                             );
    };
    
};