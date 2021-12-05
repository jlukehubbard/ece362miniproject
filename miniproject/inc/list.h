#include <stdlib.h>
#include <stdint.h>

#ifndef LIST_H
#define LIST_H

typedef struct _Node {
   uint8_t note;
   uint8_t velo;
   uint32_t step;
   uint32_t offset;
   struct _Node *next;
} Node;

Node *listInsert(Node* list, uint8_t note, uint8_t velo);
Node *listDelete(Node* list, uint8_t note);
Node *newNode(uint8_t note, uint8_t velo, Node *next);



#endif

