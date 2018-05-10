#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "./libfractal/fractal.h"
//#include "./libfractal/fractal.c"
//#include "./libfractal/tools.c"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>

struct args{
  char* charP_arg;
};	

struct sbuf{
  struct fractal** buf;           		/* Buffer partagé */
  int n;             				/* Nombre de slots dans le buffer */
  int front;        				/* buf[(front+1)%n] est le premier élément */
  int rear;          				/* buf[rear%n] est le dernier */
  sem_t empty;       				/* Nombre de places libres */
  sem_t full;       				/* Nombre d'items dans le buffer */
  pthread_mutex_t mutex;
};

struct charbuf{
  char** buf;
  int n;
  int front;
  int rear;
  pthread_mutex_t mutex;
};

extern int countProd;
extern int countCons;
extern int countWrit;
extern int numberThreads;
extern int numberProd;
extern int numberCons;
extern int optionD;
extern char* fileOutName;
extern int sortie;
extern int fractCountP;
extern int fractCountC;
extern int fractCountW;
extern int maxEgal;

extern sem_t directeur;
extern pthread_mutex_t tuteur1;
extern pthread_mutex_t tuteur2;
extern pthread_mutex_t professor;
extern pthread_mutex_t gardien;
extern int countEleves;
extern struct sbuf* bufIn;
extern struct sbuf* bufOut;
extern struct sbuf* highestF;
extern struct charbuf* bufFName;
extern double average;


void sbuf_init(struct sbuf *sp, int n);
void charbuf_init(struct charbuf *sp, int n);
void sbuf_clean(struct sbuf *sp);
void charbuf_clean(struct charbuf *sp);
void sbuf_insert(struct sbuf *sp, struct fractal* f);
void sbuf_insertHighest(struct sbuf *sp, struct fractal* f);
void charbuf_insert(struct charbuf *sp, char* f);
struct fractal* sbuf_remove(struct sbuf *sp);
struct fractal* sbuf_removeHighest(struct sbuf *sp);
int charbuf_already_used(struct charbuf *sp, char* name);
void *producer(void* arguments);
void *consumer(void* arguments);
void *writer(void* arguments);	
