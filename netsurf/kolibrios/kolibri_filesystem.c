#include "kolibri_filesystem.h"
#include <sys/types.h>

int mkdir(char *dirname, long mode) {

}

 
DIR *opendir(char *path) {

}


struct dirent *readdir(DIR* dirp) {

}

int scandir(const char *dirp, struct dirent ***namelist,
	    int (*filter)(const struct dirent *),
	    int (*compar)(const struct dirent **, const struct dirent **)) {

}

char *realpath(const char *path, char *resolved_path) {

}

int closedir(DIR* opened_dir) {

}

/* int access(const char *pathname, int mode) { */

/* } */

/* ssize_t pread(int fd, void *buf, size_t count, off_t offset) */
/* { */

/* } */

/* ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset) */
/* { */

/* } */

