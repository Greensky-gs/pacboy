#include "pairs.h"
#include <stdlib.h>
#include "../aux/tools.h"

pair create_pair(char * k, char * v) {
	pair cell;
	if ((cell = malloc(sizeof(struct st_pair))) == NULL) return NULL;

	cell->name = k;
	cell->value = v;
	cell->next = NULL;

	return cell;
}

int exists(pair list, char * name) {
	while (list != NULL) {
		if (streq(list->name, name)) return 1;
		list = list->next;
	}
	return 0;
}

void destroy_pair(pair list) {
	while (list != NULL) {
		pair next = list->next;
		free(list->name);
		free(list->value);
		free(list);

		list = next;
	}
}

int append_pair(pair * list, char * name, char * value) {
	if (list == NULL) return 0;

	pair cell;
	if ((cell = create_pair(name, value)) == NULL) return 0;

	if (*list == NULL) {
		*list = cell;
		return 1;
	}

	pair previous = NULL;
	pair current = *list;

	while (current != NULL) {
		previous = current;
		current = current->next;
	}
	previous->next = cell;

	return 1;
}

char * get_value(pair list, char * name) {
	while (list != NULL) {
		if (streq(list->name, name)) return list->value;
		list = list->next;
	}
	return NULL;
}
