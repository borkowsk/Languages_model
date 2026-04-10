/// @file
/// @brief DECLARATION OF A G E N T FOR "LANGUAGES" SIMULATION. (LANGUAGES PROJECT WITH P.Culicover)
//  ================================================================================================
/// @date 2026-04-10 (modified)
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "layer.hpp"
    
class jagent:public agent_base
{
    friend class jworld;	//To simplify access to the attributes of the world.

    // STATIC ATTRIBUTES - AGENT INITIATION PARAMETERS:
    static short	ruchsily;	//!< Czy siła się zmienia (rośnie) z wiekiem
    static short	max_sila;	//!< Maksymalna siła agenta
    static short	min_sila;	//!< i minimalna
    static short	ile_kate;	//!< Liczba kategorii w mapach
    static short	kate_shift;	//!< Przesuniecie dla wczytywania gifa
    static short	Distribution;	//!< Stopień rozkładu siły. 0->n rozkłady z multiplikacją, -1->-n rozkłady z sumowaniem.
    static double	MutationLevel;	//!< Prawd. spontanicznej zmiany memu, czyli atrybutu języka.


    // SKŁADOWE DLA SYMULACJI:
    short	Power;	//!< Sila agenta
    unsigned long	Age;		//!< Wiek aktualnego języka agenta (czyli ile kroków od ostatniej zmiany)
    unsigned long	Politics;	//!< Przynależność polityczna

    /// Unia dla atrybutów języka widzianych po nazwach i jednocześnie jako tablica.
    union{
        struct{
            short	First;	//!< Pierwsze przekonanie/mem/atrybut języka.
            short	Second;	//!< Drugie przekonanie/mem/atrybut języka.
            short	Third;	//!< Trzecie przekonanie/mem/atrybut języka.
        };
        short	FST[3]={0,0,0};	///< Całość uni widziana jako tablica short-ów.
    };

    void _clean();

    // TO CO MUSI byc zdefiniowane:
    // ////////////////////////////
public:
    int IsOK()	//!< Sprawdzenie poprawności atrybutów i siły agenta.
    {
        return First!=-1 && Second!=-1 && Third!=-1 && Power!=-1;
    }

    jagent(const jagent& ini);	//!< Konstruktor kopiujący. Konkretna implementacja w "jworld.cpp"!
    jagent(const jagent* ini);	//!< Konstruktor ze wskaźnika. Konkretna implementacja w jworld!
    jagent();					//!< Konstruktor domyślny. Konkretna implementacja w "jworld.cpp"!

    jagent* clone() const		//!< Wykonanie kopii agenta na stercie.
    { return new jagent(*this);}
    
    bool try_mutate()			//!< Rzadka, spontaniczna zmiana poglądu/atrybutu języka.
    {
        if(DRAND()<=MutationLevel)
        {            
           int what=RANDOM(3);  assert(0<=what && what<3);
           FST[what]=RANDOM(ile_kate);
            return true;
        }        
        else return false;
    }

    ~jagent() override			//!< Wirtualny destruktor.
    {_clean();}

    void clean() override		//!< Wirtualny czyściciel.
    {_clean();}

    void assign123(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Wczytywanie trzech atrybutów z jednego piksela RGB.
    {
        First=Red>>kate_shift;
        Second=Green>>kate_shift;
        Third=Blue>>kate_shift;
    }

    void assign1(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Wczytywanie pierwszego atrybutu z jednego piksela RGB.
    {
        First=( (int(Red)+int(Green)+int(Blue))/3 ) >>kate_shift;	//Średnie natężenie koloru sklasyfikowane. Najlepiej gdy `R = G = B`
    }
    
    void assign2(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Wczytywanie drugiego atrybutu z jednego piksela RGB.
    {
        Second=( (int(Red)+int(Green)+int(Blue))/3 ) >>kate_shift;	//Średnie natężenie koloru sklasyfikowane. Najlepiej gdy `R = G = B`
    }

    void assign3(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Wczytywanie trzeciego atrybutu z jednego piksela RGB.
    {
        Third=( (int(Red)+int(Green)+int(Blue))/3 ) >>kate_shift;	//Średnie natężenie koloru sklasyfikowane. Najlepiej gdy `R = G = B`
    }

    void assignPow(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Wczytywanie siły agenta z jednego piksela RGB.
    {
        Power=min_sila+short((int(Red)+int(Green)+int(Blue))/(3.*255)*(max_sila-min_sila));
    }

    void killBlack(unsigned char Red,unsigned char Green,unsigned char Blue)	//!< Czyszczenie agentów w obszarach niemieszkalnych.
    {
        if(Red==0 && Green==0 && Blue==0)
            _clean();
    }

    long Classif()	//!< Zamiana atrybutów agenta na numer klasyfikacyjny języka.
    {
        return First+ile_kate*(Second+ile_kate*Third);
    }

    long RGB()	//!< Kolor agenta w wizualizacjach true-color.
    {
        return ((unsigned long) (((unsigned char) (First) |
            ((unsigned short) (Second) << 8)) |
            (((unsigned long) (unsigned char) (Third)) << 16))) ;
    }

    friend ostream& operator << (ostream& o,const jagent& a)	//!< Serializacja.
    {
        o<<'{';
        o<<' '<<a.Power<<' '<<a.First<<' '<<a.Second<<' '<<a.Third<<' '<<a.Age<<' '<<a.Politics<<' ';
        o<<'}';
        return o;
    }

    friend istream& operator >> (istream& i,jagent& a)	//!< Deserializacja.
    {
        char pom;
        i>>pom;		//ignoruje {
        i>>a.Power>>a.First>>a.Second>>a.Third>>a.Age>>a.Politics;
        i>>pom;		//ignoruje }
        return i;
    }

};

/* **************************************************************** */
/*           THIS CODE IS DESIGNED & COPYRIGHT  BY:                 */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zakład Systematyki i Geografii Roślin Uniwersytetu Warszawskiego */
/*  & Instytut Studiów Społecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */


