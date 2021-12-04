#ifndef LIST_H
#define LIST_H

typedef struct _Node {
   uint8_t note;
   uint8_t velo;
   struct _Node *next;
} Node;

#endif

