#include "fake_process.h"
#include "linked_list.h"
#pragma once
//giacomo jin 1937721

typedef struct {  //aggiungo contatore?
  ListItem list;   // fake pcb è un elemento di una lista
  int pid;   //pid di un processo
  ListHead events;  // lista di eventi di questo processo, puoi vederlo come le istruzioni del processo
} FakePCB;  //fake process control block, ne ho uno per ogni processo

struct FakeOS; //classe s.o finto
typedef void (*ScheduleFn)(struct FakeOS* os, void* args); //puntatore a funzione scheduling di cui farò dopo overriding

typedef struct FakeOS{ //ricordo ListItem*=FakePCB*
  ListHead running;  //coda pcb di running
  ListHead ready;   //code pcb di cui primo evento è cpu
  ListHead waiting; // code pcb di cui primo evento è io
  int timer;
  ScheduleFn schedule_fn;  //puntatore a funzione di scheduling, dovro assegnare qui la mia funzione(politica) di scheduling
  void* schedule_args;

  ListHead processes;  //lista in cui ci sono tutti i processi, quando questi partono, io li gestisco (li metto in running o in coda ready ecct.)
} FakeOS;

void FakeOS_init(FakeOS* os); //costruttore
void FakeOS_simStep(FakeOS* os); //prova  un ciclo di running
void FakeOS_destroy(FakeOS* os);
