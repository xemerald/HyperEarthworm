
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: neic2scn.c 5805 2013-08-14 15:55:28Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.4  2001/07/01 22:13:07  davidk
 *     Removed some unneeded function prototypes.
 *
 *     Revision 1.3  2000/09/27 18:57:08  lucky
 *     Fixed logit calls.
 *
 *     Revision 1.2  2000/05/02 19:46:15  lucky
 *     Cosmetic fixes needed for warning-free NT compilation
 *
 *     Revision 1.1  2000/03/31 18:25:48  lucky
 *     Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <earthworm.h>
#include <transport.h>
#include <neic2scn.h>

/* func prototype */
int IsComment( char string[] );

  /***************************************************************
   *                         GetNEICStaList()                    *
   *                                                             *
   *                     Read the station list                   *
   *                                                             *
   ***************************************************************/

int GetNEICStaList (NEIC2SCN **Sta, int *Nsta, char *filename)
{
   char    string[150];
   int     i;
   int     nsta;
   NEIC2SCN *sta;
   FILE    *fp;

	if ((Sta == NULL) || (Nsta == NULL) || (filename == NULL))
	{
		logit ("e", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}

/* Open the station list file
   **************************/
   if ((fp = fopen (filename, "r") ) == NULL)
   {
      logit( "e", "Error opening station list file <%s>.\n",
             filename);
      return EW_FAILURE;
   }

/* Count channels in the station file.
   Ignore comment lines and lines consisting of all whitespace.
   ***********************************************************/
   nsta = 0;
   while ( fgets( string, 130, fp ) != NULL )
      if ( !IsComment( string ) ) nsta++;

   rewind( fp );

/* Allocate the station list
   *************************/
   sta = (NEIC2SCN *) calloc( nsta, sizeof(NEIC2SCN) );
   if ( sta == NULL )
   {
      logit( "e", "Cannot allocate the station array\n" );
      return EW_FAILURE;
   }


/* Read stations from the station list file into the station array
   **************************************************************/
   i = 0;
   while ( fgets( string, 130, fp ) != NULL )
   {
      int ndecoded;

      if ( IsComment( string ) ) continue;
      ndecoded = sscanf( string,
              "%s%s%s%s", 
               sta[i].neic_sta,
               sta[i].sta,
               sta[i].comp,
               sta[i].net);
      if (ndecoded < 4)
      {
         logit( "e", "Error decoding station file.\n" );
         logit( "e", "ndecoded: %d\n", ndecoded );
         logit( "e", "Offending line:\n" );
         logit( "e", "%s\n", string );
         return EW_FAILURE;
      }
      i++;
   }
   fclose( fp );
   *Sta  = sta;
   *Nsta = nsta;
   return EW_SUCCESS;
}


    /*********************************************************************
     *                             IsComment()                           *
     *                                                                   *
     *  Accepts: String containing one line from a pick_ew station list  *
     *  Returns: 1 if it's a comment line                                *
     *           0 if it's not a comment line                            *
     *********************************************************************/

int IsComment( char string[] )
{
   int i;

   for ( i = 0; i < (int)strlen( string ); i++ )
   {
      char test = string[i];

      if ( test!=' ' && test!='\t' && test!='\n' )
      {
         if ( test == '#'  )
            return 1;          /* It's a comment line */
         else
            return 0;          /* It's not a comment line */
      }
   }
   return 1;                   /* It contains only whitespace */
}


    /*********************************************************************
     *                         MatchNeic2SCN ()                          *
     *                                                                   *
     *********************************************************************/
int  MatchNeic2SCN (char *neic_sta, char *sta, char *comp, 
			char *net, NEIC2SCN *neic2scn, int nSta)
{

	int		i;

	if ((neic2scn == NULL) || (nSta < 0) || (neic_sta == NULL) ||
			(sta == NULL) || (comp == NULL) || (net == NULL))
	{
		logit ("", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}


	/* Strip off any trailing spaces */
	i = 0;
	while ((i < 6) && (neic_sta[i] != ' '))
	{
		i = i + 1;
	}
	if (i != 6)
		neic_sta[i] = '\0';
	else
		neic_sta[i + 1] = '\0';


	/* check against the list */
	i = 0;
	while (i < nSta)
	{
		if (strcmp (neic2scn[i].neic_sta, neic_sta) == 0)
		{
			/* Found a Match! */
			strcpy (sta, neic2scn[i].sta);
			strcpy (comp, neic2scn[i].comp);
			strcpy (net, neic2scn[i].net);
			return EW_SUCCESS;
		}

		else
		{
			i = i + 1;
		}
	}

	/* Not Found! */
	strcpy (sta, neic_sta);
	strcpy (comp, "???");
	strcpy (net, "???");


	return EW_SUCCESS;

}
