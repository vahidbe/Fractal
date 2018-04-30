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
    sp->buf =(struct fractal**) calloc(n, sizeof(struct fractal));
	if(((sp->buf)==NULL)){
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
	struct args* argument=(struct args*) arguments;
	char* fileName=argument->charP_arg;
	struct sbuf* buf=argument->buf_arg;
	int* flag=argument->flag;
	int* doneFlag=argument->doneFlag;
	free(argument);
  FILE* file;
  int x;
  int done=0;
  file=fopen(fileName,"r");
  if(file==NULL)
    {
		(*doneFlag)--;(*flag)--;
		return (NULL);
    }
  char* buf1 = (char*) malloc(sizeof(char));
  int* buf2 = (int*) malloc(sizeof(int));
  int* buf3 = (int*) malloc(sizeof(int));
  double* buf4 = (double*) malloc(sizeof(double));
  double* buf5 = (double*) malloc(sizeof(double));
  char bufName[64];
  if((buf1==NULL)|(buf2==NULL)|(buf3==NULL)|(buf4==NULL)|(buf5==NULL))
    {
      printf("Erreur malloc\n");
      if(fclose(file)!=0)
	{
	  printf("Erreur close\n");
	  (*doneFlag)--;(*flag)--;
		return (NULL);
	}
	(*doneFlag)--;(*flag)--;
	return (NULL);
    }	
  int i;
  x=fscanf(file,"%64s",buf1);
  for(i=0;(!done);i++){
    if(x==EOF)
      {
	if(fclose(file)!=0)
	  {
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
		(*doneFlag)--;(*flag)--;
		return (NULL);
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
	if(buf1[0]=='#')
	  {
	    char trash[1024];
	    fgets(trash,1024,file);
	    x=fscanf(file,"%64s",buf1);
	  }
	else
	  {
	    char* name=buf1;
	    x=fscanf(file,"%d",buf2);
	    if(x==EOF)
	      {
		//TODO: gérer les erreurs/la sortie
	    if(fclose(file)!=0)
			(*doneFlag)--;(*flag)--;
			return (NULL);
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
		(*doneFlag)--;(*flag)--;
		return (NULL);
	      }
	    x=fscanf(file,"%d",buf3);
	if(x==EOF)
	  {
	    //TODO: gérer les erreurs/la sortie
	    if(fclose(file)!=0)
			(*doneFlag)--;(*flag)--;
			return (NULL);
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
		(*doneFlag)--;(*flag)--;
		return (NULL);
	  }
	x=fscanf(file,"%lf",buf4);
	if(x==EOF)
	  {
	    //TODO: gérer les erreurs/la sortie
	    if(fclose(file)!=0)
			(*doneFlag)--;(*flag)--;
			return (NULL);
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
		(*doneFlag)--;(*flag)--;
		return (NULL);
	  }
	x=fscanf(file,"%lf",buf5);
	if(x==EOF)
	  {
	    if(fclose(file)!=0)
			(*doneFlag)--;(*flag)--;
			return (NULL);
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
		(*doneFlag)--;(*flag)--;
		return (NULL);
	  }
	sbuf_insert(buf,fractal_new(name,*buf2,*buf3,*buf4,*buf5));
	x=fscanf(file,"%64s",buf1);
	  }
      }
  }
  printf("End of file");
  (*doneFlag)--;
  (*flag)--;
  return NULL;
}

void *consumer(void* arguments){
	int done=0;
	struct args* argument=(struct args*) arguments;
	struct sbuf* buf=argument->buf_arg;
	struct sbuf* bufout=argument->bufout_arg;
	int* flag=argument->flag;
	int* flagOut=argument->flagOut;
	int* doneFlag=argument->doneFlag;
	free(argument);
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
		if(((*flag)<=0)&((buf->items)==0))
		{
			done=1;
		}
	}
	(*flagOut)--;
	(*doneFlag)--;
	return NULL;
}

void *writer(void* arguments){
	int isEmpty=0;
	struct args* argument=(struct args*) arguments;
	struct sbuf* buf=argument->bufout_arg;
	int optionD=argument->optionD;
	char* fileOutName=argument->fileOutName;
	double average;
	struct fractal* highestF;
	int* flagOut=argument->flagOut;
	int* doneFlag=argument->doneFlag;
	free(argument);
	if(!optionD){
		while(!isEmpty){
			struct fractal* f = (struct fractal*) sbuf_remove(buf);
			double newAverage = fractal_compute_average(f);
			if(newAverage>average){
				average=newAverage;
				highestF=f;
			}
		}
		if(((*flagOut)<=0)&((buf->items)==0){
			isEmpty=1;
		}
		write_bitmap_sdl(highestF,fileOutName);
	}
	else{
		while(!isEmpty){
			struct fractal* f = (struct fractal*) sbuf_remove(buf);
			write_bitmap_sdl(f,fractal_get_name(f));	
			if(((*flagOut)<=0)&((buf->items)==0){
				isEmpty=1;
			}
		}
	}
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
	struct sbuf* buf;
	struct sbuf* bufout;
	int* flag=(int*) malloc(sizeof(int));
	int* flagOut=(int*) malloc(sizeof(int));
	int* doneFlag=(int*) malloc(sizeof(int));

	if((*argv[1]=='-')&(*(argv[1]+1)=='d')){
		optionD=1;
		optionsCount++;
		if((*argv[2]=='-')&(*(argv[2]+1)=='-')){
			numberThreads = *(argv[2]+2);
			optionsCount++;
		}
	}
	else{
		if((*argv[1]=='-')&(*(argv[1]+1)=='-')){
			numberThreads = *(argv[1]+2);
			optionsCount++;
		}
	}	
	if(numberThreads==0){
	        numberThreads=argc-2-optionsCount;  //Vraiment utile de retirer optionsCount ? Il sera d'office nul --> il est pas d'office nul, regarde le if juste au dessus
	}
	sbuf_init(buf, (numberThreads));            
	sbuf_init(bufout, (numberThreads));       
	pthread_t prod[argc-2-optionsCount];
	pthread_t cons[numberThreads];
	pthread_t writ[argc-2-optionsCount];
	
	flagConst=argc-2-optionsCount;
	flagOutConst=numberThreads;
	if(optionD){
		doneFlagConst=2*(argc-2-optionsCount)+numberThreads;
	}
	else{
		doneFlagConst=(argc-2-optionsCount)+numberThreads+1;
	}
	
	for(count=optionsCount+1;count<argc;count++){
	  if(((*argv[count])=='-')&(count!=(argc))){      
	  //Entree standard
	  char* chaine;
			fgets(chaine, sizeof(chaine), stdin);
			FILE* file;
			file = fopen("FractalEntree.txt",r+);
			if(file==NULL)
			  {
			    sbuf_clean(buf);sbuf_clean(bufOut);thread_kill(flag,flagOut);goto end;
			  }
			fputs(file,chaine);
			fclose(file);
			if(count!=(argc)){
				//TODO: ne pas oublier les free
				struct args* arguments=(struct args*) malloc(sizeof(struct args));
				if(arguments==NULL){
					sbuf_clean(buf);sbuf_clean(bufOut);thread_kill(flag,flagOut);goto end;
				}
				arguments->buf_arg=buf;
				//ATTENTION GERER LES FLAGS ATTENTION
				arguments->charP_arg="FractalEntree.txt";
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, (void*) arguments);
			}
			else{
				//TODO: gérer sortie
				fileOutName="FractalEntree.txt";
			}	
			
		}




	  else{
			if(count!=(argc)){
				//TODO: ne pas oublier les free
				struct args* arguments=(struct args*) malloc(sizeof(struct args));
				if(arguments==NULL){
					sbuf_clean(buf);sbuf_clean(bufOut);thread_kill(flag,flagOut);goto end;
				}
				arguments->buf_arg=buf;
				arguments->flag=flag;
				arguments->doneFlag=doneFlag;
				*(arguments->doneFlag)=doneFlagConst;
				*(arguments->flag)=flagConst;
				arguments->charP_arg=argv[count];
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, (void*) arguments);
			}
			else{
				fileOutName=argv[count];
			}	
		}
	}
	int i;
	for(i=0;(i<numberThreads);i++){
		//TODO: ne pas oublier les free
		struct args* arguments=(struct args*) malloc(sizeof(struct args));
		if(arguments==NULL){
			sbuf_clean(buf);sbuf_clean(bufOut);thread_kill(flag,flagOut);goto end;
		}
		arguments->flag=flag;
		*(arguments->flag)=flagConst;
		arguments->doneFlag=doneFlag;
		*(arguments->doneFlag)=doneFlagConst;
		arguments->flagOut=flagOut;
		*(arguments->flagOut)=flagOutConst;
		arguments->buf_arg=buf;
		arguments->bufout_arg=bufout;
		pthread_create(&(cons[i]), NULL, (void*) &consumer, (void*) arguments);
	}
	//TODO: faire plein de writers qui comparent avec sémaphore la fractale la plus haute
	if(!optionD){
		//TODO: ne pas oublier les free
		struct args* arguments=(struct args*) malloc(sizeof(struct args));
		if(arguments==NULL){
			sbuf_clean(buf);sbuf_clean(bufOut);thread_kill(flag,flagOut);goto end;
		}
		arguments->doneFlag=doneFlag;
		*(arguments->doneFlag)=doneFlagConst;
		arguments->flagOut=flagOut;
		*(arguments->flagOut)=flagOutConst;
		arguments->optionD=optionD;
		arguments->bufout_arg=bufout;
		pthread_create(&(writ[0]), NULL, (void*) &writer, (void*) arguments);
	}
	else{
		int i;
		for(i=0;i<(argc-2-optionsCount);i++){
			//TODO: ne pas oublier les free
			struct args* arguments=(struct args*) malloc(sizeof(struct args));
			if(arguments==NULL){
				sbuf_clean(buf);sbuf_clean(bufOut);thread_kill(flag,flagOut);goto end;
			}
			arguments->doneFlag=doneFlag;
			*(arguments->doneFlag)=doneFlagConst;
			arguments->flagOut=flagOut;
			*(arguments->flagOut)=flagOutConst;
			arguments->optionD=optionD;
			arguments->bufout_arg=bufout;
			pthread_create(&(writ[i]), NULL, (void*) &writer, (void*) bufout);
		}
	}
	end : 
	while((*doneFlag)!=0)
	{}
	sbuf_clean(buf);
	sbuf_clean(bufOut);
	free(flag);
	free(flagOut);
	free(doneFlag);
	return 0;
}

void thread_kill(int* flag, int* flagOut)
{
	*flag=0;
	*flagOut=0;
	return NULL;
}