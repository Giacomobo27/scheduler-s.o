#pragma once
//giacomo jin 1937721
typedef struct ListItem {
  struct ListItem* prev;
  struct ListItem* next;
} ListItem;

typedef struct ListHead { //lista di tante head in cui ce il dato int size, e sono collegate tramite ListItem
  ListItem* first;
  ListItem* last;
  int size;
} ListHead;

void List_init(ListHead* head);
ListItem* List_find(ListHead* head, ListItem* item);
ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);
ListItem* List_detach(ListHead* head, ListItem* item);
ListItem* List_pushBack(ListHead* head, ListItem* item); //inserimento in coda
ListItem* List_pushFront(ListHead* head, ListItem* item);
ListItem* List_popFront(ListHead* head); //rimozione in testa
