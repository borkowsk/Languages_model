/// @file
/// @brief Virtual output method implementation of the jworld class. (LANGUAGES PROJECT WITH P.Culicover)
/// @date 2026-05-29 (modified)
///
///     Its is rather simply implementation. You can replace it if you need.
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "jrand.h"
#include "jworld.h"  

int jworld::implement_output(ostream& o) const
{
    const int ret=world::implement_output(o);
    if(ret!=1) return ret;
    o<<separator;
    o<<MyWidth<<separator;
    o << MaxStrength << separator;
    o << MinStrength << separator;
    o << NumOfCate << separator;
    o << NeighDens << separator;
    o << NeighRadius << separator;
    o<<UseSelf<<separator;
    o<<Noise<<separator<<endl;
    o << Agents << endl;
    return 1;
}

/* **************************************************************** */
/*           THIS CODE IS DESIGNED & COPYRIGHT  BY:                 */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zakład Systematyki i Geografii Roślin Uniwersytetu Warszawskiego */
/*  & Instytut Studiow Społecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */

