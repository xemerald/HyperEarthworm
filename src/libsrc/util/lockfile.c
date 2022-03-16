#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef EARTHWORM_H
#include <earthworm.h>
#include <lockfile.h>
#endif

/* ew_lockfile_path() - returns the absolute path to EW_LOG/conifig_file_name.lock */
char * ew_lockfile_path(char *config_file_name)
{

   char * lockfile, *log_dir, *cleaned_config_file, *cptr;
   int len;

/* remove any absolute or relative paths to the config_file_name before using it */
   cleaned_config_file = config_file_name;
   if ( (cptr = strrchr(config_file_name, '\\')) != NULL ) {
	cptr++;
	cleaned_config_file = cptr;
   }

   if ( (cptr = strrchr(config_file_name, '/')) != NULL ) {
	cptr++;
	cleaned_config_file = cptr;
   }

/* Get path to log directory from environment variable EW_LOG
   **********************************************************/
   log_dir = getenv( "EW_LOG" );

   if ( log_dir == NULL )
   {
      fprintf( stderr, "ew_lockfile_path(): Fatal Error Environment variable EW_LOG not defined!\n " );
      fprintf( stderr, "trying to lock %s in EW_LOG dir.\n", config_file_name );
      exit( -1 );
   }

   len = strlen(log_dir)+10+strlen(cleaned_config_file);
   if ( (lockfile = calloc(1,len)) == NULL ) {
      fprintf( stderr, "ew_lockfile_path(): could not calloc space for lockfile path\n" );
      exit( -1 );
   }

/* Save the log-directory path and program name.
   *******************************************************/
   strcpy ( lockfile, log_dir );
   ew_addslash (lockfile);
   strcat (lockfile, cleaned_config_file);
   strcat (lockfile, ".lock");
   return(lockfile);
}

/* ew_lockfile - puts an advisory lock on a file of the pathname given:
    upon SUCCESS returns the fd of the file descriptor.
	
    upon FAILURE returns -1 indicating that the file could not be openned  for writing OR that another
      	process has obtained the lock already
*/
int ew_lockfile(char * fname) {
/*  call the OS neutral version  */
    return (ew_lockfile_os_specific(fname));

}

/* ew_unlockfile() - takes the fd returned by a successful ew_lockfile() call and unlocks it and closes the open file descriptor.
    upon SUCCESS it returns 1
    upon FAILURE it echos to stderr that the unlock failed an returns -1
*/
int ew_unlockfile(int fd) {
/*  call the OS neutral version  */
    return (ew_unlockfile_os_specific(fd));
}


/* ew_unlink_lockfile() - takes the fname created abbove and unlinks it and frees the memory allocated
    upon SUCCESS it returns 1
    upon FAILURE it echos to stderr that the unlock failed an returns -1
*/
int ew_unlink_lockfile(char * fname) {

	if (unlink(fname) == -1) {
      		fprintf( stderr, "ew_unlink_lockfile() - Unable to unlink %s.\n", fname );
		return(-1);
    	}
	free(fname);
	return (1);
}
