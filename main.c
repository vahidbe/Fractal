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

struct args{
	char* charP_arg;
	struct sbuf* buf_arg;
	struct sbuf* bufout_arg;
	int optionD;
	char* fileOutName;
};	

struct sbuf{
    struct fractal buf;           /* Buffer partagé */
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
    sp->buf = calloc(n, sizeof(struct fractal));
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
void sbuf_insert(struct sbuf *sp, struct fractal item)
{
	///**/int ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d sur %d\n",ic,sp->n);
	sem_wait(&(sp->slots));
	sem_wait(&(sp->mutex));
	sp->rear=((sp->rear)+1*sizeof(struct fractal))%(sp->n);
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
struct fractal sbuf_remove(struct sbuf *sp)
{	
	///**/int ic=0;
	///**/sem_getvalue(&(sp->items),&ic);
	///**/printf("ITEMS = %d\n",ic);
	sem_wait(&(sp->items));
	sem_wait(&(sp->mutex));
	sp->front=((sp->front)+1)%(sp->n);
	struct fractal res=sp->buf[sp->front];
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
	struct args* argument=(struct args*) arguments;
	char* fileName=argument->charP_arg;
	struct sbuf* buf=argument->buf_arg;
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
	/**/printf("--- Fin malloc producteur ---\n");
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
				(flagDone)--;(flagB1)--;
				return (NULL);
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
			/**/printf("=== Fractale lue : %s %d %d %lf %lf ===\n",name,*buf2,*buf3,*buf4,*buf5);
			/**/fflush(stdout);
			/**/printf("*INSERT DU PRODUCTEUR*\n");
			/**/fflush(stdout);
			struct fractal* new = fractal_new(name,*buf2,*buf3,*buf4,*buf5);
			/**/sbuf_insert(buf,*new);
			/**/printf("*INSERT DU PRODUCTEUR TERMINE*\n");
			/**/fflush(stdout);
			x=fscanf(file,"%64s",buf1);
			}
		}
	}
	/**/printf("--- Fin producteur ---\n");
	/**/fflush(stdout);
	(flagDone)--;
	(flagB1)--;
	/**/printf("\nPRODFLAG=%d\n\n",flagB1);
	/**/fflush(stdout);
	return NULL;
}

void *consumer(void* arguments){
	/**/printf("--- DEBUT CONSOMMATEUR ---\n");
	/**/fflush(stdout);
	int done=0;
	struct args* argument=(struct args*) arguments;
	struct sbuf* buf=argument->buf_arg;
	struct sbuf* bufout=argument->bufout_arg;
	free(argument);
	/**/printf("--- Debut calcul consommateur ---\n");
	/**/fflush(stdout);
	while(!done)
	{		
		/**/printf("va lire sem_getvalue du consommateur\n");
		/**/fflush(stdout);
		int ic=0;
		sem_getvalue(&(buf->items),&ic);
		/**/printf("FLAG=%d\n",flagB1);
		/**/fflush(stdout);
		if(((flagB1)<=0)&(ic==0))
			if(((flagB1<=0)))
		{
			/**/printf("=====DONE=1=====\n");
			/**/fflush(stdout);
			done=1;
		}
		else
		{
		/**/printf("*REMOVE DU CONSOMMATEUR*\n");
		/**/fflush(stdout);
		struct fractal f=sbuf_remove(buf);
		/**/printf("*REMOVE DU CONSOMMATEUR TERMINE*\n");
		/**/fflush(stdout);
		int i;
		int j;
		for(i=0;i<f.width;i++){
			for(j=0;j<f.height;j++){
				/**/printf("about to compute value\n");
				/**/fflush(stdout);
				fractal_set_value(&f,i,j,fractal_compute_value(&f,i,j));
				/**/printf("value computed\n");
				/**/fflush(stdout);
			}
		}
		/**/printf("*INSERT DU CONSOMMATEUR*\n");
		/**/fflush(stdout);
		sbuf_insert(bufout,f);	
		/**/printf("*INSERT DU CONSOMMATEUR TERMINE*\n");	
		/**/fflush(stdout);
		}
	}
	/**/printf("--- Fin consommateur ---\n");
	/**/fflush(stdout);
	(flagB2)--;
	(flagDone)--;
	return NULL;
}

void *writer(void* arguments){
	/**/printf("--- DEBUT WRITER ---\n");
	/**/fflush(stdout);
	int isEmpty=0;
	struct args* argument=(struct args*) arguments;
	struct sbuf* buf=argument->bufout_arg;
	int optionD=argument->optionD;
	char* fileOutName=argument->fileOutName;
	double average;
	struct fractal highestF;
	free(argument);
	/**/printf("--- Debut ecriture writer ---\n");
	/**/fflush(stdout);
	if(!optionD){
		/**/printf("===OPTIOND-0===\n");
		/**/fflush(stdout);
		while(!isEmpty){
			int ic;
			/**/printf("va lire sem_getvalue du writer\n");
			/**/fflush(stdout);
			//sem_getvalue(&(buf->items),&ic);
			/**/printf("\n FLAGOUT=%d\n\n",flagB2);
			/**/fflush(stdout);
			//if(((flagB2)<=0)&(ic==0)){
				if(flagB2<=0){
				/**/printf("===DONE=1===\n");
				/**/fflush(stdout);
				isEmpty=1;
			}
			else{
				/**/printf("*REMOVE DU WRITER*\n");
				/**/fflush(stdout);
				struct fractal f = sbuf_remove(buf);
				/**/printf("*REMOVE DU WRITER TERMINE*\n");
				/**/fflush(stdout);
				/**/printf("about to compute average\n");
				/**/fflush(stdout);
				double newAverage = fractal_compute_average(&f);
				/**/printf("average computed\n");
				/**/fflush(stdout);
				if(newAverage>average)
				{
					average=newAverage;
					highestF=f;
				}
				/**/printf("average=%lf\n",average);
				/**/fflush(stdout);
			}
			
		}
		/**/printf("=== ECRITURE ===\n");
		/**/fflush(stdout);
		write_bitmap_sdl(&highestF,fileOutName);
		/**/printf("=== FIN ECRITURE ===\n");
		/**/fflush(stdout);
	}
	else
	{
		while(!isEmpty)
		{
			/**/printf("===OPTIOND-1===\n");
			/**/fflush(stdout);
			int ic=0;
			//sem_getvalue(&(buf->items),&ic);
			//if(((flagB2)<=0)&(ic==0))
				if(flagB2<=0)
			{
				isEmpty=1;
			}
			else
			{
				struct fractal f = (struct fractal) sbuf_remove(buf);
				write_bitmap_sdl(&f,fractal_get_name(&f));
			}
		}
	}
	/**/printf("--- Fin writer ---\n");
	/**/fflush(stdout);
	(flagDone)--;
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
	/**/printf("%s","--- Initialisation des variables terminée ---\n");
	/**/fflush(stdout);

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
		numberThreads=argc-2-optionsCount; 
	}
	
	/**/printf("--- Lecture des options terminée ---\n");
	/**/fflush(stdout);
	/**/printf("\n Nombre de threads qui vont être utilisés : %d \n \n",numberThreads);
	/**/fflush(stdout);
	
	sbuf_init(buf, (numberThreads+10));            
	sbuf_init(bufout, (numberThreads+10));    

	/**/printf("--- Initialisation des buffers terminée ---\n");
	/**/fflush(stdout);
	
	pthread_t prod[argc-2-optionsCount];
	pthread_t cons[numberThreads];
	pthread_t writ[argc-2-optionsCount];
	
	/**/printf("--- Initialisation des tableaux de pthread_t terminée ---\n");
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
	/**/printf("--- Initialisation des constantes terminée ---\n");
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
				struct args* arguments=(struct args*) malloc(sizeof(struct args));
				if(arguments==NULL)
				{
					goto end;
				}
				arguments->buf_arg=buf;
				arguments->charP_arg=argv[count];
				/**/printf("---CREATION D'UN PRODUCTEUR---\n");
				/**/fflush(stdout);
				pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, (void*) arguments);
			}
			else
			{
				fileOutName=argv[count];
			}	
		}
	}
	
	/**/printf("--- Initialisation des producteurs terminée ---\n");
	/**/fflush(stdout);
	
	int i;
	for(i=0;(i<numberThreads);i++)
	{
		struct args* arguments=(struct args*) malloc(sizeof(struct args));
		if(arguments==NULL)
		{
			goto end;
		}
		arguments->buf_arg=buf;
		arguments->bufout_arg=bufout;
		/**/printf("---CREATION D'UN CONSOMMATEUR---\n");
		/**/fflush(stdout);
		pthread_create(&(cons[i]), NULL, (void*) &consumer, (void*) arguments);
	}
	
	/**/printf("--- Initialisation des consommateurs terminée ---\n");
	/**/fflush(stdout);
	
	//TODO: faire plein de writers qui comparent avec sémaphore la fractale la plus haute
	if(!optionD)
	{
		struct args* arguments=(struct args*) malloc(sizeof(struct args));
		if(arguments==NULL)
		{
			goto end;
		}
		arguments->optionD=optionD;
		arguments->bufout_arg=bufout;
		/**/printf("---CREATION D'UN WRITER---\n");
		/**/fflush(stdout);
		pthread_create(&(writ[0]), NULL, (void*) &writer, (void*) arguments);
	}
	else
	{
		int i;
		for(i=0;i<(argc-2-optionsCount);i++)
		{
			struct args* arguments=(struct args*) malloc(sizeof(struct args));
			if(arguments==NULL)
			{
				goto end;
			}
			arguments->optionD=optionD;
			arguments->bufout_arg=bufout;
			/**/printf("---CREATION D'UN WRITER---\n");
			/**/fflush(stdout);
			pthread_create(&(writ[i]), NULL, (void*) &writer, (void*) bufout);
		}
	}	
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
	
	sbuf_clean(buf);
	sbuf_clean(bufout);
	
	/**/printf("--- Buffers clean ---\n");
	/**/fflush(stdout);
	
	
	/**/printf("--- Fin du programme ---\n");
	/**/fflush(stdout);
	
	return 0;
}
