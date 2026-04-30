#ifndef __SYNC_H__
#define __SYNC_H__ 1

#define CONFIG_MAX_STR_SIZE 512

typedef enum {
	OK = 0,
	MALLOC = 1,
	OPEN = 2,
	READ = 3
} read_config_returns;
struct sSyncconfig {
	char repo_url[CONFIG_MAX_STR_SIZE];
};
typedef struct sSyncconfig * psyncconfig;

extern psyncconfig read_sync_config_file(char *, int *);
extern int save_sync_config_file(char *, psyncconfig);
extern psyncconfig default_config();

extern int sync_database(psyncconfig, char *);

#endif
