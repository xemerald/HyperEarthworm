/*
 * site.c : Station parameter routines.
 *
 *$ 95Aug31 LDD Added "site_file" command to site_com()
 *$ 95Sep01 LDD Added 2nd & 3rd args to site_index()
 *$ 95Oct19 LDD Explicitly declared return types for all functions.
 *              Added function prototypes to site.h
 *  2018-06-26 modified the "site" command to also read CNL
 */
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "kom.h"
#include "site.h"
#include "earthworm_simple_funcs.h"

/* Function Prototypes
 *********************/
int  compareSCNL( const void *s1, const void *s2 );

/* Initialization constants
 **************************/
static int initSite = 0;
static int sortSite = 0;

/* Changed from 1000 to 1800 by WMK 2/12/96 */
static int maxSite  = 1800;     /* Change to alter size of mem allocation */

/**************************************************************************
 * rtrim()  Trim trailing blanks off a string                             *
 **************************************************************************/
void rtrim( char *str )
{
   int len = strlen( str );
   int i;
 
   for( i=len-1; i>=0; i-- ) if( str[i]==' ') str[i] = '\0';
   return;
}

/**************************************************************************
 * site_init()  Allocate the site table                                   *
 **************************************************************************/
void site_init(void)
{
        if(initSite) return;
        initSite = 1;
        nSite = 0;
        Site = (SITE *)calloc(maxSite, sizeof(SITE));
        if(!Site) {
           logit("e", "site_init:  Could not allocate site table; exiting!\n");
           exit(0);
        }
        return;
}

/**************************************************************************
 * site_sort()  qsort the site table for faster lookup with bsearch       *
 **************************************************************************/
void site_sort(void)
{
        int i;

        if(sortSite) return;
        qsort( Site, nSite, sizeof(SITE), compareSCNL );
        for( i=0; i<nSite; i++ ) Site[i].index = i;
        sortSite = 1;
        return;
}


/**************************************************************************
 * site_load(name)  Process a kom.c-style command file that contains only *
 *                  commands recognized by site_com                       *
 **************************************************************************/

int site_load(char *name)
{
        char *com;

        if(!k_open(name)) {
           logit( "e", "site_load:  Cannot open site file <%s>\n", name);
           return 0;
        }
        while(k_rd()) {
           com = k_str();
           if ( !com )       continue;
           if ( site_com() ) continue;
           logit( "e", "site_load:  <%s> Unknown command\n", com);
        }
        k_close();
/*      logit("e", "Site file <%s> loaded, nSite = %d\n", name, nSite); */
        return 1;
}


/**************************************************************************
 *  site_read(name)  Read in a HYPOINVERSE format, universal station      *
 *                   code file                                            *
 **************************************************************************/

/* Sample station line:
R8075 MN  BHZ  41 10.1000N121 10.1000E   01.0     0.00  0.00  0.00  0.00 1  0.00N1
0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 12
*/

void site_read(char *name)
{
        FILE  *stafile;
        char   line[256];
        int    dlat, dlon, elev;
        float  mlat, mlon;
        char   comp, ns, ew;
        int    n;
        int    readloc;

/* initialize site table
   *********************/
        site_init();

/* open station file
   *****************/
        if( (stafile = fopen( name, "r" )) == (FILE *) NULL ) {
                logit("e",
                      "site_read: Cannot open site file <%s>; exiting!\n", name);
                exit(0);
        }

/* read in one line of the site file at a time
   *******************************************/
        while( fgets( line, sizeof(line), stafile ) != (char *) NULL )
        {
        /* see if line is long enough to include location code */
                if( strlen(line) >= 82 ) readloc = 1;
                else                     readloc = 0;
    
        /* see if internal site table has room left */
                if( nSite >= maxSite ) {
                   logit("e",
                        "site_read: Site table full; cannot load entire file <%s>\n", name );
                   logit("e",
                        "site_read: Use <maxsite> command to increase table size; exiting!\n" );
                   exit(0);
                }

        /* decode each line of the file */
                strncpy( Site[nSite].name, &line[0],  5);
                strncpy( Site[nSite].net,  &line[6],  2);
                strncpy( Site[nSite].comp, &line[10], 3);
                if( readloc ) strncpy( Site[nSite].loc, &line[80], 2);
                else          strcpy ( Site[nSite].loc, LOC_NULL_STRING );
                comp = line[9];
             
        /* trim off trailing blanks */
                rtrim( Site[nSite].name );
                rtrim( Site[nSite].net  );
                rtrim( Site[nSite].comp );
                rtrim( Site[nSite].loc  );

                line[42] = '\0';
                n = sscanf( &line[38], "%d", &elev );
                if( n < 1 ) {
                   logit("e",
                         "site_read: Error reading elevation from line "
                         "in station file\n%s\n", line );
                   continue;
                }

                ew       = line[37];
                line[37] = '\0';
                n = sscanf( &line[26], "%d %f", &dlon, &mlon );
                if( n < 2 ) {
                   logit("e",
                         "site_read: Error reading longitude from line "
                          "in station file\n%s\n", line );
                   continue;
                }

                ns       = line[25];
                line[25] = '\0';
                n = sscanf( &line[15], "%d %f", &dlat, &mlat );
                if ( n < 2 ) {
                   logit("e",
                         "site_read: Error reading latitude from line "
                         "in station file\n%s\n", line );
                   continue;
                }

        /*      printf( "%-5s %-2s %-3s %-2s %d %.4f%c%d %.4f%c%4d\n",
                         Site[nSite].name, Site[nSite].net, 
                         Site[nSite].comp, Site[nSite].loc,
                         dlat, mlat, ns,
                         dlon, mlon, ew, elev ); */ /*DEBUG*/

        /* use one-letter component if there is no 3-letter component given */
                if ( strlen(Site[nSite].comp)==0 ) Site[nSite].comp[0] = comp;

        /* convert to decimal degrees */
                if ( dlat < 0 ) dlat = -dlat;
                if ( dlon < 0 ) dlon = -dlon;
                Site[nSite].lat = (double) dlat + (mlat/60.0);
                Site[nSite].lon = (double) dlon + (mlon/60.0);

        /* make south-latitudes and west-longitudes negative */
                if ( ns=='s' || ns=='S' )
                        Site[nSite].lat = -Site[nSite].lat;
                if ( ew=='w' || ew=='W' || ew==' ' )
                        Site[nSite].lon = -Site[nSite].lon;
                Site[nSite].elev = (double) elev/1000.;

        /*      printf("%-5s %-2s %-3s %-2s %.4f %.4f %.0f\n\n",
                       Site[nSite].name, Site[nSite].net, 
                       Site[nSite].comp, Site[nSite].loc,
                       Site[nSite].lat, Site[nSite].lon, Site[nSite].elev ); */ /*DEBUG*/

       /* update the total number of stations loaded */
                if(nSite < maxSite) ++nSite;

        } /*end while*/

        fclose( stafile );
        return;
}


  /**********************************************************************
   * site_com(): Process all recognized commands.                       *
   *             Return 1 on success, 0 if command was not recognized   *
   **********************************************************************/

int site_com( void )
{
        char *name, *net, *comp, *loc;

        if(k_its("site")) {
                site_init();
                if(nSite >= maxSite) return 1;
                name = k_str();
                if(!name) return 1;
                strcpy(Site[nSite].name, name);
                net = k_str();
                if(!net) return 1;
                strcpy(Site[nSite].net,  net);
                comp = k_str();
                if(!comp) return 1;
                strcpy(Site[nSite].comp, comp);
                loc = k_str();
                if(!loc) return 1;
                strcpy(Site[nSite].loc,  loc);
                Site[nSite].lat = k_val();
                Site[nSite].lon = k_val();
                Site[nSite].elev = 0.0;
                Site[nSite].elev = k_val();
                if(nSite < maxSite)
                        nSite++;
                return 1;
        }

        if(k_its("maxsite")) {
                if(initSite) {
                     logit("e", "site_com:  Error: site table already allocated.\n" );
                     logit("e",
                           "site_com:  Use <maxsite> before any <site> or <site_file> commands" );
                     logit("e", "; exiting!\n" );
                     exit( 0 );
                }
                maxSite = k_int();
                return 1;
        }

        if(k_its("site_file")) {    /* added command to read in a HYPOINVERSE format */
                name = k_str();     /* "universal code" station file.     950831:ldd */
                if(!name) return 1;
                site_read(name);
                return 1;
        }

        return 0;
}


/***************************************************************************
 * site_index(site, net, comp, loc) :                                      *
 *             Returns index of site, or -1 if not found                   *
 * 950901:ldd  added 2nd & 3rd arguments to handle "universal" station     *
 *             naming convention                                           *
 * 040512:ldd  added 4th argument to handle location code                  * 
 *             changed to use qsort and bsearch for faster lookup          *
 ***************************************************************************/

int site_index(char *site, char *net, char *comp, char *loc)
{
        SITE  key;
        SITE *match;
      
        site_init();
        site_sort();   /* run qsort and fill index field once */
     
        strcpy( key.name, site );
        strcpy( key.net,  net  );
        strcpy( key.comp, comp );
        strcpy( key.loc,  loc  );

        match = (SITE *) bsearch( &key, Site, nSite, sizeof(SITE), 
                                  compareSCNL );

        if( match == NULL ) return -1;   /* SCNL not in list */
        return( match->index );          /* index if SCNL    */
}


/*************************************************************
 *  compareSCNL()                                            *
 *  This function is passed to qsort() and bsearch().        *
 *  We use qsort() to sort the station list by SCNL numbers, *
 *  and we use bsearch to look up an SCNL in the list.       *
 *************************************************************/
 int compareSCNL( const void *s1, const void *s2 )
{
   int rc;
   SITE *t1 = (SITE *) s1;
   SITE *t2 = (SITE *) s2;
 
   rc = strcmp( t1->name, t2->name );
   if ( rc != 0 ) return rc;
   rc = strcmp( t1->comp, t2->comp );
   if ( rc != 0 ) return rc;
   rc = strcmp( t1->net,  t2->net );
   if ( rc != 0 ) return rc;
   rc = strcmp( t1->loc,  t2->loc );
   return rc;
}

