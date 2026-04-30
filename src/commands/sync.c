#include "./sync.h"
#include "./core.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

psyncconfig read_sync_config_file(char * path, int * returncode) {
	psyncconfig config;
	if ((config = malloc(sizeof(struct sSyncconfig))) == NULL) {
		perror("malloc");
		SET_PTR_VAL(returncode, MALLOC);
		return NULL;
	}

	int fd;
	if ((fd = open(path, O_RDONLY, 0)) == -1) {
		perror("open");
		free(config);
		SET_PTR_VAL(returncode, OPEN);
		return NULL;
	}

	if (read(fd, config, sizeof(struct sSyncconfig)) != sizeof(struct sSyncconfig)) {
		perror("read");
		fprintf(stderr, "Read of unexpected size");
		free(config);
		close(fd);
		SET_PTR_VAL(returncode, READ);
		return NULL;
	}
	close(fd);
	SET_PTR_VAL(returncode, OK);

	return config;
}

int save_sync_config_file(char * path, psyncconfig config) {
	int fd;
	if ((fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1) {
		perror("open");
		return 1;
	}

	if (write(fd, config, sizeof(struct sSyncconfig)) != sizeof(struct sSyncconfig)) {
		perror("write");
		fprintf(stderr, "Write of unexpected size");
		close(fd);
		return 1;
	}
	close(fd);

	return 0;
}

psyncconfig default_config() {
	psyncconfig config;
	if ((config = malloc(sizeof(struct sSyncconfig))) == NULL) {
		perror("malloc");
		return NULL;
	}

	config->repo_url[0] = 0;
	return config;
}
