#include "fake_process.h"
#include "linked_list.h"
#pragma once


typedef struct {
  ListItem list;   // fake pcb è un elemento di una lista
  int pid;   //pid di un processo
  ListHead events;  // lista di eventi di questo processo, puoi vederlo come le istruzioni del processo
} FakePCB;  //fake process control block

struct FakeOS; //classe s.o finto
typedef void (*ScheduleFn)(struct FakeOS* os, void* args); //puntatore di funzione scheling di cui farò dopo overriding

typedef struct FakeOS{
  FakePCB* running; // ho tanti pcb, uno a processo, questo punta al pcb del processo attuale che sta runnando
  ListHead ready;   //code processi
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;  //puntatore a funzione di scheduling, dovro assegnare qui la mia funzione(politica) di scheduling
  void* schedule_args;

  ListHead processes;  //lista in cui ci sono tutti i processi, quando questi partono, io li gestisco (li metto in running o in coda ready ecct.)
} FakeOS;

void FakeOS_init(FakeOS* os); //costruttore
void FakeOS_simStep(FakeOS* os); //prova  un ciclo
void FakeOS_destroy(FakeOS* os);
