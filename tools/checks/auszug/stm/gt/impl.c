/* Eine Fassung von my_gmtime() als eigene Uebersetzungseinheit; Name per -Dmy_gmtime=... */
#include <stdint.h>
#include <time.h>
#ifdef PRUEFSTAND_SCHNELLE_TYPEN
#include "pruefstand.h"
#endif
#include SNIPPET
