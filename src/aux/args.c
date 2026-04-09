#include "args.h"
#include "tools.h"
#include <stdio.h>

int in_args(int argc, char * argv[], char * target) {
	int i = 1;
	while (i < argc) {
		if (streq(argv[i], target)) return 1;
		i++;
	}
	
	return 0;
}
char * arg_value(int argc, char * argv[], char * target) {
	int i = 1;
	while (i < argc - 1) {
		if (streq(argv[i], target)) return argv[i + 1];
		i++;
	}
	return NULL;
}

static int parse_int(char * input, int * res) {
	int resultat = 0;
	int i = 0;

	while (input[i] != 0) {
		if (input[i] < 48 || input[i] > 57) return 0;
		resultat = resultat * 10 + (input[i] - 48);
		i++;
	}

	*res = resultat;

	return 1;
}

int get_int(int argc, char * argv[], char * target, int * res) {
	int i = 1;
	while (i < argc - 1) {
		if (streq(argv[i], target)) return parse_int(argv[i + 1], res);
		i++;
	}
	return 2;
}

int find_all(int argc, char * argv[], struct arg_input args[], int size) {
	if (size == 0) return 0;

	int found = 0;
	int i = 1;
	while (i < argc) {
		if (argv[i][0] != '-') {
			i++;
			continue;
		}
		int j = 0;
		while (j < size) {
			if (args[j].found || !streq(argv[i], args[j].name)) {
				j++;
				continue;
			}
			
			switch (args[j].type) {
				case Presence:
					found += args[j].found = 1;
					break;
				case Int:
					if (i == argc - 2) break;
					args[j].found = parse_int(argv[i + 1], &(args[j].int_res));
					found += args[j].found;
					break;
				case String:
					if (i == argc - 1) break;
					found++;
					args[j].found = 1;
					args[j].str_result = argv[i + 1];
					break;
			}
			j++;
		}
		i++;
	}

	return found;
}
