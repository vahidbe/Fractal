#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

int flagB1;
int flagB2;
int flagDone;

int optionD;
char* fileOutName;

struct sbuf* bufIn;
struct sbuf* bufOut;


struct sbuf{
    int** buf;           /* Buffer partagé */
    int n;             				/* Nombre de slots dans le buffer */
    int front;        				/* buf[(front+1)%n] est le premier élément */
    int rear;          				/* buf[rear%n] est le dernier */
    sem_t mutex;       				/* Protège l'accès au buffer */
    sem_t slots;       				/* Nombre de places libres */
    sem_t items;       				/* Nombre d'items dans le buffer */
};

/*
 * @pre sp!=NULL, n>0
 * @post a construit un buffer partagé contenant n slots
 */
void sbuf_init(struct sbuf *sp, int n)
{
	if(sp==NULL)
	{
		printf("--- EXIT ---\n");
		fflush(stdout);
		exit(-1);
	}
    sp->buf = calloc(n, sizeof(int*));
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
void sbuf_insert(struct sbuf *sp, int* item)
{
	///**/int ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d sur %d\n",ic,sp->n);
	sem_wait(&(sp->slots));
	sem_wait(&(sp->mutex));
	sp->rear=((sp->rear)+1*sizeof(int*))%(sp->n);
	sp->buf[sp->rear]=item;
	sem_post(&(sp->mutex));
	sem_post(&(sp->items));
	///**/ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d\n",ic);
}

/* @pre sbuf!=NULL
 * @post retire le dernier item du buffer partagé
 */
int* sbuf_remove(struct sbuf *sp)
{	
	///**/int ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d\n",ic);
	sem_wait(&(sp->items));
	sem_wait(&(sp->mutex));
	sp->front=((sp->front)+1)%(sp->n);
	struct fractal* res=sp->buf[sp->front];
	sem_post(&(sp->mutex));
	sem_post(&(sp->slots));
	///**/ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d\n",ic);
	return res;
}

void *producer(void* arguments){	
	/**/printf("--- DEBUT PRODUCTEUR ---\n");
	/**/fflush(stdout);
	int* id=malloc(sizeof(int))
	*id=5;
	/**/printf("--- INSERT PROD ---\n");
	/**/fflush(stdout);
	sbuf_insert(bufIn,id);
	/**/printf("--- FIN INSERT PROD ---\n");
	/**/fflush(stdout);
	/**/printf("--- FIN PRODUCTEUR ---\n");
	/**/fflush(stdout);
	flagB1--;
	flagDone--;
}

void *consumer(void* arguments){
	/**/printf("--- DEBUT CONSOMMATEUR ---\n");
	/**/fflush(stdout);
	/**/printf("--- REMOVE CONS ---\n");
	/**/fflush(stdout);
	int* id=sbuf_remove(bufIn);
	/**/printf("--- FIN REMOVE CONS ---\n");
	/**/fflush(stdout);
	/**/printf("--- INSERT CONS ---\n");
	/**/fflush(stdout);
	sbuf_insert(bufOut,id);
	/**/printf("--- FIN INSERT CONS ---\n");
	/**/fflush(stdout);
	/**/printf("--- FIN CONSOMMATEUR ---\n");
	/**/fflush(stdout);
	while(flagB1>0){}
	flagB2--;
	flagDone--;
}

void *writer(void* arguments){
	/**/printf("--- DEBUT WRITER ---\n");
	/**/fflush(stdout);
	/**/printf("--- REMOVE WRIT ---\n");
	/**/fflush(stdout);
	int* id=sbuf_remove(bufOut);
	/**/printf("--- FIN REMOVE WRIT ---\n");
	/**/fflush(stdout);
	/**/printf("ID=%d\n",*id);
	/**/fflush(stdout);
	/**/printf("--- FIN WRITER ---\n");
	/**/fflush(stdout);
	while(flagB2>0){}
	flagDone--;
	return NULL;
}

int main(int argc, char *argv[])
{
	bufIn=malloc(sizeof(struct sbuf));
	bufOut=malloc(sizeof(struct sbuf));
	/**/printf("%s","--- Initialisation des variables terminée ---\n");
	/**/fflush(stdout);
	
	sbuf_init(bufIn, (10));            
	sbuf_init(bufOut, (10));    

	/**/printf("--- Initialisation des buffers terminée ---\n");
	/**/fflush(stdout);
	
	pthread_t prod[1];
	pthread_t cons[1];
	pthread_t writ[1];
	
	/**/printf("--- Initialisation des tableaux de pthread_t terminée ---\n");
	/**/fflush(stdout);
	
	flagB1=1;
	flagB2=1;
	flagDone=3;
	/**/printf("--- Initialisation des constantes terminée ---\n");
	/**/fflush(stdout);
	/**/printf("---CREATION D'UN PRODUCTEUR---\n");
	/**/fflush(stdout);
	pthread_create(&(prod[0]), NULL, (void*) &producer, NULL);
	/**/printf("--- Initialisation des producteurs terminée ---\n");
	/**/fflush(stdout);
	/**/printf("---CREATION D'UN CONSOMMATEUR---\n");
	/**/fflush(stdout);
	pthread_create(&(cons[0]), NULL, (void*) &consumer, NULL);
	/**/printf("--- Initialisation des consommateurs terminée ---\n");
	/**/fflush(stdout);
	/**/printf("---CREATION D'UN WRITER---\n");
	/**/fflush(stdout);
	pthread_create(&(writ[0]), NULL, (void*) &writer, NULL);
	/**/printf("--- Initialisation des writers terminée ---\n");
	/**/fflush(stdout);
	
	/**/printf("\n doneFlag = %d \n\n",flagDone);
	/**/fflush(stdout);
	
	while((flagDone)>0)
	{
		///**/printf("\n DONEFLAG=%d \n\n",*doneFlag);
		///**/fflush(stdout);
	}
	
	/**/printf("--- Fin des threads ---\n");
	/**/fflush(stdout);
	
	end:
	
	sbuf_clean(bufIn);
	sbuf_clean(bufOut);
	
	/**/printf("--- Buffers clean ---\n");
	/**/fflush(stdout);
	
	
	/**/printf("--- Fin du programme ---\n");
	/**/fflush(stdout);
	
	return 0;
}
