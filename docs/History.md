#  "JEZYK" means LANGUAGE:

NOTE: Polish word "J Ę Z Y K" means "LANGUAGE". Hence, the "j" prefix appearing here and there.

## HISTORY

 *    ver.  2.20c - English-language comments.
 *    ver.  2.20b - ADAPTATION TO NEW COMPILATION CONDITIONS IN clion IN 2026.

 *    ...[a dozen or so years apart]...

 *    ver.  2.20 - Introduction of 16 classes to Log-Log histogram added in version 1.401.
                 - Changing the order of series/columns in the log file.
                 - Console mode of operation introduced (in the background, without graphics completely).
 *    ver.  2.11 - Beginnings of using `OptionalParameters` classes to handle call parameters (later abandoned).
 *    ver.  2.10 - Implementation of the process of imposing power and displaying the political map.
 *    ver.  2.06 - Display rate control now working and `my_area_manager` declared.
 *    ver.  2.05 - Expanded menus, especially new visualization options. Also, `SRND` and `DUMP` parameters.
 *    ver.  2.04 - Continuous SW network dump as command line parameter.
 *    ver.  2.03a - Implementation of SW network dump in the form of NET files.
 *    ver.  2.01-2 - Fully working model in the Small Worlds version.
 *    ver.  1.99b - Transitioning to using the Small Worlds model:
                  - Preparing a new visualization layout.
                  - Preparing data sources and graph for long-distance connections.
                  - Implementable algorithm for dynamic long-distance "political" connections.
                  - Incorporating influence from distant connections into the implementation of influence models
                   (not tested for complex biases).

 *    ver.  1.53a - Minor changes necessary to run the build on BDS 2006.

 *    ver.  1.51-2 - Minor cosmetic changes.

 *    version 1.5 - Added language map in "TrueColor", and component maps in RGB component colors.
                     - Changing default startup parameters.

 *    version 1.41a - Adding the language age of a given agent and the data series representing it on a logarithmic graph.

 *    version 1.402a - Introduction of 12 classes to Log-Log histogram added in version 1.401.

 *    version 1.401b - Changing the Log-Log histogram of the language sizes to a 6-class fix histogram,
                       i.e., a very similar visual effect, but with differently labeled classes.
                     - Added fix histogram for language size classes.

 *    version 1.4 - Adding spontaneous mutations in all bias modes.
                  - Introducing the ability to disable spatial correlation calculation (use_spatial_corr) from CODE!!!
                  - The log-log plot of the language sizes distribution introduced (type "dhistosou.h").
                  - Introduction of writing this histogram to the log.
                  - Supplementing some messages about parameter settings (but some were left without).
                  - Entering the `DSTB` parameter that determines the type and degree of force distribution (obtained by * or +).
      
* TEST:
  ```
                  WIDTH=100 DSTB=-8 CLSS=8 MIPO=3 RSPC=1 VIEW=50 LOGF=10 LOGF=testW100.log
  ```

 *    version 1.35a - ???
 *    version 1.34a - Remove assertions against zero-strength agents and introduce a minimal strength limit (def. = 1).
 *    version 1.33a - Recompiling to a new library and adding a menu for MSWindows.
 *    version 1.32a - Recompiling with a new version of the visualization library and introducing
                      the "about_languages.cpp" file with time control for DEBUG mode.
 *    version 1.31a - Implemented initialization of language attributes from three grayscale image files.
 *    version 1.30a - Double bias introduced in the follow-up version, but a parallel bias version still works poorly
                      requires intelligent counter rebalancing.
 *    version 1.20a - Preparation for the introduction of "double bias" (parallel).
 *    version 1.01b - Improved default for a strength threshold.
 *    version 1.05b - Built-in support for batch work and experiment repetition.
 *    version 1.10b - Introducing "bias" for language parameters.
