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

int flagB1;
int flagB2;
int flagDone;

int numberProd;
int numberThreads;
int optionD;
int optionM=1;
char* fileOutName;
int lengthI=0;
int lengthO=0;

int isEmpty=0;
int done=0;

pthread_mutex_t mutexProd;
pthread_mutex_t mutexCons;
pthread_mutex_t mutexWrit;

struct sbuf* bufIn;
struct sbuf* bufOut;

struct args{
	char* charP_arg;
};	

struct sbuf{
    struct fractal** buf;           /* Buffer partagé */
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
    sp->buf = (struct fractal**) calloc(n, sizeof(struct fractal*));
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
void sbuf_insert(struct sbuf *sp, struct fractal* res)
{
	sem_wait(&(sp->slots));
	sem_wait(&(sp->mutex));
	sp->rear=((sp->rear)+1)%(sp->n);
	sp->buf[sp->rear]=res;	
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
	struct fractal* res=(sp->buf[sp->front]);
	sem_post(&(sp->mutex));
	sem_post(&(sp->slots));
	return res;
}

void *producer(void* arguments){	
	struct args* argument=(struct args*) arguments;
	char* fileName=argument->charP_arg;
	free(argument);
	FILE* file;
	int x;
	int done=0;
	file=fopen(fileName,"r");
	if(file==NULL)
    {
		if(numberProd==0)
		{
			fprintf(stderr, "%s", "You did not enter a file!\n");
			exit(-1);
		}
		if(numberProd==1)
		{
			fprintf(stderr, "%s", "This file does not exist!\n");
			exit(-1);
		}
		else
		{
			fprintf(stderr, "%s", "One of your files does not exist!\n");
			exit(-1);
		}		
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
		(flagDone)--;(flagB1)--;
		return (NULL);
	}
	(flagDone)--;(flagB1)--;
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
			(flagDone)--;(flagB1)--;
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
				{
					(flagDone)--;(flagB1)--;
					return (NULL);
				}
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				(flagDone)--;(flagB1)--;
				return (NULL);
			}
			x=fscanf(file,"%d",buf3);
			if(x==EOF)
			{
				//TODO: gérer les erreurs/la sortie
				if(fclose(file)!=0)
				{
					(flagDone)--;(flagB1)--;
					return (NULL);
				}
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				(flagDone)--;(flagB1)--;
				return (NULL);
			}
			x=fscanf(file,"%lf",buf4);
			if(x==EOF)
			{
				//TODO: gérer les erreurs/la sortie
				if(fclose(file)!=0)
				{
					(flagDone)--;(flagB1)--;
					return (NULL);
				}
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				(flagDone)--;(flagB1)--;
				return (NULL);
			}
			x=fscanf(file,"%lf",buf5);
			if(x==EOF)
			{
				if(fclose(file)!=0)
				{
					(flagDone)--;(flagB1)--;
					return (NULL);
				}
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				(flagDone)--;(flagB1)--;
				return (NULL);
			}
			struct fractal* f = (struct fractal*)malloc(sizeof(struct fractal));
			f = fractal_new(name,*buf2,*buf3,*buf4,*buf5);
			sbuf_insert(bufIn,f);
			x=fscanf(file,"%64s",buf1);
			}
		}
	}
	(flagDone)--;
	(flagB1)--;	
	return NULL;
}

void *consumer(void* arguments){	
	pthread_mutex_lock(&mutexCons);
	int ic=0;
	sem_getvalue(&(bufIn->items),&ic);
	
	if(((flagB1)<=0)&(ic==0))
	{
		done=1;
	}
	pthread_mutex_unlock(&mutexCons);
	while(!done)
	{		
		pthread_mutex_lock(&mutexCons);
		int ic=0;
		sem_getvalue(&(bufIn->items),&ic);
		printf("%d ",ic);
		printf("%d\n",flagB1);
		if(((flagB1)<=0)&(ic==0))
		{
			done=1;
		}
		else
		{
			struct fractal* f=(sbuf_remove(bufIn));
			pthread_mutex_unlock(&mutexCons);
			int i;
			int j;
			for(i=0;i<f->width;i++){
				for(j=0;j<f->height;j++)
				{
					fractal_set_value(f,i,j,fractal_compute_value(f,i,j));
				}
			}
			printf("debut insert cons\n");
			sbuf_insert(bufOut,f);	
			printf("fin insert cons\n");
		}
	}
	(flagB2)--;
	(flagDone)--;
	pthread_mutex_unlock(&mutexCons);
	printf("cons FIN\n");
	return NULL;
}

void *writer(void* arguments){
	double average;
	struct fractal* highestF=malloc(sizeof(struct fractal));
	if(!optionD){
		while(!isEmpty){			
			pthread_mutex_lock(&mutexWrit);
			int ic;
			sem_getvalue(&(bufOut->items),&ic);
			if(((flagB2)<=0)&(ic==0))
			{
				isEmpty=1;
			}
			else{
				struct fractal* f = (sbuf_remove(bufOut));
				pthread_mutex_unlock(&mutexWrit);
				double newAverage = fractal_compute_average(f);
				if(newAverage>average)
				{
					average=newAverage;
					*highestF=*f;
				}				
				fractal_free(f);		
				//pthread_mutex_unlock(&mutexWrit);
			}
		}
		char* fileOut=strcat(fileOutName,".bmp");
		write_bitmap_sdl(highestF,fileOut);
	}
	else
	{
		while(!isEmpty)
		{
			pthread_mutex_lock(&mutexWrit);
			int ic=0;
			sem_getvalue(&(bufOut->items),&ic);			
			printf("W%d ",ic);
			printf("W%d\n",flagB2);
			if(((flagB2)<=0)&(ic==0))
			{
				isEmpty=1;
			}
			else
			{
				printf("Debut remove writer\n");
				struct fractal* f = (sbuf_remove(bufOut));
				printf("Fin remove writer\n");
				pthread_mutex_unlock(&mutexWrit);
				char* fileOut=strcat(fractal_get_name(f),".bmp");
				write_bitmap_sdl(f,fileOut);
				fractal_free(f);
			}	
		}
	}	
	free(highestF);
	(flagDone)--;
	printf("flagDone = %d \n",flagDone);
	printf("writer FIN\n");
	pthread_mutex_unlock(&mutexWrit);
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_mutex_init(&mutexProd,NULL);
	pthread_mutex_init(&mutexCons,NULL);
	pthread_mutex_init(&mutexWrit,NULL);
	numberThreads=0;
	int count;
	int optionsCount=0;
	optionD=0;
	bufIn=(malloc(sizeof(struct sbuf)));
	bufOut=(malloc(sizeof(struct sbuf)));

	if((*argv[1]=='-')&(*(argv[1]+1)=='d')){
		optionD=1;
		optionsCount++;
		if(argc>2){
		if((*argv[2]=='-')&(*(argv[2]+1)=='-')){
			numberThreads = atoi(argv[3]);
			optionsCount++;
			optionsCount++;
		}}
	}
	else{
		if((*argv[1]=='-')&(*(argv[1]+1)=='-')){
			numberThreads = atoi(argv[2]);
			optionsCount++;
			optionsCount++;
		}
	}	
	if(numberThreads==0){
		numberThreads=argc-2-optionsCount; 
		optionM=0;
	}
	
	char* optionDs;
	if(optionD)
	{
		optionDs="OUI";
	}
	else 
	{
		optionDs="NON";
	}
	/**/printf("--- Option d : %s ---\n",optionDs);
	/**/fflush(stdout);
	/**/printf("--- Nombre de threads qui vont être utilisés : %d ---\n",numberThreads);
	/**/fflush(stdout);
	
	sbuf_init(bufIn, (10));            
	sbuf_init(bufOut, (10));    
	
	numberProd=argc-2-optionsCount;
	printf("argc = %d\n",argc);
	printf("OptionCout = %d\n",optionsCount);
	if(numberProd==0)
	{
		fprintf(stderr, "%s", "You did not enter a file!\n");
		exit(-1);
	}
	
	pthread_t prod[numberProd];
	pthread_t cons[numberThreads];
	pthread_t writ[numberProd];
	
	flagB1=argc-2-optionsCount;
	flagB2=numberThreads;
	if(optionD)
	{
		flagDone=2*(numberProd)+numberThreads;
	}
	else
	{
		flagDone=(numberProd)+numberThreads+1;
	}
	/**/printf("--- Initialisation du main terminée ---\n");
	/**/fflush(stdout);
	
	for(count=optionsCount+1;count<argc;count++)
	{
		if(((*argv[count])=='-')&(count!=(argc)))
		{ 
/*	
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
			if(count!=(argc-1))
			{
				//TODO: ne pas oublier les free
				struct args* arguments=(struct args*) malloc(sizeof(struct args));
				if(arguments==NULL)
				{
					goto end;
				}
				arguments->buf_arg=buf;
				//ATTENTION GERER LES FLAGS ATTENTION
				arguments->charP_arg="FractalEntree.txt";
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, (void*) arguments);
			}
			else
			{
				//TODO: gérer sortie
				fileOutName="FractalEntree.txt";
			}	
*/			
		}
		else
		{
			if(count!=(argc-1))
			{
				struct args* arguments=malloc(sizeof(struct args));
				if(arguments==NULL)
				{
					goto end;
				}
				arguments->charP_arg=argv[count];
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, arguments);
			}
			else
			{
				fileOutName=argv[count];  //Demande verification, faut pas une etoile ?
			}	
		}
	}
	
	/**/printf("--- Initialisation des producteurs terminée ---\n");
	/**/fflush(stdout);
	
	int i;
	for(i=0;(i<numberThreads);i++)
	{
		pthread_create(&(cons[i]), NULL, (void*) &consumer, NULL);
	}
	
	/**/printf("--- Initialisation des consommateurs terminée ---\n");
	/**/fflush(stdout);
	
	//TODO: faire plein de writers qui comparent avec sémaphore la fractale la plus haute
	if(!optionD)
	{
		pthread_create(&(writ[0]), NULL, (void*) &writer, NULL);
	}
	else
	{
		int i;
		for(i=0;i<(argc-2-optionsCount);i++)
		{
			pthread_create(&(writ[i]), NULL, (void*) &writer, NULL);
		}
	}	
	/**/printf("--- Initialisation des writers terminée ---\n");
	/**/fflush(stdout);
	/**/printf("=== Initialisation terminée ===\n");
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
	
	
	/**/printf("=== Fin du programme ===\n");
	/**/fflush(stdout);
	
	return 0;
}
