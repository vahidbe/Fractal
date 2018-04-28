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
//TODO: gérer l'exit des threads (faut-il une variable qui vérifie si les étapes sont terminées?)

struct args{
	char* charP_arg;
	struct sbuf* buf_arg;
	struct sbuf* bufout_arg;
	int optionD;
	char* fileOutName;
};	

struct sbuf{
    struct fractal* *buf;          /* Buffer partagé */
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
    sp->buf =(struct fractal*) calloc(n, sizeof(struct fractal));
	if((buf==NULL)){
		exit(-1); //TODO: gérer erreurs
	}
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
	int fdi=0;
	int done = 0;
	struct args* argument=(struct args*) arguments;
	char* fileName=argument->charP_arg;
	struct sbuf* buf=argument->buf_arg;
	free(args);
	if(open(fileName,fdi)<0)
		exit(-1);
		//TODO: gérer les erreurs/la sortie
	char* buf1 = (char*) malloc(sizeof(char));
	int* buf2 = (int*) malloc(sizeof(int));
	int* buf3 = (int*) malloc(sizeof(int));
	double* buf4 = (double*) malloc(sizeof(double));
	double* buf5 = (double*) malloc(sizeof(double));
	char bufName[64];
	if((buf1==NULL)|(buf2==NULL)|(buf3==NULL)|(buf4==NULL)|(buf5==NULL))
	{
		if(close(fdi)!=0)
			exit(-1);		
		//TODO: gérer les erreurs/la sortie
		exit(-1);
	}
	ssize_t x=0;
	int i;
	int j;
	x=read(fdi,buf1,sizeof(char));
	for(i=0;!done;i++){
		if(x!=sizeof(char))
		{
			if(close(fdi)!=0)
			{
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				exit(-1);
			}
			free(buf1);
			free(buf2);
			free(buf3);
			free(buf4);
			free(buf5);
			done=1;
		}
		else
		{
			bufName[0]=*buf1;
			j=1;
			for(x=read(fdi,buf1,sizeof(char));(*buf1)!=' ';x=read(fdi,buf1,sizeof(char)))
			{
				if(x==-1)
				{
					//TODO: gérer les erreurs/la sortie
					if(close(fdi)!=0)
						exit(-1);
					free(buf1);
					free(buf2);
					free(buf3);
					free(buf4);
					free(buf5);
					exit(-1);
				}
				bufName[j]=*buf1;
				j++;
			}
			char name[j];
			int k;
			for(k=0;k<j;k++)
			{
				name[k]=bufName[k];
			}
			x=read(fdi,buf2,sizeof(int));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					exit(-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				exit(-1);
			}
			x=read(fdi,buf3,sizeof(int));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					exit(-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				exit(-1);
			}
			x=read(fdi,buf4,sizeof(double));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					exit(-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				exit(-1);
			}
			x=read(fdi,buf5,sizeof(double));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					exit(-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				exit(-1);
			}
			sbuf_insert(buf,fractal_new(name,*buf2,*buf3,*buf4,*buf5));
		}
	}
		//TODO: gérer quand lecture terminée
	//TODO: exit? thread_exit? buf_free? etc
}

void *consumer(void* arguments){
	int done=0;
	struct args* argument=(struct args*) arguments;
	struct sbuf* buf=argument->buf_arg;
	struct sbuf* bufout=argument->bufout_arg;
	free(args);
	while(!done){
		struct fractal* f=sbuf_remove(buf);
		int i;
		int j;
		for(i=0;i<f->width;i++){
			for(j=0;j<f->height;j++){
				fractal_set_value(f,i,j,fractal_compute_value(f,i,j));
			}
		}
		sbuf_insert(bufout,f);		
		//TODO: gérer quand lecture terminée
	}
	//TODO: exit? thread_exit? buf_free? etc
}

void *writer(void* arguments){
	int isEmpty=0;
	struct args* argument=(struct args*) arguments;
	struct sbuf* buf=argument->bufout_arg;
	int optionD=argument->optionD;
	char* fileOutName=argument->fileOutName;
	double average;
	struct fractal* highestF;
	free(argument);
	if(!optionD){
		while(!isEmpty){
			struct fractal* f = (struct fractal*) sbuf_remove(buf);
			double newAverage = compute_average(f->values);
			if(newAverage>average){
				average=newAverage;
				highestF=f;
			}
		}
		write_bitmap_sdl(highestF,fileOutName);
		//TODO: exit? thread_exit? buf_free? etc
	}
	else{
		while(!isEmpty){
			struct fractal* f = (struct fractal*) sbuf_remove(buf);
			//TODO: où écrire le bitmap? quel nom?
			//TODO: besoin de semaphore? les writers peuvent-ils ecrire en meme temps?
			write_bitmap_sdl(f,fractal_get_name(f));	
		}			
}

int main(int argc, char *argv[])
{	
	//TODO: gérer la lecture des options -d et --maxthreads
	int numberThreads=0;
	int count;
	int optionsCount=0;
	int optionD=0;
	char* fileOutName;	
	struct sbuf* buf;
	struct sbuf* bufout;

	if((*argv[0]=='-')&(*(argv[0]+1)=='d'){
		optionD=1;
		optionsCount++;
		if((*argv[1]=='-')&(*(argv[1]+1)=='-'){
			numberThreads = *(argv[1]+2);
			optionsCount++;
		}
	}
	else{
		if(*argv[0]=='-')&(*(argv[0]+1)=='-'){
			numberThreads = *(argv[0]+2);
			optionsCount++;
		}
	}	
	if(numberThreads==0){
		numberThreads=argc-2-optionsCount;
	}
	sbuf_init(buf, (numberThreads));
	sbuf_init(bufout, (numberThreads));
	pthread_t prod[argc-2-optionsCount];
	pthread_t cons[numberThreads];
	pthread_t writ[argc-2-optionsCount];
	
	for(count=optionsCount;count<argc-1;count++){
		if(((*argv[count])=='-')&(count!=(argc-1)){
			//TODO: gérer l'entrée standard
		}
		else{
			if(count!=(argc-1)){
				//TODO: ne pas oublier les free
				struct arg* arguments=(struct arg*) malloc(sizeof(struct arg));
				if(arg==NULL){
					exit(-1); //TODO: gérer les erreurs et la fermeture des threads
				}
				arguments->buf_arg=buf;
				arguments->charP_arg=argv[count];
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, (void*) arguments);
			}
			else{
				//TODO: gérer sortie
				fileOutName=argv[count];
			}	
		}
	}
	int i;
	for(i=0;(i<numberThreads);i++){
		//TODO: ne pas oublier les free
		struct arg* arguments=(struct arg*) malloc(sizeof(struct arg));
		if(arg==NULL){
			exit(-1); //TODO: gérer les erreurs et la fermeture des threads
		}
		arguments->buf_arg=buf;
		arguments->bufout_arg=bufout;
		pthread_create(&(cons[i]), NULL, (void*) &consumer, (void*) arguments);
	
	//TODO: faire plein de writers qui comparent avec sémaphore la fractale la plus haute
	if(!optionD){
		//TODO: ne pas oublier les free
		struct arg* arguments=(struct arg*) malloc(sizeof(struct arg));
		if(arg==NULL){
			exit(-1); //TODO: gérer les erreurs et la fermeture des threads
		}
		arguments->optionD=optionD;
		arguments->bufout_arg=bufout;
		pthread_create(&(writ[0], NULL, (void*) &writer, (void*) arguments);
	}
	else{
		int i;
		for(i=0;i<(argc-2-optionsCount);i++){
			//TODO: ne pas oublier les free
			struct arg* arguments=(struct arg*) malloc(sizeof(struct arg));
			if(arg==NULL){
				exit(-1); //TODO: gérer les erreurs et la fermeture des threads
			}
			arguments->optionD=optionD;
			arguments->bufout_arg=bufout;
			pthread_create(&(writ[i], NULL, (void*) &writer, (void*) bufout);
		}
	}
	
	//TODO: gérer la destuction des threads et des buffers
	//pthread_exit(NULL);
}