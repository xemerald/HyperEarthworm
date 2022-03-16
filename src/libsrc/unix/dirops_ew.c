
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: dirops_ew.c 3069 2007-07-19 18:30:34Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2007/07/19 18:30:34  paulf
 *     added 2 new funcs OpenDir() and GetNextFileName() for traversing a directory
 *
 *     Revision 1.1  2005/07/15 18:20:19  friberg
 *     Unix version of libsrc for POSIX systems
 *
 *     Revision 1.5  2001/04/09 17:46:03  davidk
 *     Added check for ';' and '*' in directory name string in RecursiveCreateDir, in
 *     hopes of preventing some havoc.
 *
 *     Revision 1.4  2001/04/06 21:29:04  davidk
 *     RecursiveCreateDir() to execute a system command instead of calling mkdirp(),
 *     because mkdirp() required libgen, which would require changes to all of
 *     the makefiles that include dirops_ew.o.  This seemed like the least amount
 *     of work, and I didn't think that performance on creating directory trees
 *     was that important.
 *
 *     Revision 1.3  2001/04/05 18:23:26  cjbryan
 *     added RecursiveCreateDir function to recursively create all
 *     directories in a specified path
 *
 *     Revision 1.2  2000/09/28 23:20:12  dietz
 *     added function rename_ew
 *
 *     Revision 1.1  2000/02/14 18:46:17  lucky
 *     Initial revision
 *
 *
 */

/*
 *   dirops_ew.c  Solaris version
 *                Contains system-specific functions for 
 *                dealing with directories
 *
 *   960112:LDD
 * Alex added Will's GetFileName() 10/20/99
 */

#include <platform.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <earthworm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <libgen.h>		/* for mkdirp */


/*****************************************************************************
 *  chdir_ew( )  changes current working directory; Solaris version          *
 *****************************************************************************/
int chdir_ew( char *path )
{
     return( chdir( path ) );
}

/*****************************************************************************
 *  RecursiveCreateDir ()  Creates a directories in a specified path. 	     *
 *  Solaris version.					                     *
 *  This is just CreateDir with the call to mkdir replaced by a call to the  *
 *  function mkdirp.							     *
 *                                                                           *
 *  if dirname exists and is accessible, EW_SUCCESS is returned. Otherwise,  *
 *  we attempt to create it. If it all goes well, we return EW_SUCCESS,      *
 *  otherwise we report error and return EW_FAILURE.                         *
 *       CJB 4/4/2001                                                        *
 *                                                                           *
 *  modified to use system("mkdir -p DIRNAME") instead of mkdirp(), so as    *
 *  not to have to change the makefiles for all programs that use            *
 *  dirops_ew.c.        Davidk 04/06/2001                                    *
 *                                                                           *
 *  *NOTE*:  The calling program is responsible to verify that directory     *
 *  names passed to this function do not include malicious commands, as      *
 *  this function executes a system() call that includes the text of the     *
 *  directory name.    Davidk 04/09/20001                                    *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
int RecursiveCreateDir (char *dirname)
{

	struct	stat	buf;
        char szDirectoryCommand[1024];

	if (dirname == NULL)
	{
		logit ("", "%s: ERROR! Invalid pointer passed in; returning!\n",
		       "RecursiveCreateDir()");
		return EW_FAILURE;
	}

	/* Added check for ; or *, in hopes of limiting the possible dangers that
           the resulting system() call could make.  Davidk 04/09/2001 */
	if (strchr(dirname, ';') || strchr(dirname, '*'))
	{
		logit ("", "%s: ERROR! Invalid directory name passed in; returning!\n",
		       "RecursiveCreateDir()");
		return EW_FAILURE;
	}

	/* does it already exist? */
	if (stat (dirname, &buf) != 0)
	{
		if (errno == ENOENT)
		{

               		sprintf(szDirectoryCommand, "mkdir -p %s", dirname);         
			system(szDirectoryCommand);
			if (stat (dirname, &buf) != 0)
			{
				logit ("e", "CreateDir: Cannot create %s: %s\n",
								dirname, strerror(errno) );
				return( EW_FAILURE);
			}


/********************************************
			if (mkdirp (dirname, (mode_t)0775) != 0)
			{
				logit ("e", "CreateDir: Cannot create %s: %s\n",
								dirname, strerror(errno) );
				return( EW_FAILURE);
			}
 ********************************************/

		}
		else
		{
			logit ("e", "CreateDir: Cannot stat %s - %s \n", 
								dirname, strerror (errno));
			return EW_FAILURE;
		}

	}
	else
	{
		/* do nothing - hope that this is a directory */
	}

	return EW_SUCCESS;

}

/*****************************************************************************
 *  CreateDir ()  Creates a directory. Solaris version.                      *
 *                                                                           *
 *  if dirname exists and is accessible, EW_SUCCESS is returned. Otherwise,  *
 *  we attempt to create it. If it all goes well, we return EW_SUCCESS,      *
 *  otherwise we report error and return EW_FAILURE.                         *
 *       LV 8/16/1999                                                        *
 *****************************************************************************/
int CreateDir (char *dirname)
{

   struct stat buf;

   if (dirname == NULL)
   {
      logit ("e", "Invalid argument passed in; exiting!\n");
      return EW_FAILURE;
   }

/* Doesn't exist, make it */
   if (stat (dirname, &buf) != 0)
   {
      if (errno == ENOENT)
      {
         if (mkdir (dirname, (mode_t)0775) != 0)
         {
            logit ("e", "CreateDir: Cannot create %s: %s\n",
                   dirname, strerror(errno) );
            return( EW_FAILURE);
         }
      }
      else
      {
         logit ("e", "CreateDir: Cannot stat %s - %s \n", 
                dirname, strerror (errno));
         return EW_FAILURE;
      }
   }

/* It exists - hope that this is a directory */
   else
   {
      /* do nothing */  
   }

   return EW_SUCCESS;
}


/***************************************************************
 *  GetFileName    (from sendfile, by Will Kohler)             *
 *                                                             *
 *  Function to get the name of a file in the current          *
 *  directory.                                                 *
 *                                                             * 
 *  Returns 0 if all ok                                        *
 *          1 if no files were found                           *
 ***************************************************************/

int GetFileName( char fname[] )
{
   DIR         *dp;
   struct stat ss;

   dp = opendir( "." );
   if ( dp == NULL ) return 2;

   do
   {
      struct dirent *dentp = readdir( dp );
      if ( dentp == NULL )
      {
         closedir( dp );
         return 1;
      }
      strcpy( fname, dentp->d_name );

      if ( stat( fname, &ss ) == -1 )
      {
         closedir( dp );
         return 1;
      }
   } while ( !S_ISREG(ss.st_mode) );

   closedir( dp );
   return 0;
}

/*********************************************************************
 OpenDir(char *) - opens the directory for reading and subsequent use
        by the GetNextFileName(char fname[]) func
        
        returns 0 if all is ok
        returns 1 if any problem openning the directory

THIS IS NOT THREAD SAFE BY ANY STRETCH OF THE IMAGINATION - paul friberg 7/18/2007
*************************************************************************/
static DIR *open_dir;
static char *open_dir_name;

int OpenDir(char *path) 
{

    open_dir_name = path;
    open_dir = opendir(path);
    if (open_dir == NULL) 
        return 1;

    return 0;
}
/*********************************************************************
 GetNextFileName(char fname[]) func - returns regular files only in fname
        to be called iteratively AFTER OpenDir()

        returns 0 if all is ok with next file name copied to fname
        returns 1 if no files remain or problem with dir

THIS IS NOT THREAD SAFE BY ANY STRETCH OF THE IMAGINATION - paul friberg 7/18/2007
*************************************************************************/
int GetNextFileName(char fname[]) 
{
   struct stat ss;
   char fullpath[2048];

   if (open_dir == NULL)
        return 1;
   do
   {
      struct dirent *dentp = readdir( open_dir );
      if ( dentp == NULL )
      {
#ifdef DEBUG_DIR
         fprintf(stderr, "no more files in dir\n");
#endif
         closedir( open_dir );
         open_dir=NULL;
         return 1;
      }
      strcpy( fname, dentp->d_name );

#ifdef DEBUG_DIR
      fprintf(stderr, "found %s in dir\n", fname);
#endif

      sprintf (fullpath, "%s/%s", open_dir_name, fname);
      if ( stat( fullpath, &ss ) == -1 )
      {
         /* problem getting status */
#ifdef DEBUG_DIR
         fprintf(stderr, "unable to get status on %s in dir\n", fullpath);
#endif
         closedir( open_dir );
         open_dir=NULL;
         return 1;
      }
   } while ( !S_ISREG(ss.st_mode) );
   return 0;
}

/**************************************************************************
 *  rename_ew( )  Moves a file                   Solaris version          *
 *  path1 = name of file to be moved.  path2 = destination name           *
 **************************************************************************/

int rename_ew( char *path1, char *path2 )
{
     int  rc;

     rc = rename( path1, path2 );
     if ( rc == -1 ) perror( "rename" );
     return rc;
}

