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

int numberThreads;
int optionD;
char* fileOutName;
int lengthI=0;
int lengthO=0;

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
		printf("Ini - --- EXIT ---\n");
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
	///**/int ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d sur %d\n",ic,sp->n);
	sem_wait(&(sp->slots));
	sem_wait(&(sp->mutex));
	//printf("Ins - Fractale to insert : %s, %d, %d, %f, %f\n",res->name,fractal_get_width(res),fractal_get_height(res), fractal_get_a(res), fractal_get_b(res));
	//fflush(stdout);
	sp->rear=((sp->rear)+1)%(sp->n);
	sp->buf[sp->rear]=res;	
	//printf("Ins - Number Rear : %d\n",sp->rear);
	//fflush(stdout);
	sem_post(&(sp->mutex));
	sem_post(&(sp->items));
	///**/ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d\n",ic);
}

/* @pre sbuf!=NULL
 * @post retire le dernier item du buffer partagé
 */
struct fractal* sbuf_remove(struct sbuf *sp)
{	
	///**/int ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d\n",ic);
	sem_wait(&(sp->items));
	sem_wait(&(sp->mutex));
	sp->front=((sp->front)+1)%(sp->n);
	//printf("R - Number Front: %d\n",sp->front);
	//fflush(stdout);
	struct fractal* res=(sp->buf[sp->front]);	
	//printf("R - Fractale to remove : %s, %d, %d, %f, %f\n",fractal_get_name(res),fractal_get_width(res),fractal_get_height(res), fractal_get_a(res), fractal_get_b(res));
	//fflush(stdout);
	sem_post(&(sp->mutex));
	sem_post(&(sp->slots));
	///**/ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d\n",ic);
	return res;
}

void *producer(void* arguments){	
	/**/printf("P - --- DEBUT PRODUCTEUR ---\n");
	/**/fflush(stdout);
	struct args* argument=(struct args*) arguments;
	char* fileName=argument->charP_arg;
	free(argument);
	FILE* file;
	int x;
	int done=0;
	file=fopen(fileName,"r");
	if(file==NULL)
    {
		(flagDone)--;(flagB1)--;
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
	/**/printf("P - --- Fin malloc producteur ---\n");
	/**/fflush(stdout);
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
		pthread_mutex_unlock(&mutexProd);
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
			/**/printf("P - === Fractale lue : %s, %d, %d, %f, %f ===\n",f->name,fractal_get_width(f),fractal_get_height(f), fractal_get_a(f), fractal_get_b(f));
			/**/fflush(stdout);
			/**/printf("P - *INSERT DU PRODUCTEUR*\n");
			/**/fflush(stdout);
			pthread_mutex_lock(&mutexProd);
			/**/sbuf_insert(bufIn,f);
			//lengthI++;
			/**/printf("P - *INSERT DU PRODUCTEUR TERMINE*\n");
			/**/fflush(stdout);
			x=fscanf(file,"%64s",buf1);
			}
		}
	}
	/**/printf("P - --- Fin producteur ---\n");
	/**/fflush(stdout);
	(flagDone)--;
	(flagB1)--;	
	pthread_mutex_unlock(&mutexProd);
	/**/printf("\nP - PRODFLAG=%d\n\n",flagB1);
	/**/fflush(stdout);
	return NULL;
}

void *consumer(void* arguments){
	/**/printf("C - --- DEBUT CONSOMMATEUR ---\n");
	/**/fflush(stdout);
	int done=0;
	/**/printf("C - --- Debut calcul consommateur ---\n");
	/**/fflush(stdout);
	while(!done)
	{		
		///**/printf("va lire sem_getvalue du consommateur\n");
		/**/fflush(stdout);		
		pthread_mutex_lock(&mutexCons);
		int ic=0;
		sem_getvalue(&(bufIn->items),&ic);
		/**/printf("C - FLAGB1=%d\n",flagB1);
		/**/fflush(stdout);		
		/**/printf("C - LENGTHI=%d\n",ic);
		/**/fflush(stdout);	
		if(((flagB1)<=0)&(ic==0))
		//if(((lengthI)<=0)&(flagB1<=0))
		{
			/**/printf("C - =====DONE=1=====\n");
			/**/fflush(stdout);
			done=1;
			pthread_mutex_unlock(&mutexCons);
		}
		else
		{
		/**/printf("C - *REMOVE DU CONSOMMATEUR*\n");
		/**/fflush(stdout);
		struct fractal* f=(sbuf_remove(bufIn));
		//lengthI--;		
		pthread_mutex_unlock(&mutexCons);
		int ic=0;
		sem_getvalue(&(bufIn->items),&ic);
		/**/printf("C - *REMOVE DU CONSOMMATEUR TERMINE* === LENGTHI : %d\n",ic);
		/**/fflush(stdout);
		/**/printf("C - === Fractale lue : %s, %d, %d, %f, %f ===\n",fractal_get_name(f),fractal_get_width(f),fractal_get_height(f), fractal_get_a(f), fractal_get_b(f));
		/**/fflush(stdout);
		int i;
		int j;
		for(i=0;i<f->width;i++){
			for(j=0;j<f->height;j++)
			{
				fractal_set_value(f,i,j,fractal_compute_value(f,i,j));
				///**/printf("%d\n",fractal_get_value(f,i,j));
				///**/fflush(stdout);
			}
			fflush(stdout);
		}
		/**/printf("C - *INSERT DU CONSOMMATEUR*\n");
		/**/fflush(stdout);
		sbuf_insert(bufOut,f);	
		lengthO++;
		/**/printf("C - *INSERT DU CONSOMMATEUR TERMINE*\n");	
		/**/fflush(stdout);
		}
		pthread_mutex_lock(&mutexCons);
		if(((flagB1)<=0)&(ic==0))
		//if(((lengthI)<=0)&(flagB1<=0))
		{
			/**/printf("C - =====DONE=1=====\n");
			/**/fflush(stdout);
			done=1;
		}
		pthread_mutex_unlock(&mutexCons);
	}
	(flagB2)--;
	(flagDone)--;
	/**/printf("C - --- Fin consommateur ---\n");
	/**/fflush(stdout);
	return NULL;
}

void *writer(void* arguments){
	/**/printf("W - --- DEBUT WRITER ---\n");
	/**/fflush(stdout);
	int isEmpty=0;
	double average;
	struct fractal* highestF=malloc(sizeof(struct fractal));
	/**/printf("W - --- Debut ecriture writer ---\n");
	/**/fflush(stdout);
	if(!optionD){
		/**/printf("W - ===OPTIOND-0===\n");
		/**/fflush(stdout);
		while(!isEmpty){
			int ic;
			///**/printf("va lire sem_getvalue du writer\n");
			///**/fflush(stdout);
			sem_getvalue(&(bufOut->items),&ic);
			/**/printf("\nlengthO=%d\n\n",ic);
			/**/fflush(stdout);
			/**/printf("\nW - FLAGB2=%d\n\n",flagB2);
			/**/fflush(stdout);
			pthread_mutex_lock(&mutexWrit);
			if(((flagB2)<=0)&(ic==0))
			//if(((lengthO)<=0)&(flagB2<=0))
			{
				/**/printf("W - ===DONE=1===\n");
				/**/fflush(stdout);
				isEmpty=1;
				pthread_mutex_unlock(&mutexWrit);
			}
			else{
				/**/printf("W - *REMOVE DU WRITER*\n");
				/**/fflush(stdout);
				struct fractal* f = (sbuf_remove(bufOut));
				pthread_mutex_unlock(&mutexWrit);
				//lengthO--;
				/**/printf("W - *REMOVE DU WRITER TERMINE*\n");
				/**/fflush(stdout);
				/**/printf("W - === Fractale lue : %s, %d, %d, %f, %f ===\n",f->name,fractal_get_width(f),fractal_get_height(f), fractal_get_a(f), fractal_get_b(f));
				/**/fflush(stdout);
				/**/printf("W - About to compute average\n");
				/**/fflush(stdout);
				double newAverage = fractal_compute_average(f);
				/**/printf("W - Average computed == %f\n", newAverage);
				/**/fflush(stdout);
				if(newAverage>average)
				{
					average=newAverage;
					*highestF=*f;
				}				
				fractal_free(f);
				/**/printf("W - HighestF=%s\n",fractal_get_name(highestF));
				/**/fflush(stdout);
			}
			
		}
		/**/printf("W - === ECRITURE ===\n");
		/**/fflush(stdout);
		write_bitmap_sdl(highestF,fileOutName);
		/**/printf("W - === FIN ECRITURE ===\n");
		/**/fflush(stdout);
	}
	else
	{
		while(!isEmpty)
		{
			/**/printf("W - ===OPTIOND-1===\n");
			/**/fflush(stdout);
			int ic=0;
			sem_getvalue(&(bufOut->items),&ic);
			/**/printf("\nlengthO=%d\n\n",ic);
			/**/fflush(stdout);
			pthread_mutex_lock(&mutexWrit);
			if(((flagB2)<=0)&(ic==0))
			//if((flagB2<=0)&(lengthO<=0))
			{
				isEmpty=1;
				pthread_mutex_unlock(&mutexWrit);
			}
			else
			{
				struct fractal* f = (sbuf_remove(bufOut));
				pthread_mutex_unlock(&mutexWrit);
				write_bitmap_sdl(f,fractal_get_name(f));
				fractal_free(f);
			}
		}
	}
	/**/printf("W - --- Fin writer ---\n");
	/**/fflush(stdout);
	(flagDone)--;
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
	/**/printf("M - --- Initialisation des variables terminée ---\n");
	/**/fflush(stdout);

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
	}
	
	/**/printf("M - --- Lecture des options terminée ---\n");
	/**/fflush(stdout);
	/**/printf("\nM - Nombre de threads qui vont être utilisés : %d \n \n",numberThreads);
	/**/fflush(stdout);
	
	sbuf_init(bufIn, (numberThreads+10));            
	sbuf_init(bufOut, (numberThreads+10));    

	/**/printf("M - --- Initialisation des buffers terminée ---\n");
	/**/fflush(stdout);
	
	pthread_t prod[argc-2-optionsCount];
	pthread_t cons[numberThreads];
	pthread_t writ[argc-2-optionsCount];
	
	/**/printf("M - --- Initialisation des tableaux de pthread_t terminée ---\n");
	/**/fflush(stdout);
	
	flagB1=argc-2-optionsCount;
	flagB2=numberThreads;
	if(optionD)
	{
		flagDone=2*(argc-2-optionsCount)+numberThreads;
	}
	else
	{
		flagDone=(argc-2-optionsCount)+numberThreads+1;
	}
	/**/printf("M - --- Initialisation des constantes terminée ---\n");
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
				/**/printf("M - ---CREATION D'UN PRODUCTEUR---\n");
				/**/fflush(stdout);
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, arguments);
			}
			else
			{
				fileOutName=argv[count];  //Demande verification, faut pas une etoile ?
			}	
		}
	}
	
	/**/printf("M - --- Initialisation des producteurs terminée ---\n");
	/**/fflush(stdout);
	
	int i;
	for(i=0;(i<numberThreads);i++)
	{
		/**/printf("M - ---CREATION D'UN CONSOMMATEUR---\n");
		/**/fflush(stdout);
		pthread_create(&(cons[i]), NULL, (void*) &consumer, NULL);
	}
	
	/**/printf("M - --- Initialisation des consommateurs terminée ---\n");
	/**/fflush(stdout);
	
	//TODO: faire plein de writers qui comparent avec sémaphore la fractale la plus haute
	if(!optionD)
	{
		/**/printf("M - ---CREATION D'UN WRITER---\n");
		/**/fflush(stdout);
		pthread_create(&(writ[0]), NULL, (void*) &writer, NULL);
	}
	else
	{
		int i;
		for(i=0;i<(argc-2-optionsCount);i++)
		{
			/**/printf("M - ---CREATION D'UN WRITER---\n");
			/**/fflush(stdout);
			pthread_create(&(writ[i]), NULL, (void*) &writer, NULL);
		}
	}	
	/**/printf("M - --- Initialisation des writers terminée ---\n");
	/**/fflush(stdout);
	
	/**/printf("\nM - doneFlag = %d \n\n",flagDone);
	/**/fflush(stdout);
	
	while((flagDone)>0)
	{
		///**/printf("\n DONEFLAG=%d \n\n",*doneFlag);
		///**/fflush(stdout);
	}
	
	/**/printf("M - --- Fin des threads ---\n");
	/**/fflush(stdout);
	
	end:
	
	sbuf_clean(bufIn);
	sbuf_clean(bufOut);
	
	/**/printf("M - --- Buffers clean ---\n");
	/**/fflush(stdout);
	
	
	/**/printf("M - --- Fin du programme ---\n");
	/**/fflush(stdout);
	
	return 0;
}
