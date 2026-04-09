#ifndef __PAIRS_H__
#define __PAIRS_H__ 1

struct st_pair {
	char * name;
	char * value;
	struct st_pair * next;
};

typedef struct st_pair * pair;

extern pair create_pair(char * name, char * value);
extern void destroy_pair(pair);
extern int append_pair(pair *, char * name, char * value);
extern char * get_value(pair, char * name);
extern int exists(pair, char * name);


#endif
