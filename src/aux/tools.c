#include "../commands/commands.h"
#include "tools.h"
#include "../cl/string_cl.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define CHUNK_SIZE 512

int streq(char * a, char * b) {
	int i = 0;
	while (a[i] != 0 && b[i] != 0) {
		if (a[i] != b[i]) return 0;
		i++;
	}

	return a[i] == b[i];
}

int exec_command(char * args[]) {
	pid_t pid;
	int status;

	switch (pid = fork()) {
		case -1:
			perror("fork");
			return -1;
		case 0:
			execvp(args[0], args);
			perror("execvp");
			exit(127);
			break;
		default:
			waitpid(pid, &status, 0);

			if (WIFEXITED(status)) return WEXITSTATUS(status);
			return 127;
	}
}
static int do_include(char * name) {
	char * last_dot = name;
	int i = 0;
	while (name[i] != 0) {
		if (name[i] == '.') last_dot = name + i;
		i++;
	}

	return streq(last_dot, ".c");
}

static int copy_content(const char * sourcename, char * destname, chained_cell includes, int recursion_level) {
	int source, dest;
	if ((source = open(sourcename, O_RDONLY)) == -1) return 1;
	if ((dest = open(destname, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR)) == -1) {
		close(source);
		return 1;
	}

	if (do_include(destname)) {
		char include_buffer[BUFSIZ] = {0};
		char recursion_buffer[4096] = {0};

		while (recursion_level-- > 0) {
			strcat(recursion_buffer, "../");
		}
		
		chained_cell current = includes;
		while (current != NULL) {
			sprintf(include_buffer, "#include \"%s%s\"\n", recursion_buffer, current->value);
			write(dest, include_buffer, strlen(include_buffer));

			current = current->next;
		}
	}

	int bytes;
	char buffer[CHUNK_SIZE] = {0};
	while ((bytes = read(source, buffer, CHUNK_SIZE)) > 0) write(dest, buffer, bytes);

	close(source);
	close(dest);
	return 0;
}

static int skip(char * path) {
	int i = 0;
	char * slash = path;
	while (path[i] != 0) {
		if (path[i] == '/') slash = path + i + 1;
		i++;
	}

	return streq(slash, "paquet.boy");
}

int copy_rec(char * base_source, char * base_dest, char * restpath, chained_cell includes, int recursion_level, char ** selected_features, int selected_features_size) {
	char * sourcename;
	char * destname;

	if ((sourcename = malloc(strlen(base_source) + (restpath == NULL ? 0 : strlen(restpath)) + 2)) == NULL) return 1;
	if ((destname = malloc(strlen(base_dest) + (restpath == NULL ? 0 : strlen(restpath)) + 2)) == NULL) {
		free(sourcename);
		return 1;
	}

	if (restpath != NULL) {
		sprintf(sourcename, "%s/%s", base_source, restpath);
		sprintf(destname, "%s/%s", base_dest, restpath);
	} else {
		sprintf(sourcename, "%s", base_source);
		sprintf(destname, "%s", base_dest);
	}

	DIR * directory;
	if ((directory = opendir(sourcename)) == NULL) {
		free(sourcename);
		free(destname);
		return 1;
	}

	struct dirent * entry;
	int fails = 0;
	while ((entry = readdir(directory)) != NULL) {
		if (is_sys(entry->d_name)) continue;

		if (entry->d_type == DT_DIR) {
			if (recursion_level == 0) {
				int i = 0;
				int included_feature = 0;
				while (i < selected_features_size && !included_feature) {
					if (streq(selected_features[i], entry->d_name)) included_feature = 1;
					i++;
				}
				if (!included_feature) continue;
			}

			char * newpath;
			if ((newpath = malloc((restpath == NULL ? 0 : strlen(restpath)) + strlen(entry->d_name) + 2)) == NULL) {
				perror("malloc");
				fails++;
				continue;
			}
			*newpath = 0;
			if (restpath == NULL) strcat(newpath, entry->d_name);
			else sprintf(newpath, "%s/%s", restpath, entry->d_name);

			char newbuff[PATH_MAX] = {0};

			snprintf(newbuff, PATH_MAX - 1, "%s/%s", base_dest, newpath);

			char * mkdirargs[] = { "mkdir", "-p", newbuff, NULL };
			exec_command(mkdirargs);
			copy_rec(base_source, base_dest, newpath, includes, recursion_level + 1, selected_features, selected_features_size);

			free(newpath);
		} else {
			if (skip(entry->d_name)) continue;
			char * newsource, * newdest;
			if ((newsource = malloc((strlen(sourcename) + strlen(entry->d_name) + 2))) == NULL) {
				perror("malloc");
				fails++;
				continue;
			}
			if ((newdest = malloc((strlen(destname) + strlen(entry->d_name) + 2))) == NULL) {
				perror("malloc");
				fails++;
				free(newsource);
				continue;
			}
			sprintf(newsource, "%s/%s", sourcename, entry->d_name);
			sprintf(newdest, "%s/%s", destname, entry->d_name);

			fails += copy_content(newsource, newdest, includes, recursion_level);
			free(newsource);
			free(newdest);
		}
	}

	free(sourcename);
	free(destname);
	closedir(directory);
	return fails == 0 ? 0 : 1;
}

chained_cell get_uniq_deps(plib_config config) {
	chained_cell list = NULL;

	if (config->dependencies_count > 0) {
		int i = 0;
		while (i < config->dependencies_count) {
			if (!stringcl_exists(list, config->dependencies[i])) stringcl_append(&list, config->dependencies[i]);
			i++;
		}
	}
	if (config->features_count > 0) {
		int i = 0;
		while (i < config->features_count) {
			int j = 0;
			while (config->features[i]->dependencies != NULL && config->features[i]->dependencies[j] != NULL) {
				if (!stringcl_exists(list, config->features[i]->dependencies[j])) stringcl_append(&list, config->features[i]->dependencies[j]);
				j++;
			}
			i++;
		}
	}

	return list;
}
