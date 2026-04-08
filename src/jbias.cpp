//#include <limits.h>
//#include <assert.h>
//#include <string.h>
//#include <math.h>
#include <string.h>  
#include <math.h>
#include <strstrea.h>
#include "jrand.h"
#include "jworld.h"
#include "histosou.hpp"
#include "clstsour.hpp" //Jest tez statsour
#include "coincsou.hpp"
#include "gadgets.hpp" 
#include "wb_ptrio.h"


void jworld::set_bias_from_str(const char* lst)
//Ustawianie dodatkowych parametrow symulacji z tekstu - NO_BIAS=0,SIMPLE_BIAS=1,CONDITIONAL_BIAS=2,SEQUENTIONAL_BIAS=3,INVALID_BIAS_MODE=4
{
    if(*lst=='\0')//Jesli pusty to tylko czyszczenie
    {
        if(BiasDefinition.OK())
            BiasDefinition->clean();
        return;
    }
    else
    if(strchr(lst,'?')!=NULL)
    {
        cout<<"ASSUMED SEQUENTIONAL BIAS\n"<<flush;
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

    wb_pchar src(lst);//Kopia lst
    istrstream in(src.get_ptr_val());
    _read_bias_from_stream(in); 
}

void jworld::_read_bias_from_stream(istream& in)        
//Ustawianie dodatkowych parametrow symulacji ze strumienia
{
    cout<<"PARSING OF BIAS DEFINITIONS: "<<flush;       assert(BiasDefinition.OK());
    
    do
    {
        switch(BiasDefinition->read_one_bias_item(in)){
        case EOF:goto OK;
        case -10:goto ON_ERROR;
        case -11:goto INVALID_PARAM;
        case 0:
        default:
            break; //Do nothing!
        }
    }while(1);

OK:
    cout<<endl<<flush;
    return;
ON_ERROR:
    cerr<<"Syntax error on bias definition:"<<endl;
    cerr<<"syntax exaple:  A1:4 A2&B1:1 A1&B1&C2:3 B2:1"<<endl;
    cerr<<"\aALL BIAS DEFINITIONS WILL BE IGNORED!"<<endl;
    return;
INVALID_PARAM:
    cerr<<"Invalid parameters in bias string."<<endl;
    cerr<<"\aALL BIAS DEFINITIONS WILL BE IGNORED!"<<endl;
    return;
}


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
    
    Znak=tolower(Znak);//Ujednolicenie wielkosci liter
    
    switch(Znak){   //Konwersja z zapisu f[irst], s[econd], t[hird]
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

struct BiasInfo
{  
    int Warstwa[3];//Wartosc dla kazdej z trzech warstw 
    BiasInfo(){Warstwa[0]=Warstwa[1]=Warstwa[2]=BIAS_FOR_ANY;} 
    
    int reg(int Index,int Wartosc)//Rejestruje wartosc dla warstwy, pod warunkiem ze to po raz pierwszy
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
                cerr<<"Second definition for layer "<<char('A'+Index)<<". ";
                return -11;
            }
        }
        else
        {
            cerr<<"Invalid layer "<<char('A'+Index)<<". ";
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
//Wczytanie elementarnej definicji bias'u
{
    char Separator;
    BiasInfo info;
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

    if(Separator!=':')
    {
        cerr<<"Expected ':' not "<<Separator<<endl;
        return -10;
    }

    in>>BiasValue;
    if(in.fail())
        return -10;
    
    cout<<info<<':'<<BiasValue<<' '<<flush;//Echo
    
    if( (info.Warstwa[0]<0 || info.Warstwa[0]>=IleKate()) && info.Warstwa[0]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[0]+1<<" for layer A are invalid"<<endl;return -11;}
    if( (info.Warstwa[1]<0 || info.Warstwa[1]>=IleKate()) && info.Warstwa[1]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[1]+1<<" for layer B are invalid"<<endl;return -11;}
    if( (info.Warstwa[2]<0 || info.Warstwa[2]>=IleKate()) && info.Warstwa[2]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[2]+1<<" for layer C are invalid"<<endl;return -11;}

    assert((info.Warstwa[0]==BIAS_FOR_ANY)+(info.Warstwa[1]==BIAS_FOR_ANY)+(info.Warstwa[2]==BIAS_FOR_ANY)==2);//Jesli sa dwa BIAS_FOR_ANY to bias bezwarunkowy
    
    {//Zapis do tablicy bias'ow warunkowych UncdBias[3][8]
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
                    assert("This line should never be reached."==0);//Upss...
                }
    }
    
    //Can we continue next time?
    in.eatwhite();
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
//Wczytanie elementarnej definicji bias'u
{
    char Separator;
    BiasInfo info;
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

    if(Separator=='&') //jeszcze jeden
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

        if(Separator=='&') //i jeszcze jeden
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
    
    cout<<info<<':'<<BiasValue<<' '<<flush;//Echo
    
    if( (info.Warstwa[0]<0 || info.Warstwa[0]>=IleKate()) && info.Warstwa[0]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[0]+1<<" for layer A are invalid"<<endl;return -11;}
    if( (info.Warstwa[1]<0 || info.Warstwa[1]>=IleKate()) && info.Warstwa[1]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[1]+1<<" for layer B are invalid"<<endl;return -11;}
    if( (info.Warstwa[2]<0 || info.Warstwa[2]>=IleKate()) && info.Warstwa[2]!=BIAS_FOR_ANY )
        {cerr<<"The value "<<info.Warstwa[2]+1<<" for layer C are invalid"<<endl;return -11;}
    
    Biases[info.Warstwa[0]][info.Warstwa[1]][info.Warstwa[2]]=BiasValue;//Wszytkie mozliwe bias'y
    
    //Can we continue next time?
    in.eatwhite();
    int test=in.get();
    if(test!=0 && test!=-1)
    {
        in.putback(test);
        return 0;
    }
    else
        return EOF;
}

int jworld::_sequentional_bias_information::IfBias::reg(int Index,int Wartosc)//Rejestruje wartosc dla warstwy, pod warunkiem ze to po raz pierwszy
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
            cerr<<"Second definition for layer "<<char('A'+Index)<<". ";
            return -11;
        }
    }
    else
    {
        cerr<<"Invalid layer "<<char('A'+Index)<<". ";
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
            cerr<<"Second definition for target layer "<<char('A'+Index)<<". ";
            return -11;
        }
    }
    else
    {
        cerr<<"Invalid target layer "<<char('A'+Index)<<". ";
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
    o<<char('A'+b.whatley)<<b.lstate+1<<':'<<b.value;
    return o;
}   

int  jworld::_sequentional_bias_information::read_one_bias_item(istream& in)
//Wczytanie elementarnej definicji bias'u
{
    char Separator;
    int    Index,Number;
    double BiasValue;
    IfBias& Item=SeqBiases[use_next_item()];
    
    cout<<endl<<flush;
    
    int ret=_read_local(in,Index,Number);   
    if(ret!=0) 
        return ret;
    
    in>>Separator;//Trzeba wczytac separator zeby zadecydowac
    if(in.fail())
        return -10;
    
    if(Separator!=':') //Jesli nie jest bezwarunkowy to jest warunkowy sekwencyjny - czyli OK
    {
        assert(Separator=='&'||Separator=='?');                                                    
        ret=Item.reg(Index,Number);//Pierwszy zarejestrowany
        if(ret!=0)
            return ret;
        
        if(Separator=='&') //jeszcze jeden
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
            
            if(Separator=='&') //i jeszcze jeden
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
    
    cout<<Item<<' '<<flush;//Echo
    
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
    in.eatwhite();
    int test=in.get();
    if(test!=0 && test!=-1)
    {
        in.putback(test);
        return 0;
    }
    else
        return EOF;
}

//          IMPLEMENTACJE KROKOW SYMULACJI Z ROZNYMI BIAS'AMI
///////////////////////////////////////////////////////////////////////////
void    jworld::_one_step_no_bias()
{
    int testowanie=0;
    const geometry_base* MyGeom=Agenci.get_geometry();
    //TABLICE POMOCNICZE
    wb_dynarray<int> Firsts(IleKate);
    wb_dynarray<int> Seconds(IleKate);
    wb_dynarray<int> Thirds(IleKate);
    assert(MyGeom && Firsts.IsOK() && Seconds.IsOK() && Thirds.IsOK());
    
    //Alokujemy iterator Monte-Carlo
    iteratorh Monte=MyGeom->make_random_global_iterator();
    
    //Idziemy po agentach iteratorem Monte-Carlo. Niektórzy moga sie powtórzyc
    while(Monte)
    {   
        size_t index=MyGeom->get_next(Monte);//Uzyskujemy index losowo wybranego agenta 
        //if(index==FULL)                        //Ignorujemy jesli trafil za tablice (zdaza sie dla wycinkow?)
        //      continue;                        
        assert(index!=MyGeom->FULL);               //... tutaj nie powinno sie zdarzyc
        jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());// Uzyskujemy referencje do agenta omijajac asercje na NULL
        if(Agenci.is_empty(CenterAgent))    // Sprawdzamy czy nie jest to pusta komórka (NULL)
            continue;                   // bo wtedy robic dalej by³oby bez sensu.
        
        if(CenterAgent.Power>TrsSila)       // Czy nie ma juz immunitedu na zmiany
            goto STARZENIE;             // Ma - nie robimy nic
        
        {   //KOD NA SZUKANIA WPLYWOW
            /////////////////////////////////////
            // Alokujemy iterator sasiedztwa
            iteratorh Neigh=MyGeom->make_random_neighbour_iterator(index,OdlSasiad,IleSasiad);
            //iteratorh Neigh=MyGeom->make_neighbour_iterator(index,OdlSasiad);
            unsigned zliczanie=0;//Zliczanie sasiadów
            
            //Czyszczenie licznikow
            fill(Firsts,0); //memset(Firsts.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(Seconds,0);//memset(Seconds.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(Thirds,0); //memset(Thirds.get_ptr_val(),0,sizeof(int)*IleKate);
            
            while(Neigh)
            {
                size_t index2=MyGeom->get_next(Neigh);//Uzyskujemy index sasiada        
                if(index2==MyGeom->FULL || index2==index)   //Jesli poza obszarem symulacji lub w 
                    continue;               //centrum obszaru to dalej byloby bez sensu.
                
                jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val());//Uzyskujemy referencje do sasiada omijajac asercje na NULL
                if(Agenci.is_empty(PeryfAgent))     //Sprawdzamy czy nie jest to pusta komórka (NULL)
                    continue;                      // bo wtedy robic dalej by³oby bez sensu.
                
                zliczanie++;
                //Dodawanie sil sasiadow do licznikow w tablicach
                Firsts[PeryfAgent.First]+=PeryfAgent.Power;
                Seconds[PeryfAgent.Second]+=PeryfAgent.Power;
                Thirds[PeryfAgent.Third]+=PeryfAgent.Power;
            }
            
            MyGeom->destroy_iterator(Neigh);    // upewniamy sie ze iterator zostanie usuniety
            //Zlicza wylosowanych agentow
            testowanie++;   
            
            //Dodawanie wlasnych sil do licznikow w tablicach
            if(UseSelf)
            {
                Firsts[CenterAgent.First]+=CenterAgent.Power;
                Seconds[CenterAgent.Second]+=CenterAgent.Power;
                Thirds[CenterAgent.Third]+=CenterAgent.Power;
            }
            
            //Do szukanie maksimow
            int maxF=0,indF=-1;
            int maxS=0,indS=-1;
            int maxT=0,indT=-1;
            int offset=RANDOM(IleKate);         assert(0<=offset && offset<IleKate);//Jak IleKate==2 to 0 albo 1 itd..
        
            
            //W petli dodawanie szumu i szukanie maksimow
            //------------------------------------------------
            for(int g=0;g<IleKate;g++)
            {
                int h=(g+offset)%IleKate;
                assert(h>=0 && h<IleKate);
                //Dodawanie szumu
                if(Firsts[h]>0)
                    Firsts[h]+=long(DRAND()*Noise*(4.5*MaxSila));
                if(Seconds[h]>0)
                    Seconds[h]+=long(DRAND()*Noise*(4.5*MaxSila));
                if(Thirds[h]>0)
                    Thirds[h]+=long(DRAND()*Noise*(4.5*MaxSila));
                
                //Testowanie
                if(Firsts[h]>maxF)
                    {maxF=Firsts[h];indF=h;}
                if(Seconds[h]>maxS)
                    {maxS=Seconds[h];indS=h;}
                if(Thirds[h]>maxT)
                    {maxT=Thirds[h];indT=h;}
            }

            //assert(indF!=-1 && indS!=-1 && indT!=-1);
            
            if(indF!=-1)
                CenterAgent.First=indF;         //zmieniamy w agencie centralnym
            if(indS!=-1)
                CenterAgent.Second=indS;        //zmieniamy w agencie centralnym
            if(indT!=-1)
                CenterAgent.Third=indT;         //zmieniamy w agencie centralnym
            
        }//KONIEC ZMIAN STANU
        //////
        
        //Sila jako wiek
STARZENIE:
        if(jagent::ruchsily)
        {
            CenterAgent.Power+=jagent::ruchsily;
            CenterAgent.Power%=jagent::max_sila;//Nigdy nie przekracza sily maksymalnej
        }
    }
    // upewniamy sie ze iterator zostanie usuniety
    MyGeom->destroy_iterator(Monte);
}

void    jworld::_one_step_simple_bias()
{   
                                                                                                      assert(BiasDefinition.OK());
    _simple_bias_information* BiasData=dynamic_cast<_simple_bias_information*>
                                                            (BiasDefinition.get_ptr_val());           assert(BiasData!=NULL);
    const geometry_base* MyGeom=Agenci.get_geometry();                                                assert(MyGeom!=NULL);  

    //TABLICE POMOCNICZE
    wb_dynarray<int> Firsts(IleKate);
    wb_dynarray<int> Seconds(IleKate);
    wb_dynarray<int> Thirds(IleKate);               assert(MyGeom && Firsts.IsOK() && Seconds.IsOK() && Thirds.IsOK());
    
    //INNE ZMIENNE
    int testowanie=0;
    iteratorh Monte=MyGeom->make_random_global_iterator();//Alokujemy iterator Monte-Carlo
    
    //Idziemy po agentach iteratorem Monte-Carlo. Niektórzy moga sie powtórzyc
    while(Monte)
    {   
        size_t index=MyGeom->get_next(Monte);//Uzyskujemy index losowo wybranego agenta 
        //if(index==FULL)                        //Ignorujemy jesli trafil za tablice (zdaza sie dla wycinkow?)
        //      continue;                        
        assert(index!=MyGeom->FULL);                //... tutaj nie powinno sie zdarzyc
        jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());// Uzyskujemy referencje do agenta omijajac asercje na NULL
        if(Agenci.is_empty(CenterAgent))    // Sprawdzamy czy nie jest to pusta komórka (NULL)
            continue;                   // bo wtedy robic dalej by³oby bez sensu.
        
        if(CenterAgent.Power>TrsSila)       // Czy nie ma juz immunitedu na zmiany
            goto STARZENIE;             // Ma - nie robimy nic
        
        {   //KOD NA SZUKANIA WPLYWOW
            /////////////////////////////////////
            // Alokujemy iterator sasiedztwa
            iteratorh Neigh=MyGeom->make_random_neighbour_iterator(index,OdlSasiad,IleSasiad);
            //iteratorh Neigh=MyGeom->make_neighbour_iterator(index,OdlSasiad);
            unsigned zliczanie=0;//Zliczanie sasiadów
            
            //Czyszczenie licznikow
            fill(Firsts,0); //memset(Firsts.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(Seconds,0);//memset(Seconds.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(Thirds,0); //memset(Thirds.get_ptr_val(),0,sizeof(int)*IleKate);
                        
            while(Neigh)
            {
                size_t index2=MyGeom->get_next(Neigh);//Uzyskujemy index sasiada        
                if(index2==MyGeom->FULL || index2==index)   //Jesli poza obszarem symulacji lub w 
                    continue;               //centrum obszaru to dalej byloby bez sensu.
                
                jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val());//Uzyskujemy referencje do sasiada omijajac asercje na NULL
                if(Agenci.is_empty(PeryfAgent))     //Sprawdzamy czy nie jest to pusta komórka (NULL)
                    continue;                      // bo wtedy robic dalej by³oby bez sensu.
                
                zliczanie++;
                //Dodawanie sil sasiadow do licznikow w tablicach
                Firsts[PeryfAgent.First]+=PeryfAgent.Power;
                Seconds[PeryfAgent.Second]+=PeryfAgent.Power;
                Thirds[PeryfAgent.Third]+=PeryfAgent.Power;
            }
            
            MyGeom->destroy_iterator(Neigh);    // upewniamy sie ze iterator zostanie usuniety
            //Zlicza wylosowanych agentow
            testowanie++;   
            
            //Dodawanie wlasnych sil do licznikow w tablicach
            if(UseSelf)
            {
                Firsts[CenterAgent.First]+=CenterAgent.Power;
                Seconds[CenterAgent.Second]+=CenterAgent.Power;
                Thirds[CenterAgent.Third]+=CenterAgent.Power;
            }
            
            //Szukanie maksimow
            int maxF=0,indF=-1;
            int maxS=0,indS=-1;
            int maxT=0,indT=-1;
            int offset=RANDOM(IleKate);             assert(0<=offset && offset<IleKate);//Jak IleKate==2 to 0 albo 1 itd..
            
            
            //W petli dodawanie szumu i biasu i szukanie maksimow
            //----------------------------------------------------
            for(int g=0;g<IleKate;g++)
            {
                int h=(g+offset)%IleKate;
                assert(h>=0 && h<IleKate);
                //Dodawanie szumu
                if(Firsts[h]>0)
                    Firsts[h]+=long(DRAND()*Noise*(4.5*MaxSila))+BiasData->UncdBias[0][h];
                if(Seconds[h]>0)
                    Seconds[h]+=long(DRAND()*Noise*(4.5*MaxSila))+BiasData->UncdBias[1][h];
                if(Thirds[h]>0)
                    Thirds[h]+=long(DRAND()*Noise*(4.5*MaxSila))+BiasData->UncdBias[2][h];
                
                //Testowanie
                if(Firsts[h]>maxF)
                {maxF=Firsts[h];indF=h;}
                if(Seconds[h]>maxS)
                {maxS=Seconds[h];indS=h;}
                if(Thirds[h]>maxT)
                {maxT=Thirds[h];indT=h;}
            }
            
            //assert(indF!=-1 && indS!=-1 && indT!=-1);
            if(indF!=-1)
                CenterAgent.First=indF;         //zmieniamy w agencie centralnym
            if(indS!=-1)
                CenterAgent.Second=indS;        //zmieniamy w agencie centralnym
            if(indT!=-1)
                CenterAgent.Third=indT;         //zmieniamy w agencie centralnym
            
        }//KONIEC ZMIAN STANU
        //////
        
        //Sila jako wiek
STARZENIE:
        if(jagent::ruchsily)
        {
            CenterAgent.Power+=jagent::ruchsily;
            CenterAgent.Power%=jagent::max_sila;//Nigdy nie przekracza sily maksymalnej
        }
    }
    // upewniamy sie ze iterator zostanie usuniety
    MyGeom->destroy_iterator(Monte);
}

void    jworld::_one_step_conditional_bias()
{
    
                                                                                                      assert(BiasDefinition.OK());
    _conditional_bias_information* BiasData=dynamic_cast<_conditional_bias_information*>
                                                            (BiasDefinition.get_ptr_val());           assert(BiasData!=NULL);
    
    const geometry_base* MyGeom=Agenci.get_geometry();                                                assert(MyGeom!=NULL);//Geometria "swiata" symulacji

    //TROJWYMIAROWA TABLICA NA ZLICZANIE WPLYWOW
    //Ilosc dopuszczalnych kategori w kazdym memeie + pozycje na pojedyncze biasy i kombinacje podwojne
    int Wplywy[BIAS_FOR_ANY+1][BIAS_FOR_ANY+1][BIAS_FOR_ANY+1];                                       assert(IleKate<=BIAS_FOR_ANY);//Czy nie ma za duzo kategori na taka tablice wplywow
    
    //Alokujemy iterator Monte-Carlo
    iteratorh Monte=MyGeom->make_random_global_iterator();  //Losowanie kolejnego agenta
    int testowanie=0;

    //Idziemy po agentach iteratorem Monte-Carlo. Niektórzy moga sie powtórzyc
    while(Monte)
    {   
        size_t index=MyGeom->get_next(Monte);       //Uzyskujemy index losowo wybranego agenta  
        //if(index==FULL) continue;                 //Ignorujemy jesli trafil za tablice - Moze sie zdazyc tylko dla wycinkow?)
                                                                    assert(index!=MyGeom->FULL);    //... tutaj nie powinno sie zdarzyc
        
        jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());// Uzyskujemy referencje do agenta omijajac asercje na NULL
        
        if(Agenci.is_empty(CenterAgent))    // Sprawdzamy czy nie jest to pusta komórka (NULL)
                continue;                   // bo wtedy robic dalej by³oby bez sensu.
        
        if(CenterAgent.Power>TrsSila)       // Czy nie ma juz immunitedu na zmiany
                goto STARZENIE;             // Ma - nie robimy nic
        
        {   //KOD SZUKANIA WPLYWOW
            /////////////////////////////////////           
            iteratorh Neigh=MyGeom->make_random_neighbour_iterator(index,OdlSasiad,IleSasiad);  // Alokujemy iterator sasiedztwa
            unsigned zliczanie=0;           //Zliczanie sasiadów
            
            //Czyszczenie tabeli licznika
            memset(Wplywy,0,sizeof(Wplywy));//Trzeba zerowac cala, nawet jesli nie cala uzywamy - bo sa kolumny dla BIAS_FOR_ANY
            
            while(Neigh)
            {
                size_t index2=MyGeom->get_next(Neigh);//Uzyskujemy index sasiada        
                if(index2==MyGeom->FULL || index2==index)   //Jesli poza obszarem symulacji lub w 
                    continue;               //centrum obszaru to dalej byloby bez sensu.
                
                jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val());//Uzyskujemy referencje do sasiada omijajac asercje na NULL
                if(Agenci.is_empty(PeryfAgent))     //Sprawdzamy czy nie jest to pusta komórka (NULL)
                    continue;                      // bo wtedy robic dalej by³oby bez sensu.
                
                zliczanie++;                        //Zlicza wylosowanych sasiadow

                //Dodawanie sil sasiadow do licznikow w tablicach
                Wplywy[PeryfAgent.First][PeryfAgent.Second][PeryfAgent.Third]+=3*PeryfAgent.Power;//"zlicznik" dla koincydencji ABC
                
                Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][PeryfAgent.Third]+=2*PeryfAgent.Power;//"zlicznik" histogramu BxC
                Wplywy[PeryfAgent.First][BIAS_FOR_ANY][PeryfAgent.Third]+=2*PeryfAgent.Power;//"zlicznik" histogramu AxC
                Wplywy[PeryfAgent.First][PeryfAgent.Second][BIAS_FOR_ANY]+=2*PeryfAgent.Power;//"zlicznik" histogramu AxB
                
                Wplywy[PeryfAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=PeryfAgent.Power;//"zlicznik" histogramu dla Axx
                Wplywy[BIAS_FOR_ANY][PeryfAgent.Second][BIAS_FOR_ANY]+=PeryfAgent.Power;//"zlicznik" histogramu dla xBx
                Wplywy[BIAS_FOR_ANY][BIAS_FOR_ANY][PeryfAgent.Third]+=PeryfAgent.Power;//"zlicznik" histogramu dla xxC
                
            }
            
            MyGeom->destroy_iterator(Neigh);    // upewniamy sie ze iterator zostanie usuniety
            testowanie++;                       //Zlicza wylosowanych agentow
            
            //Dodawanie wlasnych sil do licznikow w tablicach
            if(UseSelf)
            {
                Wplywy[CenterAgent.First][CenterAgent.Second][CenterAgent.Third]+=3*CenterAgent.Power;//"zlicznik" dla koincydencji ABC
                
                Wplywy[BIAS_FOR_ANY][CenterAgent.Second][CenterAgent.Third]+=2*CenterAgent.Power;//"zlicznik" histogramu BxC
                Wplywy[CenterAgent.First][BIAS_FOR_ANY][CenterAgent.Third]+=2*CenterAgent.Power;//"zlicznik" histogramu AxC
                Wplywy[CenterAgent.First][CenterAgent.Second][BIAS_FOR_ANY]+=2*CenterAgent.Power;//"zlicznik" histogramu AxB
                
                Wplywy[CenterAgent.First][BIAS_FOR_ANY][BIAS_FOR_ANY]+=CenterAgent.Power;//"zlicznik" histogramu dla Axx
                Wplywy[BIAS_FOR_ANY][CenterAgent.Second][BIAS_FOR_ANY]+=CenterAgent.Power;//"zlicznik" histogramu dla xBx
                Wplywy[BIAS_FOR_ANY][BIAS_FOR_ANY][CenterAgent.Third]+=CenterAgent.Power;//"zlicznik" histogramu dla xxC
                
            }

            //W petli dodawanie szumu i biasu
            //--------------------------------------------------
            for(int i=0,width=(BIAS_FOR_ANY+1)*(BIAS_FOR_ANY+1)*(BIAS_FOR_ANY+1);i<width;i++)
            {
                ((int*)Wplywy)[i]+=long(DRAND()*Noise*(4.5*MaxSila))+
                                    ((float*)BiasData->Biases)[i];//cast!!! - sztuczka zeby uniknac potrojnie zagniezdzonej petli
            }

            //Szukanie maksimow - niebanalne (?)
            //////////////////////////////////////////////////////////////////////////
            wb_dynarray<int> FillStat(4);fill(FillStat,0);  //Tablica statystyki z petli szukania maksimow
            

            int indF=-1;
            int indS=-1;
            int indT=-1;
            
            do{ //Petla poszukiwania kolejnych maksimow - do wypelnienia ind{FST}'ow        
                ////////////////////////////////////////////////////////////////////////////
            int width=BIAS_FOR_ANY+1;       //"Szerokosc" tablicy szesciennej na liczniki
            int offsetA=RANDOM(IleKate);            assert(0<=offsetA && offsetA<IleKate);//Jak IleKate==2 to 0 albo 1 itd..
            int offsetB=RANDOM(IleKate);            assert(0<=offsetB && offsetB<IleKate);//Jak IleKate==2 to 0 albo 1 itd..
            int offsetC=RANDOM(IleKate);            assert(0<=offsetC && offsetC<IleKate);//Jak IleKate==2 to 0 albo 1 itd..            
            int Max=-1,pA=-1,pB=-1,pC=-1;
            FillStat[0]++; //Ile nawrotów

            //Szukanie aktualnego maksimum  (nieco rozrzutne, mozna troche przyspieszyc przez jesli BIAS_FOR_ANY bedzie zmienna ==IleKate)
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
                            pA=a;pB=b;pC=c;         //Zapamietanie gdzie znaleziono maksimum
                        }
                    }
                }
            }
            
                                                    assert(pA!=-1 && pB!=-1 && pC!=-1);//Cos musial znalezc (?)
            //Co zrobic z maksimum ???
            Wplywy[pA][pB][pC]=0;//Wyzerowac, zeby nie bruzdzilo w nastepnym szukaniu

            if(pA!=BIAS_FOR_ANY && indF==-1)//Zapamientanie memow do zmiany - tylko wtedy gdy slot jest jeszcze wolny
                indF=pA;
            if(pB!=BIAS_FOR_ANY && indS==-1)
                indS=pB;
            if(pC!=BIAS_FOR_ANY && indT==-1)
                indT=pC;
            FillStat[(indF!=-1)+(indS!=-1)+(indT!=-1)]++;
            }while( indF==-1 || indS==-1 || indT==-1  );

                                                    assert(indF!=-1 && indS!=-1 && indT!=-1);//Po wyjsciu z petli wszystkie musza juz byc ustawione
            
            CenterAgent.First=indF;         //zmieniamy w agencie centralnym            
            CenterAgent.Second=indS;        //zmieniamy w agencie centralnym
            CenterAgent.Third=indT;         //zmieniamy w agencie centralnym

            //cout<<FillStat[0]<<'='<<FillStat[1]<<'+'<<FillStat[2]<<'+'<<FillStat[3]<<flush<<endl;//Wypisanie stastyki nawrotow petli
        }//KONIEC ZMIAN STANU
        //////
        
        //Sila jako wiek
STARZENIE:
        if(jagent::ruchsily)
        {
            CenterAgent.Power+=jagent::ruchsily;
            CenterAgent.Power%=jagent::max_sila;//Nigdy nie przekracza sily maksymalnej
        }
    }
    // upewniamy sie ze iterator zostanie usuniety
    MyGeom->destroy_iterator(Monte);
}


void jworld::_sequentional_bias_information::UseBiasForAgent(       //Implementacja uzycia biasu - tu dosyc skomplikowana, przypomina wykonanie programu
                                int FirstVal,int SecondVal,int ThirdVal,
                                wb_dynarray<int>& Firsts,
                                wb_dynarray<int>& Seconds,
                                wb_dynarray<int>& Thirds
                             )
{
    for(int a=0;a<for_use;a++)//Dla kazdego rekordu "warunkowej instrukcji biasu"
    {
       if( SeqBiases[a].much(FirstVal,SecondVal,ThirdVal))
       {
           switch(SeqBiases[a].whatley){
           case 0: Firsts[SeqBiases[a].lstate]+=SeqBiases[a].value;break;
           case 1: Seconds[SeqBiases[a].lstate]+=SeqBiases[a].value;break;
           case 2: Thirds[SeqBiases[a].lstate]+=SeqBiases[a].value;break;
           default:
               assert("Invalid index of layer during bias processing."==0);
               break;
           }
       }
    }
}

void    jworld::_one_step_sequentional_bias()
{   
                                                                                                      assert(BiasDefinition.OK());
    _sequentional_bias_information* BiasData=dynamic_cast<_sequentional_bias_information*>
                                                            (BiasDefinition.get_ptr_val());           assert(BiasData!=NULL);
    const geometry_base* MyGeom=Agenci.get_geometry();                                                assert(MyGeom!=NULL);  

    //TABLICE POMOCNICZE
    wb_dynarray<int> Firsts(IleKate);
    wb_dynarray<int> Seconds(IleKate);
    wb_dynarray<int> Thirds(IleKate);               assert(MyGeom && Firsts.IsOK() && Seconds.IsOK() && Thirds.IsOK());
    
    //INNE ZMIENNE
    int testowanie=0;
    iteratorh Monte=MyGeom->make_random_global_iterator();//Alokujemy iterator Monte-Carlo
    
    //Idziemy po agentach iteratorem Monte-Carlo. Niektórzy moga sie powtórzyc
    while(Monte)
    {   
        size_t index=MyGeom->get_next(Monte);//Uzyskujemy index losowo wybranego agenta 
        //if(index==MyGeom->FULL)            //Ignorujemy jesli trafil za tablice (zdaza sie dla wycinkow?)
        //      continue;                        
        assert(index!=MyGeom->FULL);         //... tutaj nie powinno sie zdarzyc
        jagent& CenterAgent=*(Agenci.get_ptr(index).get_ptr_val());// Uzyskujemy referencje do agenta omijajac asercje na NULL
        if(Agenci.is_empty(CenterAgent))     // Sprawdzamy czy nie jest to pusta komórka (NULL)
            continue;                        // bo wtedy robic dalej by³oby bez sensu.
        
        if(CenterAgent.Power>TrsSila)        // Czy nie ma juz immunitedu na zmiany
            goto STARZENIE;                  // Ma - nie robimy nic
        
        {   //KOD NA SZUKANIA WPLYWOW
            /////////////////////////////////////
            // Alokujemy iterator sasiedztwa
            iteratorh Neigh=MyGeom->make_random_neighbour_iterator(index,OdlSasiad,IleSasiad);
            //iteratorh Neigh=MyGeom->make_neighbour_iterator(index,OdlSasiad);
            unsigned zliczanie=0;//Zliczanie sasiadów
            
            //Czyszczenie licznikow
            fill(Firsts,0); //memset(Firsts.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(Seconds,0);//memset(Seconds.get_ptr_val(),0,sizeof(int)*IleKate);
            fill(Thirds,0); //memset(Thirds.get_ptr_val(),0,sizeof(int)*IleKate);
                        
            while(Neigh)
            {
                size_t index2=MyGeom->get_next(Neigh);//Uzyskujemy index sasiada        
                if(index2==MyGeom->FULL || index2==index)   //Jesli poza obszarem symulacji lub w 
                    continue;               //centrum obszaru to dalej byloby bez sensu.
                
                jagent& PeryfAgent=*(Agenci.get_ptr(index2).get_ptr_val());//Uzyskujemy referencje do sasiada omijajac asercje na NULL
                if(Agenci.is_empty(PeryfAgent))     //Sprawdzamy czy nie jest to pusta komórka (NULL)
                    continue;                      // bo wtedy robic dalej by³oby bez sensu.
                
                zliczanie++;
                //Dodawanie sil sasiadow do licznikow w tablicach
                Firsts[PeryfAgent.First]+=PeryfAgent.Power;
                Seconds[PeryfAgent.Second]+=PeryfAgent.Power;
                Thirds[PeryfAgent.Third]+=PeryfAgent.Power;
            }
            
            MyGeom->destroy_iterator(Neigh);    // upewniamy sie ze iterator zostanie usuniety
            //Zlicza wylosowanych agentow
            testowanie++;   
            
            //Dodawanie wlasnych sil do licznikow w tablicach
            if(UseSelf)
            {
                Firsts[CenterAgent.First]+=CenterAgent.Power;
                Seconds[CenterAgent.Second]+=CenterAgent.Power;
                Thirds[CenterAgent.Third]+=CenterAgent.Power;
            }
            
            //Szukanie maksimow
            int maxF=INT_MIN,indF=-1;
            int maxS=INT_MIN,indS=-1;
            int maxT=INT_MIN,indT=-1;
            int offset=RANDOM(IleKate);             assert(0<=offset && offset<IleKate);//Jak IleKate==2 to 0 albo 1 itd..
            
            //Dodawanie  biasu za pomoc¹ procedury z BiasData - jesli stan agenta zgadza siê z zadanym to do odpowiedniej tablicy jest dodawany bias
            //-----------------------------------------------------------------------------------------------------------------------------------------
            BiasData->UseBiasForAgent(CenterAgent.First,CenterAgent.Second,CenterAgent.Third,
                                        Firsts,Seconds,Thirds);


            //W petli dodawanie szumu i szukanie maksimow
            //--------------------------------------------------------
            for(int g=0;g<IleKate;g++)
            {
                int h=(g+offset)%IleKate;
                assert(h>=0 && h<IleKate);

                //Dodawanie szumu
                if(Noise>0)
                {
                    if(Firsts[h]>0)
                        Firsts[h]+=long(DRAND()*Noise*(4.5*MaxSila));
                    if(Seconds[h]>0)
                        Seconds[h]+=long(DRAND()*Noise*(4.5*MaxSila));
                    if(Thirds[h]>0)
                        Thirds[h]+=long(DRAND()*Noise*(4.5*MaxSila));
                }

                //Testowanie
                if(Firsts[h]>maxF)
                {
                    maxF=Firsts[h];indF=h;
                }
                if(Seconds[h]>maxS)
                {
                    maxS=Seconds[h];indS=h;
                }
                if(Thirds[h]>maxT)
                {
                    maxT=Thirds[h];indT=h;
                }
            }
            
            assert(indF!=-1 && indS!=-1 && indT!=-1);

            CenterAgent.First=indF;         //zmieniamy w agencie centralnym
            CenterAgent.Second=indS;        //zmieniamy w agencie centralnym
            CenterAgent.Third=indT;         //zmieniamy w agencie centralnym
            
        }//KONIEC ZMIAN STANU
        //////
        
        //Sila jako wiek
STARZENIE:
        if(jagent::ruchsily)
        {
            CenterAgent.Power+=jagent::ruchsily;
            CenterAgent.Power%=jagent::max_sila;//Nigdy nie przekracza sily maksymalnej
        }
    }
    // upewniamy sie ze iterator zostanie usuniety
    MyGeom->destroy_iterator(Monte);
}


/*
friend 
int sort(BiasInfo tab[],size_t N)
{
for(int i=0;i<N;i++)
for(int j=0;j<N-1;j++)
{

  if( tab[j].Warstwa!=BIAS_FOR_ANY && 
  tab[j+1].Warstwa!=BIAS_FOR_ANY && 
  tab[j].Warstwa==tab[j+1].Warstwa )
  {
  cerr<<tab[j]<<" & "<<tab[j+1]<<" concern the same layer"<<endl;
  return -11;//Invalid definition
  }
  else
  if(tab[j].Warstwa>tab[j+1].Warstwa)
  {
  BiasInfo pom=tab[j];
  tab[j]=tab[j+1];
  tab[j+1]=pom;
  }
  else
  {
  //Nothing to do ????
  }
  }
        return 0;
        }
        
          friend
          int MakeOrder(wb_dynarray<BiasInfo> tab)
          {
          wb_dynarray<BiasInfo> old=tab;//Przemieszcza wektor
          tab.alloc(old.get_size());    //Alokuje i wypelnia za pomoca konstruktora
          //Poszukiwanie wspolrzednych i sprawdzenie czy najwyzej po jednej na kazda warstwe!!!
          //------------------------------------------------------------------------------------
          for(int i=0;i<tab.get_size();i++)
          {
          ???///...
          }
          return 0;
          }
          
            friend 
            ostream& PrintConditionalBias(ostream& o,wb_dynarray<BiasInfo> tab) 
            {
            o<<tab[0];
            for(int i=1;i<tab.get_size();i++)
            if(tab[i].Warstwa!=BIAS_FOR_ANY)
            {
            o<<" "<<tab[i];
            }
            
              return o;
              }
              
                */