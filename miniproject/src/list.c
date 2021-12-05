#include "../inc/list.h"
#include <math.h>

#define N 1000
#define RATE 20000
#define TWELFTHROOT 1.0595

Node *listInsert(Node *list, uint8_t note, uint8_t velo) {
	Node *curr = list;
	Node *new = newNode(note, velo, (Node *) NULL);

	while(curr -> next) {
		if (curr -> note == note) {
			return list;
		}
		curr = curr -> next;
	}

	curr -> next = new;

	return list;
}

Node *listDelete(Node *list, uint8_t note) {
	Node *prev = list;
	Node *curr = list -> next;

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

Node *newNode(uint8_t note, uint8_t velo, Node *next) {
	Node *new = malloc(sizeof(Node));
	new -> note = note;
	new -> velo = velo;
	new -> next = next;
	float f = pow(TWELFTHROOT, (new -> note) - 0x45) * 440;
	new -> step = f * N / RATE * (1<<16);
	new -> offset = 0;

	return new;
}
