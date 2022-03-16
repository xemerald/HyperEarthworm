#ifndef _WORM_STATUS_CODES_H
#define _WORM_STATUS_CODES_H

/*
**  Use only negative numbers for error codes to support methods
**  that might want to return a count as a positive number.
*/
enum WORM_STATUS_CODE
{
    WORM_STAT_BADFILE     = -12 // error
  , WORM_STAT_FILEOPEN    = -11 // error
  , WORM_STAT_BADMALLOC   = -10 //   "
  , WORM_STAT_UNSUPPORT   =  -9 //   "
  , WORM_STAT_MSGSIZE     =  -8 //   "
  , WORM_STAT_FORMATERROR =  -7 //   "
  , WORM_STAT_BADPARM     =  -6 //   "
  , WORM_STAT_NOTINIT     =  -5 //   "
  , WORM_STAT_ARRAYINDEX  =  -4 //   "
  , WORM_STAT_INVALID     =  -3 //   "
  , WORM_STAT_FAILURE     =  -2 //   "
  , WORM_STAT_BADSTATE    =  -1 //   "
  , WORM_STAT_SUCCESS     =   0
  , WORM_STAT_CHANGED     =   1 // state
  , WORM_STAT_NOMATCH     =   2 //   "
  , WORM_STAT_NONEREMAIN  =   3 //   "
  , WORM_STAT_REJECTITEM  =   4 //   "
  , WORM_STAT_DROPITEM    =   5 //   "
  , WORM_STAT_LAPPED      =   6 //   "
  , WORM_STAT_SEQGAP      =   7 //   "
  , WORM_STAT_NODATA      =   8 //   "
  , WORM_STAT_DISCONNECT  =   9 //   "
};



/*
** Handler Status -- commonly for configuration line handling
*/
enum HANDLE_STATUS
{
    HANDLER_INVALID = -1
  , HANDLER_UNUSED  =  0
  , HANDLER_USED    =  1
};

#endif
 