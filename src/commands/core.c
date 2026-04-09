#include "core.h"
#include "../aux/tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setup(char * path) {
	char * args[] = { "mkdir", "-p", path, NULL };

	exec_command(args);
}

char ** read_config_file(char * input, int * psize) {
	FILE * stream;
	*psize = -1;
	if ((stream = fopen(input, "rt")) == NULL) {
		perror("fopen");
		return NULL;
	}

	char buffer[BUFSIZ];
	int started = 0;
	int count = 0;

	while (fgets(buffer, BUFSIZ - 1, stream) != NULL) {
		if (streq(buffer, "[deps]\n")) {
			started = 1;
			continue;
		}
		if (!started) continue;
		
		int i = 0;
		int p = 0;
		while (buffer[i] != 0) {
			if (buffer[i] == '\n') {
				p = i;
				break;
			}
			i++;
		}
		if (p != 0 && buffer[0] == '[' && buffer[p - 1] == ']') {
			break;
		}

		count++;
	}
	if (count == 0) {
		fclose(stream);
		return NULL;
	}

	char ** array;
	if ((array = malloc(sizeof(char *) * count)) == NULL) {
		perror("malloc");
		fclose(stream);
		return NULL;
	}

	started = 0;
	int index = 0;
	fseek(stream, 0, SEEK_SET);
	while (fgets(buffer, BUFSIZ - 1, stream) != NULL) {
		if (streq("[deps]\n", buffer)) {
			started = 1;
			continue;
		}
		if (!started) continue;

		int i = 0;
		int p = 0;
		while (buffer[i] != 0) {
			if (buffer[i] == '\n') {
				p = i;
				break;
			}
			i++;
		}
		if (p != 0 && buffer[0] == '[' && buffer[p - 1] == ']') {
			break;
		}

		buffer[p] = 0;
		array[index] = strdup(buffer);
		index++;
	}

	*psize = count;
	fclose(stream);
	return array;
}

void destroy_array(char ** tab, int size) {
	int i = 0;
	while (i < size) {
		if (tab[i] != NULL) free(tab[i]);
		i++;
	}
	free(tab);
}
