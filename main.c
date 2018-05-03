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

int countProd;
int countCons;
int countWrit;
int numberThreads;
int numberProd;
int numberCons;
int optionD;
char* fileOutName;
int sortie=0;

sem_t directeur;
pthread_mutex_t tuteur1;
pthread_mutex_t tuteur2;
pthread_mutex_t professor;
pthread_mutex_t gardien;
int countEleves;
struct sbuf* bufIn;
struct sbuf* bufOut;
struct fractal* highestF;
double average;


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
    sp->n = n;                       		/* Buffer content les entiers */
    sp->front = sp->rear = 0;        		/* Buffer vide si front == rear */
    pthread_mutex_init(&sp->mutex, NULL);      	/* Exclusion mutuelle */
    sem_init(&sp->empty, 0, n);      		/* Au début, n slots vides */
    sem_init(&sp->full, 0, 0);      		/* Au début, rien à consommer */
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
void sbuf_insert(struct sbuf *sp, struct fractal* f)
{
	sem_wait(&(sp->empty));
	pthread_mutex_lock(&(sp->mutex));
	sp->buf[sp->rear]=f;
	sp->rear = (sp->rear)+1;
	printf("Ins - Fractale inserted : %s, %d, %d, %f, %f\n",f->name,fractal_get_width(f),fractal_get_height(f), fractal_get_a(f), fractal_get_b(f));
	fflush(stdout);
	pthread_mutex_unlock(&(sp->mutex));
	sem_post(&(sp->full));
}

/* @pre sbuf!=NULL
 * @post retire le dernier item du buffer partagé
 */
struct fractal* sbuf_remove(struct sbuf *sp)
{	
	sem_wait(&(sp->full));
	struct fractal* f = (struct fractal*)malloc(sizeof(struct fractal));
	pthread_mutex_lock(&(sp->mutex));
	sp->rear = (sp->rear)-1;
	f = sp->buf[sp->rear];
	printf("Ins - Fractale removed : %s, %d, %d, %f, %f\n",f->name,fractal_get_width(f),fractal_get_height(f), fractal_get_a(f), fractal_get_b(f));
	fflush(stdout);
	pthread_mutex_unlock(&(sp->mutex));
	sem_post(&(sp->empty));
	return f;
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
			if(numberThreads==1)
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
			return (NULL);
		}
		return (NULL);
    	}	
	/**/printf("P - --- Fin malloc producteur ---\n");
	/**/fflush(stdout);


	x=fscanf(file,"%64s",buf1);
	while(!done){
    	if(x==EOF)
	{		
		pthread_mutex_lock(&gardien);
		/**/printf("P - lockG\n");
		/**/fflush(stdout);
		if(fclose(file)!=0)
		{
			free(buf1);
			free(buf2);
			free(buf3);
			free(buf4);
			free(buf5);
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
					return (NULL);
				}
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				return (NULL);
			}
			x=fscanf(file,"%d",buf3);
			if(x==EOF)
			{
				//TODO: gérer les erreurs/la sortie
				if(fclose(file)!=0)
				{
					return (NULL);
				}
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				return (NULL);
			}
			x=fscanf(file,"%lf",buf4);
			if(x==EOF)
			{
				//TODO: gérer les erreurs/la sortie
				if(fclose(file)!=0)
				{
					return (NULL);
				}
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				return (NULL);
			}
			x=fscanf(file,"%lf",buf5);
			if(x==EOF)
			{
				if(fclose(file)!=0)
				{
					return (NULL);
				}
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				return (NULL);
			}

			struct fractal* f = (struct fractal*)malloc(sizeof(struct fractal));
			f = fractal_new(name,*buf2,*buf3,*buf4,*buf5);
			/**/printf("P - === Fractale lue : %s, %d, %d, %f, %f ===\n",f->name,fractal_get_width(f),fractal_get_height(f), fractal_get_a(f), fractal_get_b(f));
			/**/fflush(stdout);
			/**/printf("P - *INSERT DU PRODUCTEUR*\n");
			/**/fflush(stdout);
			/**/sbuf_insert(bufIn,f);
			x=fscanf(file,"%64s",buf1);
			}
		}
	}
	/**/printf("P - --- Fin producteur ---\n");
	/**/fflush(stdout);
	countProd++;
	sem_post(&directeur);	
	pthread_mutex_unlock(&gardien);
	/**/printf("P - unlockG\n");
	/**/fflush(stdout);
	return NULL;
}

void *consumer(void* arguments){
	int done=0;
	/**/printf("C - --- DEBUT CONSOMMATEUR ---\n");
	/**/fflush(stdout);
	while(!done)
	{		
		pthread_mutex_lock(&gardien);
		/**/printf("P - lockG\n");
		/**/fflush(stdout);
		pthread_mutex_lock(&tuteur1);
		printf("countProd : %d, numberProd : %d\nfront=rear : %d\n",countProd,numberProd,(bufIn->front==bufIn->rear));
		fflush(stdout);
		if((numberProd-countProd)==1)
		{
			printf("1 PROD RESTANT\n");
			fflush(stdout);
			sleep(1);
		}
		if((countProd==numberProd)&(bufIn->front==bufIn->rear))
		{
			/**/printf("P - unlockG\n");
			/**/fflush(stdout);
			pthread_mutex_unlock(&gardien);
			/**/printf("C - =====DONE=C=====\n");
			/**/fflush(stdout);
			done=1;
		}
		else
		{
			/**/printf("P - unlockG\n");
			/**/fflush(stdout);
			pthread_mutex_unlock(&gardien);
			/**/printf("C - *REMOVE DU CONSOMMATEUR*\n");
			/**/fflush(stdout);
			struct fractal* f=(sbuf_remove(bufIn));
			pthread_mutex_unlock(&tuteur1);
			int i;
			int j;
			for(i=0;i<f->width;i++)
			{
				for(j=0;j<f->height;j++)
				{
					fractal_set_value(f,i,j,fractal_compute_value(f,i,j));
				}
			}
			/**/printf("C - *INSERT DU CONSOMMATEUR*\n");
			/**/fflush(stdout);
			sbuf_insert(bufOut,f);
		}
	}
	pthread_mutex_unlock(&tuteur1);
	/**/printf("C - --- Fin consommateur ---\n");
	/**/fflush(stdout);
	pthread_mutex_lock(&gardien);
	countCons++;
	pthread_mutex_unlock(&gardien);
	sem_post(&directeur);
	return NULL;
}

void *writer(void* arguments){
	int done2=0;
	/**/printf("W - --- DEBUT WRITER ---\n");
	/**/fflush(stdout);
	printf("optionD = %d",optionD);
	/**/printf("W - --- Debut ecriture writer ---\n");
	/**/fflush(stdout);
	if(!optionD){
		/**/printf("W - ===OPTIOND-0===\n");
		/**/fflush(stdout);
		while(!done2){			
			pthread_mutex_lock(&tuteur2);
			if((countCons==numberThreads)&(bufOut->front==bufOut->rear))
			{
				/**/printf("W - ===DONE=W===\n");
				/**/fflush(stdout);
				pthread_mutex_lock(&professor);
				countEleves++;
				pthread_mutex_unlock(&professor);
				done2=1;
			}
			else{
				/**/printf("W - *REMOVE DU WRITER*\n");
				/**/fflush(stdout);
				struct fractal* f = (sbuf_remove(bufOut));
				pthread_mutex_unlock(&tuteur2);
				/**/printf("W - === Fractale lue : %s, %d, %d, %f, %f ===\n",f->name,fractal_get_width(f),fractal_get_height(f), fractal_get_a(f), fractal_get_b(f));
				/**/fflush(stdout);
				/**/printf("W - About to compute average\n");
				/**/fflush(stdout);
				double newAverage = fractal_compute_average(f);
				/**/printf("W - Average computed == %f\n", newAverage);
				/**/fflush(stdout);
				pthread_mutex_lock(&professor);
				if(newAverage>average)
				{
					average=newAverage;
					*highestF=*f;
				}
				pthread_mutex_unlock(&professor);	
				fractal_free(f);
			}
			
		}
		pthread_mutex_unlock(&tuteur2);
		/**/printf("W - === ECRITURE ===\n");
		/**/fflush(stdout);
		pthread_mutex_lock(&professor);
		printf("countEleves = %d, numberthreads = %d\n",countEleves,numberThreads);
		fflush(stdout);
		if((!sortie)&(countEleves==numberThreads))
		{	
			/**/printf("W - Plus grande fractale : %s avec une moyenne de : %f\n", highestF->name, average);
			/**/fflush(stdout);
			write_bitmap_sdl(highestF,fileOutName);
			sortie=1;
		}		
		pthread_mutex_unlock(&professor);
		/**/printf("W - === FIN ECRITURE ===\n");
		/**/fflush(stdout);
	}
	else
	{
		while(!done2)
		{
			/**/printf("W - ===OPTIOND-1===\n");
			/**/fflush(stdout);
			if((countCons==numberThreads)&(bufOut->front==bufOut->rear))
			{
				done2=1;
			}
			else
			{
				struct fractal* f = (sbuf_remove(bufOut));
				write_bitmap_sdl(f,fractal_get_name(f));
				fractal_free(f);
			}
		}
	}
	/**/printf("W - --- Fin writer ---\n");
	/**/fflush(stdout);
	pthread_mutex_lock(&gardien);
	countWrit++;
	pthread_mutex_unlock(&gardien);
	sem_post(&directeur);
	return NULL;
}

int main(int argc, char *argv[])
{
	numberThreads=0;
	countProd = 0;
	countCons = 0;
	countWrit = 0;
	countEleves = 0;
	average = 0.0;
	pthread_mutex_init(&tuteur1,NULL);
	pthread_mutex_init(&tuteur2,NULL);
	pthread_mutex_init(&professor,NULL);
	pthread_mutex_init(&gardien,NULL);
	int count;
	int optionsCount=0;
	optionD=0;
	bufIn=(malloc(sizeof(struct sbuf)));
	bufOut=(malloc(sizeof(struct sbuf))); 					//TODO : Close si malloc echoue
	highestF=malloc(sizeof(struct fractal));
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
	numberProd = argc-2-optionsCount;
	
	/**/printf("M - --- Lecture des options terminée ---\n");
	/**/fflush(stdout);
	/**/printf("\nM - Nombre de threads qui vont être utilisés : %d \n \n",numberThreads);
	/**/fflush(stdout);
	sem_init(&directeur, 0 ,numberThreads);
	sbuf_init(bufIn, 100);        						//TODO : Si pas assez de place (trop de fractales), code peut planter (pas de consommateur)    
	sbuf_init(bufOut, 100);    

	/**/printf("M - --- Initialisation des buffers terminée ---\n");
	/**/fflush(stdout);
	
	if(numberThreads==0)
	{
		fprintf(stderr,"You did not enter a file!\n");
		exit(-1);
	}
	pthread_t prod[numberProd];
	pthread_t cons[numberThreads];
	pthread_t writ[numberThreads];
	
	/**/printf("M - --- Initialisation des tableaux de pthread_t terminée ---\n");
	/**/fflush(stdout);
	
	for(count=optionsCount+1;count<argc;count++)
	{
		if(((*argv[count])=='-')&(count!=(argc)))
		{ 	
			//TODO : ENTREE STANDARD			
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
				sem_wait(&directeur);
				/**/printf("M - ---CREATION D'UN PRODUCTEUR---\n");
				/**/fflush(stdout);
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, arguments);
			}
			else
			{
				fileOutName=argv[count];
			}	
		}
	}
	
	/**/printf("M - --- Initialisation des producteurs terminée ---\n");
	/**/fflush(stdout);
	
	int i;/*
	if((numberThreads%2)!=0)
	{
		numberCons=(numberThreads+1)/2;
	}
	else
	{
		numberCons=numberThreads/2;
	}*/
	for(i=0;(i<numberThreads);i++)
	{
		sem_wait(&directeur);
		/**/printf("M - ---CREATION D'UN CONSOMMATEUR---\n");
		/**/fflush(stdout);
		pthread_create(&(cons[i]), NULL, (void*) &consumer, NULL);
	}
	
	/**/printf("M - --- Initialisation des consommateurs terminée ---\n");
	/**/fflush(stdout);

	for(i=0;i<numberThreads;i++)
	{
		sem_wait(&directeur);
		/**/printf("M - ---CREATION D'UN WRITER---\n");
		/**/fflush(stdout);
		pthread_create(&(writ[i]), NULL, (void*) &writer, NULL);
	}	
	/**/printf("M - --- Initialisation des writers terminée ---\n");
	/**/fflush(stdout);

	while(countWrit<numberThreads)
	{
		
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
