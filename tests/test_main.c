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

#define MUTEX 0
#define FULL 1
#define EMPTY 2

//TODO: redemander l'architecture à suivre
//TODO: gérer l'exit des threads (faut-il une variable qui vérifie si les étapes sont terminées?)

int flagConst;
int flagOutConst;
int doneFlagConst;

struct args{
	char* charP_arg;
	struct sbuf* buf_arg;
	struct sbuf* bufout_arg;
	int optionD;
	char* fileOutName;
	int* flag;
	int* flagOut;
	int* doneFlag;
};	

struct sbuf{
    int** buf;          /* Buffer partagé */
    int n;             /* Nombre de slots dans le buffer */
    int front;         /* buf[(front+1)%n] est le premier élément */
    int rear;          /* buf[rear%n] est le dernier */
    sem_t mutex;       /* Protège l'accès au buffer */
    sem_t slots;       /* Nombre de places libres */
    sem_t items;       /* Nombre d'items dans le buffer */
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
		exit(-1); //TODO: gérer erreurs
	}
	printf("--- Debut de l'initialisation du buffer ---\n");
fflush(stdout);
    sp->buf = calloc(n, sizeof(int*));
	printf("--- Malloc d'initialisation de buffer terminé ---\n");
fflush(stdout);
    sp->n = n;                       /* Buffer content les entiers */
    sp->front = sp->rear = 0;        /* Buffer vide si front == rear */
    sem_init(&sp->mutex, 0, 1);      /* Exclusion mutuelle */
    sem_init(&sp->slots, 0, n);      /* Au début, n slots vides */
    sem_init(&sp->items, 0, 0);      /* Au début, rien à consommer */
	printf("--- Initialisation du buffer terminée ---\n");
fflush(stdout);
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
	sem_wait(&(sp->slots));
	sem_wait(&(sp->mutex));
	sp->rear=((sp->rear)+1*sizeof(int*))%(sp->n);
	sp->buf[sp->rear]=item;
	sem_post(&(sp->mutex));
	sem_post(&(sp->items));
}

/* @pre sbuf!=NULL
 * @post retire le dernier item du buffer partagé
 */
int* sbuf_remove(struct sbuf *sp)
{
	sem_wait(&(sp->items));
	sem_wait(&(sp->mutex));
	sp->front=((sp->front)+1)%(sp->n);
	int* res=sp->buf[sp->front];
	sem_post(&(sp->mutex));
	sem_post(&(sp->slots));
	return res;
}

void *producer(void* arguments){	
	printf("--- DEBUT PRODUCTEUR ---\n");
fflush(stdout);
	struct args* argument=(struct args*) arguments;
	char* fileName=argument->charP_arg;
	struct sbuf* buf=argument->buf_arg;
	int* flag=argument->flag;
	int* doneFlag=argument->doneFlag;
	free(argument);
fflush(stdout);
  int i;
  int* tab=malloc(sizeof(int));
  for(i=0;i<15;i++)
  {
	  *tab=i
	  sbuf_insert(buf,tab);
  }
  printf("--- Fin producteur ---\n");
	fflush(stdout);
  (*doneFlag)--;
  (*flag)--;
  printf("\nPRODFLAG=%d\n\n",*flag);
	fflush(stdout);
  return NULL;
}

void *consumer(void* arguments){
	printf("--- DEBUT CONSOMMATEUR ---\n");
	fflush(stdout);
	int done=0;
	struct args* argument=(struct args*) arguments;
	struct sbuf* buf=argument->buf_arg;
	struct sbuf* bufout=argument->bufout_arg;
	int* flag=argument->flag;
	int* flagOut=argument->flagOut;
	int* doneFlag=argument->doneFlag;
	free(argument);
	printf("--- Debut calcul consommateur ---\n");
	fflush(stdout);
	while(!done){		
		int ic=0;
		printf("va lire sem_getvalue du consommateur\n");
	fflush(stdout);
		sem_getvalue(&(buf->items),&ic);
		printf("FLAG=%d\n",*flag);
	fflush(stdout);
		if(((*flag)<=0)&(ic==0))
		{
			printf("=====DONE=1=====\n");
	fflush(stdout);
			done=1;
		}
		else{
		printf("*REMOVE DU CONSOMMATEUR*\n");
	fflush(stdout);
		int* f=sbuf_remove(buf);
		printf("*REMOVE DU CONSOMMATEUR TERMINE*\n");
	fflush(stdout);
		
		printf("*INSERT DU CONSOMMATEUR*\n");
	fflush(stdout);
		sbuf_insert(bufout,f);	
		printf("*INSERT DU CONSOMMATEUR TERMINE*\n");	
	fflush(stdout);
		}
	}
	printf("--- Fin consommateur ---\n");
	fflush(stdout);
	(*flagOut)--;
	(*doneFlag)--;
	return NULL;
}

void *writer(void* arguments){
	printf("--- DEBUT WRITER ---\n");
fflush(stdout);
	int isEmpty=0;
	struct args* argument=(struct args*) arguments;
	struct sbuf* buf=argument->bufout_arg;
	int optionD=argument->optionD;
	char* fileOutName=argument->fileOutName;
	int average;
	int* highestF;
	int* flagOut=argument->flagOut;
	int* doneFlag=argument->doneFlag;
	free(argument);
	printf("--- Debut ecriture writer ---\n");
fflush(stdout);
		while(!isEmpty){
			int ic;
			printf("va lire sem_getvalue du writer\n");
fflush(stdout);
			sem_getvalue(&(buf->items),&ic);
			printf("\n FLAGOUT=%d\n\n",*flagOut);
fflush(stdout);
			if(((*flagOut)<=0)&(ic==0)){
				printf("===DONE=1===\n");
fflush(stdout);
				isEmpty=1;
			}
			else{
				printf("*REMOVE DU WRITER*\n");
fflush(stdout);
				int* f = sbuf_remove(buf);
				printf("*REMOVE DU WRITER TERMINE*\n");
fflush(stdout);
				if(*f>average){
					average=*f;
				highestF=f;
				}
				printf("average=%lf\n",average);
fflush(stdout);
			}
			
		}
		
		printf("=== ECRITURE ===\n");
fflush(stdout);
		printf("FINALEMENT ON A ECRIT %d",*f);
		printf("=== FIN ECRITURE ===\n");
fflush(stdout);
	printf("--- Fin writer ---\n");
fflush(stdout);
	(*doneFlag)--;
	return NULL;
}

int main(int argc, char *argv[])
{
	int numberThreads=0;
	int count;
	int optionsCount=0;
	int optionD=0;
	char* fileOutName;	
	struct sbuf* buf=malloc(sizeof(struct sbuf));
	struct sbuf* bufout=malloc(sizeof(struct sbuf));
	int* flag=(int*) malloc(sizeof(int));
	int* flagOut=(int*) malloc(sizeof(int));
	int* doneFlag=(int*) malloc(sizeof(int));
	printf("%s","--- Initialisation des variables terminée ---\n");
fflush(stdout);

	if(numberThreads==0){
	        numberThreads=argc-2-optionsCount;  //Vraiment utile de retirer optionsCount ? Il sera d'office nul --> il est pas d'office nul, regarde le if juste au dessus
	}
	
	printf("\n Nombre de threads qui vont être utilisés : %d \n \n",numberThreads);
fflush(stdout);
	
	sbuf_init(buf, (numberThreads+3));            
	sbuf_init(bufout, (numberThreads+3));    

	printf("--- Initialisation des buffers terminée ---\n");
fflush(stdout);
	
	pthread_t prod[argc-2-optionsCount];
	pthread_t cons[numberThreads];
	pthread_t writ[argc-2-optionsCount];
	
	printf("--- Initialisation des tableaux de pthread_t terminée ---\n");
fflush(stdout);
	
	flagConst=argc-2-optionsCount;
	flagOutConst=numberThreads;
	if(optionD){
		doneFlagConst=2*(argc-2-optionsCount)+numberThreads;
	}
	else{
		doneFlagConst=(argc-2-optionsCount)+numberThreads+1;
	}
	*doneFlag=doneFlagConst;
	*flag=flagConst;
	*flagOut=flagOutConst;
	printf("--- Initialisation des constantes terminée ---\n");
fflush(stdout);
	
	for(count=optionsCount+1;count<argc;count++){
	  if(((*argv[count])=='-')&(count!=(argc))){      
	  }
	  else{
			if(count!=(argc-1)){
				struct args* arguments=(struct args*) malloc(sizeof(struct args));
				if(arguments==NULL){
					goto end;
				}
				arguments->buf_arg=buf;
				arguments->flag=flag;
				arguments->doneFlag=doneFlag;
				arguments->charP_arg=argv[count];
				printf("---CREATION D'UN PRODUCTEUR---\n");
fflush(stdout);
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, (void*) arguments);
			}
			else{
				fileOutName=argv[count];
			}	
		}
	}
	
	printf("--- Initialisation des producteurs terminée ---\n");
fflush(stdout);
	
	int i;
	for(i=0;(i<numberThreads);i++){
		struct args* arguments=(struct args*) malloc(sizeof(struct args));
		if(arguments==NULL){
			goto end;
		}
		arguments->flag=flag;
		arguments->doneFlag=doneFlag;
		arguments->flagOut=flagOut;
		arguments->buf_arg=buf;
		arguments->bufout_arg=bufout;
		printf("---CREATION D'UN CONSOMMATEUR---\n");
fflush(stdout);
		pthread_create(&(cons[i]), NULL, (void*) &consumer, (void*) arguments);
	}
	
	printf("--- Initialisation des consommateurs terminée ---\n");
fflush(stdout);
	
	
		struct args* arguments=(struct args*) malloc(sizeof(struct args));
		if(arguments==NULL){
			goto end;
		}
		arguments->doneFlag=doneFlag;
		arguments->flagOut=flagOut;
		arguments->optionD=optionD;
		arguments->bufout_arg=bufout;
		printf("---CREATION D'UN WRITER---\n");
fflush(stdout);
		pthread_create(&(writ[0]), NULL, (void*) &writer, (void*) arguments);
	
	
	printf("--- Initialisation des writers terminée ---\n");
fflush(stdout);
	*/
	printf("\n doneFlag = %d \n\n",*doneFlag);
fflush(stdout);
	
	while((*doneFlag)>0){
		//printf("\n DONEFLAG=%d \n\n",*doneFlag);
//fflush(stdout);
	}
	
	printf("--- Fin des threads ---\n");
fflush(stdout);
	
	end :
	
	sbuf_clean(buf);
	sbuf_clean(bufout);
	printf("--- Buffers clean ---\n");
fflush(stdout);
	free(flag);
	free(flagOut);
	free(doneFlag);
	printf("--- Flags free ---\n");
fflush(stdout);
	
	printf("--- Fin du programme ---\n");
fflush(stdout);
	
	return 0;
}
