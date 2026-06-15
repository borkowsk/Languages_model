/// @file
/// @brief Default context menu definition for area manager class.
/// @date 2026-06-15 (last modification)
//        ============================================================
//  Created by borkowsk on 17.04.26.

#ifdef __cplusplus
#error This file is only for pure "C" compilation.
#endif

#include "symshell.h"
#include "lang_res.h"

/* The context menu definition for area manager class.
 * App-specific: "LANGUAGES" */
ssh_menu_item_definition  context_menu_default[]= {
        { "QUIT", SSH_FILE_EXIT },
        { "ABOUT", SSH_HELP_AUTHORSWWWPAGE },
 //       { "One step more", SSH_ONESTEP }, //This doesn't work as it should.
        { "Start/Stop", SSH_STARTSTOP },
        { "Dump screen", SSH_FILE_DUMPSCREEN },
        { "Shortcuts help", SSH_HELP_SHORTCUTHELP  },
        {"A R E A S:",0},
        { "Tile All", SSH_WINDOWS_TILE_ALL },
        { "Mark All", SSH_WINDOWS_MARKALLAREAS },
        { "Unmark All", SSH_WINDOWS_UNMARKALLAREAS },
        { "Tile Marked", SSH_WINDOWS_TILEMARKEDAREAS  },
        { "Hide All Marked", SSH_WINDOWS_HIDEMARKEDAREAS },
        { "Uncover hidden", SSH_WINDOWS_UNCOVERHIDDENAREAS },
        { "Restore original", SSH_WINDOWS_RESTORETOORGINALPOSITION },
        {"B A C K G R O U N D   W O R K:",0},
        {"View every step", ID_VIEWOPT_EVERY1},
        {"More often", ID_VIEWOPT_MOREOFT},
        {"Less often", ID_VIEWOPT_LESSOFT},
        {"View every 10 steps", ID_VIEWOPT_EVERY10},
        {"View every 100 steps", ID_VIEWOPT_EVERY100},
        {"View every 1000 steps", ID_VIEWOPT_EVERY1000},
        {"Dump screen every time", ID_VIEWOPT_DUMPCO},
        {"... ", -1 } //Give the decision to the program (like the right mouse button)
    };

unsigned context_menu_default_size= sizeof(context_menu_default) / sizeof(context_menu_default[0]);

/*v******************************************************************/
/*                    LANGUAGES version 2026                        */
/*v******************************************************************/
/*           THIS CODE IS DESIGNED & COPYRIGHT BY:                  */
/*            W O J C I E C H   B O R K O W S K I                   */
/*    Instytut Studiów Społecznych Uniwersytetu Warszawskiego       */
/*    WWW: https://www.researchgate.net/profile/WOJCIECH_BORKOWSKI  */
/*    GITHUB: https://github.com/borkowsk                           */
/*                                                                  */
/*                               (Don't change or remove this note) */
/*v******************************************************************/
