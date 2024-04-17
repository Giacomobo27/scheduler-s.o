#pragma once
#include "linked_list.h"
//giacomo jin 1937721
typedef enum {CPU=0, IO=1} ResourceType;  //tipo risorsa

// event of a process
typedef struct {
  ListItem list;   //processevent è un elemento di una lista
  ResourceType type; // evento processo è cpu o io
  int duration;
} ProcessEvent;   // carico i processevent dentro lista dentro fakeprocess

// fake process
typedef struct {
  ListItem list;   //fakeprocess è un elemento di una lista (la coda per scheduling) 
  int pid; // assigned by us
  int arrival_time; 
  ListHead events;  //lista di processevent, puoi vederlo come le istruzioni del processo
} FakeProcess;
 
int FakeProcess_load(FakeProcess* p, const char* filename); //carica fakeprocess da filename a puntatore p e returna i numeri di eventi che è riuscito a caricare

int FakeProcess_save(const FakeProcess* p, const char* filename); // salvo, poi controllo che processo caricato= salvato per evitare errori
