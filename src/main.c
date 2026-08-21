#include <stdio.h>
#include <stdlib.h>
#include "aux/args.h"
#include "commands/commands.h"
#include "commands/core.h"
#include "commands/sync.h"

#define MAX_VAR_LENGTH 256

#ifndef LIB_DIR_NAME
#define LIB_DIR_NAME ".local/share/paclibs"
#endif
#ifndef CONFIG_FILE_NAME
#define CONFIG_FILE_NAME ".config/pacboy"
#endif

static void init_lib(char * str) {
	char * home = getenv("HOME");
	if (home == NULL) {
		fprintf(stderr, "Cannot find HOME env variable.");
		return;
	}

	snprintf(str, MAX_VAR_LENGTH - 1, "%s/%s", home, LIB_DIR_NAME);
}
static void init_conf(char * str) {
	char * home = getenv("HOME");
	if (home == NULL) {
		fprintf(stderr, "Cannot find HOME env variable");
		return;
	}

	snprintf(str, MAX_VAR_LENGTH - 1, "%s/%s", home, CONFIG_FILE_NAME);
}

static void help_page(struct arg_input args[], int size) {
	printf("Pacboy packet manager.\n  Basic usage : \x1b[90mpacboy <lib name> <install path> [OPTIONS]\x1b[0m, or \x1b[90mpacboy [OPTIONS]\x1b[0m\n  Options are :\n");

	int i = 0;
	while (i < size) {
		printf("    \x1b[90m%-19s\x1b[0m : %s. Of type \x1b[91m", args[i].name, args[i].description);
		switch (args[i].type) {
			case String:
				printf("string\x1b[0m");
				if (args[i].str_result == NULL) printf(", no default value is provided\n");
				else printf(", default value set to \x1b[33m%s\x1b[0m", args[i].str_result);
				break;
			case Int:
				printf("integer\x1b[0m, default value is set to \x1b[33m%d\x1b[0m\n", args[i].int_res);
				break;
			case Presence:
				printf("toggle\x1b[0m\n");
				break;
		}
		i++;
	}
}

int main(int argc, char * argv[]) {
	struct arg_input arguments[] = {
		{ "-D", "Display the libraries database", Presence, 0, 0, NULL },
		{ "--generate-config", "Generate a config file", Presence, 0, 0, NULL },
		{ "-od", "Specify the output directory, if used with \x1b[90m--generate-config\x1b[0m", String, 0, 0, NULL },
		{ "--help", "Display help page", Presence, 0, 0, NULL },
		{ "-h", "Alias for \x1b[90m--help\x1b[0m", Presence, 0, 0, NULL },
		{ "--deps", "Specify a dependencies list, by comma-separated values", String, 0, 0, NULL },
		{ "--include", "Specify dependencies headers, by comma-separated pairs (eg: \"function1=string.h,function2=src/test.h\" or \"*=thing.h\"", String, 0, 0, NULL },
		{ "-I", "Display informations about a library, the first given argument", Presence, 0, 0, NULL },
		{ "--change-config", "Open the interactive pacboy configuration editor", Presence, 0, 0, NULL },
		{ "-P", "Pull changes from the database, if it exists. It overwrites any local changes", Presence, 0, 0, NULL },
		{ "--features", "Specify the features that must be included during installation", String, 0, 0, NULL },
		{ "-v", "Displays program version", Presence, 0, 0, NULL },
		{ "-u", "Checks for update", Presence, 0, 0, NULL }
	};
	int size = sizeof(arguments) / sizeof(struct arg_input);

	find_all(argc, argv, arguments, size);

	char libs_path[MAX_VAR_LENGTH]   = {0};
	char config_path[MAX_VAR_LENGTH] = {0};

	psyncconfig syncconfig;

	init_conf(config_path);
	init_lib(libs_path);

	if ((syncconfig = read_sync_config_file(config_path, NULL)) == NULL) syncconfig = default_config();
	setup(libs_path);

	if (arguments[3].found || arguments[4].found) {
		help_page(arguments, size);
		free(syncconfig);
		return 0;
	}
	if (arguments[11].found) {
		version();
		free(syncconfig);
		return 0;
	}
	if (arguments[12].found) {
		free(syncconfig);
		return check_update();
	}

	if (arguments[0].found) {
		display_list(libs_path);
		free(syncconfig);
		return 0;
	}
	if (arguments[1].found) {
		free(syncconfig);
		return generate_config(arguments[2].str_result, arguments[5].str_result, arguments[10].str_result);
	}
	if (arguments[7].found) {
		free(syncconfig);
		if (argc < 3) {
			printf("You need to specify a library to display.\n");
			return 1;
		}

		return display_info(libs_path, argv[1]);
	}
	if (arguments[8].found) {
		int res = update_config(syncconfig, config_path);
		free(syncconfig);
		return res;
	}
	if (arguments[9].found) {
		int res = sync_database(syncconfig, libs_path);
		free(syncconfig);
		return res;
	}

	if (argc < 3) {
		help_page(arguments, size);
		free(syncconfig);
		return 0;
	}

	free(syncconfig);
	return install(libs_path, argv[1], argv[2], arguments[6].str_result, arguments[10].str_result);
}
