struct dirent {

};

struct stat {

};

struct DIR {

};

typedef struct DIR DIR;

int mkdir(char *dirname, long mode); 
DIR *opendir(char *path);
int stat(char *, struct stat *);
int closedir(DIR *dirp);
struct dirent *readdir(DIR* dirp);
int scandir(const char *dirp, struct dirent ***namelist,
	    int (*filter)(const struct dirent *),
	    int (*compar)(const struct dirent **, const struct dirent **));
