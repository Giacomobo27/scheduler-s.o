#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"
//giacomo jin 1937721
void FakeOS_init(FakeOS* os) {
  List_init(&os->running); //dim max è data da numcpu da schedrrargs
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
  ListItem* aux=os->running.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");  //controllo che i pid siano univoci
    aux=aux->next;
  }

  aux=os->ready.first;
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
  ProcessEvent* e=(ProcessEvent*)new_pcb->events.first; //converte da listitem*(preso da lista di processevent) a processEvent*, allora conserva tutte le proprietà di processevent
  switch(e->type){
  case CPU:
    List_pushBack(&os->ready, (ListItem*) new_pcb);     //se primo process event è cpu->metto pcb in ready
    break;                                //e quindi qui riconverte da processevent* a listitem* per rimetterlo in lista
  case IO:
    List_pushBack(&os->waiting, (ListItem*) new_pcb);  //se primo process event è io->metto pcb in waiting
    break;
  default:
    assert(0 && "illegal resource");
    ;
  }
}




void FakeOS_simStep(FakeOS* os){ // fa giro di giostra   e implemento il timer
  //scannerizza di fakeos prima lista processi,
  //poi scannerizza lista waiting,
  //poi scannerizza lista running
  //e infine chiama scheduling che scannerizza lista ready
  printf("************** TIME: %08d **************\n", os->timer);

  //scan process waiting to be started  
  //and create all processes starting now
  ListItem* aux=os->processes.first; // é listitem*
  while (aux){
    FakeProcess* proc=(FakeProcess*)aux; //converte listitem* a fakeprocess*
    FakeProcess* new_process=0;
    if (proc->arrival_time==os->timer){  //scannerizza tutti i processi, se il timer di uno di questo è immminente, crea quel processo 
      new_process=proc;  // processo che sta per partire trovato
    }
    aux=aux->next;
    if (new_process) {
      printf("\tcreate pid:%d\n", new_process->pid);
      new_process=(FakeProcess*)List_detach(&os->processes, (ListItem*)new_process); // stacco il processo che sto creando dalla lista di tutti i processi dentro fake os
      FakeOS_createProcess(os, new_process); //crea pcb ( è la funzione sopra) e alloca in ready o waiting
      free(new_process); //libero spazio 
    }
  }

  // scan waiting list, and put in ready all items whose event terminates, per vedere se i/o sono terminati
  aux=os->waiting.first;  // ogni volta che ciclo su un evento, gli decremento di 1 la sua durata del suo evento, è come se ho implementato un timer cosi
  while(aux) { //finche ce elemento in waiting list
    FakePCB* pcb=(FakePCB*)aux;
    aux=aux->next; //tanto il primo aux lo ha gia salvato in pcb 
    ProcessEvent* e=(ProcessEvent*) pcb->events.first;
    printf("\twaiting pid: %d\n", pcb->pid);
    assert(e->type==IO); //perche primo event di pcb in waiting deve sempre essere di tipo IO
    e->duration--; //decremento durata IO in ogni pcb in waiting 
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
  // if event over, destroy event and reschedule process
  // if last event, destroy running


  aux= os->running.first;
 // FakePCB* running=(FakePCB*)os->running.first;  // running= primo della coda running, ricordo ListItem*=FakePCB* (un po ambiguo)
  if(!aux) printf("\trunning pid:-1\n");


   //ce almeno un pcb in running, guardo anche gli altri della coda running
   
   // decremento tempo dei processi event cpu in running , quindi per ogni pcb in coda running
  while(aux){
   FakePCB* running=(FakePCB*)aux;    
  //  printf("\trunning analizzato pid %d\n",running->pid);       
    ProcessEvent* e=(ProcessEvent*)running->events.first;
    assert(e->type==CPU);
    e->duration--; 


    printf("\trunning pid: %d\n", running?running->pid:-1); // tanto pid=-1 se aux2 è nullo, succede solo a fine lista
    printf("\t\tremaining time:%d\n",e->duration);

  if (e->duration==0){
      printf("\t\tend burst\n");
      List_popFront(&running->events);  //elimina primo evento del pcb running attuale perche finito
      free(e);
       List_detach(&os->running,aux); //proviamo
      

      if (! running->events.first) {  //se finito l evento non ci sono altri eventi dopo, allora sono finiti tutti gli eventi 
        printf("\t\tend process ( non faccio free)\n");
        // elimina pcb da coda running perche ha finito gli eventi
        // FakePCB* trash=(FakePCB*)auxtrash;
        // free(running);  // va in comflitto con reference prima del ciclo

      } else {
        e=(ProcessEvent*) running->events.first;
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n");
         // List_detach(&os->running,aux); //toglie da coda running
          List_pushBack(&os->ready, (ListItem*) running);  //mette in coda rady
          break;
        case IO:
          printf("\t\tmove to waiting\n");
         // List_detach(&os->running,aux);
          List_pushBack(&os->waiting, (ListItem*) running); //mette in coda waiting
          break;
        }
      }
      //perche cmq se è finito cpu burst, devo leva il pcb running al cpu per lascia spazio ad un altro pcb in coda in ready
  }

  aux=aux->next; //guardo pcb running successivo 
}  //chiusura ciclo while


  // call schedule, if defined
  if (os->schedule_fn){ //ho tolto una condizione sul running
    (*os->schedule_fn)(os, os->schedule_args); //funzione scheduling, guarda tutte le strutture del fake os e decide coda pcb running
  }

  // if running not defined and ready queue not empty
  // put the first in ready to run

  //forse lo devo levare sta parte (?)
  //if (! os->running && os->ready.first) {
  //  os->running=(FakePCB*) List_popFront(&os->ready);
  //}

  ++os->timer; // va avanti di un secondo il timer globale

}

void FakeOS_destroy(FakeOS* os) {
}
