#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int ew_lockfile_os_specific(char * fname) {

    int fd;
    struct flock lock;
 
	if ( (fd = open(fname, O_RDWR|O_CREAT, 0644)) == -1 ) {
		/* logit later? */
 		fprintf(stderr, "ew_lockfile(): could not open %s for locking\n", fname);
		return(-1);
	}
 	lock.l_type = F_WRLCK;
 	lock.l_whence = SEEK_SET;
 	lock.l_start = 0;
 	lock.l_len = 0;
        if ( fcntl(fd, F_SETLK, &lock) == -1 ) {
		if (errno == EAGAIN) {
			/* someone already has locked it */
			/* logit later ? */
 			fprintf(stderr, "ew_lockfile(): %s already locked by another instance.\n", fname);
			return(-1);	
		}
        }
 	return(fd);
}

int ew_unlockfile_os_specific(int fd) {

    struct flock lock;

 	lock.l_type = F_UNLCK;
 	lock.l_whence = SEEK_SET;
 	lock.l_start = 0;
 	lock.l_len = 0;
        if ( fcntl(fd, F_SETLK, &lock) == -1 ) {
 		fprintf(stderr, "ew_unlockfile():  unable to unlock fd %d.\n", fd);
		return(-1);
	}

	close(fd);
	return(1);
}
