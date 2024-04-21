#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;
//giacomo jin 1937721
typedef struct {
  int quantum;
} SchedRRArgs;  //quantum

void schedRR(FakeOS* os, void* args_){  //scheduler RR
  SchedRRArgs* args=(SchedRRArgs*)args_;

  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;
  
  //devo riordinare i pcb in &os->ready per renderlo SJF
//... lo faccio piu banale possibile

// devo sfruttare che listitem* lo posso convertire in processevent* o FakePCB*
  int len=0; 
  int minimo=9999999999;
  ListItem* aux=os->ready.first; //è primo pcb* in ready sottoforma di listitem
  while(aux){

    //devo analizzare il tempo del primo evento del pcb in questione
    FakePCB* pcbaux= (FakePCB*) aux;
    ProcessEvent* eventoaux= (ProcessEvent*)pcbaux->events.first;
    int cpuburst= eventoaux->duration;
    if(cpuburst<minimo){
    minimo=cpuburst;
   //detach
    List_detach(&os->ready, aux); // stacco il pcb
    List_pushFront(&os->ready, aux);//inserisco pcb all inizio
    //pushfront
    }

    aux=aux->next;
    len++;  //ottengo pure len totale della coda da sto ciclo iniziale
 
   //sta roba trova solo il minimo totale e lo mette all inizio della coda ready
  }


  //faccio lista ausiliare di pcb in cui inserisco man mano tutti minimi che trovo
  FakePCB* pcbog= (FakePCB*)os->ready.first; //giusto

  FakePCB* pcbfantasma=(FakePCB*) malloc(sizeof(FakePCB)); //copia del primo pcb minimo
  pcbfantasma->list=pcbog->list;
  pcbfantasma->pid=pcbog->pid;
  pcbfantasma->events=pcbog->events;
  ListItem* pcbfantasmaitem=(ListItem*) pcbfantasma;
  
  ListHead ready2;  //lista fantasma , lho inizializzato bene?
  List_init(&ready2);
  List_pushFront(&ready2, pcbfantasmaitem);  //aggiunto primo minimo


  for(int i=0;i<len-1;i++){
     //lista di pcb ready che rendo sempre piu piccola
    //  ListItem* primoprima=os->ready.first;
     // List_detach(&os->ready, primoprima); 
      List_popFront(&os->ready);// stacco il primo

      aux=os->ready.first; //studio il secondo pcb, qua è listitem*
     //trovo minimo della rimanente
     int minimo=9999999999;

 while(aux){

    FakePCB* pcbaux= (FakePCB*) aux;
    ProcessEvent* eventoaux= (ProcessEvent*)pcbaux->events.first;
    int cpuburst= eventoaux->duration;
    if(cpuburst<minimo){
    minimo=cpuburst;
   // pcbminimo=pcbaux;  //salvato il minimo sus
   //detach
    List_detach(&os->ready, aux); // stacco il pcb
    List_pushFront(&os->ready, aux);//inserisco pcb all inizio
    //pushfront
    }

    aux=aux->next;

 }
 ListItem* primofine=os->ready.first;
 List_pushFront(&ready2, primofine); 

  }
 //elimino vecchia coda
  List_popFront(&os->ready);

  //pongo lista coda ordinata a ready
  os->ready=ready2;
 
//fatto ho coda ready ordinata in teoria 
  FakePCB* pcb=(FakePCB*) List_popFront(&os->ready); // prende il primo pcb della coda
  os->running=pcb;

  assert(pcb->events.first);
  ProcessEvent* e = (ProcessEvent*)pcb->events.first; //studio il primo evento della pcb running
  assert(e->type==CPU);

  // look at the first event                           
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (e->duration>args->quantum) {     
    ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=args->quantum;
    e->duration-=args->quantum;  
    List_pushFront(&pcb->events, (ListItem*)qe); 
    // evento attuale diventa 2 eventi, uno di durata quantum all inizio, e uno di durata rimanente appena dopo(in secondo posto)
  }
};


//main vero e proprio
int main(int argc, char** argv) {
  FakeOS_init(&os);
  SchedRRArgs srr_args;  //scheduler
  srr_args.quantum=5;  //da settare
  os.schedule_args=&srr_args;
  os.schedule_fn=schedRR;  //setta scheduler RR fcfs
  
  for (int i=1; i<argc; ++i){   //carico tutti i processi
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr); // aggiungo tutti i processi nella lista del s.o.E
    }
  }
  printf("num processes in queue %d\n", os.processes.size);
  while(os.running
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os); // finche ce qualcosa, faccio giro di simulazione
  }
}
