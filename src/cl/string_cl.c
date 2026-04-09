#include "string_cl.h"
#include "../aux/tools.h"
#include <stdlib.h>

chained_cell stringcl_create(char * value) {
	chained_cell cell;
	if ((cell = malloc(sizeof(struct st_chained_cell))) == NULL) return NULL;

	cell->next = NULL;
	cell->value = value;

	return cell;
}
void stringcl_destroy(chained_cell * list) {
	if (list == NULL) return;

	chained_cell current = *list;
	while (current != NULL) {
		chained_cell next = current->next;

		free(current->value);
		free(current);

		current = next;
	}
	*list = NULL;
}
unsigned long int stringcl_size(chained_cell list) {
	unsigned long int size = 0;
	chained_cell current = list;
	while (current != NULL) {
		size++;
		current = current->next;
	}

	return size;
}

int stringcl_append(chained_cell * list, char * value) {
	if (list == NULL) return 0;

	chained_cell cell;
	if ((cell = stringcl_create(value)) == NULL) return 0;

	if (*list == NULL) {
		*list = cell;
		return 1;
	}

	chained_cell current = *list;
	chained_cell previous = NULL;

	while (current != NULL) {
		previous = current;
		current = current->next;
	}
	previous->next = cell;
	return 1;
}

int stringcl_exists(chained_cell list, char * value) {
	chained_cell current = list;
	while (current != NULL) {
		if (streq(value, current->value)) return 1;
		current = current->next;
	}
	return 0;
}
int stringcl_remove(chained_cell * list, char * value) {
	if (list == NULL) return 0;
	if (value == NULL) return 0;

	if (streq(value, (*list)->value)) {
		chained_cell cell = *list;
		*list = cell->next;

		free(cell->value);
		free(cell);
		return 1;
	}
	chained_cell previous = *list;
	chained_cell current = previous->next;

	while (current != NULL) {
		if (streq(value, current->value)) {
			previous->next = current->next;
			free(current->value);
			free(current);
			return 1;
		}
		previous = current;
		current = previous->next;
	}
	return 0;
}
int stringcl_removep(chained_cell * list, chained_cell target) {
	if (list == NULL) return 0;
	if (target == NULL) return 0;

	if (*list == target) {
		*list = target->next;

		free(target->value);
		free(target);
		return 1;
	}

	chained_cell previous = *list;
	chained_cell current = previous->next;

	while (current != NULL) {
		if (current == target) {
			previous->next = current->next;
			free(current->value);
			free(current);
			return 1;
		}
		previous = current;
		current = previous->next;
	}

	return 0;
}

void stringcl_foreach(chained_cell list, void * data, void callback(chained_cell, void *)) {
	chained_cell current = list;

	while (current != NULL) {
		callback(current, data);
		current = current->next;
	}
}
