#ifdef _LOGITMT
#error _LOGITMT is for internal use by logit.c and logit_mt.c only
#endif

/* This is the multi-threaded version of logit */

#define _LOGITMT
#include "logit_common.c"

