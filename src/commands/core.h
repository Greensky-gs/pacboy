#ifndef __CORE_H__
#define __CORE_H__ 1

extern void setup(char *);
extern char ** read_config_file(char * input, int * psize);
extern void destroy_array(char **, int);

#endif
