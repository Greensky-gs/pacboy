#include "sync.h"
#ifndef __COMMANDS_H__
#define __COMMANDS_H__ 1

extern void display_list(char *);
extern int install(char * path, char * name, char * dest, char * includes, char * features);
extern int is_sys(char *);
extern int generate_config(char * outputname, char * depstring, char * featuresstring);
extern int display_info(char *libspath, char * name);
extern int version();
extern int check_update();

extern int update_config(psyncconfig, char *);

#endif
