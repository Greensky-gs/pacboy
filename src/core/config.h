#ifndef __CONFIG_H__
#define __CONFIG_H__ 1

struct lib_feature {
	char * name;
	char ** dependencies;
};
struct lib_config_file {
	char ** dependencies;
	struct lib_feature ** features;

	int dependencies_count;
	int features_count;
};

typedef struct lib_feature * plib_feature;
typedef struct lib_config_file * plib_config;

extern plib_config parse_config_file(char *);
extern void destroy_plib_config(plib_config);

#endif
