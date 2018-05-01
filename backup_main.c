#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fractal.h"
#include "fractal.c"
#include "tools.c"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFSIZE 10

#define MUTEX 0
#define FULL 1
#define EMPTY 2

//TODO: redemander l'architecture à suivre
//TODO: gérer le fait que le producteur consommateur utilise des struct fractale* au lieu de int

struct args{
		int argc_arg;
		char** argv_arg;
		struct sbuf* buf_arg;
};	

struct sbuf{
    int *buf;          /* Buffer partagé */
    int n;             /* Nombre de slots dans le buffer */
    int front;         /* buf[(front+1)%n] est le premier élément */
    int rear;          /* buf[rear%n] est le dernier */
    sem_t mutex;       /* Protège l'accès au buffer */
    sem_t slots;       /* Nombre de places libres */
    sem_t items;       /* Nombre d'items dans le buffer */
};

int entreestd;
/*
 * @pre sp!=NULL, n>0
 * @post a construit un buffer partagé contenant n slots
 */
void sbuf_init(struct sbuf *sp, int n)
{
    sp->buf = calloc(n, sizeof(int));
    sp->n = n;                       /* Buffer content les entiers */
    sp->front = sp->rear = 0;        /* Buffer vide si front == rear */
    sem_init(&sp->mutex, 0, 1);      /* Exclusion mutuelle */
    sem_init(&sp->slots, 0, n);      /* Au début, n slots vides */
    sem_init(&sp->items, 0, 0);      /* Au début, rien à consommer */
}

/*
 * @pre sp!=NULL
 * @post libère le buffer
 */
void sbuf_clean(struct sbuf *sp)
{
    free(sp->buf);
}

/* @pre sp!=NULL
 * @post ajoute item à la fin du buffer partagé. Ce buffer est géré
 *       comme une queue FIFO
 */
void sbuf_insert(struct sbuf *sp, struct fractal* item)
{
	sem_wait(&(sp->slots));
	sem_wait(&(sp->mutex));
	sp->rear=((sp->rear)+1)%(sp->n);
	sp->buf[sp->rear]=item;
	sem_post(&(sp->mutex));
	sem_post(&(sp->items));
}
/* @pre sbuf!=NULL
 * @post retire le dernier item du buffer partagé
 */
struct fractal* sbuf_remove(struct sbuf *sp)
{
	sem_wait(&(sp->items));
	sem_wait(&(sp->mutex));
	sp->front=((sp->front)+1)%(sp->n);
	struct fractal* res=sp->buf[sp->front];
	sem_post(&(sp->mutex));
	sem_post(&(sp->slots));
	return res;
}
void *producer(void* arguments){
	struct args* argument=(struct args*) arguments;
	char** argv=argument->argv_arg;
	int argc=argument->argc_arg;
	struct sbuf* buf=argument->buf_arg;
	
	if(entreestd){
		int i;
		for(i=0;(i<argc-1) % 5;i++){
			//TODO: gérer le toInt
			struct fractal* f = fractal_create(argv[0+i*5],toInt(argv[1+i*5]),toInt(argv[2+i*5]),toInt(argv[3+i*5]),toInt(argv[4+i*5]));
			sbuf_insert(buf,f);
		}
	}
	else{
		//TODO: gérer l'entrée par fichier
	}	
}

struct fractal* consumer(void* arguments){
	struct args* argument=(struct args*) arguments;
	char** argv=argument->argv_arg;
	int argc=argument->argc_arg;
	struct sbuf* buf=argument->buf_arg;
	
	struct fractal* f=sbuf_remove(buf);
	int i;
	int j;
	for(i=0;i<f->width;i++){
		for(j=0;j<f->height;j++){
			fractal_set_value(f,i,j,fractal_compute_value(f,i,j));
		}
	}
	//TODO: envoyer f dans un autre consommateur qui va soit (si -d) sortir un bmp de f, soit garder f en mémoire et, s'il reçoit un autre f plus élevé, garde celui-la en mémoire a la place du premier f, sinon le garde
}
int main(int argc, char *argv[])
{	
	//TODO: gérer la lecture des options -d et --maxthreads
	int numberThreads = 4;
	entreestd = 1;
	
	struct sbuf* buf;
	struct sbuf* bufout;
	struct args* arguments=(struct args*) malloc(sizeof(struct args));
	arguments->argc_arg=argc;
	arguments->argv_arg=argv;
	
	sbuf_init(buf, (numberThreads));
	arguments->buf_arg=buf;
	

	pthread_t prod, cons;	
	pthread_create(&prod, NULL, (void*) &producer, (void*) arguments);
	pthread_create(&cons, NULL, (void*) &consumer, (void*) arguments);
	//TODO: créer un autre consommateur pour s'occuper de la sortie
	
	//TODO: gérer la destuction des threads
	//pthread_exit(NULL);
}