          /***************************************************
           *                  earthworm.h                    *
           *                                                 *
           *         Earthworm administrative setup:         *
           *        global info for all installations        *
           *                                                 *
           ***************************************************/

#ifndef EARTHWORM_H
#define EARTHWORM_H

/* If this is MSVC++, the minimum version is 19.x (Visual Studio 2015) */
#ifdef _MSC_VER
#if _MSC_VER < 1900
#error Earthworm requires MSVC++ 19.x, or later (Visual Studio 2015)
#endif
#endif

/* include simple definitions */
#include "earthworm_defs.h"

/* include simple functions whose prototypes do
   not require any convoluted STUFF from platform.h */
#include "earthworm_simple_funcs.h"

/* include the really ugly stuff that won't event compile
   without platform.h or other include files */
#include "earthworm_complex_funcs.h"

#endif
