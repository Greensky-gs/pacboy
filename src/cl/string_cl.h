#ifndef __STRING_CL_H__
#define __STRING_CL_H__ 1

struct st_chained_cell {
	char * value;
	struct st_chained_cell * next;
};
typedef struct st_chained_cell * chained_cell;

extern chained_cell stringcl_create(char *);
extern void stringcl_destroy(chained_cell *);
extern void stringcl_destroy_nofree(chained_cell *);
extern unsigned long int stringcl_size(chained_cell);
extern int stringcl_append(chained_cell *, char *);
extern int stringcl_exists(chained_cell, char *);
extern int stringcl_remove(chained_cell *, char *);
extern int stringcl_removep(chained_cell *, chained_cell);
extern void stringcl_foreach(chained_cell, void *, void callback(chained_cell, void *));
extern char ** stringcl_to_array(chained_cell, unsigned long *);

#endif
