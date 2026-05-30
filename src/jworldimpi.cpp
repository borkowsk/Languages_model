/// @file
/// @brief Virtual Input method implementation of the jworld class. (LANGUAGES PROJECT WITH P.Culicover)
/// @date 2026-05-31 (modified)
///
///     Its is rather simply implementation. You can replace it if you need.
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "jrand.h"
#include "jworld.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
#pragma ide diagnostic ignored "modernize-use-auto"

using namespace sym2::data;
using namespace sym2::shell;
using namespace sym2::visual;

int jworld::implement_input(istream& i)  
{
    int ret=this->world::implement_input(i);
    if(ret!=1) return ret;
    i>>MyWidth;
    i >> MaxStrength;
    i >> MinStrength;
    i >> NumOfCate;
    i >> NeighDens;
    i >> NeighRadius;
    i>>UseSelf;
    i>>Noise;
    rectangle_geometry* Geom=(rectangle_geometry*)Agents.get_geometry();                         assert(Geom != NULL);
    if(Geom->get_width()!=MyWidth||
       Geom->get_height()!=MyWidth )
            Geom->set(MyWidth,MyWidth,1);
    i >> Agents;
    return 0;
}

#pragma clang diagnostic pop
/* **************************************************************** */
/*            THIS CODE IS DESIGNED & COPYRIGHT BY:                 */
/*            W O J C I E C H   B O R K O W S K I                   */
/* Zakład Systematyki i Geografii Roślin Uniwersytetu Warszawskiego */
/*  & Instytut Studiów Społecznych Uniwersytetu Warszawskiego       */
/*        WWW:  http://moderato.iss.uw.edu.pl/~borkowsk             */
/*        MAIL: borkowsk@iss.uw.edu.pl                              */
/*                               (Don't change or remove this note) */
/* **************************************************************** */

