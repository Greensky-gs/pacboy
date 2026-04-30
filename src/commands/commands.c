#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <unistd.h>
#include "commands.h"
#include "core.h"
#include "../cl/string_cl.h"
#include "../cl/pairs.h"
#include "../aux/tools.h"
#include "sync.h"

int is_sys(char * input) {
	if (input[0] == 0) return 0;
	if (input[0] == '.' && input[1] == 0) return 1;
	if (input[0] == '.' && input[1] == '.' && input[2] == 0) return 1;
	return 0;
}

void display_list(char * path) {
	DIR * dir;
	if ((dir = opendir(path)) == NULL) {
		perror("opendir");
		return;
	}

	struct dirent * entry;

	while ((entry = readdir(dir)) != NULL) {
		if (is_sys(entry->d_name)) continue;
		if (entry->d_type == DT_DIR) printf("%s\n", entry->d_name);
	}

	closedir(dir);
}

static void handle_substring(char * sub, pair * pairs) {
	int i = 1;
	int first_end = 0;
	int second_start;
	while (sub[i] != 0) {
		if (sub[i] == '=') {
			first_end = i - 1;
			second_start = i + 1;
			break;
		} else {
			i++;
		}
	}

	sub[first_end + 1] = 0;

	char * name, * value;
	if ((name = strdup(sub)) == NULL) return;
	if ((value = strdup(sub + second_start)) == NULL) {
		free(name);
		return;
	}

	if (exists(*pairs, name)) {
		free(name);
		free(value);
		return;
	}

	append_pair(pairs, name, value);
}
static chained_cell get_includes_list(char * includesb, char ** deps, int size, int * valid) {
	*valid = 0;
	char * includes;
	if ((includes = strdup(includesb)) == NULL) {
		perror("copy malloc");
		return NULL;
	}
	pair pairs = NULL;

	int i = 1;
	int start = 0;
	while (includes[i] != 0) {
		if (includes[i] == ',') {
			includes[i] = 0;
			handle_substring(includes + start, &pairs);
			includes[i] = ',';

			start = i + 1;
			i++;
		} else {
			i++;
		}
	}
	handle_substring(includes + start, &pairs);

	free(includes);

	if (exists(pairs, "*")) {
		char * copy;
		if ((copy = strdup(get_value(pairs, "*"))) == NULL) {
			perror("Malloc copy");
			return NULL;
		}

		chained_cell list;
		if ((list = stringcl_create(copy)) == NULL) {
			perror("Malloc cell");
			free(copy);
			return NULL;
		}

		destroy_pair(pairs);

		*valid = 1;
		return list;
	}
	chained_cell list = NULL;

	int j = 0;
	*valid = 1;
	while (j < size) {
		if (exists(pairs, deps[j])) {
			char * cpy;
			if ((cpy = strdup(get_value(pairs, deps[j]))) == NULL) {
				perror("Malloc copy loop");
			} else {
				stringcl_append(&list, cpy);
			}
			j++;
		} else {
			*valid = 0;
			break;
		}
	}

	destroy_pair(pairs);
	if (!(*valid)) {
		stringcl_destroy(&list);
		return NULL;
	}
	return list;
}

int install(char * path, char * name, char * dest, char * includes) {
	int totalsize = strlen(path) + strlen(name) + 2;
	char * libpath;
	if ((libpath = malloc(totalsize)) == NULL) {
		perror("malloc");
		printf("Unable to allocate memory\n");
		return 1;
	}
	sprintf(libpath, "%s/%s", path, name);

	char * absolutepath;
	if ((absolutepath = realpath(dest, NULL)) == NULL) {
		perror("Realpath : cannot locate destination, because");

		free(libpath);
		return 1;
	}
	char * existenceargs[] = { "test", "-d", libpath, NULL };
	if (exec_command(existenceargs) != 0) {
		printf("This library doesn't exist.\n   Please make sure you are using the exact file name\n");
		free(libpath);
		free(absolutepath);
		return 1;
	}
	char ** parsed_deps;
	int size = 0;

	char * configpath;
	if ((configpath = malloc(strlen(libpath) + strlen("paquet.boy") + 2)) == NULL) {
		free(libpath);
		free(absolutepath);
		perror("Malloc config");
		printf("Unable to allocate memory\n");
		return 1;
	}
	sprintf(configpath, "%s/paquet.boy", libpath);
	char * config_existence_args[] = { "test", "-f", configpath, NULL };

	if (exec_command(config_existence_args) == 0) {
		parsed_deps = read_config_file(configpath, &size);
		
		if (parsed_deps == NULL && size == -1) {
			printf("Something went wrong reading config file.\n");
			free(libpath);
			free(absolutepath);
			free(configpath);
			return 1;
		}
	}
	chained_cell includes_list = NULL;
	if (size > 0) {
		if (includes == NULL) {
			printf("Please specify the dependencies headers\n");
			free(libpath);
			free(absolutepath);
			free(configpath);
			destroy_array(parsed_deps, size);
			return 1;
		}
		int valid;
		includes_list = get_includes_list(includes, parsed_deps, size, &valid);
		if (includes_list == NULL || !valid) {
			printf("Please specify all the dependencies headers.\n  You can use \"*=somefile.h\"\n");
			free(libpath);
			free(absolutepath);
			free(configpath);
			destroy_array(parsed_deps, size);
			return 1;
		}
	}

	if (size > 0) destroy_array(parsed_deps, size);

	printf("Copying library \x1b[33m%s\x1b[0m into \x1b[93m%s\x1b[0m...\n", name, absolutepath);

	int returncode = copy_rec(libpath, absolutepath, NULL, includes_list);

	printf("Library \x1b[33m%s\x1b[0m copied\n", name);
	
	free(libpath);
	free(absolutepath);
	free(configpath);
	stringcl_destroy(&includes_list);
	return returncode;
}

int generate_config(char * outputname, char * depstring) {
	if (depstring == NULL) {
		printf("No dependencies specified.\n  Use with \x1b[90m--deps \"first_function,second_function...\"\x1b[0m\n");
		return 1;
	}
	int fd = -1;
	if (outputname != NULL) {
		char filename[] = "paquet.boy";
		char * fullpath;
		if ((fullpath = malloc(strlen(outputname) + strlen(filename) + 2)) == 0) {
			perror("malloc");
			return 1;
		}
		*fullpath = 0;
		strcat(fullpath, outputname);
		strcat(fullpath, "/");
		strcat(fullpath, filename);

		if ((fd = open(fullpath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1) {
			perror("open");
			free(fullpath);
			return 1;
		}

		dup2(fd, STDOUT_FILENO);
		free(fullpath);
	} else {
		fd = STDOUT_FILENO;
	}

	char header[] = "[deps]\n";
	write(fd, header, strlen(header));

	int i = 0;
	int start = 0;
	int end = 0;
	while (depstring[i] != 0) {
		if (depstring[i] == ',') {
			i++;

			end = i - 1;

			write(fd, depstring + start, end - start);
			start = end + 1;


			write(fd, "\n", 1);
		} else {
			i++;
		}
	}
	end = i;

	write(fd, depstring + start, end - start);
	start = end + 1;


	write(fd, "\n", 1);

	if (fd != -1) close(fd);
	return 0;
}

int display_info(char * libspath, char * name) {
	char * libpath;
	if ((libpath = malloc(strlen(libspath) + strlen(name) + 2)) == NULL) {
		perror("Malloc libpath");
		printf("Unable to allocate memory\n");
		return 1;
	}
	sprintf(libpath, "%s/%s", libspath, name);
	char * exist_args[] = { "test", "-d", libpath, NULL };
	if (exec_command(exist_args) != 0) {
		printf("\x1b[33m%s\x1b[0m is not a valid library. Please make sure you gave the right name.\n", name);
		free(libpath);
		return 1;
	}

	char * configpath;
	if ((configpath = malloc(strlen(libpath) + strlen("paquet.boy") + 2)) == NULL) {
		perror("Malloc paquet file");
		printf("Unable to allocate memory\n");
		free(libpath);
		return 1;
	}
	sprintf(configpath, "%s/paquet.boy", libpath);
	char * cfg_file_exists_args[] = { "test", "-f", configpath, NULL };
	int exists = exec_command(cfg_file_exists_args) == 0;

	if (!exists) {
		printf("Library name : \x1b[90m\x1b[4m%s\x1b[0m\nLibrary location : \x1b[94m%s\x1b[0m\nInclude command : \x1b[90mpacboy %s ./\x1b[0m\n", name, libpath, name);

		free(configpath);
		free(libpath);
		return 0;
	}

	int size = 0;
	char ** parsed_args = read_config_file(configpath, &size);

	if (parsed_args == NULL && size == -1) {
		printf("Something went wrong reading the config file\n");

		free(configpath);
		free(libpath);
		return 1;
	}

	printf("Library name : \x1b[90m\x1b[4m%s\x1b[0m\nLibrary location : \x1b[94m%s\x1b[0m\nIt has \x1b[1m%d\x1b[0m functions requirements :\n", name, libpath, size);
	int i = 0;
	while (i < size) {
		printf("    %s\n", parsed_args[i]);
		i++;
	}

	printf("Include command : \x1b[90mpacboy %s ./ --include \"", name);
	i = 0;
	while (i < size) {
		printf("%s=somefile.h", parsed_args[i]);

		if (i != size - 1) printf(",");
		i++;
	}
	printf("\"\x1b[0m\n");
	
	destroy_array(parsed_args, size);
	free(configpath);
	free(libpath);
	return 0;
}

static int prompt_configuration(char * name, char * default_value, char * result) {
	printf("%s [\x1b[90m%s\x1b[0m]: ", name, default_value == NULL ? "" : default_value);
	fflush(stdout);

	if (fgets(result, CONFIG_MAX_STR_SIZE - 1, stdin) == NULL) {
		perror("fgets");
		return -1;
	}

	if (*result == 0) return 0;
	return 1;
}

int update_config(psyncconfig config, char * save_path) {
	char new_url[CONFIG_MAX_STR_SIZE] = {0};
	int changes = 0;

	if (prompt_configuration("new repo url", config->repo_url, new_url) == 1) {
		int i = 0;
		while (i < CONFIG_MAX_STR_SIZE) {
			config->repo_url[i] = new_url[i] == '\n' ? 0 : new_url[i];
			i++;
		}
		changes++;
	}

	if (changes > 0) save_sync_config_file(save_path, config);
	return 1;
}

int sync_database(psyncconfig config, char * files) {
	char * pre_rmargs[] = { "rm", "-rf", files, NULL };
	int res = exec_command(pre_rmargs);

	if (res != 0) {
		fprintf(stderr, "Something went wrong. Code: %d\n", res);
		return 1;
	}

	printf("\x1b[35mCloning from %s...\x1b[0m\n", config->repo_url);

	char * args[] = { "git", "clone", "-b", "mirror", "--single-branch", config->repo_url, files, NULL };
	res = exec_command(args);
	if (res != 0) {
		fprintf(stderr, "Something went wrong. Code: %d\n", res);
		return 1;
	}

	char dotgit[PATH_MAX];
	snprintf(dotgit, PATH_MAX - 1, "%s/%s", files, ".git");

	printf("\x1b[35mRemoving \x1b[90m.git\x1b[35m folder...\x1b[0m\n");
	char * rmargs[] = { "rm", "-rf", dotgit, NULL };
	res = exec_command(rmargs);

	if (res != 0) {
		fprintf(stderr, "Something went wrong. Code: %d\n", res);
		return 1;
	}

	printf("\x1b[32mDone\x1b[0m\n");
	return 0;
}
