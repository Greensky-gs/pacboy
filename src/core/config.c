#include "./config.h"
#include "../aux/tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
	StateNothing = 0,
	StateDeps = 1,
	StateFeatures = 2
};

struct fill_callback_struct {
	plib_feature feature;
	int count;
};
static void fill_callback(chained_cell cell, void * pdata) {
	struct fill_callback_struct * data = pdata;

	data->feature->dependencies[data->count] = cell->value;
	data->count++;
}

static void handle_pushing(chained_cell * feature_deps, int features_index, plib_config config) {
	unsigned long size = stringcl_size(*feature_deps);
	if ((config->features[features_index]->dependencies = malloc(sizeof(char *) * (size + 1))) == NULL) {
		perror("malloc");
	} else {
		struct fill_callback_struct data = {
			config->features[features_index],
			0
		};
		stringcl_foreach(*feature_deps, &data, fill_callback);
		stringcl_destroy_nofree(feature_deps);
		config->features[features_index]->dependencies[size] = NULL;
	}
}

plib_config parse_config_file(char * path) {
	FILE * stream;
	if ((stream = fopen(path, "rt")) == NULL) return NULL;

	plib_config config;
	if ((config = malloc(sizeof(struct lib_config_file))) == NULL) {
		fclose(stream);
		return NULL;
	}

	config->dependencies_count = 0;
	config->features_count = 0;
	config->dependencies = NULL;
	config->features = NULL;

	char buffer[BUFSIZ] = {0};
	int state = StateNothing;
	while (fgets(buffer, BUFSIZ - 1, stream) != NULL) {
		if (streq("[deps]\n", buffer)) {
			state = StateDeps;
		} else if (streq("[features]\n", buffer)) {
			state = StateFeatures;
		} else if (state == StateDeps) {
			config->dependencies_count++;
		} else if (state == StateFeatures) {
			if (!(buffer[0] == ' ' && buffer[1] == '-' && buffer[2] == ' ' && buffer[3] != '\n' && buffer[3] != 0)) config->features_count++;
		}
	}
	if (config->dependencies_count > 0) {
		if ((config->dependencies = malloc(sizeof(char *) * config->dependencies_count)) == NULL) {
			free(config);
			fclose(stream);
			return NULL;
		}
	}
	if (config->features_count > 0) {
		if ((config->features = calloc(config->features_count, sizeof(struct lib_feature))) == NULL) {
			if (config->dependencies != NULL) free(config->dependencies);
			free(config);
			fclose(stream);
			return NULL;
		}
	}

	int deps_index = 0;
	int features_index = -1;
	chained_cell feature_deps = NULL;
	fseek(stream, 0, SEEK_SET);

	state = StateNothing;
	while (fgets(buffer, BUFSIZ - 1, stream) != NULL) {
		if (streq("[deps]\n", buffer)) {
			state = StateDeps;
		} else if (streq("[features]\n", buffer)) {
			state = StateFeatures;
		} else if (state == StateDeps) {
			int i = 0;
			while (buffer[i] != 0) {
				if (buffer[i] == '\n') buffer[i] = 0;
				i++;
			}

			char * clone = strdup(buffer);
			if ((config->dependencies[deps_index] = clone) == NULL) {
				fprintf(stderr, "failed to clone buffer");
			}
			deps_index++;
		} else if (state == StateFeatures) {
			if (buffer[0] == ' ' && buffer[1] == '-' && buffer[2] == ' ' && buffer[3] != '\n' && buffer[3] != 0) {
				if (config->features[features_index] == NULL) continue;

				int i = 0;
				while (buffer[i] != 0) {
					if (buffer[i] == '\n') buffer[i] = 0;
					i++;
				}

				char * clone;
				if ((clone = strdup(buffer + 3)) == NULL) {
					perror("clone");
					continue;
				}
				stringcl_append(&feature_deps, clone);
			} else {
				if (features_index >= 0 && feature_deps != NULL) {
					handle_pushing(&feature_deps, features_index, config);
				}

				features_index++;
				int i = 0;
				while (buffer[i] != 0) {
					if (buffer[i] == '\n') buffer[i] = 0;
					i++;
				}

				if ((config->features[features_index] = malloc(sizeof(struct lib_feature))) == NULL) {
					perror("malloc");
				} else {
					if ((config->features[features_index]->name = strdup(buffer)) == NULL) {
						fprintf(stderr, "failed to clone buffer for feature name");
					}
					config->features[features_index]->dependencies = NULL;
				}
			}
		}
	}
	if (features_index >= 0 && feature_deps != NULL) handle_pushing(&feature_deps, features_index, config);

	return config;
}

void destroy_plib_config(plib_config config) {
	int i = 0;
	while (i < config->dependencies_count) {
		free(config->dependencies[i]);
		i++;
	}
	if (config->dependencies != NULL) free(config->dependencies);
	i = 0;
	while (i < config->features_count) {
		int j = 0;
		while (config->features[i]->dependencies != NULL && config->features[i]->dependencies[j] != NULL) {
			free(config->features[i]->dependencies[j]);
			j++;
		}
		if (config->features[i]->dependencies != NULL) free(config->features[i]->dependencies);
		free(config->features[i]->name);
		free(config->features[i]);
		i++;
	}
	if (config->features != NULL) free(config->features);
	free(config);
}
