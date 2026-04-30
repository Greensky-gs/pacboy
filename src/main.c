#include <stdio.h>
#include <stdlib.h>
#include "aux/args.h"
#include "commands/commands.h"
#include "commands/core.h"

#define MAX_VAR_LENGTH 256
#define LIB_DIR_NAME "paclibs"

static void init_lib(char * str) {
	char * home = getenv("HOME");
	if (home == NULL) {
		fprintf(stderr, "Cannot find HOME env variable.");
		return;
	}

	snprintf(str, MAX_VAR_LENGTH - 1, "%s/%s", home, LIB_DIR_NAME);
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
	};
	int size = sizeof(arguments) / sizeof(struct arg_input);

	find_all(argc, argv, arguments, size);

	char libs_path[MAX_VAR_LENGTH] = {0};
	init_lib(libs_path);
	setup(libs_path);

	if (arguments[3].found || arguments[4].found) {
		help_page(arguments, size);
		return 0;
	}

	if (arguments[0].found) {
		display_list(libs_path);
		return 0;
	}
	if (arguments[1].found) {
		return generate_config(arguments[2].str_result, arguments[5].str_result);
	}
	if (arguments[7].found) {
		if (argc < 3) {
			printf("You need to specify a library to display.\n");
			return 1;
		}

		return display_info(libs_path, argv[1]);
	}

	if (argc < 3) {
		help_page(arguments, size);
		return 0;
	}

	return install(libs_path, argv[1], argv[2], arguments[6].str_result);
}
