/*
 * This file is part of the Simutrans project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef DISPLAY_CLIP_NUM_H
#define DISPLAY_CLIP_NUM_H


#include "../simtypes.h"


#define CLIP_NUM_VAR
#define CLIP_NUM_PDECL
#define CLIP_NUM_COMMA
#define CLIP_NUM_DEFAULT_VALUE
#define CLIP_NUM_DEFAULT_ZERO
#define CLIP_NUM_INDEX


/// parameter declaration to be used as first parameter
#define CLIP_NUM_DEF0  CLIP_NUM_PDECL CLIP_NUM_VAR
/// parameter declaration to be used as non-first parameter
#define CLIP_NUM_DEF   CLIP_NUM_COMMA CLIP_NUM_DEF0

/// parameter declaration to be used as first parameter, no variable name
#define CLIP_NUM_DEF_NOUSE0  CLIP_NUM_PDECL
/// parameter declaration to be used as non-first parameter, no variable name
#define CLIP_NUM_DEF_NOUSE   CLIP_NUM_COMMA CLIP_NUM_DEF_NOUSE0
/// pass clip_num as parameter
#define CLIP_NUM_PAR CLIP_NUM_COMMA CLIP_NUM_VAR
/// default value for passing clip_num around
#define CLIP_NUM_DEFAULT CLIP_NUM_COMMA CLIP_NUM_DEFAULT_VALUE


#endif
