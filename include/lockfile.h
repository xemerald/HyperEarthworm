
#include "lockfile_ew.h"

char * ew_lockfile_path(char *config_file_name);
int ew_lockfile(char * fname);
int ew_unlockfile(int fd);
int ew_unlink_lockfile(char * fname);

