#include "commands.h"
#include "../main.h"
#include "../cl/string_cl.h"
#include "../cl/pairs.h"
#include "../aux/tools.h"
#include "../core/config.h"
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <unistd.h>

#define FREE_CONFIG(alloced, config) if (alloced) destroy_plib_config(config);

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

static char ** extract_features(char * input, int * size) {
	char * work = strdup(input);
	if (work == NULL) return NULL;

	int count = 1;
	int i = 0;
	while (work[i] != 0) {
		if (work[i] == ',') count++;
		i++;
	}
	*size = count;

	char ** array;
	if ((array = malloc(sizeof(char *) * count)) == NULL) {
		perror("malloc");
		free(work);
		return NULL;
	}
	i = 0;
	int index = 0;
	char * start = work;
	while (work[i] != 0) {
		if (work[i] == ',') {
			work[i] = 0;
			array[index] = start;
			start = work + 1;
		}
		i++;
	}
	array[index] = start;

	return array;
}
static void destroy_features_array(char ** arr) {
	if (arr == NULL) return;
	free(arr[0]);
	free(arr);
}
int install(char * path, char * name, char * dest, char * includes, char * features) {
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

	plib_config config = NULL;
	int alloced = 1;
	if (exec_command(config_existence_args) == 0) {
		config = parse_config_file(configpath);
		
		if (config == NULL) {
			printf("Something went wrong reading config file.\n");
			free(libpath);
			free(absolutepath);
			free(configpath);
			return 1;
		}
	} else {
		struct lib_config_file default_config = {};

		default_config.dependencies = NULL;
		default_config.dependencies_count = 0;
		default_config.features_count = 0;
		default_config.features = NULL;

		config = &default_config;
		alloced = 0;
	}
	free(configpath);

	char ** selected_features = NULL;
	int selected_features_size = 0;
	if (features != NULL && (selected_features = extract_features(features, &selected_features_size)) == NULL) {
		fprintf(stderr, "Unable to extract features");
		free(libpath);
		free(absolutepath);
		FREE_CONFIG(alloced, config);
		return 1;
	}
	if (selected_features != NULL) {
		int i = 0;
		while (i < selected_features_size) {
			int j = 0;
			int valid = 0;
			while (!valid && j < config->features_count) {
				if (streq(selected_features[i], config->features[j]->name)) valid = 1;
				j++;
			}
			if (!valid) {
				fprintf(stderr, "Unknown feature : %s\n", selected_features[i]);

				destroy_features_array(selected_features);
				free(libpath);
				free(absolutepath);
				FREE_CONFIG(alloced, config);
				return 1;
			}
			i++;
		}
	}
	int updated = 0;
	chained_cell required_deps = NULL;
	if (config->dependencies_count > 0) {
		int i = 0;
		while (i < config->dependencies_count) {
			if (!stringcl_exists(required_deps, config->dependencies[i])) {
				stringcl_append(&required_deps, config->dependencies[i]);
				updated = 1;
			}
			i++;
		}
	}

	if (selected_features_size > 0) {
		int i = 0;
		while (i < selected_features_size) {
			plib_feature feature = NULL;
			int j = 0;
			while (j < config->features_count && feature == NULL) {
				if (streq(config->features[j]->name, selected_features[i])) feature = config->features[j];
				j++;
			}

			j = 0;
			while (feature->dependencies != NULL && feature->dependencies[j] != NULL) {
				if (!stringcl_exists(required_deps, feature->dependencies[j])) {
					stringcl_append(&required_deps, feature->dependencies[j]);
					updated = 1;
				}
				j++;
			}
			i++;
		}
	}
	chained_cell includes_list = NULL;
	if (updated && required_deps != NULL) {
		if (includes == NULL) {
			fprintf(stderr, "Please specify the dependencies headers\n");
			free(libpath);
			free(absolutepath);
			stringcl_destroy_nofree(&required_deps);
			destroy_features_array(selected_features);
			FREE_CONFIG(alloced, config);
			return 1;
		}

		char ** deps_array;
		unsigned long size;
		if ((deps_array = stringcl_to_array(required_deps, &size)) == NULL) {
			perror("stringcl_to_array");
			fprintf(stderr, "Cannot convert required_deps to array");
			free(libpath);
			free(absolutepath);
			stringcl_destroy_nofree(&required_deps);
			destroy_features_array(selected_features);
			FREE_CONFIG(alloced, config);
			return 1;
		}
		stringcl_destroy_nofree(&required_deps);

		int valid;
		includes_list = get_includes_list(includes, deps_array, size, &valid);
		free(deps_array);

		if (includes_list == NULL || !valid) {
			fprintf(stderr, "Please specify all the dependencies headers.\n  You can use \"*=somefile.h\"\n");
			free(libpath);
			free(absolutepath);
			destroy_features_array(selected_features);
			FREE_CONFIG(alloced, config);
			return 1;
		}
	} else {
		stringcl_destroy_nofree(&required_deps);
	}

	printf("Copying library \x1b[33m%s\x1b[0m into \x1b[93m%s\x1b[0m...\n", name, absolutepath);
	int returncode = copy_rec(libpath, absolutepath, NULL, includes_list, 0, selected_features, selected_features_size);
	printf("Library \x1b[33m%s\x1b[0m copied\n", name);
	
	free(libpath);
	free(absolutepath);
	destroy_features_array(selected_features);
	stringcl_destroy(&includes_list);
	FREE_CONFIG(alloced, config);
	return returncode;
}

int generate_config(char * outputname, char * depstring, char * featuresstring) {
	if (depstring == NULL && featuresstring == NULL) {
		printf("No dependencies/features specified.\n  Use with \x1b[90m--deps \"first_function,second_function...\" --features \"feature1,feature2[dep1,dep2]\"\x1b[0m\n");
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

	if (depstring != NULL) {
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
	}
	if (featuresstring != NULL) {
		char features_header[] = "[features]\n";
		write(fd, features_header, strlen(features_header));

		int j = 0;
		int feature_start = 0;
		int feature_end = 0;
		int writing_deps = 0;
		while (featuresstring[j] != 0) {
			if (featuresstring[j] == '[') {
				if (writing_deps) {
					fprintf(stderr, "unexpected format, aborting. This will result in a bad paquet file");
					break;
				}
				j++;
				feature_end = j - 1;
				write(fd, featuresstring + feature_start, feature_end - feature_start);
				feature_start = feature_end + 1;
				writing_deps = 1;
				write(fd, "\n", 1);
			} else if (featuresstring[j] == ']') {
				if (!writing_deps) {
					fprintf(stderr, "unexpected format, aborting. This will result in a bad paquet file");
					break;
				}
				writing_deps = 0;
				j++;
				feature_end = j - 1;
				char before[] = " - ";
				write(fd, before, 3);
				write(fd, featuresstring + feature_start, feature_end - feature_start);
				feature_start = feature_end + 1;
			} else if (featuresstring[j] == ',') {
				j++;
				feature_end = j - 1;
				if (writing_deps) {
					char before[] = " - ";
					write(fd, before, 3);
				}
				write(fd, featuresstring + feature_start, feature_end - feature_start);
				write(fd, "\n", 1);
				feature_start = feature_end + 1;
			} else j++;
		}
		feature_end = j;

		write(fd, featuresstring + feature_start, feature_end - feature_start);
		write(fd, "\n", 1);
	}

	if (fd != -1) close(fd);
	return 0;
}

static void display_uniqcl(chained_cell cell, void * data) {
	printf("%s=somefile.h", cell->value);
	if (cell->next != NULL) printf(",");
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

	plib_config config;
	if ((config = parse_config_file(configpath)) == NULL) {
		perror("parse_config_file");
		fprintf(stderr, "Incorrect read of config file");
		return 1;
	}

	printf("Library name : \x1b[90m\x1b[4m%s\x1b[0m\nLibrary location : \x1b[94m%s\x1b[0m\n", name, libpath);
	if (config->dependencies_count > 0) {
		printf("Functions requirements \x1b[90m(%d)\x1b[0m:\n", config->dependencies_count);

		int i = 0;
		while (i < config->dependencies_count) {
			printf("    %s\n", config->dependencies[i]);
			i++;
		}
	}
	if (config->features_count > 0) {
		printf("Features \x1b[90m(%d)\x1b[0m:\n", config->features_count);

		int i = 0;
		while (i < config->features_count) {
			printf("    %s", config->features[i]->name);
			if (config->features[i]->dependencies != NULL && config->features[i]->dependencies[0] != NULL) {
				printf(" (requires following dependencies) :\n");

				int j = 0;
				while (config->features[i]->dependencies[j] != NULL) {
					printf("        - %s\n", config->features[i]->dependencies[j]);
					j++;
				}
			} else printf("\n");
			i++;
		}
	}

	chained_cell uniq_deps = get_uniq_deps(config);

	printf("Include command : \x1b[90mpacboy %s ./", name);

	if (uniq_deps != NULL) {
		printf(" --include \"");

		stringcl_foreach(uniq_deps, NULL, display_uniqcl);
		printf("\"");
	}
	if (config->features_count > 0) {
		int i = 0;
		printf(" --features \"");
		while (i < config->features_count) {
			printf("%s", config->features[i]->name);
			if (i != config->features_count - 1) printf(",");
			i++;
		}
		printf("\"");
	}
	printf("\x1b[0m\n");

	if (uniq_deps != NULL) stringcl_destroy_nofree(&uniq_deps);
	destroy_plib_config(config);
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
int version() {
	printf(PROG_VERSION "\n");
	return 0;
};
