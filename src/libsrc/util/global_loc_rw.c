
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <chron3.h>  /* needed for ascii time conversion routines */
#include <global_amp_rw.h>
#include <global_loc_rw.h>

#include <kom.h>  /* string parsing */

/*** Versions currently handled by this code */
#define GLOBAL_AMPLINE_VERSION (short)4

#define GLOBAL_PHSLINE_VERSION (short)2

#define GLOBAL_LOC_VERSION (short)3

#include <earthworm.h>

/* ==========================================================================
**                     FUNCTION DEFINITIONS
** ========================================================================= */

/* ------------------------------------------------------------------------- */
/*
** ===============================================================================
*/

GLOBAL_MSG_STATUS InitGlobalAmpLine( GLOBAL_AMPLINE_STRUCT * p_struct )
{
	if ( p_struct == NULL )
   {
     logit( "", "InitGlobalAmpLine(): structure to be initialize is NULL\n");
     return GLOBAL_MSG_NULL;
   }

	p_struct->version     = GLOBAL_AMPLINE_VERSION;
           p_struct->logo.instid   = 0;
           p_struct->logo.mod      = 0;
           p_struct->logo.type     = 0;
           p_struct->pick_sequence = 0;
   strcpy( p_struct->station      , "?" );
   strcpy( p_struct->channel      , "?" );
   strcpy( p_struct->network      , "?" );
   strcpy( p_struct->location     , "?" );
   strcpy( p_struct->amp_time     , "?" );
           p_struct->amptype      = MAGTYPE_UNDEFINED;
           p_struct->adcounts     = 0.0;
           p_struct->period       = 0.0;

   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS InitGlobalPhaseLine( GLOBAL_PHSLINE_STRUCT * p_struct )
{
   short _a;

   if ( p_struct == NULL )
   {
     return GLOBAL_MSG_NULL;
   }
           p_struct->version     = GLOBAL_PHSLINE_VERSION;
           p_struct->logo.instid = 0;
           p_struct->logo.mod    = 0;
           p_struct->logo.type   = 0;
           p_struct->sequence    = 0;
   strcpy( p_struct->station     , "?" );
   strcpy( p_struct->channel     , "?" );
   strcpy( p_struct->network     , "?" );
   strcpy( p_struct->location    , "?" );
   strcpy( p_struct->phase_name  , "?" );
           p_struct->tPhase      = 0.0;
           p_struct->quality     = 0.0;
           p_struct->polarity    = '?';

   for ( _a = 0 ; _a < MAX_AMPS_PER_GLOBALPHASE ; _a++ )
   {
      InitGlobalAmpLine( &p_struct->amps[_a] );
   }
   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS InitGlobalLoc( GLOBAL_LOC_STRUCT * p_struct )
{
   int _i;

   if ( p_struct == NULL )
   {
     return GLOBAL_MSG_NULL;
   }

           p_struct->version     = GLOBAL_LOC_VERSION;
           p_struct->logo.instid = 0;
           p_struct->logo.mod    = 0;
           p_struct->logo.type   = 0;
           p_struct->event_id    = 0;
           p_struct->origin_id   = 0;
           p_struct->tOrigin     = 0.0;
           p_struct->lat         = 0.0;
           p_struct->lon         = 0.0;
           p_struct->depth       = 0.0;
           p_struct->gap         = 0.0;
           p_struct->dmin        = 0.0;
           p_struct->rms         = 0.0;
           p_struct->pick_count  = 0;
           p_struct->nphs        = 0;


   for ( _i = 0 ; _i < GLOBAL_LOC_MAXPHS ; _i++ )
   {
      InitGlobalPhaseLine( &p_struct->phases[_i] );
   }

   return GLOBAL_MSG_SUCCESS;
}

/*
** ===============================================================================
*/

GLOBAL_MSG_STATUS WriteAmpLineToBuffer( GLOBAL_AMPLINE_STRUCT * p_struct
                                      , char                  * p_buffer
                                      , unsigned int            p_length
                                      )
{
   char _author[10];

   if ( p_struct == NULL )
   {
      return GLOBAL_MSG_NULL;
   }

   if ( p_buffer == NULL )
   {
      return GLOBAL_MSG_BADPARAM;
   }

   if ( p_length < GLOBAL_AMPLINE_MAXBUFSIZE )
   {
      return GLOBAL_MSG_TOOSMALL;
   }

   if ( p_struct->version != GLOBAL_AMPLINE_VERSION )
   {
      return GLOBAL_MSG_VERSINVALID;
   }

   EncodeAuthor( p_struct->logo, _author );

   sprintf( p_buffer
          , "%s %ld %d %s %s %s %s %s %d %1.4f %1.4f\n"
          , _author
          , p_struct->pick_sequence
          , p_struct->version
          , p_struct->station
          , p_struct->channel
          , p_struct->network
          , p_struct->location
          , p_struct->amp_time
          , p_struct->amptype
          , p_struct->adcounts
          , p_struct->period
          );

   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */
 
GLOBAL_MSG_STATUS WritePhaseLineToBuffer( GLOBAL_PHSLINE_STRUCT * p_struct
                                        , char                  * p_buffer
                                        , unsigned int            p_length
                                        )
{
   char _author[10];
   char pick_time[20];

   if ( p_struct == NULL )
   {
      return GLOBAL_MSG_NULL;
   }

   if ( p_buffer == NULL )
   {
      return GLOBAL_MSG_BADPARAM;
   }

   if ( p_length < GLOBAL_PHSLINE_MAXBUFSIZE )
   {
      return GLOBAL_MSG_TOOSMALL;
   }

   if ( p_struct->version != GLOBAL_PHSLINE_VERSION )
   {
      return GLOBAL_MSG_VERSINVALID;
   }

   EncodeAuthor( p_struct->logo, _author );

   sprintf( p_buffer
          , "%s %ld %d %s %s %s %s %s %s %1.4f %c\n"
          ,  _author
          , p_struct->sequence
          , p_struct->version
          , p_struct->station
          , p_struct->channel
          , p_struct->network
          , p_struct->location
          , ew_t_to_Date17(p_struct->tPhase, pick_time)
          , p_struct->phase_name
		     , p_struct->quality
		     , p_struct->polarity
          );

   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS WriteLocToBuffer( GLOBAL_LOC_STRUCT * p_loc
                                  , char              * p_buffer
                                  , unsigned int        p_length
                                  )
{
   GLOBAL_MSG_STATUS r_code = GLOBAL_MSG_SUCCESS;

   char * _writelocation = p_buffer;

   char _author[10];
   char origin_time[20];

   int _phaseindex
     , _ampindex
     ;

   GLOBAL_PHSLINE_BUFFER _phs_buffer;

   GLOBAL_AMPLINE_BUFFER _amp_buffer;


   if ( p_loc == NULL )
   {
      return GLOBAL_MSG_NULL;
   }

   if ( p_buffer == NULL )
   {
      return GLOBAL_MSG_BADPARAM;
   }

   if ( p_length < (unsigned int)(  GLOBAL_LOC_SUM_MAXSIZE
                                  + p_loc->nphs * GLOBAL_PHSLINE_MAXBUFSIZE
                                 )
      )
   {
      return GLOBAL_MSG_TOOSMALL;
   }

   if ( p_loc->version != GLOBAL_LOC_VERSION )
   {
      return GLOBAL_MSG_VERSINVALID;
   }

   EncodeAuthor( p_loc->logo, _author );



   sprintf( _writelocation
          , "SUM %s %d %ld %ld %s %.4f %.4f %.3f %.3f %.3f %.3f %d %d\n"
          , _author
          , p_loc->version
          , p_loc->event_id
          , p_loc->origin_id
          , ew_t_to_Date17(p_loc->tOrigin,origin_time)
          , p_loc->lat
          , p_loc->lon
          , p_loc->depth
          , p_loc->gap
          , p_loc->dmin
          , p_loc->rms
          , p_loc->pick_count
          , p_loc->nphs
          );

   _writelocation = p_buffer + strlen(p_buffer);

   for ( _phaseindex = 0 ; _phaseindex < p_loc->nphs ; _phaseindex++ )
   {
      if ( (r_code = WritePhaseLineToBuffer( &p_loc->phases[_phaseindex]
                                           , _phs_buffer
                                           , GLOBAL_PHSLINE_MAXBUFSIZE
                                           ) ) == GLOBAL_MSG_SUCCESS )
      {
         if ( GLOBAL_LOC_MAXBUFSIZE <= (strlen(p_buffer) + strlen("PHS ") + strlen(_phs_buffer) + 1) )
         {
            return GLOBAL_MSG_DEFINESMALL;
         }
         strcat( _writelocation, "PHS " );
         strcat( _writelocation, _phs_buffer );

         for ( _ampindex = 0 ; _ampindex < MAX_AMPS_PER_GLOBALPHASE ; _ampindex++ )
		 {
            if ( p_loc->phases[_phaseindex].amps[_ampindex].logo.type == 0 )
			{
               /* no amp of this type */
			   continue;
			}

			if ( (r_code = WriteAmpLineToBuffer( &p_loc->phases[_phaseindex].amps[_ampindex]
                                               , _amp_buffer
                                               , GLOBAL_AMPLINE_MAXBUFSIZE )
                 ) == GLOBAL_MSG_SUCCESS )
            {
               if ( GLOBAL_LOC_MAXBUFSIZE <= (strlen(p_buffer) + strlen("AMP ") + strlen(_amp_buffer) + 1) )
               {
                  return GLOBAL_MSG_DEFINESMALL;
               }
               strcat( _writelocation, "AMP " );
               strcat( _writelocation, _amp_buffer );
            }
            else
            {
               return r_code;
            }
         }
      }
      else
      {
         return r_code;
      }
   }

   /* Message-Terminating newline */
   strcat( p_buffer, "\n" );

   return r_code;
}
/*
** ===============================================================================
*/

GLOBAL_MSG_STATUS StringToAmpLine( GLOBAL_AMPLINE_STRUCT * p_amp, char * p_string )
{
   char _str[80];
   long amptype;

   GLOBAL_AMPLINE_BUFFER workbuff;

   if ( p_amp == NULL )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE A\n");
      return GLOBAL_MSG_NULL;
   }

   if ( p_string == NULL )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE B\n");
      return GLOBAL_MSG_BADPARAM;
   }

   InitGlobalAmpLine(p_amp);

   if ( GLOBAL_AMPLINE_MAXBUFSIZE < strlen( p_string ) )
   {
      strncpy( workbuff , p_string , GLOBAL_AMPLINE_MAXBUFSIZE );
	  workbuff[GLOBAL_AMPLINE_MAXBUFSIZE] = '\0';
   }
   else
   {
	   strcpy( workbuff, p_string );
   }

   k_put(workbuff);

   /*  Author  */
   strcpy( _str , k_str() );
   if ( strlen(_str) == 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE D\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   if ( DecodeAuthor( &(p_amp->logo), _str ) != GLOBAL_MSG_SUCCESS )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE E\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  pick sequence number  */
   p_amp->pick_sequence = k_long();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE F\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  version number  */
   p_amp->version = (short)k_long();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE F\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   if ( p_amp->version != GLOBAL_AMPLINE_VERSION )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE G %d\n", p_amp->version );
      return GLOBAL_MSG_VERSINVALID;
   }

   /*  Station  */
   strcpy( p_amp->station , k_str() );
   if ( strlen(p_amp->station) == 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE H\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Channel  */
   strcpy( p_amp->channel , k_str() );
   if ( strlen(p_amp->channel) == 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE I\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Network  */
   strcpy( p_amp->network , k_str() );
   if ( strlen(p_amp->network) == 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE J\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Location  */
   strcpy( p_amp->location , k_str() );
   if ( strlen(p_amp->location) == 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE K\n");
      return GLOBAL_MSG_FORMATERROR;
   }


   /*  Amplitude Time  */
   strcpy( p_amp->amp_time , k_str() );
   if ( strlen(p_amp->amp_time) == 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE L\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Amplitude Type  */
   amptype = (int)k_long();
   if (amptype == AMPTYPE_ML)
       p_amp->amptype = MAGTYPE_LOCAL_PEAK2PEAK;
   else if (amptype == AMPTYPE_MB)
       p_amp->amptype = MAGTYPE_BODYWAVE;
   else if (amptype == AMPTYPE_MBLG)
       p_amp->amptype = MAGTYPE_MBLG;
   else
   {
       logit ("", "Invalid amplitude type <%ld>\n", amptype);
       return GLOBAL_MSG_FORMATERROR;
   }
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE M\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  A / D Counts  */
   p_amp->adcounts = k_val();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE N\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Period  */
   p_amp->period = k_val();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToAmpLine() FAILURE O\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS StringToPhaseLine( GLOBAL_PHSLINE_STRUCT * p_struct, char * p_string )
{
   char _str[80];
   char * pStr;

   GLOBAL_PHSLINE_SUM_BUFFER workbuff;

   if ( p_struct == NULL )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE A\n");
      return GLOBAL_MSG_NULL;
   }

   if ( p_string == NULL )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE C\n");
      return GLOBAL_MSG_BADPARAM;
   }

   InitGlobalPhaseLine(p_struct);

   if ( GLOBAL_PHSLINE_SUM_MAXSIZE < strlen( p_string ) )
   {
      strncpy( workbuff , p_string , GLOBAL_PHSLINE_SUM_MAXSIZE );
	  workbuff[GLOBAL_PHSLINE_SUM_MAXSIZE] = '\0';
   }
   else
   {
	   strcpy( workbuff, p_string );
   }

   k_put(workbuff);

   /*  Author  */
   strcpy( _str , k_str() );

   if ( strlen(_str) == 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE D\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   if ( DecodeAuthor( &(p_struct->logo), _str ) != GLOBAL_MSG_SUCCESS )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE E\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  sequence number  */
   p_struct->sequence = k_long();
   if ( k_err() != 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE F\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  version number  */
   p_struct->version = (short)k_long();
   if ( k_err() != 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE G\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   if ( p_struct->version != GLOBAL_PHSLINE_VERSION )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE H\n");
      return GLOBAL_MSG_VERSINVALID;
   }

   /*  Station  */
   strcpy( p_struct->station , k_str() );
   if ( strlen(p_struct->station) == 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE I\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Channel  */
   strcpy( p_struct->channel , k_str() );
   if ( strlen(p_struct->channel) == 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE J\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Network  */
   strcpy( p_struct->network , k_str() );
   if ( strlen(p_struct->network) == 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE K\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Location  */
   strcpy( p_struct->location , k_str() );
   if ( strlen(p_struct->location) == 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE L\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Pick Time  */
   p_struct->tPhase = ew_Date17_to_t(pStr=k_str() );
   if ( p_struct->tPhase <= 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE M (%s)=%.2f\n",
      pStr, p_struct->tPhase);
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Phase Name  */
   strcpy( p_struct->phase_name , k_str() );
   if ( strlen(p_struct->phase_name) == 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE N\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Quality  */
   p_struct->quality = k_val();
   if ( k_err() != 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE O\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Polarity  */
   strcpy( _str, k_str() );
   if ( strlen(_str) == 0 )
   {
logit( "", "DEBUG StringToPhaseLine() FAILURE P\n");
      return GLOBAL_MSG_FORMATERROR;
   }
   p_struct->polarity = _str[0];


   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS StringToLoc( GLOBAL_LOC_STRUCT * p_loc, char * p_string )
{
   /*
   **  Storage space for the lines read from the file.
   **  Will allow for contatenation of null-terminator
   **  to simplify chunking.
   */

   GLOBAL_PHSLINE_STRUCT * _phs_pointer = NULL;

   GLOBAL_AMPLINE_STRUCT   _amp_data;

   GLOBAL_MSG_STATUS _processresult;

   char * _readpoint = p_string
      , * _linemark
      ;

   int _phscount;

   char _readingchildren = 1;

   char _str[80];

   GLOBAL_LOC_SUM_BUFFER sumbuff;
   int  _index;

   if ( p_loc == NULL || p_string == NULL )
   {
       logit( "", "StringToAmpLine(): item is null\n");
	   return GLOBAL_MSG_NULL;
   }

   InitGlobalLoc(p_loc);

   /* Find the end of the SUM line */
   if ( (_linemark = strchr( _readpoint, '\n' )) == NULL )
   {
      /* didn't find the newline delimiter */
      logit( "", "DEBUG StringToLoc() FAILURE A\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   _index = _linemark - _readpoint;

   strncpy( sumbuff , _readpoint , _index );
   sumbuff[_index] = '\0';

   k_put(sumbuff);

   /*  Line Type (SUM) */
   strcpy( _str , k_str() );
   if ( strcmp(_str, "SUM") != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE B\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Author  */
   strcpy( _str , k_str() );
   if ( strlen(_str) == 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE C\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   if ( DecodeAuthor( &(p_loc->logo), _str ) != GLOBAL_MSG_SUCCESS )
   {
      logit( "", "DEBUG StringToLoc() FAILURE D\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  version number  */
   p_loc->version = (short)k_long();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE E\n");
      return GLOBAL_MSG_FORMATERROR;
   }


   if ( p_loc->version != GLOBAL_LOC_VERSION )
   {
      logit( "", "DEBUG StringToLoc() FAILURE E 2\n");
      return GLOBAL_MSG_VERSINVALID;
   }


   /*  Event Id  */
   p_loc->event_id = k_long();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE F\n");
      return GLOBAL_MSG_FORMATERROR;
   }


   /*  Origin Id  */
   p_loc->origin_id = k_long();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE G\n");
      return GLOBAL_MSG_FORMATERROR;
   }


   /*  Origin Time  */
   p_loc->tOrigin = ew_Date17_to_t(k_str() );
   if ( p_loc->tOrigin <= 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE H\n");
      return GLOBAL_MSG_FORMATERROR;
   }


   /*  Latitude  */
   p_loc->lat = k_val();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE I\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Longitude  */
   p_loc->lon = k_val();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE J\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  Depth  */
   p_loc->depth = (float)k_val();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE K\n");
      return GLOBAL_MSG_FORMATERROR;
   }


   /*  Gap  */
   p_loc->gap = (float)k_val();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE L\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  DMin  */
   p_loc->dmin = (float)k_val();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE M\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /*  RMS  */
   p_loc->rms = (float)k_val();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE N\n");
      return GLOBAL_MSG_FORMATERROR;
   }


   /*  Number of Picks used to calculate origin  */
   p_loc->pick_count = (int)k_long();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE O\n");
      return GLOBAL_MSG_FORMATERROR;
   }



   /*  Number of Phase structures included  */
   _phscount = (int)k_long();
   if ( k_err() != 0 )
   {
      logit( "", "DEBUG StringToLoc() FAILURE P\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   /* END OF SUM LINE */

   do
   {
      /* Set read point to start of next child line */
      _readpoint = _linemark + 1;

	  /*
	  ** need the \r to handle file2ring
	  */
      if ( *_readpoint == '\n' || *_readpoint == '\r' )
      {
         /* next char in file is newline, this indicated file termination */
         _readingchildren = 0;
         break;
      }
/*
logit( ""
	 , "DEBUG parseline  (%d) [%d  %d]\n>%s<\n"
	 , strlen(_readpoint)
	 , *_readpoint
	 , '\n'
	 , _readpoint
	 );
*/
      if ( strncmp( _readpoint, "PHS ", 4 ) == 0 )
      {
         /* Phase Line */

         if ( p_loc->nphs == GLOBAL_LOC_MAXPHS )
         {
            /* no space for any more Phases */
logit( "", "DEBUG StringToLoc() FAILURE Q\n");
            return GLOBAL_MSG_MAXCHILDREN;
         }

         /* Advance the pointer to the first data location */
         _readpoint += 4;

         /* Find the end of the PHS line */
         if ( (_linemark = strchr( _readpoint, '\n' )) == NULL )
         {
            /* didn't find the newline delimiter */
logit( "", "DEBUG StringToLoc() FAILURE R\n");
            return GLOBAL_MSG_FORMATERROR;
         }

         /* null-terminate the PHS line (into a string) */
         *_linemark = '\0';

         _phs_pointer = &p_loc->phases[p_loc->nphs];

         /* parse the working buffer into the location's next Phase struct */
         if ( (_processresult = StringToPhaseLine( _phs_pointer, _readpoint )) != GLOBAL_MSG_SUCCESS )
         {
logit( "", "DEBUG StringToLoc() FAILURE S on phase %d\n", p_loc->nphs+1);
            return _processresult;
         }


         /* increment the phase count */
         p_loc->nphs++;

         /* End of PHS Line */

      }
      else if ( strncmp( _readpoint, "AMP ", 4 ) == 0 )
      {
         /* Amplitude Line */

         if ( p_loc->nphs == 0 || _phs_pointer == NULL )
         {
            /*
            ** should already have read a phase line to contain this amp,
            ** thus, this is an error
            */
logit( "", "DEBUG StringToLoc() FAILURE T\n");
            return GLOBAL_MSG_FORMATERROR;
         }


         /* Advance the pointer to the first data location */
         _readpoint += 4;

         /* Find the end of the AMP line */
         if ( (_linemark = strchr( _readpoint, '\n' )) == NULL )
         {
            /* didn't find the newline delimiter */
logit( "", "DEBUG StringToLoc() FAILURE V\n");
            return GLOBAL_MSG_FORMATERROR;
         }

         /* null-terminate the AMP line (into a string) */
         *_linemark = '\0';


         /* parse the working buffer into the current Phase's next Amp struct */
         if ( (_processresult = StringToAmpLine( &_amp_data, _readpoint )) != GLOBAL_MSG_SUCCESS )
         {
logit( "", "DEBUG StringToLoc() FAILURE W\n");
            return _processresult;
         }

         /* increment the phase count */
         if ( (_processresult = AddAmpLineToPhase( _phs_pointer, &_amp_data )) < 0 )
		 {
logit( "", "DEBUG StringToLoc() FAILURE X\n");
            return _processresult;
		 }

         /* End of AMP Line */

      }
      else
      {
         /* Unrecognized Line start tag */
logit( "", "DEBUG StringToLoc() FAILURE Y\n>%s<\n", _readpoint);
         return GLOBAL_MSG_BADCHILD;
      }

   } while( _readingchildren == 1 );

   if ( p_loc->nphs != _phscount )
   {
      logit( "", "DEBUG StringToLoc() FAILURE Z\n");
      return GLOBAL_MSG_FORMATERROR;
   }

   return GLOBAL_MSG_SUCCESS;
}

/*
** ===============================================================================
*/

int AddAmpLineToPhase( GLOBAL_PHSLINE_STRUCT * p_phase
                     , GLOBAL_AMPLINE_STRUCT * p_amp
                     )
{
   GLOBAL_AMPLINE_STRUCT * _destin;

   int ampindex = GLOBAL_MSG_BADAMPTYPE;

   if ( p_phase == NULL || p_amp == NULL )
   {
      return GLOBAL_MSG_NULL;
   }

   if ( p_phase->version != GLOBAL_PHSLINE_VERSION )
   {
      return GLOBAL_MSG_VERSINVALID;
   }

   if ( p_amp->version != GLOBAL_AMPLINE_VERSION )
   {
      return GLOBAL_MSG_BADPARAM;
   }

   /* Index into amp array does not include unknown ('?') */
   ampindex = p_amp->amptype - 1;

   if ( 0 <= ampindex && ampindex < MAX_AMPS_PER_GLOBALPHASE )
   {
      _destin = &p_phase->amps[ampindex];

              _destin->logo.type     = p_amp->logo.type;
              _destin->logo.mod      = p_amp->logo.mod;
              _destin->logo.instid   = p_amp->logo.instid;
              _destin->pick_sequence = p_amp->pick_sequence;
              _destin->version       = p_amp->version;
      strcpy( _destin->station       , p_amp->station );
      strcpy( _destin->channel       , p_amp->channel );
      strcpy( _destin->network       , p_amp->network );
      strcpy( _destin->location      , p_amp->location );
      strcpy( _destin->amp_time      , p_amp->amp_time );
              _destin->amptype       = p_amp->amptype;
              _destin->adcounts      = p_amp->adcounts;
              _destin->period        = p_amp->period;

   }

   return ampindex;
}

/* ------------------------------------------------------------------------- */

int AddPhaseLineToLoc( GLOBAL_LOC_STRUCT     * p_loc
                     , GLOBAL_PHSLINE_STRUCT * p_phase
                     )
{
   int r_code;

   GLOBAL_PHSLINE_STRUCT * _destin;

   short   _a;

   if ( p_loc == NULL || p_phase == NULL )
   {
      return GLOBAL_MSG_NULL;
   }

   if ( p_loc->version != GLOBAL_LOC_VERSION )
   {
      return GLOBAL_MSG_VERSINVALID;
   }

   if ( p_phase->version != GLOBAL_PHSLINE_VERSION )
   {
      return GLOBAL_MSG_BADPARAM;
   }

   if ( p_loc->nphs == GLOBAL_LOC_MAXPHS )
   {
      return GLOBAL_MSG_MAXCHILDREN;
   }

   _destin = &p_loc->phases[p_loc->nphs];

           _destin->version     = p_phase->version;
           _destin->logo.type   = p_phase->logo.type;
           _destin->logo.mod    = p_phase->logo.mod;
           _destin->logo.instid = p_phase->logo.instid;
           _destin->sequence    = p_phase->sequence;
   strcpy( _destin->station     , p_phase->station );
   strcpy( _destin->channel     , p_phase->channel );
   strcpy( _destin->network     , p_phase->network );
   strcpy( _destin->location    , p_phase->location );
   strcpy( _destin->phase_name  , p_phase->phase_name );
           _destin->tPhase      = p_phase->tPhase;
           _destin->quality     = p_phase->quality;
           _destin->polarity    = p_phase->polarity;

   for ( _a = 0 ; _a < MAX_AMPS_PER_GLOBALPHASE ; _a++ )
   {
	  if ( p_phase->amps[_a].logo.type == 0 )
	  {
		  continue;
	  }
      if ( (r_code = AddAmpLineToPhase( _destin, &p_phase->amps[_a] )) < 0 )
      {
         return r_code;
      }
   }

   p_loc->nphs++;

   return p_loc->nphs - 1;
}

/*
** ===============================================================================
*/

int GetLocPhaseIndex( GLOBAL_LOC_STRUCT     * p_loc
                    , GLOBAL_PHSLINE_STRUCT * p_phase
                    )
{
   int r_index = 0
     , _idx = 0
     ;

   for ( ; _idx < p_loc->nphs ; _idx++ )
   {
      if (   p_phase->sequence    == p_loc->phases[_idx].sequence
          && p_phase->logo.mod    == p_loc->phases[_idx].logo.mod
          && p_phase->logo.instid == p_loc->phases[_idx].logo.instid
         )
      {
         /* this is the match, return */
         r_index = _idx;
         break;
      }
   }

   return r_index;
}

/*
** ===============================================================================
*/

GLOBAL_MSG_STATUS ClearAmpLines( GLOBAL_PHSLINE_STRUCT * p_phase )
{
   int _a;

   if ( p_phase == NULL )
   {
      return GLOBAL_MSG_NULL;
   }
   if ( p_phase->version != GLOBAL_LOC_VERSION )
   {
      return GLOBAL_MSG_VERSINVALID;
   }
   for ( _a = 0 ; _a < MAX_AMPS_PER_GLOBALPHASE ; _a++ )
   {
      InitGlobalAmpLine( &p_phase->amps[_a] );
   }
   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS ClearPhaseLines( GLOBAL_LOC_STRUCT * p_loc )
{
   if ( p_loc == NULL )
   {
      return GLOBAL_MSG_NULL;
   }
   if ( p_loc->version != GLOBAL_LOC_VERSION )
   {
      return GLOBAL_MSG_VERSINVALID;
   }
   p_loc->nphs = 0;
   return GLOBAL_MSG_SUCCESS;
}

/*
** ===============================================================================
*/
/*
** ===============================================================================
*/

