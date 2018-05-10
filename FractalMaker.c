#include "./libfractal/fractal.h"
#include "./libfractal/tools.c"
#include "./FractalMaker.h"

int countProd;
 int countCons;
 int countWrit;
 int numberThreads;
 int numberProd;
 int numberCons;
 int optionD;
 char* fileOutName;
 int sortie=0;
 int fractCountP=0;
 int fractCountC=0;
 int fractCountW=0;
 int maxEgal;

 sem_t directeur;
 pthread_mutex_t tuteur1;
 pthread_mutex_t tuteur2;
 pthread_mutex_t professor;
 pthread_mutex_t gardien;
 int countEleves;
 struct sbuf* bufIn;
 struct sbuf* bufOut;
 struct sbuf* highestF;
 struct charbuf* bufFName;
 double average;

/*
 * @pre sp!=NULL, n>0
 * @post a construit un buffer partagé contenant n slots
 */
void sbuf_init(struct sbuf *sp, int n)
{
  if(sp==NULL)
    {
      fprintf(stderr,"Malloc error!\n");
      exit(-1);
    }
  sp->buf = (struct fractal**) calloc(n, sizeof(struct fractal*));
  sp->n = n;                       		/* Buffer content les entiers */
  sp->front = sp->rear = 0;        		/* Buffer vide si front == rear */
  pthread_mutex_init(&sp->mutex, NULL);      	/* Exclusion mutuelle */
  sem_init(&sp->empty, 0, n);      		/* Au début, n slots vides */
  sem_init(&sp->full, 0, 0);      		/* Au début, rien à consommer */
}

void charbuf_init(struct charbuf *sp, int n)
{
  if(sp==NULL)
    {
      fprintf(stderr,"Malloc error!\n");
      fflush(stderr);
      exit(-1);
    }
  sp->buf = (char**) calloc(n, sizeof(char*));
  sp->n = n;                       		/* Buffer content les entiers */
  sp->front = sp->rear = 0;        		/* Buffer vide si front == rear */
  pthread_mutex_init(&sp->mutex, NULL);      	/* Exclusion mutuelle */
}

/*
 * @pre sp!=NULL
 * @post libère le buffer
 */
void sbuf_clean(struct sbuf *sp)
{
  if(sp->rear!=sp->front)
    while(sp->rear!=sp->front)
      {
	free(sp->buf[sp->rear-1]);
	sp->rear = sp->rear-1;
      }
  free(sp->buf); 
}

void charbuf_clean(struct charbuf *sp)
{
  if(sp->rear!=sp->front)
    while(sp->rear!=sp->front)
      {
	free(sp->buf[sp->rear-1]);
	sp->rear = sp->rear-1;
      }
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
  pthread_mutex_unlock(&(sp->mutex));
  sem_post(&(sp->full));
}

void sbuf_insertHighest(struct sbuf *sp, struct fractal* f)
{
  pthread_mutex_lock(&(sp->mutex));
  sp->buf[sp->rear]=f;
  sp->rear = (sp->rear)+1;
  pthread_mutex_unlock(&(sp->mutex));
}

void charbuf_insert(struct charbuf *sp, char* f)
{
  int n = (int)strlen(f);
  pthread_mutex_lock(&(sp->mutex));
  sp->buf[sp->rear] = (char*)malloc((n+1)*sizeof(char));
  strcpy(sp->buf[sp->rear],f);
  sp->rear = (sp->rear)+1;
  pthread_mutex_unlock(&(sp->mutex));

}

/* @pre sbuf!=NULL
 * @post retire le dernier item du buffer partagé
 */
struct fractal* sbuf_remove(struct sbuf *sp)
{	
  sem_wait(&(sp->full));
  struct fractal* f;
  pthread_mutex_lock(&(sp->mutex));
  sp->rear = (sp->rear)-1;
  f = sp->buf[sp->rear];
  pthread_mutex_unlock(&(sp->mutex));
  sem_post(&(sp->empty));
  return f;
}


struct fractal* sbuf_removeHighest(struct sbuf *sp)
{	
  struct fractal* f;
  pthread_mutex_lock(&(sp->mutex));
  sp->rear = (sp->rear)-1;
  f = sp->buf[sp->rear];
  pthread_mutex_unlock(&(sp->mutex));
  return f;
}

int charbuf_already_used(struct charbuf *sp, char* name)
{
  int i;
  pthread_mutex_lock(&(sp->mutex));
  for(i=0; i<(sp->rear); i++)
    {

      if(strcmp(sp->buf[i],name)==0)
	{
	  pthread_mutex_unlock(&(sp->mutex));
	  return 1;
	}
    }
  pthread_mutex_unlock(&(sp->mutex));
  charbuf_insert(bufFName,name);
  return 0;
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
      fclose(file);				
      free(buf1);
      free(buf2);
      free(buf3);
      free(buf4);
      free(buf5);
      fprintf(stderr,"Malloc error!\n");
      exit(-1);
    }	


  x=fscanf(file,"%64s",buf1);
  while(!done){
    if(x==EOF)
      {		

	if(fclose(file)!=0)
	  {				
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
	    fprintf(stderr,"Error closing your file!\n");
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
		fclose(file);				
		free(buf1);
		free(buf2);
		free(buf3);
		free(buf4);
		free(buf5);
		fprintf(stderr,"Error in you file, you don't respect the structure of input!\n");
		exit(-1);
	      }
	    x=fscanf(file,"%d",buf3);
	    if(x==EOF)
	      {
		fclose(file);				
		free(buf1);
		free(buf2);
		free(buf3);
		free(buf4);
		free(buf5);
		fprintf(stderr,"Error in you file, you don't respect the structure of input!\n");
		exit(-1);
	      }
	    x=fscanf(file,"%lf",buf4);
	    if(x==EOF)
	      {
		fclose(file);				
		free(buf1);
		free(buf2);
		free(buf3);
		free(buf4);
		free(buf5);
		fprintf(stderr,"Error in you file, you don't respect the structure of input!\n");
		exit(-1);
	      }
	    x=fscanf(file,"%lf",buf5);
	    if(x==EOF)
	      {
		fclose(file);			
		free(buf1);
		free(buf2);
		free(buf3);
		free(buf4);
		free(buf5);
		fprintf(stderr,"Error in you file, you don't respect the structure of input!\n");
		exit(-1);
	      }

	    struct fractal* f;
	    f = fractal_new(name,*buf2,*buf3,*buf4,*buf5);
	    if(charbuf_already_used(bufFName,name)){fprintf(stderr,"Erreur de duplicata pour le nom de fractale : %s\n",name);}
	    else{
	      sbuf_insert(bufIn,f);
	      fractCountP++;
              sleep(0);
	    }
	    x=fscanf(file,"%64s",buf1);
	  }
      }
  }
  pthread_mutex_lock(&gardien);
  countProd++;
  sem_post(&directeur);	
  pthread_mutex_unlock(&gardien);
  return NULL;
}


void *consumer(void* arguments){
  int done=0;
  while(!done)
    {
      sleep(0);
      pthread_mutex_lock(&tuteur1);
      if(bufIn->front==bufIn->rear){
	pthread_mutex_unlock(&tuteur1);
	sleep(0);
	if(((countProd==numberProd)&(bufIn->front==bufIn->rear))|((fractCountC==fractCountP)&(fractCountP!=0)))
	  {
	    done=1;
	  }
      }
      else
	{
	  pthread_mutex_lock(&gardien);
	  fractCountC++;
	  pthread_mutex_unlock(&gardien);
	  if(bufIn->front!=bufIn->rear)
	    {
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
	      sbuf_insert(bufOut,f);
		
	    }
	  else
	    {
	      pthread_mutex_unlock(&tuteur1);
	    }
	}
    }
  pthread_mutex_lock(&gardien);
  countCons++;
  pthread_mutex_unlock(&gardien);
  sem_post(&directeur);
  return NULL;
}

void *writer(void* arguments)
{
  int done2=0;
  while(!done2)
    {			
      pthread_mutex_lock(&tuteur2);
      sleep(0);
      if(bufOut->front==bufOut->rear)
	{
	  sleep(0);
	  pthread_mutex_unlock(&tuteur2);
	  if(((countCons==numberThreads)&(bufOut->front==bufOut->rear))|((fractCountW==fractCountP)&(fractCountP!=0)))
	    {
	      pthread_mutex_lock(&professor);
	      countEleves++;
	      pthread_mutex_unlock(&professor);
	      done2=1;
	    }
	}
      else
	{
	  pthread_mutex_lock(&gardien);
	  fractCountW++;
	  pthread_mutex_unlock(&gardien);
	  if(bufOut->front!=bufOut->rear)
	    {
	      struct fractal* f = (sbuf_remove(bufOut));
	      pthread_mutex_unlock(&tuteur2);
	      double newAverage = fractal_compute_average(f);
	      pthread_mutex_lock(&professor);
	      if(newAverage>average)
		{
		  average=newAverage;
		  sbuf_clean(highestF);
		  sbuf_init(highestF,50);
		  struct fractal* newF = fractal_new(fractal_get_name(f),fractal_get_width(f),fractal_get_height(f),fractal_get_a(f),fractal_get_b(f));
		  int i, j;
		  for(i=0;i<fractal_get_width(f);i++){
		    for(j=0;j<fractal_get_height(f);j++){
		      fractal_set_value(newF,i,j,fractal_get_value(f,i,j));
		    }
		  }
		  sbuf_insertHighest(highestF,newF);
		  maxEgal=1;
		}
	      else if(newAverage==average)
		{
		  struct fractal* newF = fractal_new(fractal_get_name(f),fractal_get_width(f),fractal_get_height(f),fractal_get_a(f),fractal_get_b(f));
		  int i, j;
		  for(i=0;i<fractal_get_width(f);i++){
		    for(j=0;j<fractal_get_height(f);j++){
		      fractal_set_value(newF,i,j,fractal_get_value(f,i,j));
		    }
		  }
		  sbuf_insertHighest(highestF,newF);
		  maxEgal++;
		}
	      pthread_mutex_unlock(&professor);
	      if(optionD)
		{
		  write_bitmap_sdl(f,strcat(fractal_get_name(f),".bmp"));
		}	
	      fractal_free(f);
	      sleep(0);
	    }
	  else
	    {
	      pthread_mutex_unlock(&tuteur2);
	    }
	}
			
    }
  pthread_mutex_lock(&professor);
  if((!sortie)&(countEleves==numberThreads))
    {	
      if(highestF->rear!=highestF->front)
	{
	  if(maxEgal>1)
	    {
	      printf("Les fractales avec la plus grande moyenne calculée sont :\n");
	      fflush(stdout);
	      int i;
	      for(i = 0; i<maxEgal;i++)
		{
		  struct fractal* f = (sbuf_removeHighest(highestF));
		  char* zone = (char*)malloc(strlen(f->name)+strlen("fractout_")+strlen(".bmp")+1);
		  strcpy(zone,"fractout_");
		  strcat(zone,f->name);
		  strcat(zone,".bmp");
		  write_bitmap_sdl(f,zone);
		  printf("%s avec une moyenne de %f\n",f->name,average);
		  fflush(stdout);
		  fractal_free(f);
		}
	    }
	  else
	    {
	      struct fractal* f = (sbuf_removeHighest(highestF));
	      write_bitmap_sdl(f,strcat(fileOutName,".bmp"));
	      printf("La fractale avec la plus grande moyenne cacluée est \"%s\" avec une moyenne de %f\n",f->name,average);
	      fflush(stdout);
	      fractal_free(f);
	    }
	  sortie = 1;
	}
      else
	{
	  /**/printf("\n- You did not enter a fractal in your file!\n\n");
	  /**/fflush(stdout);
	  sortie=1;
	}
    }
  pthread_mutex_unlock(&professor);
  pthread_mutex_lock(&gardien);
  countWrit++;
  pthread_mutex_unlock(&gardien);
  sem_post(&directeur);

  return NULL;
}

