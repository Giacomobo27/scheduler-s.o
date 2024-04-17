#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"

void FakeOS_init(FakeOS* os) {
  os->running=0;
  List_init(&os->ready);
  List_init(&os->waiting);
  List_init(&os->processes);
  os->timer=0;
  os->schedule_fn=0;
}

void FakeOS_createProcess(FakeOS* os, FakeProcess* p) { //attiva processo p al tempo corrente, crea pcb e lo mette nella coda giusta
  // sanity check
  assert(p->arrival_time==os->timer && "time mismatch in creation");  //tempi sono corretti
  // we check that in the list of PCBs there is no
  // pcb having the same pid
  assert( (!os->running || os->running->pid!=p->pid) && "pid taken");

  ListItem* aux=os->ready.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");  //controllo che i pid siano univoci
    aux=aux->next;
  }

  aux=os->waiting.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  // all fine, no such pcb exists , procedo con creazione pcb per nuovo processo
  FakePCB* new_pcb=(FakePCB*) malloc(sizeof(FakePCB));
  new_pcb->list.next=new_pcb->list.prev=0;
  new_pcb->pid=p->pid;
  new_pcb->events=p->events;

  assert(new_pcb->events.first && "process without events");

  // depending on the type of the first event
  // we put the process either in ready or in waiting
  ProcessEvent* e=(ProcessEvent*)new_pcb->events.first;
  switch(e->type){
  case CPU:
    List_pushBack(&os->ready, (ListItem*) new_pcb);     //se primo process event è cpu->metto pcb in ready
    break;
  case IO:
    List_pushBack(&os->waiting, (ListItem*) new_pcb);  //se primo process event è io->metto pcb in waiting
    break;
  default:
    assert(0 && "illegal resource");
    ;
  }
}




void FakeOS_simStep(FakeOS* os){ // fa giro di giostra   e implemento il timer
  
  printf("************** TIME: %08d **************\n", os->timer);

  //scan process waiting to be started
  //and create all processes starting now
  ListItem* aux=os->processes.first;
  while (aux){
    FakeProcess* proc=(FakeProcess*)aux;
    FakeProcess* new_process=0;
    if (proc->arrival_time==os->timer){  //scannerizza tutti i processi, se il timer di uno di questo è immminente, crea quel processo per farlo runnare
      new_process=proc;  // processo che sta per partire trovato
    }
    aux=aux->next;
    if (new_process) {
      printf("\tcreate pid:%d\n", new_process->pid);
      new_process=(FakeProcess*)List_detach(&os->processes, (ListItem*)new_process); // stacco il processo che sto creando dalla lista di tutti i processsi dentro fake os
      FakeOS_createProcess(os, new_process); //creo pcb ( è la funzione sopra)
      free(new_process); //libero spazio 
    }
  }

  // scan waiting list, and put in ready all items whose event terminates, per vedere se i/o sono terminati
  aux=os->waiting.first;  // ogni volta che ciclo su un evento, gli decremento di 1 la sua durata del suo evento, è come se ho implementato un timer cosi
  while(aux) { //finche ce elemento in waiting list
    FakePCB* pcb=(FakePCB*)aux;
    aux=aux->next;
    ProcessEvent* e=(ProcessEvent*) pcb->events.first;
    printf("\twaiting pid: %d\n", pcb->pid);
    assert(e->type==IO); //perche primo event di pcb in waiting deve sempre essere di tipo IO
    e->duration--; //decremento durata IO
    printf("\t\tremaining time:%d\n",e->duration);
    if (e->duration==0){  //evento IO terminato
      printf("\t\tend burst\n");
      List_popFront(&pcb->events);  //elimino evento perche terminato
      free(e);
      //prendo processo e studio evento successivo del processo
      List_detach(&os->waiting, (ListItem*)pcb);  //stacco pcb da waiting
      if (! pcb->events.first) {
        // kill process
        printf("\t\tend process\n");  //se non ha piu eventi il processo, lo termini
        free(pcb);
      } else {
        //handle next event
        e=(ProcessEvent*) pcb->events.first; 
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n");
          List_pushBack(&os->ready, (ListItem*) pcb); // se prossimoevento è CPU, sposto pcb in coda ready
          break;
        case IO:
          printf("\t\tmove to waiting\n");
          List_pushBack(&os->waiting, (ListItem*) pcb); // se prossimo evento è IO, sposto pcb in waiting
          break;
        }
      }
    }
  }

  
//parte scheduling ossia gestione running
  // decrement the duration of running
  // if event over, destroy event
  // and reschedule process
  // if last event, destroy running
  FakePCB* running=os->running;
  printf("\trunning pid: %d\n", running?running->pid:-1);
  if (running) {
    ProcessEvent* e=(ProcessEvent*) running->events.first;
    assert(e->type==CPU);
    e->duration--;  // decremento tempo del processo event in running 
    printf("\t\tremaining time:%d\n",e->duration);



    if (e->duration==0){
      printf("\t\tend burst\n");
      List_popFront(&running->events);  //elimina evento perche finito
      free(e);
      if (! running->events.first) {
        printf("\t\tend process\n");
        free(running); // elimina processo perche ha finito gli eventi
      } else {
        e=(ProcessEvent*) running->events.first;
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n");
          List_pushBack(&os->ready, (ListItem*) running);
          break;
        case IO:
          printf("\t\tmove to waiting\n");
          List_pushBack(&os->waiting, (ListItem*) running);
          break;
        }
      }
      os->running = 0;
    }

 // qua non rimpiazzo mai prememptivamente i processi 

  }


  // call schedule, if defined
  if (os->schedule_fn && ! os->running){
    (*os->schedule_fn)(os, os->schedule_args); //funzione scheduling, guarda tutte le strutture del fake os e decide il prossimo running
  }

  // if running not defined and ready queue not empty
  // put the first in ready to run
  if (! os->running && os->ready.first) {
    os->running=(FakePCB*) List_popFront(&os->ready);
  }

  ++os->timer;

}

void FakeOS_destroy(FakeOS* os) {
}
