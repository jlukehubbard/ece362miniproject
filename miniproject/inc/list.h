#include <stdlib.h>

#ifndef LIST_H
#define LIST_H

typedef struct _Node {
   uint8_t note;
   uint8_t velo;
   struct _Node *next;
} Node;

Node *listInsert(Node *list, uint8_t note, uint8_t velo) {
	Node *curr = list;
	Node *new = malloc(sizeof Node);
	new -> note = note;
	new -> velo = velo;
	new -> next = NULL;

	while(curr -> next) {
		curr = curr -> next;
	}

	curr -> next = new;

	return list;
}

Node *listDelete(Node *list, uint8_t note) {
	Node *prev = list;
	Node *curr = list -> next;

	if (list -> note == note) {
		free(list);
		return curr;
	}

	while (curr && (curr -> note != note)) {
		prev = curr;
		curr = curr -> next;
	}

	if (!curr) {
		return list;
	} else {
		prev -> next = curr -> next;
		free(curr);
		return list;
	}
}

#endif

