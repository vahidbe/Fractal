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
int fractCount=0;

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
    struct fractal** buf;          /* Buffer partagé */
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
		exit(-1); //TODO: gérer erreurs
	}
	printf("--- Debut de l'initialisation du buffer ---\n");
    sp->buf = malloc(n*sizeof(struct fractal*));
	printf("--- Malloc d'initialisation de buffer terminé ---\n");
    sp->n = n;                       /* Buffer content les entiers */
    sp->front = sp->rear = 0;        /* Buffer vide si front == rear */
    sem_init(&sp->mutex, 0, 1);      /* Exclusion mutuelle */
    sem_init(&sp->slots, 0, n);      /* Au début, n slots vides */
    sem_init(&sp->items, 0, 0);      /* Au début, rien à consommer */
	printf("--- Initialisation du buffer terminée ---\n");
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
	printf("--- DEBUT PRODUCTEUR ---\n");
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
      if(fclose(file)!=0)
	{
	  (*doneFlag)--;(*flag)--;
		return (NULL);
	}
	(*doneFlag)--;(*flag)--;
	return (NULL);
    }	
	printf("--- Fin malloc producteur ---\n");
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
	  printf("=== Fractale lue : %s %d %d %lf %lf ===\n",name,*buf2,*buf3,*buf4,*buf5);
	  printf("*INSERT DU PRODUCTEUR*\n");
	sbuf_insert(buf,fractal_new(name,*buf2,*buf3,*buf4,*buf5));
	fractCount++;
	printf("*INSERT DU PRODUCTEUR TERMINE*\n");
	x=fscanf(file,"%64s",buf1);
	  }
      }
  }
  printf("--- Fin producteur ---\n");
  (*doneFlag)--;
  (*flag)--;
  printf("\nPRODFLAG=%d\n\n",*flag);
  return NULL;
}

void *consumer(void* arguments){
	printf("--- DEBUT CONSOMMATEUR ---\n");
	int done=0;
	struct args* argument=(struct args*) arguments;
	struct sbuf* buf=argument->buf_arg;
	struct sbuf* bufout=argument->bufout_arg;
	int* flag=argument->flag;
	int* flagOut=argument->flagOut;
	int* doneFlag=argument->doneFlag;
	free(argument);
	printf("--- Debut calcul consommateur ---\n");
	while(!done){		
		int ic=0;
		printf("va lire sem_getvalue du consommateur\n");
		sem_getvalue(&(buf->items),&ic);
		printf("FLAG=%d\n",*flag);
		if(((*flag)<=0)&(ic==0))
		{
			printf("=====DONE=1=====\n");
			done=1;
		}
		else{
		printf("*REMOVE DU CONSOMMATEUR*\n");
		struct fractal* f=sbuf_remove(buf);
		printf("*REMOVE DU CONSOMMATEUR TERMINE*\n");
		int i;
		int j;
		for(i=0;i<f->width;i++){
			for(j=0;j<f->height;j++){
				fractal_set_value(f,i,j,fractal_compute_value(f,i,j));
			}
		}
		printf("*INSERT DU CONSOMMATEUR*\n");
		sbuf_insert(bufout,f);	
		printf("*INSERT DU CONSOMMATEUR TERMINE*\n");	
		}
	}
	printf("--- Fin consommateur ---\n");
	(*flagOut)--;
	(*doneFlag)--;
	return NULL;
}

void *writer(void* arguments){
	printf("--- DEBUT WRITER ---\n");
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
	printf("--- Debut ecriture writer ---\n");
	if(!optionD){
		printf("===OPTIOND-0===\n");
		while(!isEmpty){
			//int* ic;
			//printf("va lire sem_getvalue du writer\n");
			//sem_getvalue(&(buf->items),ic);
			printf("\n FLAGOUT=%d\n\n",*flagOut);
			//if(((*flagOut)<=0)&(*ic==0)){
			if(((*flagOut)<=0)&(fractCount<=0)){
				isEmpty=1;
			}
			else{
				printf("*REMOVE DU WRITER*\n");
				struct fractal* f = (struct fractal*) sbuf_remove(buf);
				printf("*REMOVE DU WRITER TERMINE*\n");
				double newAverage = fractal_compute_average(f);
				if(newAverage>average){
					average=newAverage;
					highestF=f;
				}
				fractCount--;
			}
			
		}
		
		printf("=== ECRITURE ===\n");
		write_bitmap_sdl(highestF,fileOutName);
		printf("=== FIN ECRITURE ===\n");
	}
	else{
		while(!isEmpty){
			printf("===OPTIOND-1===\n");
			int* ic;
			sem_getvalue(&(buf->items),ic);
			if(((*flagOut)<=0)&(*ic==0)){
				isEmpty=1;
			}
			else{
				struct fractal* f = (struct fractal*) sbuf_remove(buf);
				write_bitmap_sdl(f,fractal_get_name(f));
			}
		}
	}
	printf("--- Fin writer ---\n");
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

	if((*argv[1]=='-')&(*(argv[1]+1)=='d')){
		optionD=1;
		optionsCount++;
		if(argc>2){
		if((*argv[2]=='-')&(*(argv[2]+1)=='-')){
			numberThreads = *(argv[2]+2);
			optionsCount++;
		}}
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
	
	printf("--- Lecture des options terminée ---\n");
	printf("\n Nombre de threads qui vont être utilisés : %d \n \n",numberThreads);
	
	sbuf_init(buf, (numberThreads+3));            
	sbuf_init(bufout, (numberThreads+3));    

	printf("--- Initialisation des buffers terminée ---\n");
	
	pthread_t prod[argc-2-optionsCount];
	pthread_t cons[numberThreads];
	pthread_t writ[argc-2-optionsCount];
	
	printf("--- Initialisation des tableaux de pthread_t terminée ---\n");
	
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
	
	for(count=optionsCount+1;count<argc;count++){
	  if(((*argv[count])=='-')&(count!=(argc))){      
	  //Entree standard
	  char* chaine;
			fgets(chaine, sizeof(chaine), stdin);
			FILE* file;
			file = fopen("FractalEntree.txt","r");
			if(file==NULL)
			  {
			    goto end;
			  }
			fputs(file,chaine);
			fclose(file);
			if(count!=(argc-1)){
				//TODO: ne pas oublier les free
				struct args* arguments=(struct args*) malloc(sizeof(struct args));
				if(arguments==NULL){
					goto end;
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
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, (void*) arguments);
			}
			else{
				fileOutName=argv[count];
			}	
		}
	}
	
	printf("--- Initialisation des producteurs terminée ---\n");
	
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
		pthread_create(&(cons[i]), NULL, (void*) &consumer, (void*) arguments);
	}
	
	printf("--- Initialisation des consommateurs terminée ---\n");
	
	//TODO: faire plein de writers qui comparent avec sémaphore la fractale la plus haute
	if(!optionD){
		struct args* arguments=(struct args*) malloc(sizeof(struct args));
		if(arguments==NULL){
			goto end;
		}
		arguments->doneFlag=doneFlag;
		arguments->flagOut=flagOut;
		arguments->optionD=optionD;
		arguments->bufout_arg=bufout;
		printf("---CREATION D'UN WRITER---\n");
		pthread_create(&(writ[0]), NULL, (void*) &writer, (void*) arguments);
	}
	else{
		int i;
		for(i=0;i<(argc-2-optionsCount);i++){
			struct args* arguments=(struct args*) malloc(sizeof(struct args));
			if(arguments==NULL){
				goto end;
			}
			arguments->doneFlag=doneFlag;
			arguments->flagOut=flagOut;
			arguments->optionD=optionD;
			arguments->bufout_arg=bufout;
			printf("---CREATION D'UN WRITER---\n");
			pthread_create(&(writ[i]), NULL, (void*) &writer, (void*) bufout);
		}
	}
	
	printf("--- Initialisation des writers terminée ---\n");
	
	printf("\n doneFlag = %d \n\n",*doneFlag);
	
	while((*doneFlag)>0){
		//printf("\n DONEFLAG=%d \n\n",*doneFlag);
	}
	
	printf("--- Fin des threads ---\n");
	
	end :
	
	sbuf_clean(buf);
	sbuf_clean(bufout);
	printf("--- Buffers clean ---\n");
	free(flag);
	free(flagOut);
	free(doneFlag);
	printf("--- Flags free ---\n");
	
	printf("--- Fin du programme ---\n");
	
	return 0;
}
