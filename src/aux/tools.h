#ifndef __TOOLS_H__
#define __TOOLS_H__ 1

#include "../cl/string_cl.h"
#include "../core/config.h"

extern int streq(char *, char *);
extern int exec_command(char *[]);
extern int copy_rec(char * base_source, char * base_dest, char * restpath, chained_cell includes);
extern chained_cell get_uniq_deps(plib_config);

#endif
