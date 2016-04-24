#include<sys/types.h>
#include<regex.h>
int regcomp(regex_t * regexp, const char *str, int val)
{

}


size_t	regerror(int val, const regex_t *r, char *c, size_t t)
{

}


int regexec(const regex_t *r, const char *c, size_t s, regmatch_t regm[], int i)
{

}


void regfree(regex_t *r)
{

}
