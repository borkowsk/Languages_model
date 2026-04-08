//DECLARATION OF    A G E N T   FOR "LANGUAGES" SIMULATION
/////////////////////////////////////////////////////////////
#include "layer.hpp"

class jagent:public agent_base
{
	friend class jworld;//Na razie tak - zeby uproscic dostep do skladowych.
	
	// STATYCZNE SKLADOWE - PARAMETRY INICJOWANIA AGENTÓW
	static short ruchsily;//Czy sila sie zmienia (rosnie) z wiekiem
	static short max_sila;//Maksymalna sila agenta
    static short min_sila;//i minimalna
	static short ile_kate;//Ilosc kategori w mapach	
	static short kate_shift;//Przesuniecie dla wczytywania gifa
	
	// SKLADOWE DLA SYMULACJI
	short Power;	//Sila agenta
	short First;	//Pierwsze przekonanie
	short Second;	//Drugie przekonanie
	short Third;	//Trzecie przekonanie
	
	void _clean()
	{
		First=-1;
		Second=-1;
		Third=-1;
		Power=-1;
	}
	
	// TO CO MUSI byc zdefiniowane
	///////////////////////////////////
public:
	int IsOK()
	{
		return First!=-1 && Second!=-1 && Third!=-1 && Power!=-1;
	}
	
	jagent(const jagent& ini);	//Konkretna implementacja w jworld!
	
	jagent();					//Konkretna implementacja w jworld!

	jagent* clone() const
	{ return new jagent(*this);}
		
	~jagent()
	{_clean();}
	
	void clean()
	{_clean();}
	
	void assign123(unsigned char Red,unsigned char Green,unsigned char Blue)
	{
		First=Red>>kate_shift;
		Second=Green>>kate_shift;
		Third=Blue>>kate_shift;		
	}

    void assign1(unsigned char Red,unsigned char Green,unsigned char Blue)
	{
		First=( (int(Red)+int(Green)+int(Blue))/3 ) >>kate_shift;	//Srednie natezenie koloru sklasyfikowane. Najlepiej gdy R=G=B 
	}
    
    void assign2(unsigned char Red,unsigned char Green,unsigned char Blue)
	{
		Second=( (int(Red)+int(Green)+int(Blue))/3 ) >>kate_shift;	//Srednie natezenie koloru sklasyfikowane. Najlepiej gdy R=G=B 
	}

    void assign3(unsigned char Red,unsigned char Green,unsigned char Blue)
	{
		Third=( (int(Red)+int(Green)+int(Blue))/3 ) >>kate_shift;	//Srednie natezenie koloru sklasyfikowane. Najlepiej gdy R=G=B 
	}

	void assignPow(unsigned char Red,unsigned char Green,unsigned char Blue)
	{
		Power=min_sila+short((int(Red)+int(Green)+int(Blue))/(3.*255)*(max_sila-min_sila));
	}
	
	void killBlack(unsigned char Red,unsigned char Green,unsigned char Blue)
	{
		if(Red==0 && Green==0 && Blue==0)
			_clean();
	}
	
	long Classif()
	{
		return First+ile_kate*(Second+ile_kate*Third);
	}
	
	long RGB()
	{
		return ((unsigned long) (((unsigned char) (First) | 
			((unsigned short) (Second) << 8)) | 
			(((unsigned long) (unsigned char) (Third)) << 16))) ;
	}

	friend
	ostream& operator << (ostream& o,const jagent& a)
	{
		o<<'{';
		o<<' '<<a.Power<<' '<<a.First<<' '<<a.Second<<' '<<a.Third<<' ';	
		o<<'}';
		return o;
	}
	
	friend
	istream& operator >> (istream& i,jagent& a)
	{
		char pom;
		i>>pom;		//ignoruje {
		i>>a.Power>>a.First>>a.Second>>a.Third;
		i>>pom;		//ignoruje }
		return i;
	}

};


